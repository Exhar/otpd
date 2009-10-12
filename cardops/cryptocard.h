/*
 * $Id: //trid/release/otpd-3.1.0/cardops/cryptocard.h#1 $
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

#ifndef CARDOPS_CRYPTOCARD_H
#define CARDOPS_CRYPTOCARD_H

#include "../ident.h"
RCSIDH(cardops_cryptocard_h, "$Id: //trid/release/otpd-3.1.0/cardops/cryptocard.h#1 $")

#include <inttypes.h>
#include <sys/types.h>

#include "../extern.h"
#include "../otp.h"
#include "../cardops.h"

/* card modes */
#define CRYPTOCARD_CF_HD OTP_CF_VSA0	/* hex display              */
#define CRYPTOCARD_CF_DD OTP_CF_VSA1	/* dec display              */
#define CRYPTOCARD_CF_PH OTP_CF_VSA2	/* [7 digit] phone response */

#define CRYPTOCARD_CF_LEN7 (OTP_CF_LEN2|OTP_CF_LEN1|OTP_CF_LEN0)
#define CRYPTOCARD_CF_LEN8 OTP_CF_LEN3

#define CRYPTOCARD_H8_RC (CRYPTOCARD_CF_HD|CRYPTOCARD_CF_LEN8|OTP_CF_AM)
#define CRYPTOCARD_H7_RC (CRYPTOCARD_CF_HD|CRYPTOCARD_CF_LEN7|OTP_CF_AM)
#define CRYPTOCARD_HP_RC (CRYPTOCARD_CF_HD|CRYPTOCARD_CF_LEN8|CRYPTOCARD_CF_PH|OTP_CF_AM)
#define CRYPTOCARD_D8_RC (CRYPTOCARD_CF_DD|CRYPTOCARD_CF_LEN8|OTP_CF_AM)
#define CRYPTOCARD_D7_RC (CRYPTOCARD_CF_DD|CRYPTOCARD_CF_LEN7|OTP_CF_AM)
#define CRYPTOCARD_DP_RC (CRYPTOCARD_CF_DD|CRYPTOCARD_CF_LEN8|CRYPTOCARD_CF_PH|OTP_CF_AM)
#define CRYPTOCARD_H8_ES (CRYPTOCARD_CF_HD|CRYPTOCARD_CF_LEN8|OTP_CF_ES)
#define CRYPTOCARD_H7_ES (CRYPTOCARD_CF_HD|CRYPTOCARD_CF_LEN7|OTP_CF_ES)
#define CRYPTOCARD_HP_ES (CRYPTOCARD_CF_HD|CRYPTOCARD_CF_LEN8|CRYPTOCARD_CF_PH|OTP_CF_ES)
#define CRYPTOCARD_D8_ES (CRYPTOCARD_CF_DD|CRYPTOCARD_CF_LEN8|OTP_CF_ES)
#define CRYPTOCARD_D7_ES (CRYPTOCARD_CF_DD|CRYPTOCARD_CF_LEN7|OTP_CF_ES)
#define CRYPTOCARD_DP_ES (CRYPTOCARD_CF_DD|CRYPTOCARD_CF_LEN8|CRYPTOCARD_CF_PH|OTP_CF_ES)
#define CRYPTOCARD_H8_RS (CRYPTOCARD_H8_RC|CRYPTOCARD_H8_ES)
#define CRYPTOCARD_H7_RS (CRYPTOCARD_H7_RC|CRYPTOCARD_H7_ES)
#define CRYPTOCARD_HP_RS (CRYPTOCARD_HP_RC|CRYPTOCARD_HP_ES)
#define CRYPTOCARD_D8_RS (CRYPTOCARD_D8_RC|CRYPTOCARD_D8_ES)
#define CRYPTOCARD_D7_RS (CRYPTOCARD_D7_RC|CRYPTOCARD_D7_ES)
#define CRYPTOCARD_DP_RS (CRYPTOCARD_DP_RC|CRYPTOCARD_DP_ES)

static int cryptocard_name2fm(user_t *, state_t *);
static int cryptocard_keystring2key(user_t *, state_t *);
static int cryptocard_nullstate(const config_t *, const user_t *, state_t *,
                                time_t);
static int cryptocard_challenge(const user_t *, state_t *,
                                unsigned char [OTP_MAX_CHALLENGE_LEN], time_t,
                                int, int);
static int cryptocard_response(const user_t *, state_t *,
                               const unsigned char [OTP_MAX_CHALLENGE_LEN],
                               size_t, char [OTP_MAX_RESPONSE_LEN + 1]);
static int cryptocard_updatecsd(state_t *, time_t, int, int, int);
static int cryptocard_isconsecutive(const user_t *, state_t *, int);
static int cryptocard_maxtwin(const user_t *, state_t *, time_t);
static char *cryptocard_printchallenge(char [OTP_MAX_CHALLENGE_LEN * 2 + 1],
                                   const unsigned char [OTP_MAX_CHALLENGE_LEN],
                                       size_t);

#if defined(__GNUC__)
__attribute__ ((constructor))
#elif defined(__SUNPRO_C)
#pragma init(cryptocard_init)
#endif
void cryptocard_init(void);

#endif /* CARDOPS_CRYPTOCARD_H */
