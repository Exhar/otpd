/*
 * $Id: //trid/release/otpd-3.1.0/uhash.c#1 $
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

#include "ident.h"
RCSID("$Id: //trid/release/otpd-3.1.0/uhash.c#1 $")

#define NDEBUG
#include <assert.h>

#include "extern.h"

/* almost base64; we only encode 0-9 A-Z a-z */
static char base64[256] = {
  /* nul */  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  /* dle */  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  /* sp  */  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  /*  0  */  0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  0,  0,  0,  0,  0,  0,
  /*  @  */  0, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24,
  /*  P  */ 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35,  0,  0,  0,  0,  0,
  /*  `  */  0, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50,
  /*  p  */ 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61,  0,  0,  0,  0,  0
};

/*
 * hash username
 * returns the hash bucket
 */
int
uhash(const char *username)
{
  int i;
  int bucket = 0;
  /* treat username as uchar so 127-255 aren't negative */
  const unsigned char *u = (const unsigned char *) username;

  /* base64-like reduction of each char and xor with next */
  for (i = 0; u[i]; ++i)
    bucket ^= base64[(unsigned) u[i]];
  assert(bucket < UHASH_SIZE);	/* guaranteed; doco only */
  return bucket;
}
