/*
 * $Id: //trid/release/otpd-3.1.0/helix.h#1 $
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

#ifndef HELIX_H
#define HELIX_H

#include "ident.h"
RCSIDH(helix_h, "$Id: //trid/release/otpd-3.1.0/helix.h#1 $")

#include <inttypes.h>
#include "extern.h"

static uint32_t helix_block(const helix_scontext_t *, helix_dcontext_t *,
                            uint32_t);

#define HELIX_MAGIC 0x912d94f1

#endif /* HELIX_H */
