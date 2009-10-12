/*
 * $Id: //trid/release/otpd-3.1.0/gsmd.c#1 $
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
RCSID("$Id: //trid/release/otpd-3.1.0/gsmd.c#1 $")

#include <string.h>
#include "extern.h"

static struct gsmd_t *gsmd_chain[UHASH_SIZE];

/* add a gsmd_t to the hash chain */
void
gsmd_hash(gsmd_t *gsmd)
{
  int bucket;
  gsmd_t *next;

  bucket = uhash(gsmd->name);
  next = gsmd_chain[bucket];
  gsmd_chain[bucket] = gsmd;
  gsmd->next = next;
}

/* find a gsmd */
gsmd_t *
gsmd_get(const char *name)
{
  gsmd_t *gsmd = NULL;

  gsmd = gsmd_chain[uhash(name)];
  while (gsmd) {
    if (!strcmp(gsmd->name, name))
      break;
    gsmd = gsmd->next;
  }
  return gsmd;
}

/* iterate over gsmds */
gsmd_t *
gsmd_next(gsmd_t *gsmd)
{
  static int i;

  if (gsmd == NULL)
    for (i = 0; i < UHASH_SIZE; ++i)
      if (gsmd_chain[i])
        return gsmd_chain[i];

  if (gsmd == NULL)	/* no gsmd's configured */
    return NULL;

  if (gsmd->next)	/* return next in chain */
    return gsmd->next;

  /* find the next chain with a gsmd */
  for (++i; i < UHASH_SIZE; ++i)
    if (gsmd_chain[i])
      return gsmd_chain[i];

  /* no more gsmds */
  return NULL;
}
