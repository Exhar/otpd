/*
 * $Id$
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
 * Copyright 2005,2006,2008 TRI-D Systems, Inc.
 */

#ifndef GSM_H
#define GSM_H

#include "ident.h"
RCSIDH(gsm_h, "$Id$")

/*
 * NOTE: This file must be synced between otpd and gsmd.
 */

#define GSM_DEFAULT_GPORT 1220	/* gsmd port */

#define GSM_MAX_NAME_LEN 32
#define GSM_MAX_STATE_LEN 892	/* so update_q is < 1k */
#define GSM_MAX_USERNAME_LEN 31	/* must match OTP_MAX_USERNAME_LEN */

#define HELIX_NONCE_LEN 16
#define HELIX_MAC_LEN 16
#define HELIX_KEY_LEN 16

/*
 * update: cmd-space + timestamp-space + username-space + state + term
 *             2     +         9             32         + 892   +   1  = 936
 * getput: cmd-space + zero-space      + username-space + state + term
 *             2     +         9       +     32         + 892   +   1  = 936
 * NOTE: must be uint32 aligned for MAC append
 */
#define GSM_MAX_MSG_LEN 936
#define GSM_ULTRA_MSG_LEN HELIX_NONCE_LEN+GSM_MAX_MSG_LEN+HELIX_MAC_LEN

#endif /* GSM_H */
