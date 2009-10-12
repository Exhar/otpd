/*
 * $Id: //trid/release/otpd-3.1.0/mlog.c#1 $
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
RCSID("$Id: //trid/release/otpd-3.1.0/mlog.c#1 $")

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <syslog.h>

#include "extern.h"

/*
 * By default we log at syslog debug level, and depend on system
 * configuration to throw away noise.  This has a slight race;
 * on HUP we might change this, but it won't be noticed right away.
 * No big deal; certainly not worth even an uncontended lock.
 */
int log_level = LOG_DEBUG;
int log_facility = LOG_AUTH;

static int log_syslog = 0;	/* log to stderr or syslog? */

/* maybe log */
void
mlog(int level, const char *fmt, ...)
{
  va_list ap;
  char ffmt[1024];	/* modified fmt for stderr */

  /* discard if log level isn't high enough */
  if (level > log_level)
    return;
  /* map "super debug" levels to syslog debug level */
  if (level > LOG_DEBUG)
    level = LOG_DEBUG;

  va_start(ap, fmt);
  if (log_syslog) {
    (void) vsyslog(level, fmt, ap);
  } else {
    /* prefix progname and suffix '\n' to fmt */
    (void) snprintf(ffmt, sizeof(ffmt), "%s: %s\n", progname, fmt);
    ffmt[1023] = '\0';
    (void) vfprintf(stderr, ffmt, ap);
  }
  va_end(ap);
  return;
}

/* start logging to syslog instead of stderr */
void
mopenlog(const char *ident, int logopt)
{
  openlog(ident, logopt, log_facility);
  log_syslog = 1;
  return;
}
