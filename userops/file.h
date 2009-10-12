/*
 * $Id: //trid/release/otpd-3.1.0/userops/file.h#1 $
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
 * Copyright 2006,2008 TRI-D Systems, Inc.
 */

#ifndef USEROPS_FILE_H
#define USEROPS_FILE_H

#include "../ident.h"
RCSIDH(userops_file_h, "$Id: //trid/release/otpd-3.1.0/userops/file.h#1 $")

#include <sys/types.h>

#include "../extern.h"

static int file_get(const char *, user_t **, const config_t *, time_t);
static void file_put(user_t *);
static void file_init1(const config_t *);

#if defined(__GNUC__)
__attribute__ ((constructor))
#elif defined(__SUNPRO_C)
#pragma init(userops_file_init)
#endif
void userops_file_init(void);

#endif /* USEROPS_FILE_H */
