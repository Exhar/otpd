/*
 * $Id$
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 *
 *  For alternative licensing terms, contact licensing@tri-dsystems.com.
 *
 * Copyright 2008 TRI-D Systems, Inc.
 */

#include "ident.h"
RCSID("$Id$")

#include <errno.h>
#include <poll.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
/* #include <stropts.h> */
#ifdef __sun__
/* FIONBIO */
#define BSD_COMP
#endif
#include <sys/ioctl.h>

#include "extern.h"
#include "gsm.h"
#include "bitmanip.h"

static __thread helix_dcontext_t dcontext;	/* helix dynamic context */
/* make sure we're reading the current value */
static __thread pthread_mutex_t active_mutex = PTHREAD_MUTEX_INITIALIZER;

#define PAUSE do {		\
  struct timespec ts = { .tv_sec = 0, .tv_nsec = 200000000LL }; /* .2s */ \
  (void) nanosleep(&ts, NULL);	\
} while(0)

#define RESET_FAILBACK \
  if (gsmd->active == 1) \
    timeout = xgethrtime() + gsmd->fb_timeout

/*
 * Monitor and report gsmd status.
 */
void *
failover_thread(void *arg)
{
  gsmd_t	*gsmd = (gsmd_t *) arg;
  const char	*name = gsmd->name;
  int		s;
  struct pollfd	pfd;

  union {
    unsigned char	msg[GSM_ULTRA_MSG_LEN];
    uint64_t		u;	/* force uint64 aligned msg */
  } c, p;
  uint32_t	seq;	/* seqno part of nonce       */
  int		len;	/* msg length before padding */
  int		plen;	/* msg length after padding  */
  size_t	xlen;	/* xmit length after encap   */
  ssize_t	rlen;	/* recv length before decap  */
  size_t	ulen;	/* username length           */
  unsigned char	*cmsg = &c.msg[HELIX_NONCE_LEN];
  unsigned char	*pmsg = &p.msg[HELIX_NONCE_LEN];

  if (gsmd->n == 1) {	/* nothing to do */
    mlog(LOG_DEBUG2, "%s: tid=%lu gsmd=%s (primary only, exiting)", __func__,
         (unsigned long) pthread_self(), name);
    return NULL;
  }

  mlog(LOG_DEBUG2, "%s: tid=%lu gsmd=%s", __func__,
       (unsigned long) pthread_self(), name);

  /*
   * create a connected socket for the primary gsmd (we don't poll backup)
   * too bad we're already daemonized here (error reporting goes to log only)
   */
  s = xsocket(PF_INET, SOCK_DGRAM, 0);
  /* TODO: possibly bind to a specific local IP */
  xconnect(s, (const struct sockaddr *) &gsmd->sin[0], sizeof(gsmd->sin[0]));
  /* set socket to non-blocking */
  xioctl1(s, FIONBIO);

  /* one-time gsmd request setup */
  (void) sprintf((char *) cmsg, "G !");
  len  = 4;
  ulen = 1;
  plen = 4;			/* no padding required, conveniently */
  xlen = HELIX_NONCE_LEN + plen + HELIX_MAC_LEN;

  /* populate static part of nonce */
  HEUINT32TOLEUCHAR(&c.msg[0], streamid[0]);		/* random constant */
  HEUINT32TOLEUCHAR(&c.msg[4], streamid[1]);		/* random constant */
  c.msg[8] = len & 0xff;				/* low-order len   */
  c.msg[9] = (len & 0xff00) >> 8;			/* high-order len  */
  c.msg[10] = cmsg[0];					/* request type    */
  c.msg[11] = 0;					/* reserved        */

  pfd.fd     = s;
  pfd.events = POLLIN;
  /* wait until we are signalled */
  for (;;) {
    hrtime_t timeout;

    (void) sem_wait(&gsmd->sem);
    mlog(LOG_DEBUG2, "%s(%s): signalled", __func__, name);

    /*
     * At this point we are using the primary server, which has timed out.
     *
     * Rules for signalling:
     *   - we are only signalled when primary is active and has timed out
     *   - after setting backup active, we do not get signalled even on timeout
     *   - we keep the backup active for the duration of the timeout +1s
     *
     * Many threads can be using the primary after we select the backup.
     * But since all of these should timeout before we switch back, the only
     * way we can get signalled late is if a thread stalls.  While highly
     * unlikely, this is certainly possible since we don't synchronize
     * with work threads.
     *
     * Even if that happens, it's quite alright.  We'll find that the
     * primary is alive and go back to sleep.
     */

    /*
     * NOTE: Failover timeout is inexact.  config.y sets it to the
     *       configured value - config.timeout, a weak attempt to
     *       ensure that the MAXIMUM timeout is the configured value.
     *       But userops->get() might use up a lot of config.timeout,
     *       so the actual gsmd timeout might a lot less.  It could
     *       also be more, depending on how work is scheduled relative
     *       to when gsmd becomes unavailable.  We could make it exact
     *       if we signalled failover as soon as one packet is lost
     *       but that's a lot of extra activity that's not worthwhile.
     */

    /*
     * we need a new seqno each time to guarantee we aren't seeing an
     * old response retrans, otherwise we'd do the crypto once
     */

    /* populate dynamic part (seqno) of nonce */
    seq = nonce32();
    HEUINT32TOLEUCHAR(&c.msg[12], seq);
    helix_nonce(gsmd->scontext, &dcontext, c.msg);

    /* encrypt message */
    (void) helix_encrypt(gsmd->scontext, &dcontext, cmsg, plen, cmsg,
                         &c.msg[HELIX_NONCE_LEN + plen]);

    /* setup for failover timeout */
    timeout = xgethrtime() + gsmd->fo_timeout;
something:
    while (xgethrtime() < timeout) {
      int rc;

      if (send(s, c.msg, xlen, 0) == -1) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
          /* NOPAUSE */
          /* NORESET */
          continue;
        } else {
          /* shit */
          mlog(LOG_ERR, "%s(%s): send: %s", __func__, name, strerror(errno));
          /* don't get stuck in a tight loop */
          goto reset;
        }
      }
      rc = poll(&pfd, 1, 200 /* .2s */);
      if (rc == -1) {
        if (errno == EAGAIN) {
          /* NOPAUSE */
          /* NORESET */
          continue;
        } else {
          /* shit */
          mlog(LOG_ERR, "%s(%s): poll: %s", __func__, name, strerror(errno));
          /* don't get stuck in a tight loop */
          goto reset;
        }
      } else if (rc == 0) {
        RESET_FAILBACK;
        continue;
      }

      /* data is available */
      if ((rlen = recv(s, p.msg, sizeof(p.msg), 0)) == -1) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
          /* NOPAUSE */
          RESET_FAILBACK;
          continue;
        } else if (errno == ECONNREFUSED) {
          PAUSE;
          RESET_FAILBACK;
          continue;
        } else {
          /* shit */
          mlog(LOG_ERR, "%s(%s): recv: %s", __func__, name, strerror(errno));
          /* consider it a timeout, retrans */
          PAUSE;
          RESET_FAILBACK;
          continue;
        }
      }

      /* decrypt */
      {
        union {
          unsigned char mac[HELIX_MAC_LEN];
          uint32_t m;	/* force alignment (superfluous) */
        } m;

        helix_nonce(gsmd->scontext, &dcontext, p.msg);
        (void) helix_decrypt(gsmd->scontext, &dcontext,
                             pmsg, rlen - HELIX_NONCE_LEN - HELIX_MAC_LEN,
                             pmsg, m.mac);
        if (memcmp(m.mac, &p.msg[rlen - HELIX_MAC_LEN], HELIX_MAC_LEN)) {
          mlog(LOG_ERR, "%s(%s): decrypt integrity check failed", __func__,
               name);
          PAUSE;
          /*
           * NOTE: Could just be a forged packet, with nasty side effect
           *       of stopping us from failing back, but we can't risk
           *       allowing a failback to a broken gsmd, so we must
           *       reset the timer.
           * NOTE: This cannot be a misconfigured (wrong key) gsmd,
           *       b/c it won't be able to decrypt our request.
           */
          RESET_FAILBACK;
          continue;
        }

        /* decrypt successful, extract length */
        len = p.msg[8] + (p.msg[9] << 8);
        /* validate the length before claiming successful decrypt */
        if ((unsigned) len < 3+ulen ||
            len > GSM_MAX_MSG_LEN ||
            rlen - (HELIX_NONCE_LEN + HELIX_MAC_LEN) - len > 3) {
          mlog(LOG_ERR, "%s(%s): plaintext length error", __func__, name);
          PAUSE;
          RESET_FAILBACK;
          continue;
        }

        /* verify seqno */
        {
          uint32_t rseq;

          LEUCHAR2HEUINT32(rseq, &p.msg[12]);
          if (rseq != seq) {
            /* could be a retrans, try again */
            mlog(LOG_INFO, "%s(%s): bad msg format (seqno)", __func__, name);
            PAUSE;
            /*
             * NOTE: Could be an old packet (from last time through), which
             *       messes up our failback timer, but we should eventually
             *       catch up.  There could be a LOT of them, but the chance
             *       of that is slim.  Like the MAC failure, we can't risk
             *       failing back to a broken gsmd.
             *
             *       We could almost guarantee no stale packets by using a
             *       new socket (probably a new src port) each time, but then
             *       we'd have to tolerate failures of socket(),connect(),
             *       ioctl().  Just as that's unlikely to happen, we're
             *       unlikely to ever see a significant number of old packets.
             *       OTOH, if we *are* seeing lots of old packets the network
             *       is probably broken anyway so WTD?
             */
            RESET_FAILBACK;
            continue;
          }
        }
        /* verify space after command letter */
        if (pmsg[1] != ' ') {
          mlog(LOG_ERR, "%s(%s): bad msg format (command delimiter)", __func__,
               name);
          PAUSE;
          RESET_FAILBACK;
          continue;
        }
        /* verify username-space after command */
        if (strncmp("!", (const char *) &pmsg[2], ulen) ||
            (pmsg[ulen+2] != ' ' && pmsg[ulen+2] != '\0')) {
          mlog(LOG_ERR, "%s(%s): bad msg format (username)", __func__, name);
          PAUSE;
          RESET_FAILBACK;
          continue;
        }
        /* verify proper NUL term */
        if (pmsg[len - 1] != '\0') {
          mlog(LOG_ERR, "%s(%s): bad msg format (NUL term)", __func__, name);
          PAUSE;
          RESET_FAILBACK;
          continue;
        }
      } /* decrypt */

      /*
       * decrypt successful, process response
       */

      if (pmsg[0] == 'A') {
        if (gsmd->active == 0) {
          /* primary is active && responsive, go back to sleep */
          goto reset;
        } else {
          /* backup is active, keep polling primary */
          PAUSE;
          /* NORESET */
          continue;
        }

      } else if (pmsg[0] == 'N') {
        char		*reason;
        static char	*oreason = NULL;	/* prevent log spam */

        if (pmsg[ulen + 2] != '\0')
          reason = (char *) &pmsg[ulen + 3];	/* possibly empty string */
        else
          reason = (char *) "[no reason given]";	/* shouldn't happen */
        /* don't spam the log */
        if (!(oreason && !strcmp(oreason, reason))) {
          mlog(LOG_ERR, "%s(%s): NAK: %s", __func__, name, reason);
          oreason = xstrdup(reason);
        }
        PAUSE;
        RESET_FAILBACK;
        continue;

      } else {
        mlog(LOG_ERR, "%s(%s): bad msg format (command)", __func__, name);
        PAUSE;
        RESET_FAILBACK;
        continue;
      }

    } /* while (timeout) */

    /*
     * timeout
     */

    if (gsmd->active == 0) {
      /* failover */
      gsmd->active = 1;
      /* current read only guaranteed after unlock()/lock() sequence    */
      xpthread_mutex_lock(&active_mutex);	/* this is nothing      */
      xpthread_mutex_unlock(&active_mutex);	/* this is the unlock() */
      mlog(LOG_WARNING, "%s(%s): failover", __func__, name);
      timeout = xgethrtime() + gsmd->fb_timeout;
      goto something;
    } else {
      /* failback */
      gsmd->active = 0;
      /* current read only guaranteed after unlock()/lock() sequence    */
      xpthread_mutex_lock(&active_mutex);	/* this is nothing      */
      xpthread_mutex_unlock(&active_mutex);	/* this is the unlock() */
      mlog(LOG_WARNING, "%s(%s): failback", __func__, name);
    }

reset:
    while(sem_trywait(&gsmd->sem) != -1 && errno != EAGAIN)
      ;
    mlog(LOG_DEBUG2, "%s(%s): sleeping", __func__, name);
  }

  return NULL;
}
