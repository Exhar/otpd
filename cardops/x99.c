/*
 * $Id: //trid/release/otpd-3.1.0/cardops/x99.c#1 $
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
RCSID("$Id: //trid/release/otpd-3.1.0/cardops/x99.c#1 $")

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "../extern.h"
#include "../otp.h"
#include "../cardops.h"
#include "x99.h"

/* Card name to feature mask mappings */
static struct {
  const char *name;
  uint32_t fm;
} card[] = {
  { "x99-h8", X99_H8 },	/* 8 digit hex response */
  { "x99-h7", X99_H7 },	/* 7 digit hex response */
  { "x99-d8", X99_D8 },	/* 8 digit dec response */
  { "x99-d7", X99_D7 },	/* 7 digit dec response */

  { NULL, 0 }		/* end of list */
};

/*
 * Convert card name to feature mask.
 * Returns 0 on success, non-zero otherwise.
 */
static int
x99_name2fm(user_t *user, __unused__ state_t *state)
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
x99_keystring2key(user_t *user, __unused__ state_t *state)
{
  /* 64-bit DES key */
  if (strlen(user->keystring) != 16)
    return -1;

  return a2nx(user->keystring, user->key, 8);
}

/*
 * We don't support sync mode, so we don't need state.
 */
static int
x99_nullstate(__unused__ const config_t *config, __unused__ const user_t *user,
              __unused__ state_t *state, __unused__ time_t when)
{
  return 0;
}

/*
 * Return a synchronous challenge.
 */
static int
x99_challenge(__unused__ const user_t *user, __unused__ state_t *state,
              __unused__ unsigned char challenge[OTP_MAX_CHALLENGE_LEN],
              __unused__ time_t when, __unused__ int twin, __unused__ int ewin)
{
  return -1;
}

/*
 * Return the expected card response for a given challenge.
 * Returns 0 on success, non-zero otherwise.
 *
 * The X9.9 MAC is generated identically to CRYPTOCard:
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
x99_response(const user_t *user, state_t *state,
             const unsigned char challenge[OTP_MAX_CHALLENGE_LEN],
             __unused__ size_t len, char response[OTP_MAX_RESPONSE_LEN + 1])
{
  unsigned char output[8];
  const char *conversion;

  /* Step 1, 2. */
  if (x99_mac(challenge, len, output, user->key) !=0)
    return 1;

  /* Setup for step 4. */
  if (user->featuremask & X99_CF_DD)
    conversion = x2a_cc_dec_conversion;
  else
    conversion = x2a_hex_conversion;

  /* Step 3, 4. */
  (void) x2a(output, 4, response, conversion);

  /* Step 5. */
  if ((user->featuremask & OTP_CF_LEN) >> OTP_CF_LEN_SHIFT == 7)
    (void) memmove(&response[3], &response[4], 5);
  else if (user->featuremask & X99_CF_PH)
    response[3] = '-';

  return 0;
}

/*
 * No sync mode, so no csd or rd.
 * This should never be called, so return -1.
 */
static int
x99_updatecsd(__unused__ state_t *state, __unused__ time_t when,
              __unused__ int twin, __unused__ int ewin, __unused__ int auth_rc)
{
  return -1;
}

/*
 * Determine if a window position if consecutive relative to a saved
 * (rwindow candidate) window position, for rwindow override.
 * Since we don't have a sync mode, this is never called.
 * Just return 0 (not consecutive).
 */
static int
x99_isconsecutive(__unused__ const user_t *user, __unused__ state_t *state,
                  __unused__ int thisewin)
{
  return 0;
}

/* no twin so just return 0 */
static int
x99_maxtwin(__unused__ const user_t *user, __unused__ state_t *state,
            __unused__ time_t when)
{
  return 0;
}

/* return human-readable challenge */
static char *
x99_printchallenge(char s[OTP_MAX_CHALLENGE_LEN * 2 + 1],
                   const unsigned char challenge[OTP_MAX_CHALLENGE_LEN],
                   size_t len)
{
  return x2a(challenge, len, s, x2a_hex_conversion);
}

/* cardops instance */
static cardops_t x99_cardops = {
  .prefix		= "x99",
  .plen			= 3, /* strlen("x99") */

  .name2fm		= x99_name2fm,
  .keystring2key	= x99_keystring2key,
  .nullstate		= x99_nullstate,
  .challenge		= x99_challenge,
  .response		= x99_response,
  .updatecsd		= x99_updatecsd,
  .isconsecutive	= x99_isconsecutive,
  .maxtwin		= x99_maxtwin,
  .printchallenge	= x99_printchallenge,
};

/* constructor */
void
x99_init(void)
{
  if (ncardops == OTP_MAX_VENDORS) {
    mlog(LOG_CRIT, "x99_init: module limit exceeded");
    exit(1);
  }

  cardops[ncardops++] = x99_cardops;
}
