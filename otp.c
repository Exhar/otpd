/*
 * $Id: //trid/release/otpd-3.1.0/otp.c#1 $
 *
 * Passcode verification functions for otpd.
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
 * Copyright 2001,2002 Google, Inc.
 * Copyright 2005-2008 TRI-D Systems, Inc.
 */

#include "ident.h"
RCSID("$Id: //trid/release/otpd-3.1.0/otp.c#1 $")

#if 0
#if defined(__linux__) && !defined(_GNU_SOURCE)
#define _GNU_SOURCE	/* RTLD_DEFAULT */
#endif
#include <dlfcn.h>
#endif
#include <inttypes.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>	/* strncasecmp() */
#include <time.h>

#include <openssl/des.h>
#include <openssl/md4.h>
#include <openssl/md5.h>
#include <openssl/sha.h>

#include "extern.h"
#include "otp.h"
#include "cardops.h"

static int compare(const otp_request_t *, const char *);
static void key_from_hash(des_cblock *, const unsigned char [7]);
static void decrypt(unsigned char *, size_t, unsigned char *, size_t);

/* Global data */
cardops_t cardops[OTP_MAX_VENDORS];	/* cardops objects */
int ncardops = 0;			/* number of cardops modules loaded */

/* fc (failcondition) shortcuts */
#define OTP_FC_FAIL_NONE  0	/* no failures */
#define OTP_FC_FAIL_HARD  1	/* failed hard */
#define OTP_FC_FAIL_SOFT  2	/* failed soft */
#define OTP_FC_FAIL_DELAY 6	/* in delay mode (includes FAIL_SOFT) */

/*
 * Test for passcode validity.
 *
 * If challenge is supplied, it is used to generate the card response
 * against which the passcode will be compared.  If challenge is not
 * supplied, or if the comparison fails, synchronous responses are
 * generated and tested.  NOTE: for async authentications, sync mode
 * responses are still considered valid!  (Assuming plugin configuration
 * allows sync mode.)
 *
 * If challenge is supplied, then resync is used to determine if the card
 * should be resynced or if this is a one-off response.  (If challenge is
 * not supplied, this is sync mode response and the card is always resynced.)
 *
 * Returns one of the OTP_RC_* return codes.
 */
int
verify(config_t *config, const otp_request_t *request, otp_reply_t *reply)
{
  int		rc, nmatch, i;
  int		e = 0, t = 0;	/* must initialize for async auth path */
  int		fc;		/* failcondition */
  time_t	now;
  hrtime_t	hrlast, hrtime;

  char		*username = (char *) request->username;
  unsigned char	challenge[OTP_MAX_CHALLENGE_LEN];
  int		resync = request->resync;

  user_t	*user;
  state_t	state = { .locked = 0 };

    	/* expected response */
  char 	e_response[OTP_MAX_RESPONSE_LEN + OTP_MAX_PIN_LEN + 1];
  int	pin_offset = 0;	/* pin offset in e_response (prepend or append) */
  int	otp_len = 0;
  int	key_len;
  int	pin_len;

  now = time(NULL);
  hrlast = xgethrtime();

  rc = config->userops->get(request->username, &user, config, now);
  if (rc == -1) {
    /* userops->get() log is sufficient */
    rc = OTP_RC_USER_UNKNOWN;
    goto auth_done_service_err;
  } else if (rc == -2) {
    mlog(LOG_ERR, "%s: unable to get user info for [%s]", __func__, username);
    rc = OTP_RC_AUTHINFO_UNAVAIL;
    goto auth_done_service_err;
  }

  /* find the correct cardops module */
  for (i = 0; cardops[i].prefix; i++) {
    if (!strncasecmp(cardops[i].prefix, user->card, cardops[i].plen)) {
      user->cardops = &cardops[i];
      break;
    }
  }
  if (!user->cardops) {
    mlog(LOG_ERR, "%s: invalid card type '%s' for [%s]",
         __func__, user->card, username);
    rc = OTP_RC_SERVICE_ERR;
    goto auth_done_service_err;
  }

  /* static password override */
  if (user->password) {
    if (user->password[0] == '{' && request->pwe.pwe != PWE_PAP) {
      /* can't compare password */
      mlog(LOG_ERR,
           "%s: invalid auth type with static password override for [%s]",
           __func__, username);
      rc = OTP_RC_SERVICE_ERR;
      goto auth_done_service_err;
    }

    if (strncmp(user->password, "{MD5", 4)) {
      unsigned char md[16];

      (void) MD5((unsigned char *) request->pwe.u.pap.passcode,
                 strlen(request->pwe.u.pap.passcode), md);
      nmatch = memcmp(user->password_hash, md, sizeof(md));

    } else if (strncmp(user->password, "{SHA", 4)) {
      unsigned char md[20];

      (void) SHA1((unsigned char *) request->pwe.u.pap.passcode,
                  strlen(request->pwe.u.pap.passcode), md);
      nmatch = memcmp(user->password_hash, md, sizeof(md));

    } else {
      nmatch = compare(request, user->password);
    }

    if (!nmatch)
      rc = OTP_RC_OK;
    else
      rc = OTP_RC_AUTH_ERR;
    goto auth_done_service_err;
  } /* if (user->password) */

  /* convert name to a feature mask once, for fast operations later */
  if (user->cardops->name2fm(user, &state)) {
    mlog(LOG_ERR, "%s: invalid card type '%s' for [%s]",
         __func__, user->card, username);
    rc = OTP_RC_SERVICE_ERR;
    goto auth_done_service_err;
  }
  otp_len = (user->featuremask & OTP_CF_LEN) >> OTP_CF_LEN_SHIFT;

  /* convert keystring to a key */
  if ((key_len = user->cardops->keystring2key(user, &state)) == -1) {
    mlog(LOG_ERR, "%s: invalid key '%s' for [%s]",
         __func__, user->keystring, username);
    rc = OTP_RC_SERVICE_ERR;
    goto auth_done_service_err;
  }

  /* convert pinstring to a pin */
  pin_len = a2x(user->pinstring, (unsigned char *) user->pin);
  /*
   * pinstring is a hex representation of an ASCII string, so
   * we NUL terminate it for use as string.  It's stored as
   * hex so that we can encrypt/decrypt it.
   */
  user->pin[pin_len] = '\0';

  /* decrypt keyblock and pin */
  if (user->encryptmode == EMODE_PIN || user->encryptmode == EMODE_PINMD5) {
    unsigned char md[MD5_DIGEST_LENGTH];	/* 16 */

    if (request->pwe.pwe != PWE_PAP) {
      /* can't extract PIN, needed to decrypt key */
      mlog(LOG_ERR, "%s: invalid auth type with pin encrypt mode for [%s]",
           __func__, username);
      rc = OTP_RC_SERVICE_ERR;
      goto auth_done_service_err;
    }
    if (pin_len) {
      /* PIN data is not supposed to be present when used as enc key */
      mlog(LOG_ERR, "%s: pin present with pin encrypt mode for [%s]",
           __func__, username);
      rc = OTP_RC_SERVICE_ERR;
      goto auth_done_service_err;
    }

    pin_len = strlen(request->pwe.u.pap.passcode) - otp_len;
    if (pin_len < 0) {
      mlog(LOG_DEBUG1, "%s: passcode wrong length for [%s]",
           __func__, username);
      rc = OTP_RC_AUTH_ERR;
      goto auth_done;
    }
    if (pin_len > OTP_MAX_PIN_LEN) {
      mlog(LOG_DEBUG1, "%s: passcode wrong length for [%s]",
           __func__, username);
      rc = OTP_RC_AUTH_ERR;
      goto auth_done;
    }

    /* extract the PIN from the passcode */
    if (config->prepend_pin) {
      (void) strncpy(user->pin, request->pwe.u.pap.passcode, pin_len);
      user->pin[pin_len] = '\0';
    } else {
      (void) strcpy(user->pin,&request->pwe.u.pap.passcode[otp_len]); /* safe */
    }

    if (user->encryptmode == EMODE_PINMD5) {
      (void) MD5((unsigned char *) user->pin, pin_len, md);
      decrypt(user->key, key_len, md, sizeof(md));
    } else {
      decrypt(user->key, key_len, (unsigned char *) user->pin, pin_len);
    }

  } else if (user->encryptmode == EMODE_KEYID) {
    decrypt(user->key, key_len,
            config->key[user->keyid]->key, config->key[user->keyid]->len);
    decrypt((unsigned char *) user->pin, pin_len,
            config->key[user->keyid]->key, config->key[user->keyid]->len);
  }

  /* early exit if passcode is wrong length */
  /* with pin encrypt modes, we don't know the expected length of the pin */
  if (user->encryptmode != EMODE_PIN && user->encryptmode != EMODE_PINMD5 &&
      request->pwe.pwe == PWE_PAP) {
    if (otp_len + pin_len != (int) strlen(request->pwe.u.pap.passcode)) {
      mlog(LOG_DEBUG1, "%s: passcode wrong length for [%s]",
           __func__, username);
      rc = OTP_RC_AUTH_ERR;
      goto auth_done;
    }
  }

  /* adjust e_response for PIN prepend */
  if (config->prepend_pin) {
    (void) strcpy(e_response, user->pin);
    pin_offset = strlen(e_response);
  }

  /* make sure at least 500ms left to do compute work and state ops */
  hrtime = xgethrtime() - hrlast;
  config->timeout -= hrtime;
  hrlast += hrtime;
  if (config->timeout < 500000000LL) {
    mlog(LOG_ERR, "%s: userops get timeout for [%s]", __func__, username);
    rc = OTP_RC_SERVICE_ERR;
    goto auth_done_service_err;
  }
  /* get user state */
  if (state_get(config, user, &state) != 0) {
    mlog(LOG_ERR, "%s: unable to get state for [%s]", __func__, username);
    rc = OTP_RC_SERVICE_ERR;
    goto auth_done_service_err;
  }
  if (state.nullstate) {
    if (user->cardops->nullstate(config, user, &state, now)) {
      mlog(LOG_ERR, "%s: unable to set null state for [%s]",
           __func__, username);
      rc = OTP_RC_SERVICE_ERR;
      goto auth_done_service_err;
    }
  } else if ((int32_t) state.authtime < 0) {
    /* negative authtime is sentinel for nullstate without successful auth */
    state.authtime = - (int32_t) state.authtime;
    state.nullstate = 1;
  }
  /* make sure at least 250ms left to do compute work and put state */
  hrtime = xgethrtime() - hrlast;
  config->timeout -= hrtime;
  hrlast += hrtime;
  if (config->timeout < 250000000LL) {
    config->timeout = 250000000LL;
    mlog(LOG_ERR, "%s: state_get timeout for [%s]", __func__, username);
    rc = OTP_RC_SERVICE_ERR;
    goto auth_done_service_err_timeout;
  }

  /* set fc (failcondition) */
  if (config->hardfail && state.failcount >= config->hardfail) {
    /* NOTE: persistent softfail stops working */
    fc = OTP_FC_FAIL_HARD;
  } else if (config->softfail && state.nullstate) {
    fc = OTP_FC_FAIL_DELAY;
  } else if (config->softfail && state.failcount >= config->softfail) {
    uint32_t nextauth;
    int fcount;

    /*
     * Determine the next time this user can authenticate.
     *
     * Once we hit softfail, we introduce a 1m (64s) delay before the user
     * can authenticate.  For each successive failed authentication,
     * we double the delay time, up to a max of 32 "minutes".  While in
     * the "delay mode" of operation, all authentication ATTEMPTS are
     * considered failures.  Also, each attempt during the delay period
     * restarts the clock.
     *
     * The advantage of a delay instead of a simple lockout is that an
     * attacker can't lock out a user as easily; the user need only wait
     * a bit before he can authenticate.
     */
    fcount = state.failcount - config->softfail;
    /*
     * nextauth and state.authtime are uint32, but time is saved as int32,
     * so this can't overflow
     */
    nextauth = state.authtime +
               (fcount > 5 ? 32 << 6 : (1 << fcount) << 6);
    if ((uint32_t) now < nextauth)
      fc = OTP_FC_FAIL_DELAY;
    else
      fc = OTP_FC_FAIL_SOFT;
  } else {
    fc = OTP_FC_FAIL_NONE;
  }

  /*
   * Test async response.
   */
  {
    /* NOTE: async challenge comes in NUL-terminated ... */
    size_t clen = strlen(request->challenge);
#if 0 /* guaranteed by caller */
    assert(clen < sizeof(challenge));
#endif
    /* ... but is handled unterminated. */
    (void) memcpy(challenge, request->challenge, clen);

    if (clen && (user->featuremask & OTP_CF_AM) && request->allow_async) {

      /* perform any site-specific transforms of the challenge */
      if (config->site_transform) {
        if ((clen = challenge_transform(username, challenge, clen)) < 0) {
          mlog(LOG_ERR, "%s: challenge transform failed for [%s]",
               __func__, username);
          rc = OTP_RC_SERVICE_ERR;
          goto auth_done_service_err;
          /* NB: state not updated */
        }
      }

      /* calculate the async response */
      if (user->cardops->response(user, &state, challenge, clen,
                                  &e_response[pin_offset])) {
        char s[OTP_MAX_CHALLENGE_LEN * 2 + 1];

        mlog(LOG_ERR, "%s: unable to calculate async response "
                      "to challenge %s for [%s]",
             __func__, user->cardops->printchallenge(s, challenge, clen),
             username);
        rc = OTP_RC_SERVICE_ERR;
        goto auth_done_service_err;
        /* NB: state not updated */
      }

      /*
       * Debug output.  We test for log_level explicitly, rather than
       * just depend on mlog() filtering, to avoid printchallenge().
       * NOTE: We do not display the PIN.
       */
      if (log_level >= LOG_DEBUG1) {
        char s[OTP_MAX_CHALLENGE_LEN * 2 + 1];

        mlog(LOG_DEBUG1, "%s: [%s], async challenge %s, expecting response %s",
             __func__, username,
             user->cardops->printchallenge(s, challenge, clen),
             &e_response[pin_offset]);
      }

      /* add PIN if needed */
      if (!config->prepend_pin)
        (void) strcat(e_response, user->pin);

      /* test user-supplied passcode */
      nmatch = compare(request, e_response);
      if (!nmatch) {
          if (!request->allow_async) {
          mlog(LOG_NOTICE, "%s: bad async auth for [%s]: "
                           "valid but disallowed by plugin config",
               __func__, username);
          rc = OTP_RC_AUTH_ERR;
          goto auth_done;
        }
        if (fc == OTP_FC_FAIL_HARD) {
          mlog(LOG_NOTICE, "%s: bad async auth for [%s]: "
                           "valid but in hardfail (%d/%d failed/max)",
               __func__, username, state.failcount, config->hardfail);
          rc = OTP_RC_MAXTRIES;
          goto auth_done;
        }
        if (fc == OTP_FC_FAIL_DELAY) {
          mlog(LOG_NOTICE, "%s: bad async auth for [%s]: "
                           "valid but in softfail delay (%d/%d failed/max)",
               __func__, username, state.failcount, config->softfail);
          rc = OTP_RC_MAXTRIES;
          goto auth_done;
        }
        if (request->challenge_delay &&
            (uint32_t) now - state.authtime < request->challenge_delay) {
          mlog(LOG_NOTICE, "%s: bad async auth for [%s]: valid but too soon",
               __func__, username);
          rc = OTP_RC_MAXTRIES;
          goto auth_done;
        }

        /* authenticated in async mode */
        rc = OTP_RC_OK;
        /* special log message for sync users */
        if (user->featuremask & OTP_CF_SM)
          mlog(LOG_INFO, "%s: [%s] authenticated in async mode",
               __func__, username);
        goto auth_done;
      } /* if (user authenticated async) */
    } /* if (async mode possible) */
  } /* async_response code block */

  /*
   * Calculate and test sync responses in the window.  Note that we
   * always accept a sync response, even if a challenge or resync was
   * explicitly requested.
   *
   * Note that we always start at the t=0 e=0 window position, even
   * though we may already know a previous authentication is further
   * ahead in the window (when in softfail).  This has the effect that
   * an rwindow softfail override can occur in a sequence like 6,3,4.
   * That is, the user can always move backwards in the window to
   * restart the rwindow sequence, as long as they don't go beyond
   * (prior to) the last successful authentication.
   */
  if ((user->featuremask & OTP_CF_SM) && request->allow_sync) {
    int tend, eend;

    /* set ending ewin counter */
    if (user->featuremask & OTP_CF_FRW) {
      /* force rwindow for e+t cards */
      eend = (user->featuremask & OTP_CF_FRW) >> OTP_CF_FRW_SHIFT;
    } else if (user->featuremask & OTP_CF_ES) {
      /* increase window for softfail+rwindow */
      if (config->rwindow_size && fc & OTP_FC_FAIL_SOFT)
        eend = user->rwindow_size ? user->rwindow_size : config->rwindow_size;
      else
        eend = config->ewindow_size;
    } else {
      eend = 0;
    }

    /* setup initial challenge */
    (void) memcpy(challenge, state.u.challenge, state.clen);

    /* test each sync response in the window */
    tend = user->cardops->maxtwin(user, &state, now);
    if (tend < 0) {
      /* error already logged */
      rc = OTP_RC_SERVICE_ERR;
      goto auth_done_service_err;
      /* NB: state not updated */
    }
    for (t = 0; t <= tend; ++t) {
      for (e = 0; e <= eend; ++e) {
        /* get next challenge */
        rc = user->cardops->challenge(user, &state, challenge, now, t, e);
        if (rc == -1) {
          mlog(LOG_ERR, "%s: unable to get sync challenge t:%d e:%d for [%s]",
               __func__, t, e, username);
          rc = OTP_RC_SERVICE_ERR;
          goto auth_done_service_err;
          /* NB: state not updated */
        } else if (rc == -2) {
          /*
           * For event synchronous modes, we can never go backwards (the
           * challenge() method can only walk forward on the event counter),
           * so there is an implicit guarantee that we'll never get a
           * response matching an event in the past.
           *
           * But for time synchronous modes, challenge() can walk backwards,
           * in order to accomodate clock drift.  We must never allow a
           * successful auth for a correct passcode earlier in time than
           * one already used successfully, so we skip out early here.
           */
          mlog(LOG_DEBUG1, "%s: [%s], sync challenge t:%d e:%d is early",
               __func__, username, t, e);
          continue;
        }

        /* calculate sync response */
        if (user->cardops->response(user, &state, challenge, state.clen,
                                    &e_response[pin_offset])) {
          char s[OTP_MAX_CHALLENGE_LEN * 2 + 1];

          mlog(LOG_ERR, "%s: unable to calculate sync response t:%d e:%d "
                        "to challenge %s for [%s]",
               __func__, t, e,
               user->cardops->printchallenge(s, challenge, state.clen),
               username);
          rc = OTP_RC_SERVICE_ERR;
          goto auth_done_service_err;
          /* NB: state not updated */
        }

        /*
         * Debug output.  We test for log_level explicitly, rather than
         * just depend on mlog() filtering, to avoid printchallenge().
         * NOTE: We do not display the PIN.
         */
        if (log_level >= LOG_DEBUG1) {
          char s[OTP_MAX_CHALLENGE_LEN * 2 + 1];

          mlog(LOG_DEBUG1, "%s: [%s], sync challenge t:%d e:%d %s, "
                           "expecting response %s",
               __func__, username, t, e,
               user->cardops->printchallenge(s, challenge, state.clen),
               &e_response[pin_offset]);
        }

        /* add PIN if needed */
        if (!config->prepend_pin)
          (void) strcat(e_response, user->pin);

        /* test user-supplied passcode */
        nmatch = compare(request, e_response);
        if (!nmatch) {
          if (fc == OTP_FC_FAIL_HARD) {
            mlog(LOG_NOTICE, "%s: bad sync auth for [%s]: "
                             "valid but in hardfail (%d/%d failed/max)",
                 __func__, username, state.failcount, config->hardfail);
            rc = OTP_RC_MAXTRIES;

          } else if (fc & OTP_FC_FAIL_SOFT) {
            /*
             * rwindow logic
             */

            /* user can override softfail with 2 consecutive correct auths */
            if (user->cardops->isconsecutive(user, &state, e)) {
              /* but not too quickly (NOTE: possible time sync probs) */
              if ((uint32_t) now <= state.authtime) {
                mlog(LOG_NOTICE, "%s: softfail override valid for [%s] at "
                                 "window position t:%d e:%d, but too soon",
                     __func__, username, t, e);
                rc = OTP_RC_MAXTRIES;
                goto auth_done;
              }
              mlog(LOG_NOTICE, "%s: softfail override for [%s] at "
                               "window position t:%d e:%d",
                   __func__, username, t, e);
              rc = OTP_RC_OK;
              goto sync_done;
            } /* if (passcode is consecutive) */

            /* otherwise ... */
            if (fc == OTP_FC_FAIL_DELAY) {
              /* ... user is delayed, save as first of 2 */
              mlog(LOG_DEBUG1, "%s: [%s], rwindow candidate at t:%d e:%d",
                   __func__, username, t, e);
              rc = OTP_RC_NEXTPASSCODE;
              goto sync_done;
            } else {
              /* ... user has outwaited the delay, check range */
              if (e > (int) config->ewindow_size) {
                /* out of range */
                mlog(LOG_NOTICE, "%s: bad sync auth for [%s]: "
                                 "valid but in softfail (%d/%d failed/max)",
                     __func__, username, state.failcount, config->softfail);
                rc = OTP_RC_MAXTRIES;
                goto auth_done;
              } else {
                /* in range (normal sync mode auth) */
                rc = OTP_RC_OK;
                goto sync_done;
              }
            } /* else !delayed */

          } else {
            /* normal sync mode auth */
            rc = OTP_RC_OK;
          } /* else (!hardfail && !softfail) */

sync_done:
          /* force resync; this only has an effect if (rc == OTP_RC_OK) */
          resync = 1;
          /* update csd (et al.) on successful auth or rwindow candidate */
          if (user->cardops->updatecsd(&state, now, t, e, rc)) {
            mlog(LOG_ERR, "%s: unable to update csd for [%s]",
                 __func__, username);
            rc = OTP_RC_SERVICE_ERR;
            goto auth_done_service_err;
            /* NB: state not updated */
          }
          goto auth_done;

        } /* if (passcode is valid) */
      } /* for (each slot in the ewindow) */
    } /* for (each slot in the twindow) */
  } /* if (sync mode possible) */

  /* Both async and sync mode failed. */
  rc = OTP_RC_AUTH_ERR;

auth_done:
  if (rc == OTP_RC_OK) {
    if (resync)
      (void) memcpy(state.u.challenge, challenge, state.clen);
    (void) strcpy(reply->passcode, e_response);
    state.failcount = 0;
    state.authtime  = now;
    mlog(LOG_NOTICE, "%s: user [%s] authentication succeeded", __func__,
         username);
  } else {
    if (++state.failcount == UINT_MAX)
      state.failcount--;
    if (state.nullstate)	/* use negative authtime as sentinel */
      state.authtime = - (int32_t) now;	/* not 64-bit safe */
    else
      state.authtime = (int32_t) now;	/* not 64-bit safe */
    mlog(LOG_NOTICE, "%s: user [%s] authentication failed", __func__, username);
    /*
     * Note that we don't update the challenge.  Even for softfail (where
     * we might have actually had a good auth), we want the challenge
     * unchanged because we always start our sync loop at e=0 t=0 (and so
     * challenge must stay as the 0'th challenge regardless of next valid
     * window position, because the challenge() method can't return
     * arbitrary event window positions--since we start at e=0 the challenge
     * must be the 0th challenge, i.e. unchanged).
     */
  } /* else (rc != OTP_RC_OK) */
  state.updated = 1;

auth_done_service_err:	/* exit here for system errors */
  /*
   * Release and update state.
   *
   * We "fail-out" if we can't do this, because for sync mode the
   * response can be reused until state data is updated, an obvious
   * replay attack.
   *
   * For async mode with RADIUS, if we can't update the last auth
   * time, we will be open to a less obvious replay attack over the
   * lifetime of the State attribute (request->challenge_delay): if someone
   * that can see RADIUS traffic captures an Access-Request containing
   * a State attribute, and can cause the NAS to cycle the request id
   * within request->challenge_delay secs, then they can login to the NAS
   * and insert the captured State attribute into the new Access-Request,
   * and we'll give an Access-Accept.
   */
  if (state.locked) {
    /* give at least 250ms timeout for 1 network retrans */
    hrtime = xgethrtime() - hrlast;
    config->timeout -= hrtime;
    hrlast =+ hrtime;	/* just for consistency */
    if (config->timeout < 250000000LL)
      config->timeout = 250000000LL;
auth_done_service_err_timeout:
    if (state_put(config, user, &state) != 0) {
      mlog(LOG_ERR, "%s: unable to put state for [%s]", __func__, username);
      rc = OTP_RC_SERVICE_ERR;	/* no matter what it might have been */
    }
  }

  config->userops->put(user);

  reply->rc = rc;
  return rc;
}

/* decrypt helper for otppasswd key and pin fields */
static void
decrypt(unsigned char *data, size_t data_len,
        unsigned char *key, size_t key_len)
{
  size_t i, j;

  if (!key_len)
    return;

  for (i = 0, j = 0; i < data_len; ++i, ++j) {
    /* probably faster than mod */
    if (j == key_len)
      j = 0;
    data[i] ^= key[j];
  }
}

/* 
 * Compare the encoded passcode against the expected passcode.
 * Length of input (passcode,challenge,response) validated by caller.
 * Returns 0 on match, non-zero on non-match.
 */
static int
compare(const otp_request_t *request, const char *passcode)
{
  size_t plen = strlen(passcode);

  switch (request->pwe.pwe) {
  case PWE_PAP:
    return strcmp(request->pwe.u.pap.passcode, passcode);

  case PWE_CHAP:
  {
    /*
     * See RFC 1994.
     * A CHAP password is MD5(CHAP_ID|SECRET|CHAP_CHALLENGE).
     * CHAP_ID is a value set by the authenticator (the NAS), and used
     * in the response calculation.  It is available as the first byte
     * of the CHAP-Password attribute.
     * SECRET is the password.
     * CHAP_CHALLENGE is the challenge given to the peer (the user).
     * The CHAP-Challenge Attribute may be missing, in which case the
     * challenge is taken to be the Request Authenticator.  We don't
     * handle this case.
     */
    /*                 ID       password      chal */
    unsigned char input[1 + OTP_MAX_PASSCODE_LEN + 16];
    unsigned char output[MD5_DIGEST_LENGTH];

    input[0] = *(request->pwe.u.chap.response);
    (void) memcpy(&input[1], passcode, plen);
    (void) memcpy(&input[1+plen], request->pwe.u.chap.challenge,
                  request->pwe.u.chap.clen);
    (void) MD5(input, 1 + plen + request->pwe.u.chap.clen, output);
    return memcmp(output, &request->pwe.u.chap.response[1], MD5_DIGEST_LENGTH);
  }

  case PWE_MSCHAP:
  {
    /*
     * See RFCs 2548, 2433, 3079.
     * An MS-CHAP response is (IDENT|FLAGS|LM_RESPONSE|NT_RESPONSE).
     *                 octets:   1     1       24           24
     * IDENT is not used by RADIUS (it is the PPP MS-CHAP Identifier).
     * FLAGS is 1 to indicate the NT_RESPONSE should be preferred.
     * LM_RESPONSE is the LAN Manager compatible response.
     * NT_RESPONSE is the NT compatible response.
     * Either response may be zero-filled indicating its absence.
     * Use of the LM response has been deprecated (RFC 2433, par. 6),
     * so we don't handle it (validated by work_thread()).
     *
     * The NT_RESPONSE is (DES(CHAL,K1)|DES(CHAL,K2)|DES(CHAL,K3)), where
     * CHAL is the 8-octet challenge, and K1, K2, K3 are 7-octet pieces
     * of MD4(unicode(password)), zero-filled to 21 octets.  Sigh.
     */
    unsigned char nt_keys[21];			/* sized for 3 DES keys */
    unsigned char input[OTP_MAX_PASSCODE_LEN * 2]; /* doubled for unicode */
    unsigned char output[24];			/* just NT_RESPONSE     */
    unsigned i;

    /*
     * Start by hashing the unicode password into K1,K2,K3.
     * This is broken because unicode chars are machine-ordered,
     * but the spec (RFC 2433) doesn't say how to prepare
     * the password for md4 (other than by example values).
     */
    for (i = 0; i < plen; ++i) {
      /* set the high order 8 bits to 0 (little-endian) */
      input[i * 2] = passcode[i];
      input[i * 2 + 1] = 0;
    }
    (void) memset(nt_keys, 0, sizeof(nt_keys));
    (void) MD4(input, 2 * plen, nt_keys);

    /* The challenge gets encrypted. */
    (void) memcpy(input, request->pwe.u.chap.challenge, 8);

    /* convert the password hash to keys, and do the encryptions */
    for (i = 0; i < 3; ++i) {
      des_cblock key;
      des_key_schedule ks;

      key_from_hash(&key, &nt_keys[i * 7]);
      des_set_key_unchecked(&key, ks);
      des_ecb_encrypt((des_cblock *) input,
                      (des_cblock *) &output[i * 8],
                      ks, DES_ENCRYPT);
    }

    return memcmp(output, &request->pwe.u.chap.response[26], 24);
  } /* case PWE_MSCHAP */

  case PWE_MSCHAP2:
  {
    /*
     * See RFCs 2548, 2759, 3079.
     * An MS-CHAPv2 response is
     *          (IDENT|FLAGS|PEER_CHALLENGE|RESERVED|NT_RESPONSE).
     *   octets:   1     1         16          8        24
     * IDENT is the PPP MS-CHAPv2 Identifier, used in MS-CHAP2-Success.
     * FLAGS is currently unused.
     * PEER_CHALLENGE is a random number, generated by the peer.
     * NT_RESPONSE is (DES(CHAL,K1)|DES(CHAL,K2)|DES(CHAL,K3)), where
     * K1, K2, K3 are 7-octet pieces of MD4(unicode(password)), zero-
     * filled to 21 octets (just as in MS-CHAP); and CHAL is
     * MSB8(SHA(PEER_CHALLENGE|MS_CHAP_CHALLENGE|USERNAME)).
     */
    unsigned char nt_keys[21];			/* sized for 3 DES keys */
    unsigned char input[OTP_MAX_PASSCODE_LEN * 2]; /* doubled for unicode */
    unsigned char output[24];
    unsigned i;

    /*
     * Start by hashing the unicode password into K1,K2,K3.
     * This is broken because unicode chars are machine-ordered,
     * but the spec (RFC 2433) doesn't say how to prepare
     * the password for md4 (other than by example values).
     */
    for (i = 0; i < plen; ++i) {
      /* set the high order 8 bits to 0 (little-endian) */
      input[i * 2] = passcode[i];
      input[i * 2 + 1] = 0;
    }
    (void) memset(nt_keys, 0, sizeof(nt_keys));
    (void) MD4(input, 2 * plen, nt_keys);

    /* now calculate the CHAL value from our various inputs */
    {
      SHA_CTX ctx;
      unsigned char md[SHA_DIGEST_LENGTH];

      SHA1_Init(&ctx);
      SHA1_Update(&ctx, &request->pwe.u.chap.response[2], 16);
      SHA1_Update(&ctx, request->pwe.u.chap.challenge, 16);
      SHA1_Update(&ctx, request->username, strlen(request->username));
      SHA1_Final(md, &ctx);

      (void) memcpy(input, md, 8);
    }

    /* convert the password hash to keys, and do the encryptions */
    for (i = 0; i < 3; ++i) {
      des_cblock key;
      des_key_schedule ks;

      key_from_hash(&key, &nt_keys[i * 7]);
      des_set_key_unchecked(&key, ks);
      des_ecb_encrypt((des_cblock *) input,
                      (des_cblock *) &output[i * 8],
                      ks, DES_ENCRYPT);
    }

    /* NOTE: plugin must provide MS-CHAPv2 mutual auth */
    return memcmp(output, &request->pwe.u.chap.response[26], 24);
  } /* case PWE_MSCHAP2 */
  } /* switch (request->pwe.pwe) */

  /*NOTREACHED*/
  return -1;
}

/*
 * #$!#@ have to convert 7 octet ranges into 8 octet keys.
 * Implementation cribbed (and slightly modified) from
 * FreeRADIUS rlm_mschap.c by Jay Miller <jaymiller@socket.net>.
 * We don't bother checking/setting parity.
 */
static void
key_from_hash(des_cblock *key, const unsigned char hashbytes[7])
{
  int i;
  unsigned char working;
  unsigned char next = 0;

  for (i = 0; i < 7; ++i) {
    working = hashbytes[i];
    (*key)[i] = (working >> i) | next;
    next = (working << (7 - i));
  }
  (*key)[i] = next;
}

/* initialize cardops modules */
void
cardops_init(void)
{
#if 0
  void (*init)(void);

  /*
   * In the Cyclades ACS environment (2.3.0 tested), the runtime linker
   * apparently does not run static constructors in ELF .ctors sections.
   * Since that is how we initialize cardops modules, we have an ugly
   * workaround here.  Our other choice is to implement cardops modules
   * as full-fledged shared libraries, which is just too much work.
   */
  if (ncardops == 0) {
    /* ctors did not run; execute all known constructors */
    if ((init = dlsym(RTLD_DEFAULT, "cryptocard_init")))
      (*init)();
    if ((init = dlsym(RTLD_DEFAULT, "hotp_init")))
      (*init)();
    if ((init = dlsym(RTLD_DEFAULT, "trid_init")))
      (*init)();
    if ((init = dlsym(RTLD_DEFAULT, "x99_init")))
      (*init)();
    /* ctors increment ncardops */
  }
#endif
}
