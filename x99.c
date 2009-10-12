/*
 * $Id: //trid/release/otpd-3.1.0/x99.c#1 $
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
 * Copyright 2001,2002  Google, Inc.
 * Copyright 2005,2007 TRI-D Systems, Inc.
 */

#include "ident.h"
RCSID("$Id: //trid/release/otpd-3.1.0/x99.c#1 $")

#include <string.h>
#include <sys/types.h>
#include <openssl/des.h>

#include "extern.h"
#include "otp.h"	/* OTP_MAX_CHALLENGE_LEN */

/*
 * The ANSI X9.9 MAC algorithm is:
 * 1. Perform a CBC mode DES encryption of the plaintext.  The last plaintext
 *    block must be zero padded.
 * 2. The MAC is the most significant 32 bits of the last cipherblock.
 *
 * Most tokens support a max of an 8 character challenge, but at least one
 * (CRYPTOCard RB-1) supports performing the full CBC mode encryption
 * of an arbitrary length challenge.  So we don't limit ourselves
 * to just an ECB mode encryption.
 *
 * This routine returns the entire 64 bit last cipherblock, at least one sync
 * mode needs this (and ANSI X9.9 states that the MAC can be 48, and 64 bit
 * MACs should be supported).  Returns 0 on success, non-zero otherwise.
 */
int
x99_mac(const unsigned char *input, size_t len, unsigned char output[8],
        const unsigned char *key)
{
  des_key_schedule ks;
  des_cblock ivec;
  des_cblock l_output[OTP_MAX_CHALLENGE_LEN / sizeof(des_cblock)];
  int rc;

  /*
   * Setup and verify the key.
   * This may be a bit expensive to do every time, but it
   * makes more sense for calling functions to deal with
   * the key itself, rather than the schedule.  In practice,
   * I don't think this will amount to much, but I haven't
   * actually profiled it.
   * TODO: store in user_t after generating
   */
  if ((rc = des_set_key_checked((const_des_cblock *) key, ks)) != 0) {
    mlog(LOG_ERR, "%s: DES key %s",
         __func__, rc == -1 ? "has incorrect parity" : "is weak");
    return -1;
  }

  (void) memset(ivec, 0, sizeof(ivec));
  des_cbc_encrypt(input, (unsigned char *) l_output, len,
                  ks, &ivec, DES_ENCRYPT);
  (void) memcpy(output, l_output[(len - 1) / 8], 8);
  return 0;
}
