/*
 * $Id: //trid/release/otpd-3.1.0/gsm.h#1 $
 *
 * Copyright 2005,2006,2008 TRI-D Systems, Inc.
 * All Rights Reserved.
 *
 * This is TRI-D Systems, Inc. proprietary source code and is licensed
 * under non-disclosure terms to authorized individuals only.  A copy
 * of the license should be on file in your organization or company,
 * or is available by writing
 *   TRI-D Systems, Inc.
 *   Attn: Licensing
 *   790 East Colorado Blvd., 9th Floor
 *   Pasadena, CA 91101
 * or emailing licensing@tri-dsystems.com.
 *
 * DO NOT DISTRIBUTE.
 */

#ifndef GSM_H
#define GSM_H

#include "ident.h"
RCSIDH(gsm_h, "$Id: //trid/release/otpd-3.1.0/gsm.h#1 $")

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
