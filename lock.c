/*
 * $Id: //trid/release/otpd-3.1.0/lock.c#1 $
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
RCSID("$Id: //trid/release/otpd-3.1.0/lock.c#1 $")

#include <pthread.h>
#include <stdlib.h>
#include <string.h>

#include "extern.h"
#include "lock.h"

static struct ulock_chain_t lock_chain[UHASH_SIZE];

static ulock_t *lock_alloc(void);
static int find_lock(const char *, int, ulock_t **);

/*
 * Allocate and initialize a lock node.
 * Newly allocated locks always go at the end of a chain, so we initialize
 * the next pointer to NULL.  Caller is responsible for setting prev pointer.
 */
static ulock_t *
lock_alloc(void)
{
  ulock_t *lock;

  lock = xmalloc(sizeof(*lock));
  lock->username = NULL;
  /* lock->prev = NULL; */
  lock->next = NULL;
  return lock;
}

/*
 * Find an existing lock.
 * Caller must hold the chain mutex.
 * Returns 1 if found, with lock pointing to the node.
 * Returns 0 otherwise, with lock pointing to the preallocated node to be
 *  used, or NULL if no preallocated nodes available (caller must allocate).
 */
static int
find_lock(const char *username, int bucket, ulock_t **lock)
{
  ulock_t *last;

  last = lock_chain[bucket].last;
  if (!last) {
    *lock = lock_chain[bucket].first;
    return 0;	/* no held locks */
  }

  /* walk chain looking for username match */
  for (*lock = lock_chain[bucket].first; *lock != last; *lock = (*lock)->next)
    if (username == (*lock)->username)
      return 1;
  /* now look at the last node */
  if (username == (*lock)->username)
    return 1;

  /* return the node after the last node (possibly NULL) */
  *lock = (*lock)->next;
  return 0;
}

/*
 * lock state
 * returns !NULL if successful,
 * NULL otherwise (unexpired lock already held by someone else)
 */
ulock_t *
lock_get(const char *username)
{
  ulock_t	*lock = NULL;
  int		bucket, waslocked;
  hrtime_t	now;

  /* lock the appropriate chain */
  bucket = uhash(username);
  xpthread_mutex_lock(&lock_chain[bucket].mutex);

  waslocked = find_lock(username, bucket, &lock);
  /* TODO: use configurable expiry */
  now = xgethrtime();
  if (waslocked) {
    if (now >= lock->ltime + 24000000000LL) {
      /*
       * Lock is expired.  The easiest thing to do is relock it.  This
       * breaks LRU ordering, but that seems ok.
       */
      lock->ltime = now;
      mlog(LOG_NOTICE, "%s: stale lock for %s broken", __func__, username);
    } else {
      /* existing lock not expired, report error */
      mlog(LOG_INFO, "%s: lock for %s already held", __func__, username);
    }
  } else {
    /* no existing lock */
    if (lock == NULL) {
      lock = lock_alloc();
      /* if no available lock node, 'last' must be end of chain */
      lock_chain[bucket].last->next = lock;
      lock->prev = lock_chain[bucket].last;
    }

    /* lock */
    lock->username = (char *) username;
    lock->ltime = now;
    /* new locks are always the last lock */
    lock_chain[bucket].last = lock;
  } /* else not locked */

  /* unlock chain and return */
  xpthread_mutex_unlock(&lock_chain[bucket].mutex);
  return lock;
}

/*
 * unlock state
 */
void
lock_put(ulock_t *lock)
{
  int bucket;

  /* lock the appropriate chain */
  bucket = uhash(lock->username);
  xpthread_mutex_lock(&lock_chain[bucket].mutex);

  lock->username[0] = '\0';

  if (lock_chain[bucket].last == lock) {
    /* we're in a good place, just fix up last */
    lock_chain[bucket].last = lock->prev;
  } else {
    /* unlink */
    if (lock->prev)
      lock->prev->next = lock->next;
    if (lock->next)
      lock->next->prev = lock->prev;

    /* relink after last */
    lock->next = lock_chain[bucket].last->next;
    lock->prev = lock_chain[bucket].last;
    lock_chain[bucket].last->next->prev = lock;
    lock_chain[bucket].last->next = lock;
  }

  /* unlock chain */
  xpthread_mutex_unlock(&lock_chain[bucket].mutex);
}

void
lock_init(void)
{
  int i;

  /*
   * preallocate a node for each hash chain
   * saves us some initial condition work when walking a chain
   * (lock_chain[bucket].first is guaranteed to exist)
   */
  for (i = 0; i < UHASH_SIZE; ++i) {
    xpthread_mutex_init(&lock_chain[i].mutex, NULL);
    lock_chain[i].first = lock_alloc();
    lock_chain[i].first->prev = NULL;
    lock_chain[i].last = NULL;		/* guaranteed; doco only */
  }
}
