/*
 * $Id: //trid/release/otpd-3.1.0/work_thread.c#1 $
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
 * Copyright 2005,2006 TRI-D Systems, Inc.
 */

#include "ident.h"
RCSID("$Id: //trid/release/otpd-3.1.0/work_thread.c#1 $")

#include <errno.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "extern.h"
#include "otp.h"

typedef struct otp_request_v1_t {
  int	version;					/* 1 */
  char	username[OTP_MAX_USERNAME_LEN + 1];
  char	challenge[OTP_MAX_CHALLENGE_LEN + 1];		/* USER challenge */
  struct {
    otp_pwe_t	  pwe;
    char	  passcode[OTP_MAX_PASSCODE_LEN + 1];
    unsigned char challenge[OTP_MAX_CHAP_CHALLENGE_LEN]; /* CHAP challenge */
    size_t	  clen;
    unsigned char response[OTP_MAX_CHAP_RESPONSE_LEN];
    size_t	  rlen;
  } pwe;
  int		allow_async;		/* async auth allowed?           */
  int		allow_sync;		/* sync auth allowed?            */
  unsigned	challenge_delay;	/* min delay between async auths */
  int		resync;			/* resync on async auth?         */
} otp_request_v1_t;

#define RETURN do {		\
  (void) close(config->s);	\
  free(config);			\
  return NULL;			\
} while(0)

/*
 * This is the heart of otpd.  Each connection to otpd is handled by
 * its own work_thread, which processes requests serially.
 */
void *
work_thread(void *arg)
{
  config_t	*config = (config_t *) arg;
  otp_request_t	request;
  otp_reply_t	reply = { .version = 1 };

  char *p;
  size_t nread, nwrote;
  ssize_t n;

  otp_request_v1_t request_v1;

  /* save original timeout as it gets modified as we make progress */
  int64_t timeout = config->timeout;

  mlog(LOG_DEBUG2, "%s: tid=%lu, fd=%d",
       __func__, (unsigned long) pthread_self(), config->s);

  for (;;) {
    nread = 0;
    p = (char *) &request;

    /*
     * Read v2 or v1 request.
     * Since v2 is smaller than v1, we could read v2 requests in a single
     * go, but for now we just read the version before finishing the read.
     * If we can't get rid of v1 clients quickly, we can change this.
     * We'd do it from the start but it's not exactly trivial since we
     * have to figure out struct packing stuff.
     */
    while (nread < sizeof(request.version)) {
      if ((n = read(config->s, &p[nread],
                    sizeof(request.version) - nread)) == -1) {
        if (errno == EINTR) {
          continue;
        } else {
          mlog(LOG_ERR, "%s: plugin read (fd=%d): %s", __func__, config->s,
               strerror(errno));
          RETURN;
        }
      }
      if (!n) {
        /* EOF */
        mlog(LOG_DEBUG, "%s(%lu,%d): plugin disconnect",
             __func__, (unsigned long) pthread_self(), config->s);
        RETURN;
      }
      nread += n;
    } /* while (reading request) */

    if (request.version == 2) {
      while (nread < sizeof(request)) {
        if ((n = read(config->s, &p[nread], sizeof(request) - nread)) == -1) {
          if (errno == EINTR) {
            continue;
          } else {
            mlog(LOG_ERR, "%s: plugin read (fd=%d): %s", __func__, config->s,
                 strerror(errno));
            RETURN;
          }
        }
        if (!n) {
          /* EOF */
          mlog(LOG_DEBUG, "%s(%lu,%d): plugin disconnect",
               __func__, (unsigned long) pthread_self(), config->s);
          RETURN;
        }
        nread += n;
      } /* while (reading request) */

    } else if (request.version == 1) {
      p = (char *) &request_v1;

      while (nread < sizeof(request_v1)) {
        if ((n = read(config->s, &p[nread], sizeof(request_v1) - nread)) == -1){
          if (errno == EINTR) {
            continue;
          } else {
            mlog(LOG_ERR, "%s: plugin read (fd=%d): %s", __func__, config->s,
                 strerror(errno));
            RETURN;
          }
        }
        if (!n) {
          /* EOF */
          mlog(LOG_DEBUG, "%s(%lu,%d): plugin disconnect",
               __func__, (unsigned long) pthread_self(), config->s);
          RETURN;
        }
        nread += n;
      } /* while (reading request) */

      mlog(LOG_WARNING,
           "%s: deprecated v1 auth request from fd %d, please upgrade client",
           __func__, config->s);

      /* convert to v2 */
      request.version = 2;
      (void) memcpy(request.username, request_v1.username,
                    sizeof(request.username));
      (void) memcpy(request.challenge, request_v1.challenge,
                    sizeof(request.challenge));
      request.pwe.pwe = request_v1.pwe.pwe;
      if (request.pwe.pwe == PWE_PAP) {
        (void) memcpy(request.pwe.u.pap.passcode, &request_v1.pwe.passcode,
                      sizeof(request.pwe.u.pap.passcode));
      } else {
        /* just copy, let the type be validated later */
        (void) memcpy(request.pwe.u.chap.challenge, &request_v1.pwe.challenge,
                      sizeof(request.pwe.u.chap.challenge));
        request.pwe.u.chap.clen = request_v1.pwe.clen;
        (void) memcpy(request.pwe.u.chap.response, &request_v1.pwe.response,
                      sizeof(request.pwe.u.chap.response));
        request.pwe.u.chap.rlen = request_v1.pwe.rlen;
      }
      request.allow_async     = request_v1.allow_async;
      request.allow_sync      = request_v1.allow_sync;
      request.challenge_delay = request_v1.challenge_delay;
      request.resync          = request_v1.resync;

    } else {
      mlog(LOG_ERR, "%s: plugin request (fd=%d) invalid version %d",
           __func__, config->s, request.version);
      RETURN;
    }

    /*
     * validate the request
     */

    if (request.username[OTP_MAX_USERNAME_LEN] != '\0') {
      mlog(LOG_ERR, "%s: plugin request (fd=%d) invalid (username length)",
           __func__, config->s);
      RETURN;
    }
    if (!strcmp(request.username, "!") || strchr(request.username, '=') ||
        strchr(request.username, '<') || strchr(request.username, '>') ||
        strchr(request.username, '*') || strchr(request.username, '~')) {
      mlog(LOG_ERR, "%s: plugin request (fd=%d) invalid (username charset)",
           __func__, config->s);
      RETURN;
    }

    if (request.challenge[OTP_MAX_CHALLENGE_LEN] != '\0') {
      mlog(LOG_ERR, "%s: plugin request (fd=%d) invalid (challenge length)",
           __func__, config->s);
      RETURN;
    }

    /*
     * Validate passcode/challenge/response lengths.
     * It would be cleaner to do this where it is verified,
     * but we'd rather kick out invalid requests early,
     * than do unnecessary work.
     */
    switch (request.pwe.pwe) {
    case PWE_PAP:
      if (request.pwe.u.pap.passcode[OTP_MAX_PASSCODE_LEN] != '\0') {
        mlog(LOG_ERR, "%s: plugin request (fd=%d) invalid (passcode)",
             __func__, config->s);
        RETURN;
      }
      break;

    case PWE_CHAP:
      if (request.pwe.u.chap.clen > 16) {
        mlog(LOG_ERR,
             "%s: plugin request (fd=%d) invalid (chap challenge len %zu)",
             __func__, config->s, request.pwe.u.chap.clen);
        RETURN;
      }
      if (request.pwe.u.chap.rlen != 17) {
        mlog(LOG_ERR,
             "%s: plugin request (fd=%d) invalid (chap response len %zu)",
             __func__, config->s, request.pwe.u.chap.rlen);
        RETURN;
      }
      break;

    case PWE_MSCHAP:
      if (request.pwe.u.chap.clen != 8) {
        mlog(LOG_ERR,
             "%s: plugin request (fd=%d) invalid (mschap challenge len %zu)",
             __func__, config->s, request.pwe.u.chap.clen);
        RETURN;
      }
      if (request.pwe.u.chap.rlen != 50) {
        mlog(LOG_ERR,
             "%s: plugin request (fd=%d) invalid (mschap response len %zu)",
             __func__, config->s, request.pwe.u.chap.rlen);
        RETURN;
      }
    /* We don't handle LM responses (see otp.c:compare()) */
      if (request.pwe.u.chap.response[1] != 1) {
        mlog(LOG_NOTICE, "%s: MSCHAP response bad flags (LM not supported)",
             __func__);
        reply.rc = OTP_RC_AUTH_ERR;
        goto send_reply;
      }
      break;

    case PWE_MSCHAP2:
      if (request.pwe.u.chap.clen != 16) {
        mlog(LOG_ERR,
             "%s: plugin request (fd=%d) invalid (mschap2 challenge len %zu)",
             __func__, config->s, request.pwe.u.chap.clen);
        RETURN;
      }
      if (request.pwe.u.chap.rlen != 50) {
        mlog(LOG_ERR,
             "%s: plugin request (fd=%d) invalid (mschap2 response len %zu)",
             __func__, config->s, request.pwe.u.chap.rlen);
        RETURN;
      }
      break;

    default:
      mlog(LOG_ERR, "%s: plugin request (fd=%d) invalid (pwe %d unknown)",
           __func__, config->s, request.pwe.pwe);
      RETURN;
    } /* switch (request.pwe.pwe) */

    /* we have a valid request */
    mlog(LOG_DEBUG, "%s(%lu,%d): handling plugin request for [%s]",
         __func__, (unsigned long) pthread_self(), config->s, request.username);

    config->timeout = timeout;
    (void) verify(config, &request, &reply);

send_reply:
    nwrote = 0;
    p = (char *) &reply;

    while (nwrote < sizeof(reply)) {
      if ((n = write(config->s, &p[nwrote], sizeof(reply) - nwrote)) == -1) {
        if (errno == EINTR || errno == EPIPE) {
          continue;
        } else {
          mlog(LOG_ERR, "%s: plugin write (fd=%d): %s", __func__, config->s,
               strerror(errno));
          RETURN;
        }
      }
      nwrote += n;
    } /* while (writing reply) */
  } /* for (;;) */
} /* work_thread() */
