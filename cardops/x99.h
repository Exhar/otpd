/*
 * $Id: //trid/release/otpd-3.1.0/cardops/x99.h#1 $
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

#ifndef CARDOPS_X99_H
#define CARDOPS_X99_H

#include "../ident.h"
RCSIDH(cardops_x99_h, "$Id: //trid/release/otpd-3.1.0/cardops/x99.h#1 $")

#include <inttypes.h>
#include <sys/types.h>

#include "../extern.h"
#include "../otp.h"
#include "../cardops.h"

/* card modes */
#define X99_CF_HD OTP_CF_VSA0	/* hex display              */
#define X99_CF_DD OTP_CF_VSA1	/* dec display              */
#define X99_CF_PH OTP_CF_VSA2	/* [7 digit] phone response */

#define X99_CF_LEN7 (OTP_CF_LEN2|OTP_CF_LEN1|OTP_CF_LEN0)
#define X99_CF_LEN8 OTP_CF_LEN3

#define X99_H8 (X99_CF_HD|X99_CF_LEN8|OTP_CF_AM)
#define X99_H7 (X99_CF_HD|X99_CF_LEN7|OTP_CF_AM)
#define X99_HP (X99_CF_HD|X99_CF_LEN8|X99_CF_PH|OTP_CF_AM)
#define X99_D8 (X99_CF_DD|X99_CF_LEN8|OTP_CF_AM)
#define X99_D7 (X99_CF_DD|X99_CF_LEN7|OTP_CF_AM)
#define X99_DP (X99_CF_DD|X99_CF_LEN8|X99_CF_PH|OTP_CF_AM)

static int x99_name2fm(user_t *, state_t *);
static int x99_keystring2key(user_t *, state_t *);
static int x99_nullstate(const config_t *, const user_t *, state_t *, time_t);
static int x99_challenge(const user_t *, state_t *,
                         unsigned char [OTP_MAX_CHALLENGE_LEN], time_t,
                         int, int);
static int x99_response(const user_t *, state_t *,
                        const unsigned char [OTP_MAX_CHALLENGE_LEN],
                        size_t, char [OTP_MAX_RESPONSE_LEN + 1]);
static int x99_updatecsd(state_t *, time_t, int, int, int);
static int x99_isconsecutive(const user_t *, state_t *, int);
static int x99_maxtwin(const user_t *, state_t *, time_t);
static char *x99_printchallenge(char [OTP_MAX_CHALLENGE_LEN * 2 + 1],
                                const unsigned char [OTP_MAX_CHALLENGE_LEN],
                                size_t);

#if defined(__GNUC__)
__attribute__ ((constructor))
#elif defined(__SUNPRO_C)
#pragma init(x99_init)
#endif
void x99_init(void);

#endif /* CARDOPS_X99_H */
