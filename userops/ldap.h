/*
 * $Id: //trid/release/otpd-3.1.0/userops/ldap.h#1 $
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
 * Copyright 2006-2008 TRI-D Systems, Inc.
 */


#ifndef USEROPS_LDAP_H
#define USEROPS_LDAP_H

#include "../ident.h"
RCSIDH(userops_ldap_h, "$Id: //trid/release/otpd-3.1.0/userops/ldap.h#1 $")

#include <lber.h>
#include <ldap.h>
#include <pthread.h>
#include <sys/types.h>

#include "../extern.h"
#include "../cardops.h"

typedef struct ldap_ld_t {
  pthread_mutex_t	mutex;
  LDAP			*ld;
  int			used;	/* has this connection ever been used? */
  struct ldap_ld_t	*next;
} ldap_ld_t;

static int ldap_get(const char *, user_t **, const config_t *, time_t);
static void ldap_put(user_t *);
static ldap_ld_t * getldp(const config_t *, hrtime_t *, struct timeval *);
static void putldp(ldap_ld_t *, int);
static int update_timeout(struct timeval *, hrtime_t *);
static void ldap_init1(const config_t *);

static void _xldap_initialize(LDAP **, const char *, const char *);
static void _xldap_set_option(LDAP *, int, void *, const char *, const char *);
static void _xldap_get_option(LDAP *, int, void *, const char *, const char *);

#define xldap_initialize(a, b) _xldap_initialize((a), (b), __func__)
#define xldap_set_option(a, b, c) _xldap_set_option((a), (b), (c), #b, __func__)
#define xldap_get_option(a, b, c) _xldap_get_option((a), (b), (c), #b, __func__)

#if defined(__GNUC__)
__attribute__ ((constructor))
#elif defined(__SUNPRO_C)
#pragma init(userops_ldap_init)
#endif
void userops_ldap_init(void);

#endif /* USEROPS_LDAP_H */
