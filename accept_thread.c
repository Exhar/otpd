/*
 * $Id: //trid/release/otpd-3.1.0/accept_thread.c#1 $
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
RCSID("$Id: //trid/release/otpd-3.1.0/accept_thread.c#1 $")

#include <errno.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>	/* sockaddr_un */
#include <unistd.h>

#include "extern.h"

/*
 * The accept thread's job is easy.
 * Accept a new connection and pass it off to a work thread.  Repeat.
 */
void *
accept_thread(void *arg)
{
  config_t *config = (config_t *) arg;
  int s;

  struct sockaddr_un sa;
  socklen_t salen = sizeof(sa);

  mlog(LOG_DEBUG2, "%s: tid=%lu", __func__, (unsigned long) pthread_self());

  for (;;) {
    s = accept(config->s, (struct sockaddr *) &sa, &salen);
    if (s == -1) {
      mlog(LOG_ERR, "%s: plugin accept: %s", __func__, strerror(errno));
      /* not much else we can do */
      continue;
    }
    mlog(LOG_DEBUG, "%s: plugin accept fd=%d", __func__, s);

    /*
     * start a work thread (1:1 model)
     */
    {
      pthread_t tid;
      config_t *inst;

      inst = xmalloc(sizeof(*config));
      (void) memcpy(inst, config, sizeof(*config));
      inst->s = s;
      xpthread_create(&tid, &attr_detached, work_thread, inst);
    }

  } /* for (;;) */
}
