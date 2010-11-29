/*
 *  c200.c 
 *
 *  A TOTP implementation for the Feitian c200, that is not fully
 *  compliant with the standard. 
 *  Generation time is 60 seconds and time is rounded to the next minute.
 *
 *  This file has been generated from the cardops/hotp.c, originally created
 *  by TRI-D Systems
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
 *
 * Copyright 2010 Giuseppe Paterno' (gpaterno@gpaterno.com)
 * Copyright 2006,2007 TRI-D Systems, Inc.
 */

#include "../ident.h"

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>

#include "../extern.h"
#include "../otp.h"
#include "../cardops.h"
#include "c200.h"

/* Card name to feature mask mappings */
static struct {
  const char *name;
  uint32_t fm;
} card[] = {
  { "c200-d6", HOTP6 },	/* 6 digit response */
  { "c200-d7", HOTP7 },	/* 7 digit response */
  { "c200-d8", HOTP8 },	/* 8 digit response */

  { NULL, 0 }		/* end of list */
};

static void
c2c(uint64_t counter, unsigned char challenge[8])
{
  challenge[0] = counter >> 56;
  challenge[1] = counter >> 48;
  challenge[2] = counter >> 40;
  challenge[3] = counter >> 32;
  challenge[4] = counter >> 24;
  challenge[5] = counter >> 16;
  challenge[6] = counter >> 8;
  challenge[7] = counter;
}


/*
 * Convert card name to feature mask.
 * Returns 0 on success, non-zero otherwise.
 */
static int
c200_name2fm(user_t *user, __unused__ state_t *state)
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
c200_keystring2key(user_t *user, state_t *state)
{
  size_t l = strlen(user->keystring);

  /* min 128-bit key */
  if (l < 32)
    return -1;

  state->scratch1 = l/2;	/* save keylen for hotp_response() */
  return a2nx(user->keystring, user->key, state->scratch1);
}

/*
 * Who cares about nullstate? We use time :)
 */
static int
c200_nullstate(__unused__ const config_t *config,__unused__ const user_t *user,
               __unused__ state_t *state, __unused__ time_t when)
{
  return 0;
}

/*
 * Return a synchronous challenge.
 * Returns 0 on success, -1 otherwise.
 * (we return -1)
 */
static int
c200_challenge(const user_t *user, state_t *state,
               unsigned char challenge[OTP_MAX_CHALLENGE_LEN],
               __unused__ time_t when, __unused__ int twin,__unused__ int ewin)
{
  return 0;
}

/*
 * Return the expected card response for a given challenge.
 * Returns 0 on success, non-zero otherwise.
 */
static int
c200_response(const user_t *user, state_t *state,
              __unused__ const unsigned char challenge[OTP_MAX_CHALLENGE_LEN],
              __unused__ size_t len, char response[OTP_MAX_RESPONSE_LEN + 1])
{
  time_t t;
  long t_rounding;
  uint64_t iv;
  unsigned char mychallenge[8];

  /* This is actually a bug of the Feitian c200 implementation
   * Get the time in seconds from unix birth,
   * then get the difference in seconds to the next available minute (60 seconds)
   * using a modulus and add it to the standard seconds (t + t_rounding)
   * At the end, divide by the time step (c200 is 60 seconds).
   */

  t = time(NULL);
  t_rounding = 60 - (t % 60);
  iv =  ((int) (t + t_rounding)) / TIME_STEP;

  mlog(LOG_DEBUG1, "c200: iv is set to %llu", iv);

  c2c(iv, mychallenge);
  
  return hotp_mac(mychallenge, response, user->key, state->scratch1,
                  (user->featuremask & OTP_CF_LEN) >> OTP_CF_LEN_SHIFT);
}

/*
 * No sync mode, it's time based ;-)
 * Returns 0, let it seems it's ok (fake response)
 */
static int
c200_updatecsd(state_t *state, __unused__ time_t when,
               __unused__ int twin, int ewin, int auth_rc)
{
  return 0;
}

/*
 * Determine if a window position if consecutive relative to a saved
 * (rwindow candidate) window position, for rwindow override.
 * Since we don't have a sync mode, this is never called.
 * Just return 0 (not consecutive).
 */
static int
c200_isconsecutive(const user_t *user, state_t *state, int thisewin)
{
  return 1;	
}

/* no twin so just return 0 */
static int
c200_maxtwin(__unused__ const user_t *user, __unused__ state_t *state,
             __unused__ time_t when)
{
  return 0;
}

/* return human-readable challenge */
static char *
c200_printchallenge(char s[OTP_MAX_CHALLENGE_LEN * 2 + 1],
                    const unsigned char challenge[OTP_MAX_CHALLENGE_LEN],
                    size_t len)
{
  return x2a(challenge, len, s, x2a_hex_conversion);
}

/* cardops instance */
static cardops_t c200_cardops = {
  .prefix		= "c200",
  .plen			= 4, /* strlen("hotp") */

  .name2fm		= c200_name2fm,
  .keystring2key	= c200_keystring2key,
  .nullstate		= c200_nullstate,
  .challenge		= c200_challenge,
  .response		= c200_response,
  .updatecsd		= c200_updatecsd,
  .isconsecutive	= c200_isconsecutive,
  .maxtwin		= c200_maxtwin,
  .printchallenge	= c200_printchallenge,
};

/* constructor */
void
c200_init(void)
{
  if (ncardops == OTP_MAX_VENDORS) {
    mlog(LOG_CRIT, "c200_init: module limit exceeded");
    exit(1);
  }

  cardops[ncardops++] = c200_cardops;
}
