/*
 * $Id: //trid/release/otpd-3.1.0/lock.h#1 $
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

#ifndef LOCK_H
#define LOCK_H

#include "ident.h"
RCSIDH(lock_h, "$Id: //trid/release/otpd-3.1.0/lock.h#1 $")

#include <pthread.h>    /* pthread_mutex_t */
#include "extern.h"

struct ulock_chain_t {
  pthread_mutex_t	mutex;
  ulock_t		*first;		/* start of chain */
  ulock_t		*last;		/* last held lock in chain */
};

#endif /* LOCK_H */
