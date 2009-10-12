/*
 * $Id: //trid/release/otpd-3.1.0/site.c#1 $
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

/*
 * IMPORTANT  IMPORTANT  IMPORTANT  IMPORTANT  IMPORTANT  IMPORTANT
 *
 * ANSI X9.9 has been withdrawn as a standard, due to the weakness
 * of DES.  An attacker can learn the token's secret by observing
 * two challenge/response pairs.  See ANSI document X9 TG-24-1999
 * <URL:http://www.x9.org/docs/TG24_1999.pdf>.
 *
 * This is not an issue for synchronous mode, where the challenge
 * is not presented.  (If the challenge is presented, e.g. for
 * user verification, but not entered, this is still a problem.
 * TRI-D plugins that use otpd do not work this way; when they do
 * present a challenge it is not the synchronous challenge.)
 *
 * But to use async (challenge/response) mode, we need to avoid
 * exposure of the challenge.  So we've implemented a site-local
 * transform feature.  This alters the presented challenge before
 * response generation; the user must make the same transformation
 * when entering the challenge into the token.  Thus, an attacker
 * does not have access to the actual challenge.
 *
 * When implementing a transform, note that trivial transforms
 * (say, append '0') are trivially attacked.  OTOH, the transform
 * must be easy enough for a user to perform quickly, and probably
 * mentally.
 *
 * Yes, this is security by obscurity.  No, it is not "secure".
 * But it is probably good enough; most attackers are just not
 * that sophisticated.  Certainly, it's better than doing nothing.
 * 
 * This only affects X9.9 tokens.  If your token vendor will not
 * reveal their algorithm to you, you should avoid doing business
 * with them.
 *
 * Instead of, or in addition to, the transform, you could restrict
 * async auth to secure terminals.
 *
 * IMPORTANT  IMPORTANT  IMPORTANT  IMPORTANT  IMPORTANT  IMPORTANT
 */

#include "ident.h"
RCSID("$Id: //trid/release/otpd-3.1.0/site.c#1 $")

#include <sys/types.h>
#include "otp.h"	/* OTP_MAX_CHALLENGE_LEN */

/*
 * The default transform appends the first 2 username chars to the
 * challenge.  This results in a challenge that generally cannot be
 * entered on any supported token, thus forcing a site-specific
 * implementation to support async mode.
 *
 * NOTE: challenge is not guaranteed to be terminated.  The returned
 * length must not include a termination byte.
 */
ssize_t
challenge_transform(const char *username,
                    unsigned char challenge[OTP_MAX_CHALLENGE_LEN],
                    size_t clen)
{
  /* overwrite challenge in-place if not enough room */
  switch (OTP_MAX_CHALLENGE_LEN - clen) {
    case 0: clen -= 2; break;
    case 1: clen -= 1; break;
  }

  /* append first 2 username chars to challenge */
  if (*username)
    challenge[clen++] = *username++;
  if (*username)
    challenge[clen++] = *username++;

  return clen;
}
