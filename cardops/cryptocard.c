/*
 * $Id: //trid/release/otpd-3.1.0/cardops/cryptocard.c#1 $
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
 * Copyright 2005-2007 TRI-D Systems, Inc.
 */

#include "../ident.h"
RCSID("$Id: //trid/release/otpd-3.1.0/cardops/cryptocard.c#1 $")

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "../extern.h"
#include "../otp.h"
#include "../cardops.h"
#include "cryptocard.h"

/* Card name to feature mask mappings */
static struct {
  const char *name;
  uint32_t fm;
} card[] = {
  { "cryptocard-h8-rc", CRYPTOCARD_H8_RC },
  { "cryptocard-d8-rc", CRYPTOCARD_D8_RC },
  { "cryptocard-h7-rc", CRYPTOCARD_H7_RC },
  { "cryptocard-d7-rc", CRYPTOCARD_D7_RC },
  { "cryptocard-hp-rc", CRYPTOCARD_HP_RC },
  { "cryptocard-dp-rc", CRYPTOCARD_DP_RC },
  { "cryptocard-h8-es", CRYPTOCARD_H8_ES },
  { "cryptocard-d8-es", CRYPTOCARD_D8_ES },
  { "cryptocard-h7-es", CRYPTOCARD_H7_ES },
  { "cryptocard-d7-es", CRYPTOCARD_D7_ES },
  { "cryptocard-hp-es", CRYPTOCARD_HP_ES },
  { "cryptocard-dp-es", CRYPTOCARD_DP_ES },
  { "cryptocard-h8-rs", CRYPTOCARD_H8_RS },
  { "cryptocard-d8-rs", CRYPTOCARD_D8_RS },
  { "cryptocard-h7-rs", CRYPTOCARD_H7_RS },
  { "cryptocard-d7-rs", CRYPTOCARD_D7_RS },
  { "cryptocard-hp-rs", CRYPTOCARD_HP_RS },
  { "cryptocard-dp-rs", CRYPTOCARD_DP_RS },

  { NULL, 0 }					/* end of list */
};


/*
 * Convert card name to feature mask.
 * Returns 0 on success, non-zero otherwise.
 */
static int
cryptocard_name2fm(user_t *user, __unused__ state_t *state)
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
cryptocard_keystring2key(user_t *user, __unused__ state_t *state)
{
  /* 64-bit DES key */
  if (strlen(user->keystring) != 16)
    return -1;

  return a2nx(user->keystring, user->key, 8);
}

/*
 * Set nullstate.
 * We don't currently support nullstate for CRYPTOCard, so return -1.
 * It actually wouldn't be hard to implement -- left as an exercise
 * to the reader.
 */
static int
cryptocard_nullstate(__unused__ const config_t *config,
                     __unused__ const user_t *user, __unused__ state_t *state, 
                     __unused__ time_t when)
{
  mlog(LOG_ERR, "%s: null state not supported for CRYPTOCard", __func__);
  return -1;
}

/*
 * Return a synchronous challenge.
 * Returns 0 on success, -1 otherwise.
 * (-2 rc is for early challenge, N/A for cryptocard.)
 */
static int
cryptocard_challenge(const user_t *user, state_t *state,
                     unsigned char challenge[OTP_MAX_CHALLENGE_LEN],
                     __unused__ time_t when, __unused__ int twin,
                     __unused__ int ewin)
{
  unsigned char output[8];
  int i;

  /* run x99 once on the previous challenge */
  if (x99_mac(challenge, state->clen, output, user->key))
    return -1;

  /* convert the mac into the next challenge */
  for (i = 0; i < 8; ++i) {
    output[i] &= 0x0f;
    if (output[i] > 9)
      output[i] -= 10;
    output[i] |= 0x30;
  }
  (void) memcpy(challenge, output, 8);
  state->clen = 8;

  return 0;
}

/*
 * Return the expected card response for a given challenge.
 * Returns 0 on success, non-zero otherwise.
 *
 * The X9.9 MAC is used by CRYPTOCard in the following manner:
 *
 * 1. Convert the challenge to ASCII (eg "12345" -> 0x3132333435).
 *    We don't actually do a conversion, the challenge is already ASCII.
 *    Note that Secure Computing SafeWord Gold/Platinum tokens can use
 *    "raw" challenge bytes.
 * 2. Use the challenge as the plaintext input to the X9.9 MAC algorithm.
 *
 * 3. Convert the 32 bit MAC to ASCII (eg 0x1234567f -> "1234567f").
 *    Note that SafeWord Gold/Platinum tokens can display a 64 bit MAC.
 * 4. Possibly apply transformations on chars "a" thru "f".
 * 5. Truncate the response for 7 digit display modes.
 */
static int
cryptocard_response(const user_t *user, __unused__ state_t *state,
                    const unsigned char challenge[OTP_MAX_CHALLENGE_LEN],
                    size_t len, char response[OTP_MAX_RESPONSE_LEN + 1])
{
  unsigned char output[8];
  const char *conversion;

  /* Step 1, 2. */
  if (x99_mac(challenge, len, output, user->key))
    return 1;

  /* Setup for step 4. */
  if (user->featuremask & CRYPTOCARD_CF_DD)
    conversion = x2a_cc_dec_conversion;
  else
    conversion = x2a_hex_conversion;

  /* Step 3, 4. */
  (void) x2a(output, 4, response, conversion);

  /* Step 5. */
  if ((user->featuremask & OTP_CF_LEN) >> OTP_CF_LEN_SHIFT == 7)
    (void) memmove(&response[3], &response[4], 5);
  else if (user->featuremask & CRYPTOCARD_CF_PH)
    response[3] = '-';

  return 0;
}

/*
 * Update rd (there is no csd for cryptocard).
 * Returns 0 if succesful, -1 otherwise.
 */
static int
cryptocard_updatecsd(state_t *state, __unused__ time_t when,
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
 * user_state contains the previous auth position, twin and ewin the current.
 * Returns 1 on success (consecutive), 0 otherwise.
 */
static int
cryptocard_isconsecutive(const user_t *user, state_t *state, int thisewin)
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
cryptocard_maxtwin(__unused__ const user_t *user, __unused__ state_t *state,
                   __unused__ time_t when)
{
  return 0;
}

/* return human-readable challenge */
static char *
cryptocard_printchallenge(char s[OTP_MAX_CHALLENGE_LEN * 2 + 1],
                          const unsigned char challenge[OTP_MAX_CHALLENGE_LEN],
                          size_t len)
{
  /* cryptocard challenge is implicitly ASCII */
  (void) memcpy(s, challenge, len);
  s[len] = '\0';
  return s;
}

/* cardops instance */
static cardops_t cryptocard_cardops = {
  .prefix		= "cryptocard",
  .plen			= 10, /* strlen("cryptocard") */

  .name2fm		= cryptocard_name2fm,
  .keystring2key	= cryptocard_keystring2key,
  .nullstate		= cryptocard_nullstate,
  .challenge		= cryptocard_challenge,
  .response		= cryptocard_response,
  .updatecsd		= cryptocard_updatecsd,
  .isconsecutive	= cryptocard_isconsecutive,
  .maxtwin		= cryptocard_maxtwin,
  .printchallenge	= cryptocard_printchallenge,
};

/* constructor */
void
cryptocard_init(void)
{
  if (ncardops == OTP_MAX_VENDORS) {
    mlog(LOG_CRIT, "cryptocard_init: module limit exceeded");
    exit(1);
  }

  cardops[ncardops++] = cryptocard_cardops;
}
