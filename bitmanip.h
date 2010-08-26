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
 * Copyright 2005-2008 TRI-D Systems, Inc.
 */

#ifndef BITMANIP_H
#define BITMANIP_H

#include "ident.h"
RCSIDH(bitmanip_h, "$Id$")

#include <inttypes.h>

#if defined(__linux__)
#include <endian.h>
#if __BYTE_ORDER == __LITTLE_ENDIAN
#define _LITTLE_ENDIAN
#else
#define _BIG_ENDIAN
#endif /* else __BYTE_ORDER != __LITTLE_ENDIAN */
#elif defined(__sun__)
#include <sys/byteorder.h>
#endif /* else __sun__ */

/* & is much faster than %  */
#define MOD8(n) ((n) & 0x07)
#define MOD4(n) ((n) & 0x03)
#define ROUND4(n) ((n) & ~0x03)

#if defined(__GNUC__)
#if defined(__i386__) || defined(__x86_64__)
static inline uint32_t
rol(uint32_t word, uint32_t n)
{
  asm("roll %%cl, %0"
      :"=r" (word)
      :"0" (word), "c" (n));

  return word;
}
#elif defined(__sparc__)
#define rol(w,x) (((w) << (x))|((w) >> (32 - (x))))
#elif defined(__ppc__)
#define rol(w,x) (((w) << (x))|((w) >> (32 - (x))))
#endif /* else __ppc__ */
#else
#define rol(w,x) (((w) << (x))|((w) >> (32 - (x))))
#endif /* else !__GNUC__ */

/* avoid strict aliasing */
typedef uint32_t
#if defined(__GNUC__)
__attribute__ ((may_alias))
#endif
uint32_t_a;

/*
 * little-endian to host-endian byte manip macros
 * (uint32 alignment assumed)
 *
 * LEUCHAR2HEUINT32:    little-endian uchar to host-endian uint32
 * LEUCHAR2HEUINT32ZE:  little-endian uchar to host-endian uint32 w/ extension
 * HEUINT32TOLEUCHAR:   host-endian uint32 to little-endian uchar
 * HEUINT32P2LEUCHAR:   host-endian uint32 (partial) to little-endian uchar
 * LEUCHARXORHEUINT32:  xor of little-endian uchar with host-endian uint32
 * LEUCHARXORHEUINT32P: xor of le uchar with host-endian uint32 (partial)
 */
#ifdef _LITTLE_ENDIAN

#define LEUCHAR2HEUINT32(w, cp) w = *(uint32_t_a *) cp

#define LEUCHAR2HEUINT32ZE(w, cp, n) do			\
  {							\
    unsigned char *uccp = (unsigned char *) cp;		\
    unsigned char *ucwp = (unsigned char *) &w;		\
    unsigned li;					\
    w = 0;   						\
    for (li = 0; li < n; ++li)    			\
      ucwp[li] = uccp[li];       			\
  } while (0)

#define HEUINT32TOLEUCHAR(cp, w) *(uint32_t_a *) cp = w;

#define HEUINT32P2LEUCHAR(cp, w, n) do			\
  {							\
    unsigned char *uccp = (unsigned char *) cp;		\
    unsigned char *ucwp = (unsigned char *) &w;		\
    unsigned li;					\
    for (li = 0; li < n; ++li)				\
      uccp[li] = ucwp[li];				\
  } while (0);

#define LEUCHARXORHEUINT32(wp, xp, y) do	        \
  {							\
    uint32_t *uiwp = (uint32_t *) wp;			\
    uint32_t *uixp = (uint32_t *) xp;			\
    *uiwp = *uixp ^ y;					\
  } while (0)

#define LEUCHARXORHEUINT32P(wp, xp, y, n) do		\
  {							\
    unsigned li;					\
    unsigned char *ucwp = (unsigned char *) wp;		\
    unsigned char *ucxp = (unsigned char *) xp;		\
    unsigned char *ucyp = (unsigned char *) &y;		\
    for (li = 0; li < n; ++li)				\
      ucwp[li] = ucxp[li] ^ ucyp[li];			\
  } while (0)

#else

#define LEUCHAR2HEUINT32(w, cp) do			\
  {							\
    unsigned char *ucwp = (unsigned char *) &w;		\
    unsigned char *uccp = (unsigned char *) cp;		\
    ucwp[0] = uccp[3];					\
    ucwp[1] = uccp[2];					\
    ucwp[2] = uccp[1];					\
    ucwp[3] = uccp[0];					\
  } while (0)

#define LEUCHAR2HEUINT32ZE(w, cp, n) do			\
  {							\
    unsigned char *uccp = (unsigned char *) cp;		\
    unsigned char *ucwp = (unsigned char *) &w;		\
    unsigned li;					\
    w = 0;   						\
    for (li = 0; li < n; ++li)    			\
      ucwp[3 -li] = uccp[li];       			\
  } while (0)

#define HEUINT32TOLEUCHAR(cp, w) do			\
  {							\
    unsigned char *uccp = (unsigned char *) cp;		\
    unsigned char *ucwp = (unsigned char *) &w;		\
    uccp[0] = ucwp[3];					\
    uccp[1] = ucwp[2];					\
    uccp[2] = ucwp[1];					\
    uccp[3] = ucwp[0];					\
  } while (0)

#define HEUINT32P2LEUCHAR(cp, w, n) do			\
  {							\
    unsigned char *uccp = (unsigned char *) cp;		\
    unsigned char *ucwp = (unsigned char *) &w;		\
    unsigned li;					\
    for (li = 0; li < n; ++li)				\
      uccp[li] = ucwp[3 - li];				\
  } while (0);

#define LEUCHARXORHEUINT32(wp, xp, y) do	        \
  {							\
    unsigned char *ucwp = (unsigned char *) wp;		\
    unsigned char *ucxp = (unsigned char *) xp;		\
    unsigned char *ucyp = (unsigned char *) &y;		\
    ucwp[0] = ucxp[0] ^ ucyp[3];			\
    ucwp[1] = ucxp[1] ^ ucyp[2];			\
    ucwp[2] = ucxp[2] ^ ucyp[1];			\
    ucwp[3] = ucxp[3] ^ ucyp[0];			\
  } while (0)

#define LEUCHARXORHEUINT32P(wp, xp, y, n) do		\
  {							\
    unsigned li;					\
    unsigned char *ucwp = (unsigned char *) wp;		\
    unsigned char *ucxp = (unsigned char *) xp;		\
    unsigned char *ucyp = (unsigned char *) &y;		\
    for (li = 0; li < n; ++li)				\
      ucwp[li] = ucxp[li] ^ ucyp[3 - li];		\
  } while (0)

#endif /* !_LITTLE_ENDIAN */
#endif /* BITMANIP_H */
