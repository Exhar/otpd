/*
 * $Id: //trid/release/otpd-3.1.0/state.h#1 $
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

#ifndef STATE_H
#define STATE_H

#include "ident.h"
RCSIDH(state_h, "$Id: //trid/release/otpd-3.1.0/state.h#1 $")

#include "extern.h"

static int state_get_local(const config_t *, const user_t *, state_t *);
static int state_get_global(const config_t *, const user_t *, state_t *);
static int state_put_local(const config_t *, const user_t *, state_t *);
static int state_put_global(const config_t *, const user_t *, state_t *);

static int gsmd(const config_t *, const user_t *, state_t *, unsigned char *,
                unsigned char *, const char *);
static int state_parse(const char *, size_t,
                       const config_t *, const char *, state_t *);
static void state_unparse(char *, const user_t *, const state_t *);
static ssize_t state_read(char *, size_t, const char *, const char *);
static int state_write(const char *, const char *, char *);

#endif /* STATE_H */
