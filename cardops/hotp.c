/*
 * $Id: //trid/release/otpd-3.1.0/cardops/hotp.c#1 $
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
 * Copyright 2006,2007 TRI-D Systems, Inc.
 */

#include "../ident.h"
RCSID("$Id: //trid/release/otpd-3.1.0/cardops/hotp.c#1 $")

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "../extern.h"
#include "../otp.h"
#include "../cardops.h"
#include "hotp.h"

/* Card name to feature mask mappings */
static struct {
  const char *name;
  uint32_t fm;
} card[] = {
  { "hotp-d5", HOTP5 },	/* 5 digit response */
  { "hotp-d6", HOTP6 },	/* 6 digit response */
  { "hotp-d7", HOTP7 },	/* 7 digit response */
  { "hotp-d8", HOTP8 },	/* 8 digit response */
  { "hotp-d9", HOTP9 },	/* 9 digit response */

  { NULL, 0 }		/* end of list */
};

/*
 * Convert card name to feature mask.
 * Returns 0 on success, non-zero otherwise.
 */
static int
hotp_name2fm(user_t *user, __unused__ state_t *state)
{
  int i;

  for (i = 0; card[i].name; ++i) {
    if (!strcasecmp(user->card, card[i].name)) {
      user->featuremask = card[i].fm;
      return 0;
    }
  }
  return 1;
}

/*
 * Convert an ASCII keystring to a key.
 * Returns keylen on success, -1 otherwise.
 */
static int
hotp_keystring2key(user_t *user, state_t *state)
{
  size_t l = strlen(user->keystring);

  /* 128-bit or 160-bit key */
  if (l == 32) {
    state->scratch1 = 16;	/* save keylen for hotp_response() */
  } else if (l == 40) {
    state->scratch1 = 20;	/* save keylen for hotp_response() */
  } else {
    return -1;
  }

  return a2nx(user->keystring, user->key, state->scratch1);
}

/*
 * We don't support nullstate, so return -1.
 */
static int
hotp_nullstate(__unused__ const config_t *config,__unused__ const user_t *user,
               __unused__ state_t *state, __unused__ time_t when)
{
  mlog(LOG_ERR, "%s: null state not supported for hotp", __func__);
  return -1;
}

/*
 * Return a synchronous challenge.
 * Returns 0 on success, -1 otherwise.
 * (-2 rc is for early challenge, N/A for hotp.)
 */
static int
hotp_challenge(const user_t *user, state_t *state,
               unsigned char challenge[OTP_MAX_CHALLENGE_LEN],
               __unused__ time_t when, __unused__ int twin,__unused__ int ewin)
{
  uint64_t counter;	/* 8 bytes per RFC 4226 */

  /* convert the previous challenge from network to host order */
  counter = (challenge[0] << 24) |
            (challenge[1] << 16) |
            (challenge[2] << 8)  |
             challenge[3];
  counter <<= 32;
  counter |= (challenge[4] << 24) |
             (challenge[5] << 16) |
             (challenge[6] << 8)  |
              challenge[7];

  /* just increment +1 */
  counter++;

  /* convert back to network order */
  challenge[0] = (counter >> 56) & 0xff;
  challenge[1] = (counter >> 48) & 0xff;
  challenge[2] = (counter >> 40) & 0xff;
  challenge[3] = (counter >> 32) & 0xff;
  challenge[4] = (counter >> 24) & 0xff;
  challenge[5] = (counter >> 16) & 0xff;
  challenge[6] = (counter >> 8) & 0xff;
  challenge[7] = counter & 0xff;

  state->clen = 8;
  return 0;
}

/*
 * Return the expected card response for a given challenge.
 * Returns 0 on success, non-zero otherwise.
 */
static int
hotp_response(const user_t *user, state_t *state,
              const unsigned char challenge[OTP_MAX_CHALLENGE_LEN],
              __unused__ size_t len, char response[OTP_MAX_RESPONSE_LEN + 1])
{
  return hotp_mac(challenge, response, user->key, state->scratch1,
                  (user->featuremask & OTP_CF_LEN) >> OTP_CF_LEN_SHIFT);
}

/*
 * Update rd (there is no csd for hotp).
 * Returns 0 if succesful, -1 otherwise.
 */
static int
hotp_updatecsd(state_t *state, __unused__ time_t when,
               __unused__ int twin, int ewin, int auth_rc)
{
  if (auth_rc == OTP_RC_OK)
    state->rd[0] = '\0';				/* reset */
  else
    (void) sprintf(state->rd, "%" PRIx32,
                   (int32_t) ewin);			/* rwindow candidate */

  return 0;
}

/*
 * Determine if a window position if consecutive relative to a saved
 * (rwindow candidate) window position, for rwindow override.
 * state contains the previous auth position, twin and ewin the current.
 * Returns 1 on success (consecutive), 0 otherwise.
 */
static int
hotp_isconsecutive(const user_t *user, state_t *state, int thisewin)
{
  int nextewin;

  /* successful auth clears rd */
  if (state->rd[0] == '\0')
    return 0;

  /* extract the saved rwindow candidate position */
  if (sscanf(state->rd, "%" SCNx32, &nextewin) != 1) {
    mlog(LOG_ERR, "%s: invalid rwindow data for [%s]",
         __func__, user->username);
    return 0;
  }
  nextewin++;

  /* Is this the next passcode? */
  if (thisewin == nextewin)
    return 1;	/* yes */
  else
    return 0;	/* no */
}

/* no twin so just return 0 */
static int
hotp_maxtwin(__unused__ const user_t *user, __unused__ state_t *state,
             __unused__ time_t when)
{
  return 0;
}

/* return human-readable challenge */
static char *
hotp_printchallenge(char s[OTP_MAX_CHALLENGE_LEN * 2 + 1],
                    const unsigned char challenge[OTP_MAX_CHALLENGE_LEN],
                    size_t len)
{
  return x2a(challenge, len, s, x2a_hex_conversion);
}

/* cardops instance */
static cardops_t hotp_cardops = {
  .prefix		= "hotp",
  .plen			= 4, /* strlen("hotp") */

  .name2fm		= hotp_name2fm,
  .keystring2key	= hotp_keystring2key,
  .nullstate		= hotp_nullstate,
  .challenge		= hotp_challenge,
  .response		= hotp_response,
  .updatecsd		= hotp_updatecsd,
  .isconsecutive	= hotp_isconsecutive,
  .maxtwin		= hotp_maxtwin,
  .printchallenge	= hotp_printchallenge,
};

/* constructor */
void
hotp_init(void)
{
  if (ncardops == OTP_MAX_VENDORS) {
    mlog(LOG_CRIT, "hotp_init: module limit exceeded");
    exit(1);
  }

  cardops[ncardops++] = hotp_cardops;
}
