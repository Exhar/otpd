/*
 * $Id: //trid/release/otpd-3.1.0/nonce.c#1 $
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

#include "ident.h"
RCSID("$Id: //trid/release/otpd-3.1.0/nonce.c#1 $")

#ifdef HAVE_ATOMIC_H
#include <atomic.h>
#else
#include <pthread.h>
#endif /* else !HAVE_ATOMIC_H */

#include <errno.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "extern.h"

uint32_t streamid[2];	/* for xmit cache purge */
static uint32_t seq;	/* seqno part of nonce  */

#ifndef HAVE_ATOMIC_H
static pthread_mutex_t seq_mutex;
#endif

/* increment nonce and return new value */
uint32_t
nonce32(void)
{
#ifdef HAVE_ATOMIC_H
  return atomic_add_32_nv(&seq, 2);
#else
  xpthread_mutex_lock(&seq_mutex);
  seq += 2;
  xpthread_mutex_unlock(&seq_mutex);
  return seq;
#endif /* else !HAVE_ATOMIC_H */
}

/* randomize nonce */
void
nonce_init(void)
{
  int fd, n;

  if ((fd = open("/dev/random", O_RDONLY)) == -1) {
    mlog(LOG_CRIT, "%s: open(/dev/random): %s", __func__, strerror(errno));
    exit(1);
  }
  /* streamid must be non-zero */
  while (streamid[0] == 0 && streamid[1] == 0) {
    if ((n = read(fd, streamid, sizeof(streamid))) == -1) {
      mlog(LOG_CRIT, "%s: read: %s", __func__, strerror(errno));
      exit(1);
    }
    if (n < (int) sizeof(streamid)) {
      mlog(LOG_CRIT, "%s: short read", __func__);
      exit(1);
    }
  }
  (void) close(fd);

#ifndef HAVE_ATOMIC_H
  xpthread_mutex_init(&seq_mutex, NULL);
#endif

  /* initialize to -2 so that first increment gives seq 0 */
  seq = -2;
}
