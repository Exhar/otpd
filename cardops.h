/*
 * $Id: //trid/release/otpd-3.1.0/cardops.h#1 $
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
 * Copyright 2005-2008 TRI-D Systems, Inc.
 */

#ifndef CARDOPS_H
#define CARDOPS_H

#include "ident.h"
RCSIDH(cardops_h, "$Id: //trid/release/otpd-3.1.0/cardops.h#1 $")

#include <inttypes.h>
#include <sys/types.h>
#include "otp.h"	/* OTP_MAX_CHALLENGE_LEN */
#include "gsm.h"	/* GSM_MAX_NAME_LEN */

/*
 * Card Features bitmask.
 */
#define OTP_CF_NONE		0
/* sync/async modes */
#define OTP_CF_AM		0x01 << 1  /* async mode (chal/resp) */
#define OTP_CF_ES		0x01 << 2  /* event synchronous      */
#define OTP_CF_TS		0x01 << 3  /* time synchronous       */
#define OTP_CF_SM		(OTP_CF_ES|OTP_CF_TS)
/* otp length (must support up to OTP_MAX_RESPONSE_LEN digits) */
#define OTP_CF_LEN0		0x01 << 4  /* length 2^0             */
#define OTP_CF_LEN1		0x01 << 5  /* length 2^1             */
#define OTP_CF_LEN2		0x01 << 6  /* length 2^2             */
#define OTP_CF_LEN3		0x01 << 7  /* length 2^3             */
#define OTP_CF_LEN4		0x01 << 8  /* length 2^3             */
#define OTP_CF_LEN		(OTP_CF_LEN0|OTP_CF_LEN1|OTP_CF_LEN2|OTP_CF_LEN3|OTP_CF_LEN4)
#define OTP_CF_LEN_SHIFT	4          /* convert mask to value */
/* force rwindow for event+time sync cards (TRI-D) */
#define OTP_CF_FRW0		0x01 << 9  /* force event window 2^0 */
#define OTP_CF_FRW1		0x01 << 10 /* force event window 2^1 */
#define OTP_CF_FRW2		0x01 << 11 /* force event window 2^2 */
#define OTP_CF_FRW3		0x01 << 12 /* force event window 2^3 */
#define OTP_CF_FRW		(OTP_CF_FRW0|OTP_CF_FRW1|OTP_CF_FRW2|OTP_CF_FRW3)
#define OTP_CF_FRW_SHIFT	9          /* convert mask to value  */

/* All card-specific features must use VS bits to avoid conflicts    */
#define OTP_CF_VSA0		0x01 << 13 /* vendor specific A1     */
#define OTP_CF_VSA1		0x01 << 14 /* vendor specific A2     */
#define OTP_CF_VSA2		0x01 << 15 /* vendor specific A3     */
#define OTP_CF_VSA3		0x01 << 16 /* vendor specific A4     */
#define OTP_CF_VSA4		0x01 << 17 /* vendor specific A5     */
#define OTP_CF_VSA5		0x01 << 18 /* vendor specific A6     */
#define OTP_CF_VSA6		0x01 << 19 /* vendor specific A7     */
#define OTP_CF_VSA7		0x01 << 20 /* vendor specific A8     */
#define OTP_CF_VSA		(OTP_CF_VSA0|OTP_CF_VSA1|OTP_CF_VSA2|OTP_CF_VSA3|OTP_CF_VSA4|OTP_CF_VSA5|OTP_CF_VSA6|OTP_CF_VSA7)
#define OTP_CF_VSA_SHIFT	13         /* convert mask to value  */

#define OTP_CF_VSB0		0x01 << 21 /* vendor specific B1     */
#define OTP_CF_VSB1		0x01 << 22 /* vendor specific B2     */
#define OTP_CF_VSB2		0x01 << 23 /* vendor specific B3     */
#define OTP_CF_VSB3		0x01 << 24 /* vendor specific B4     */
#define OTP_CF_VSB4		0x01 << 25 /* vendor specific B5     */
#define OTP_CF_VSB5		0x01 << 26 /* vendor specific B6     */
#define OTP_CF_VSB6		0x01 << 27 /* vendor specific B7     */
#define OTP_CF_VSB7		0x01 << 28 /* vendor specific B8     */
#define OTP_CF_VSB		(OTP_CF_VSB0|OTP_CF_VSB1|OTP_CF_VSB2|OTP_CF_VSB3|OTP_CF_VSB4|OTP_CF_VSB5|OTP_CF_VSB6|OTP_CF_VSB7)
#define OTP_CF_VSB_SHIFT	21         /* convert mask to value  */

#define OTP_CF_MAX		0x01 << 31 /* MAX placeholder        */

/* TRI-D and Secure Computing can do 16 */
#define OTP_MAX_RESPONSE_LEN 16
#define OTP_MAX_CARDNAME_LEN 31
#define OTP_MAX_KEY_LEN	256
#define OTP_MAX_PIN_LEN 16


/* user info */
struct cardops_t;
typedef struct user_t {
  const char		*username;
  struct cardops_t	*cardops;

  char			card[OTP_MAX_CARDNAME_LEN + 1];
  uint32_t		featuremask;
  time_t		timeissued;

  char			keystring[OTP_MAX_KEY_LEN * 2 + 1];
  unsigned char		key[OTP_MAX_KEY_LEN];
  char			ipinstring[OTP_MAX_PIN_LEN * 2 + 1];
  char			pinstring[OTP_MAX_PIN_LEN * 2 + 1];
  char			pin[OTP_MAX_PIN_LEN + 1];
  char			gsmd[GSM_MAX_NAME_LEN + 1];

  int			keyid;
  int			encryptmode;

  char			*password;
  unsigned char		password_hash[20];	/* big enough for SHA1 */
  unsigned		rwindow_size;
} user_t;

#define OTP_MAX_CSD_LEN	63
#define OTP_MAX_RD_LEN	31

/* state info */
struct ulock_t;
typedef struct state_t {
  int		locked;			/* locked aka success flag        */
  union {
    struct ulock_t	*lock;		/* lock [local state]             */
    struct {
      int		s;		/* socket                         */
      gsmd_t		*g;		/* gsmd data                      */
      int		active;		/* active gsmd (primary/backup)   */
      uint32_t		seq;		/* seqno                          */
    } g;				/* global state                   */
  } l;
  int		nullstate;		/* null state?                    */
  int		updated;		/* state updated? (1 unless err)  */
  ssize_t	clen;			/* challenge length               */

  union {
    unsigned char challenge[OTP_MAX_CHALLENGE_LEN]; /* prev sync chal  */
    uint64_t      u;                                /* force alignment */
  } u;
  char		csd[OTP_MAX_CSD_LEN+1];	/* card-specific data             */
  char		rd[OTP_MAX_RD_LEN+1];	/* rwindow data                   */
  uint32_t	failcount;		/* number of consecutive failures */
  uint32_t	authtime;		/* time of last auth              */
  uint32_t   	mincardtime;		/* minimum cardtime               */

  int32_t	scratch1;		/* card-specific scratch data     */
  int32_t	scratch2;		/* card-specific scratch data     */
  int32_t	scratch3;		/* card-specific scratch data     */
  int32_t	scratch4;		/* card-specific scratch data     */
  int32_t	scratch5;		/* card-specific scratch data     */
  int32_t	scratch6;		/* card-specific scratch data     */
  int32_t	scratch7;		/* card-specific scratch data     */
  int32_t	scratch8;		/* card-specific scratch data     */
  int32_t	scratch9;		/* card-specific scratch data     */
} state_t;

/* cardops object */
typedef struct cardops_t {
  const char *prefix;
  size_t plen;		/* to avoid strlen(prefix) */

  int (*name2fm)(user_t *, state_t *);
  int (*keystring2key)(user_t *, state_t *);
  int (*nullstate)(const config_t *, const user_t *, state_t *, time_t);
  int (*challenge)(const user_t *, state_t *,
                   unsigned char [OTP_MAX_CHALLENGE_LEN], time_t, int, int);
  int (*response)(const user_t *, state_t *,
                  const unsigned char [OTP_MAX_CHALLENGE_LEN], size_t,
                  char [OTP_MAX_RESPONSE_LEN + 1]);
  int (*updatecsd)(state_t *, time_t, int, int, int);
  int (*isconsecutive)(const user_t *, state_t *, int);
  int (*maxtwin)(const user_t *, state_t *, time_t);
  char *(*printchallenge)(char [OTP_MAX_CHALLENGE_LEN * 2 + 1],
                          const unsigned char [OTP_MAX_CHALLENGE_LEN], size_t);
} cardops_t;
#define OTP_MAX_VENDORS 16
extern cardops_t cardops[OTP_MAX_VENDORS];
extern int ncardops;

#endif /* CARDOPS_H */
