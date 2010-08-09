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
 * Copyright 2005-2008 TRI-D Systems, Inc.
 */

#include "ident.h"
RCSID("$Id$")

#include <errno.h>
#include <inttypes.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <unistd.h>
//#include <stropts.h>
#ifdef __sun__
/* FIONBIO */
#define BSD_COMP
#endif
#include <sys/ioctl.h>
#include <poll.h>
#include <semaphore.h>

#include "extern.h"
#include "otp.h"
#include "gsm.h"
#include "cardops.h"
#include "bitmanip.h"
#include "state.h"

static __thread helix_dcontext_t dcontext;	/* helix dynamic context */
/*
 * This is used to make sure we get a "recent" value for the active gsmd.
 * The gsmd_t could have a mutex but it would be contended and there's no
 * point in that since we don't care if we have the absolute most
 * recent value.  We could skip this altogether since sendto() et al.
 * will have a mutex but it's better not to depend on that.
 */
static __thread pthread_mutex_t active_mutex = PTHREAD_MUTEX_INITIALIZER;

/*
 * lock and retrieve state for a user
 * returns 0 on success (but state may be empty!), -1 on failure
 */
int
state_get(const config_t *config, const user_t *user, state_t *state)
{
  state->locked = 0;
  if (config->state.mode == SMODE_LOCAL)
    return state_get_local(config, user, state);
  else
    return state_get_global(config, user, state);
}

/*
 * lock and retrieve state (locally) for a user
 * returns 0 on success (but state may be empty!), -1 on failure
 */
static int
state_get_local(const config_t *config, const user_t *user, state_t *state)
{
  char		buf[GSM_MAX_STATE_LEN + 1];
  ssize_t	len;

  if ((state->l.lock = lock_get(user->username))) {
    if ((len = state_read(buf, sizeof(buf), config->state.statedir,
                          user->username)) == -1) {
      lock_put(state->l.lock);
      return -1;
    }

    if (state_parse(buf, len, config, user->username, state) == -1) {
      lock_put(state->l.lock);
      return -1;
    }
  }

  state->locked = 1;
  state->updated = 0;
  return 0;
} /* state_get_local() */

/*
 * lock and retrieve state (globally) for a user
 * returns 0 on success (but state may be empty!), -1 on failure
 */
static int
state_get_global(const config_t *config, const user_t *user, state_t *state)
{
  union {
    unsigned char	msg[GSM_ULTRA_MSG_LEN];
    uint64_t		u;	/* force uint64 aligned msg */
  } c, p;
  int	len, ulen;
  char	*cmsg = (char *) &c.msg[HELIX_NONCE_LEN];
  char	*pmsg = (char *) &p.msg[HELIX_NONCE_LEN];

  state->l.g.g = gsmd_get(user->gsmd);
  if (!state->l.g.g) {
    if (user->gsmd[0] == '\0')
      mlog(LOG_ERR, "%s: gsmd location not available for [%s]", __func__,
           user->username);
    else
      mlog(LOG_ERR, "%s: gsmd '%s' not configured for [%s]", __func__,
           user->gsmd, user->username);
    return -1;
  }

  /*
   * Unfortunately, unlike in our previous separate lsmd design, we
   * incur the overhead of creating a socket for each auth request.
   * We do this for simplicity, so that we don't require a comm
   * thread per gsmd, to which we would have to post our requests,
   * as well as not having to match up gsmd responses.  On the plus
   * side, the extra work is at the edge (at the auth server, rather
   * than gsmd), so it can be distributed easily.
   * NOTE: We handle socket() and connect() errors ourself.  In the
   *       setup-sockets-in-advance model, we can just use x* because
   *       we'll exit right away.  Here we can't afford to exit just
   *       because of a possibly transient error.
   * NOTE: We stash the socket in the state data for reuse by PUT.
   */
  if ((state->l.g.s = socket(PF_INET, SOCK_DGRAM, 0)) == -1) {
    mlog(LOG_ERR, "%s: socket: %s", __func__, strerror(errno));
    return -1;
  }
  /* current read only guaranteed after unlock()/lock() sequence      */
  xpthread_mutex_lock(&active_mutex);		/* this is the lock() */
  xpthread_mutex_unlock(&active_mutex);		/* this is nothing    */
  state->l.g.active = state->l.g.g->active;	/* stash for PUT      */
  /* TODO: possibly bind to a specific local IP */
  if (connect(state->l.g.s,
              (const struct sockaddr *) &state->l.g.g->sin[state->l.g.active],
              sizeof(state->l.g.g->sin[0]))) {
    mlog(LOG_ERR, "%s: connect: %s", __func__, strerror(errno));
    (void) close(state->l.g.s);
    return -1;
  }
  /* set socket to non-blocking */
  {
    int one = 1;

    if (ioctl(state->l.g.s, FIONBIO, &one) == -1) {
      mlog(LOG_ERR, "%s: ioctl: %s", __func__, strerror(errno));
      (void) close(state->l.g.s);
      return -1;
    }
  }

  ulen = sprintf(cmsg, "G %s", user->username) - 2;
  if ((len = gsmd(config, user, state, c.msg, p.msg, __func__)) == -1) {
    (void) close(state->l.g.s);
    return -1;
  }

  if (state_parse(&pmsg[3+ulen], len - (3+ulen), config,
                  user->username, state) == -1) {
    (void) close(state->l.g.s);
    return -1;
  }

  state->locked  = 1;
  state->updated = 0;
  return 0;
} /* state_get_global() */

/*
 * update and release state for a user
 * returns 0 on success, -1 on failure
 */
int
state_put(const config_t *config, const user_t *user, state_t *state)
{
  if (!state->locked)
    return 0;

  if (config->state.mode == SMODE_LOCAL)
    return state_put_local(config, user, state);
  else
    return state_put_global(config, user, state);
}

/*
 * update and release state (locally) for a user
 * returns 0 on success, -1 on failure
 */
static int
state_put_local(const config_t *config, const user_t *user, state_t *state)
{
  char	buf[GSM_MAX_STATE_LEN + 1];
  int	rc;

  if (state->updated) {
    state_unparse(buf, user, state);
    rc = state_write(config->state.statedir, user->username, buf);
  } else {
    rc = 0;
  }

  lock_put(state->l.lock);
  state->locked = 0;

  return rc;
} /* state_put_local() */

/*
 * update and release state (globally) for a user
 * returns 0 on success, -1 on failure
 */
static int
state_put_global(const config_t *config, const user_t *user, state_t *state)
{
  union {
    unsigned char	msg[GSM_ULTRA_MSG_LEN];
    uint64_t		u;	/* force uint64 aligned msg */
  } c, p;
  int		len, ulen;
  char		*cmsg = (char *) &c.msg[HELIX_NONCE_LEN];

  if (state->updated) {
    ulen = sprintf(cmsg, "P %s ", user->username) - 3;
    state_unparse(&cmsg[ulen+3], user, state);
  } else {
    ulen = sprintf(cmsg, "P %s", user->username) - 2;
  }
  if ((len = gsmd(config, user, state, c.msg, p.msg, __func__)) == -1) {
    (void) close(state->l.g.s);
    return -1;
  }

  (void) close(state->l.g.s);
  state->locked = 0;
  return 0;
} /* state_put_global() */

#define UPDATE_TIMEOUT do {		\
  hrtime   = xgethrtime() - hrlast;	\
  timeout -= hrtime;			\
  hrlast  += hrtime;			\
} while (0)

/*
 * communcate with gsmd
 * returns response len on success, -1 otherwise
 * errors are logged
 * p is filled in
 */
static int
gsmd(const config_t *config, const user_t *user, state_t *state,
     unsigned char cp[GSM_ULTRA_MSG_LEN], unsigned char pp[GSM_ULTRA_MSG_LEN],
     const char *caller)
{
  int		len;	/* msg length before padding */
  int		plen;	/* msg length after padding  */
  size_t	xlen;	/* xmit length after encap   */
  ssize_t	rlen;	/* recv length before decap  */
  size_t	ulen;	/* username length           */
  int		i;
  hrtime_t	hrlast, hrtime, timeout;
  int		failover, polltimeout;
  char		*reason;
  struct pollfd	pfd;
  unsigned char	*cmsg = &cp[HELIX_NONCE_LEN];
  unsigned char	*pmsg = &pp[HELIX_NONCE_LEN];

  hrlast = xgethrtime();
  timeout = config->timeout;

  len = strlen((const char *) cmsg) + 1;
  ulen = strlen(user->username);
  /* pad to uint32 */
  plen = ROUND4(len + 3);
  for (i = len; i < plen; ++i)
    cmsg[i] = '\0';
  xlen = HELIX_NONCE_LEN + plen + HELIX_MAC_LEN;

  /* populate nonce */
  HEUINT32TOLEUCHAR(&cp[0], streamid[0]);		/* random constant */
  HEUINT32TOLEUCHAR(&cp[4], streamid[1]);		/* random constant */
  cp[8] = len & 0xff;					/* low-order len   */
  cp[9] = (len & 0xff00) >> 8;				/* high-order len  */
  cp[10] = cmsg[0];					/* request type    */
  cp[11] = 0;						/* reserved        */
  if (cmsg[0] == 'G')
    state->l.g.seq = nonce32();				/* save for PUT    */
  else
    state->l.g.seq++;			/* disambiguate late GET responses */
  HEUINT32TOLEUCHAR(&cp[12], state->l.g.seq);		/* sequence        */
  helix_nonce(state->l.g.g->scontext, &dcontext, cp);

  /* encrypt message */
  (void) helix_encrypt(state->l.g.g->scontext, &dcontext, cmsg, plen, cmsg,
                       &cp[HELIX_NONCE_LEN + plen]);

  /* xmit/recv loop */
  pfd.fd      = state->l.g.s;
  pfd.events  = POLLIN;
  failover    = 1;	/* failover only if we get no responses */
  polltimeout = 0;
  reason      = "timeout";
  while (timeout >= 250000000LL /* 250ms */) {
    int rc;

    if (send(state->l.g.s, cp, xlen, 0) == -1) {
      if (errno == EINTR || errno == EWOULDBLOCK) {
        UPDATE_TIMEOUT;
        continue;
      } else {
        mlog(LOG_ERR, "%s: send: %s", caller, strerror(errno));
        return -1;
      }
    }

    rc = poll(&pfd, 1, 200 /* .2s */);
    if (rc == -1) {
      if (errno == EINTR || errno == EAGAIN) {
        UPDATE_TIMEOUT;
        continue;
      } else {
        mlog(LOG_ERR, "%s: poll: %s", caller, strerror(errno));
        return -1;
      }
    } else if (rc == 0) {
      UPDATE_TIMEOUT;
      /* only override last reason if we've timed out a few times in a row */
      if (polltimeout++ > 5)
        reason = "timeout";
      continue;
      /* TODO: track timeout rate and possibly failover */
    }

    /* data is available */
    if ((rlen = recv(state->l.g.s, pp, GSM_ULTRA_MSG_LEN, 0)) == -1) {
      if (errno == EINTR || errno == EWOULDBLOCK) {
        UPDATE_TIMEOUT;
        continue;
      } else {
        mlog(LOG_ERR, "%s: recv: %s", caller, strerror(errno));
        if (errno == ECONNREFUSED)
          goto failover;
        return -1;
      }
    }

    if ((size_t) rlen < HELIX_NONCE_LEN + 3+ulen + HELIX_MAC_LEN) {
      mlog(LOG_ERR, "%s: chiphertext length error", caller);
      return -1;
    }

    /* decrypt */
    {
      union {
        unsigned char mac[HELIX_MAC_LEN];
        uint32_t m;	/* force alignment (superfluous) */
      } m;

      helix_nonce(state->l.g.g->scontext, &dcontext, pp);
      (void) helix_decrypt(state->l.g.g->scontext, &dcontext,
                           pmsg, rlen - HELIX_NONCE_LEN - HELIX_MAC_LEN,
                           pmsg, m.mac);
      if (memcmp(m.mac, &pp[rlen - HELIX_MAC_LEN], HELIX_MAC_LEN)) {
        mlog(LOG_ERR, "%s: decrypt integrity check failed", caller);
        return -1;
      }

      /* decrypt successful, extract length */
      len = pp[8] + (pp[9] << 8);
      /* validate the length before claiming successful decrypt */
      if ((unsigned) len < 3+ulen ||
          len > GSM_MAX_MSG_LEN ||
          rlen - (HELIX_NONCE_LEN + HELIX_MAC_LEN) - len > 3) {
        mlog(LOG_ERR, "%s: plaintext length error", caller);
        return -1;
      }

      /* verify seqno */
      {
        uint32_t rseq;

        LEUCHAR2HEUINT32(rseq, &pp[12]);
        if (rseq != state->l.g.seq) {
          /* could be a retrans, try again */
          mlog(LOG_INFO, "%s: bad msg format (seqno)", caller);
          UPDATE_TIMEOUT;
          continue;
        }
      }
      /* verify space after command letter */
      if (pmsg[1] != ' ') {
        mlog(LOG_ERR, "%s: bad msg format (command delimiter)", caller);
        return -1;
      }
      /* verify username-space after command */
      if (strncmp(user->username, (const char *) &pmsg[2], ulen) ||
          (pmsg[ulen+2] != ' ' && pmsg[ulen+2] != '\0')) {
        mlog(LOG_ERR, "%s: bad msg format (username)", caller);
        return -1;
      }
      /* verify proper NUL term */
      if (pmsg[len - 1] != '\0') {
        mlog(LOG_ERR, "%s: bad msg format (NUL term)", caller);
        return -1;
      }
    } /* decrypt */

    /*
     * decrypt successful, process response
     */

    if (pmsg[0] == 'A') {
      return len;

    } else if (pmsg[0] == 'N') {
      failover = 0;	/* don't failover            */
      polltimeout = 0;	/* reset timeout reporting   */
      /* save [last] reason for reporting on timeout */
      if (pmsg[ulen + 2] != '\0')
        reason = (char *) &pmsg[ulen + 3];	/* possibly empty string */
      else
        reason = (char *) "[no reason given]";	/* shouldn't happen */

      /* try again if enough time left */
      /* TODO: return immediately on PUT failures? */
      UPDATE_TIMEOUT;
      continue;

    } else {
      mlog(LOG_ERR, "%s: bad msg format (command)", caller);
      return -1;
    }
  } /* while (250ms left) */

  mlog(LOG_ERR, "%s: gsmd: %s", caller, reason);
failover:
  /* signal failover thread on timeout of primary gsmd */
  if (failover && state->l.g.g->n == 2 && state->l.g.active == 0)
    (void) sem_post(&state->l.g.g->sem);
  return -1;
} /* gsmd() */

/*
 * Parse the state manager response into state.
 * buf must be NUL terminated.
 * Returns 0 on success, -1 on failure.
 */
static int
state_parse(const char *buf, size_t buflen,
            const config_t *config, const char *username, state_t *state)
{
  char *p, *q;
  size_t ulen = strlen(username);

  /* null state */
  if (!buflen) {
    mlog(LOG_DEBUG, "%s: null state data for [%s]", __func__, username);
    state->nullstate = 1;
    return 0;
  }
  state->nullstate = 0;

  /* sanity checks */
  if (buflen < 4 + ulen) { /* "v:username:\0" */
    mlog(LOG_ERR, "%s: invalid state for [%s]", __func__, username);
    return -1;
  }

  p = (char *) buf;
  /* version */
  if (!(p[0] == '5' && p[1] == ':')) {
    mlog(LOG_ERR, "%s: state data unacceptable version for [%s]",
         __func__, username);
    return -1;
  }
  p += 2;	/* username */

  /* sanity check username */
  if (!(strncmp(p, username, ulen) == 0 && p[ulen] == ':')) {
    mlog(LOG_ERR, "%s: state data username mismatch for [%s]",
         __func__, username);
    return -1;
  }
  p += ulen + 1;	/* challenge */

  /* extract challenge */
  if ((q = strchr(p, ':')) == NULL) {
    mlog(LOG_ERR, "%s: state data invalid challenge for [%s]",
         __func__, username);
    return -1;
  }
  *q++ = '\0';
  if (strlen(p) > OTP_MAX_CHALLENGE_LEN * 2) {
    mlog(LOG_ERR, "%s: state data challenge too long for [%s]",
         __func__, username);
    return -1;
  }
  if (strlen(p) & 1) {
    mlog(LOG_ERR, "%s: state data invalid challenge for [%s]",
         __func__, username);
    return -1;
  }
  state->clen = a2x(p, state->u.challenge);
  if (state->clen < 0) {
    mlog(LOG_ERR, "%s: state data invalid challenge for [%s]",
         __func__, username);
    return -1;
  }
  p = q;	/* csd */

  /* extract csd */
  if ((q = strchr(p, ':')) == NULL) {
    mlog(LOG_ERR, "%s: state data invalid csd for [%s]", __func__, username);
    return -1;
  }
  *q++ = '\0';
  if (strlen(p) > OTP_MAX_CSD_LEN) {
    mlog(LOG_ERR, "%s: state data csd too long for [%s]", __func__, username);
    return -1;
  }
  (void) strcpy(state->csd, p);
  p = q;	/* rd */

  /* extract rd */
  if ((q = strchr(p, ':')) == NULL) {
    mlog(LOG_ERR, "%s: state data invalid rd for [%s]", __func__, username);
    return -1;
  }
  *q++ = '\0';
  if (strlen(p) > OTP_MAX_RD_LEN) {
    mlog(LOG_ERR, "%s: state data rd too long for [%s]", __func__, username);
    return -1;
  }
  (void) strcpy(state->rd, p);
  p = q;	/* failcount */

  /* extract failcount */
  if ((q = strchr(p, ':')) == NULL) {
    mlog(LOG_ERR, "%s: state data invalid failcount for [%s]",
         __func__, username);
    return -1;
  }
  *q++ = '\0';
  if (sscanf(p, "%" SCNx32, &state->failcount) != 1) {
    mlog(LOG_ERR, "%s: state data invalid failcount for [%s]",
         __func__, username);
    return -1;
  }
  p = q;	/* authtime */

  /* extract authtime */
  if ((q = strchr(p, ':')) == NULL) {
    mlog(LOG_ERR, "%s: state data invalid authtime for [%s]",
         __func__, username);
    return -1;
  }
  *q++ = '\0';
  if (sscanf(p, "%" SCNx32, &state->authtime) != 1) {
    mlog(LOG_ERR, "%s: state data invalid authtime for [%s]",
         __func__, username);
    return -1;
  }
  p = q;	/* mincardtime */

  /* extract mincardtime */
  if ((q = strchr(p, ':')) == NULL) {
    mlog(LOG_ERR, "%s: state data invalid mincardtime for [%s]",
         __func__, username);
    return -1;
  }
  *q++ = '\0';
  if (sscanf(p, "%" SCNx32, &state->mincardtime) != 1) {
    mlog(LOG_ERR, "%s: state data invalid mincardtime for [%s]",
         __func__, username);
    return -1;
  }

  return 0;
} /* state_parse() */

/*
 * Format state into a state manager update request.
 * Returns new (filled) buflen on success.
 */
static void
state_unparse(char buf[GSM_MAX_MSG_LEN + 1], const user_t *user,
              const state_t *state)
{
  char s[OTP_MAX_CHALLENGE_LEN * 2 + 1];

  (void) sprintf(buf, "5:%s:"
                      "%s:"
                      "%s:%s:"
                      "%" PRIx32 ":%" PRIx32 ":"
                      "%" PRIx32 ":",
                 /* '5:', */ user->username,
                 x2a(state->u.challenge, state->clen, s, x2a_hex_conversion),
                 state->csd, state->rd,
                 state->failcount, state->authtime,
                 state->mincardtime);		/* safe */
} /* state_unparse() */

/*
 * Full read with logging.
 * Returns nread on success, -1 on failure.
 * buf[nread - 1] is guaranteed to be '\0' if nread > 0.
 */
static ssize_t
state_read(char *buf, size_t len, const char *statedir, const char *username)
{
  char	filename[PATH_MAX + 1];
  int	fd;
  ssize_t nread, r;

  (void) snprintf(filename, PATH_MAX, "%s/%s", statedir, username);
  filename[PATH_MAX] = '\0';

  /* open state file */
  if ((fd = open(filename, O_RDONLY)) == -1) {
    if (errno == ENOENT) {
      /* null state */
      return 0;
    } else {
      /* error */
      mlog(LOG_ERR, "%s: unable to open state file %s: %s",
           __func__, filename, strerror(errno));
      return -1;
    }
  }

  /*
   * NOTE: leave room to add a NUL terminator -- normally not needed
   *       because we overwrite a trailing \n with NUL but in case of
   *       hand editing of the state file, we must be sure.  This
   *       is extremely paranoid because in practice, state
   *       will never be that large.
   */
#if 0
  assert(len);
#endif
  len--;
  nread = 0;
  while ((r = read(fd, &buf[nread], len-nread)) != 0) {
    if (r == -1) {
      if (errno == EINTR || errno == EAGAIN)
        continue;
      mlog(LOG_ERR, "%s: unable to read state file %s: %s",
           __func__, filename, strerror(errno));
      (void) close(fd);
      return -1;
    }
    nread += r;
  }
  if ((size_t) nread == len) {
    off_t cur, end;

    if ((cur = lseek(fd, 0, SEEK_CUR)) == -1) {
      mlog(LOG_ERR, "%s: lseek(%s): %s", __func__, filename, strerror(errno));
      (void) close(fd);
      return -1;
    }
    if ((end = lseek(fd, 0, SEEK_END)) == -1) {
      mlog(LOG_ERR, "%s: lseek(%s): %s", __func__, filename, strerror(errno));
      (void) close(fd);
      return -1;
    }
    /*
     * Not perfect: file could have grown since we read it.
     * But our other choice is to fstat() the file and check the length
     * before we start, which is an overhead ALL the time, as opposed to
     * only when the state file might be too big ... which will be "never".
     */
    if (cur != end) {
      mlog(LOG_ERR, "%s: state file %s too large", __func__, filename);
      (void) close(fd);
      return -1;
    }
  } /* if (we filled all of buf) */

  (void) close(fd);

  /* ensure NUL term */
  if (nread) {
    if (buf[nread - 1] == '\n')
      buf[nread - 1] = '\0';		/* change '\n' to '\0' */
    else
      buf[nread++] = '\0';		/* add '\0' ('noeol' file) */
  }

  return nread;
} /* state_read() */

/*
 * write state for a user to disk
 * caller must hold a lock
 * returns 0 on success, -1 otherwise
 */
static int
state_write(const char *statedir, const char *username, char *buf)
{
  char	filename[PATH_MAX + 1], tmpfilename[PATH_MAX + 1];
  int	fd;
  size_t len;

  /*
   * We use a tmp file to avoid the problem where the disk is full and
   * we write partial state.  Better to fail than to corrupt the state.
   * Should this be a dot file?
   */
  (void) snprintf(tmpfilename, PATH_MAX, "%s/%s%s", statedir, username,
                  "XXXXXX");
  tmpfilename[PATH_MAX] = '\0';
  errno = 0;
  if ((fd = mkstemp(tmpfilename)) == -1) {
    /*
     * Unfortunately mkstemp() is poorly defined; it might not set errno.
     * But we'll try anyway.
     */
    if (errno == ENOENT || EACCES) {
      /*
       * It's important to report ENOENT specially since null state means
       * state_get_local() will succeed all day long w/ missing statedir,
       * but state_put_local() will result in mysterious failure.  We
       * look for statedir at startup, but it might go missing later.
       */
      mlog(LOG_ERR, "%s: mkstemp: %s: %s", __func__, statedir, strerror(errno));
    } else {
      mlog(LOG_ERR, "%s: mkstemp: failed, possibly due to: %s",
           __func__, strerror(errno));
    }
    return -1;
  }

  /* add LF-termination */
  len = strlen(buf);
  if (len)
    if (buf[len - 1] != '\n')
      buf[len++] = '\n';	/* NOTE: buf no longer NUL-termed */

  /* write new state */
  if (xwrite(fd, buf, len) == -1) {
    (void) unlink(tmpfilename);
    (void) close(fd);
    return -1;
  }

  (void) fsync(fd);	/* must guarantee data is on-disk */
  (void) close(fd);

  /* rename to permanent state file */
  (void) snprintf(filename, PATH_MAX, "%s/%s", statedir, username);
  filename[PATH_MAX] = '\0';
  if (rename(tmpfilename, filename) == -1) {
    mlog(LOG_ERR, "%s: rename: %s", __func__, strerror(errno));
    (void) unlink(tmpfilename);
    return -1;
  }

  return 0;
} /* state_write() */
