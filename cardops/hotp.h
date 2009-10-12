/*
 * $Id: //trid/release/otpd-3.1.0/cardops/hotp.h#1 $
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

#ifndef CARDOPS_HOTP_H
#define CARDOPS_HOTP_H

#include "../ident.h"
RCSIDH(cardops_hotp_h, "$Id: //trid/release/otpd-3.1.0/cardops/hotp.h#1 $")

#include <inttypes.h>
#include <sys/types.h>

#include "../extern.h"
#include "../otp.h"
#include "../cardops.h"

/* card modes */
#define HOTP_CF_LEN5 (OTP_CF_LEN2|OTP_CF_LEN0)
#define HOTP_CF_LEN6 (OTP_CF_LEN2|OTP_CF_LEN1)
#define HOTP_CF_LEN7 (OTP_CF_LEN2|OTP_CF_LEN1|OTP_CF_LEN0)
#define HOTP_CF_LEN8 OTP_CF_LEN3
#define HOTP_CF_LEN9 (OTP_CF_LEN3|OTP_CF_LEN0)

#define HOTP5 (OTP_CF_ES|HOTP_CF_LEN5)
#define HOTP6 (OTP_CF_ES|HOTP_CF_LEN6)
#define HOTP7 (OTP_CF_ES|HOTP_CF_LEN7)
#define HOTP8 (OTP_CF_ES|HOTP_CF_LEN8)
#define HOTP9 (OTP_CF_ES|HOTP_CF_LEN9)

static int hotp_name2fm(user_t *, state_t *);
static int hotp_keystring2key(user_t *, state_t *);
static int hotp_nullstate(const config_t *, const user_t *, state_t *, time_t);
static int hotp_challenge(const user_t *, state_t *,
                          unsigned char [OTP_MAX_CHALLENGE_LEN], time_t,
                          int, int);
static int hotp_response(const user_t *, state_t *,
                         const unsigned char [OTP_MAX_CHALLENGE_LEN],
                         size_t, char [OTP_MAX_RESPONSE_LEN + 1]);
static int hotp_updatecsd(state_t *, time_t, int, int, int);
static int hotp_isconsecutive(const user_t *, state_t *, int);
static int hotp_maxtwin(const user_t *, state_t *, time_t);
static char *hotp_printchallenge(char [OTP_MAX_CHALLENGE_LEN * 2 + 1],
                                 const unsigned char [OTP_MAX_CHALLENGE_LEN],
                                 size_t);

#if defined(__GNUC__)
__attribute__ ((constructor))
#elif defined(__SUNPRO_C)
#pragma init(hotp_init)
#endif
void hotp_init(void);

#endif /* CARDOPS_HOTP_H */
