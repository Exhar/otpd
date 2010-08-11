/*
 * $Id$
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
 * Copyright 2005-2007 TRI-D Systems, Inc.
 */

#include "ident.h"
RCSID("$Id$")

#include <arpa/inet.h>	/* inet_ntoa() */
#include <errno.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>	/* sockaddr_un */
#include <sys/types.h>
#include <unistd.h>
#include <stropts.h>
#ifdef __sun__
#define BSD_COMP
#endif
#include <sys/ioctl.h>

#ifdef __linux__
#ifdef HAVE_CLOCK_GETTIME
#include <time.h>
#else
#include <sys/time.h>
#endif
#endif

#include "extern.h"

/* ascii to hex; returns HEXlen on success or -1 on error */
ssize_t
a2x(const char *s, unsigned char x[])
{
  return a2nx(s, x, strlen(s) / 2);
}

/* bounded ascii to hex; returns HEXlen on success or -1 on error */
ssize_t
a2nx(const char *s, unsigned char x[], size_t l)
{
  unsigned i;

  /*
   * We could just use sscanf, but we do this a lot, and have very
   * specific needs, and it's easy to implement, so let's go for it!
   */
  for (i = 0; i < l; ++i) {
    unsigned int n[2];
    int j;

    /* extract 2 nibbles */
    n[0] = *s++;
    n[1] = *s++;

    /* verify range */
    for (j = 0; j < 2; ++j) {
      if ((n[j] >= '0' && n[j] <= '9') ||
          (n[j] >= 'A' && n[j] <= 'F') ||
          (n[j] >= 'a' && n[j] <= 'f'))
        continue;
      return -1;
    }

    /* convert ASCII hex digits to numeric values */
    n[0] -= '0';
    n[1] -= '0';
    if (n[0] > 9) {
      if (n[0] > 'F' - '0')
        n[0] -= 'a' - '9' - 1;
      else
        n[0] -= 'A' - '9' - 1;
    }
    if (n[1] > 9) {
      if (n[1] > 'F' - '0')
        n[1] -= 'a' - '9' - 1;
      else
        n[1] -= 'A' - '9' - 1;
    }

    /* store as octets */
    x[i]  = n[0] << 4;
    x[i] += n[1];
  } /* for (each octet) */

  return i;
}

/* Character maps for generic hex and vendor specific decimal modes */
const char x2a_hex_conversion[]         = "0123456789abcdef";
const char x2a_cc_dec_conversion[]      = "0123456789012345";
const char x2a_snk_dec_conversion[]     = "0123456789222333";
const char x2a_sc_friendly_conversion[] = "0123456789ahcpef";

/*
 * hex to ascii
 * Fills in s, which must point to at least len*2+1 bytes of space.
 */
char *
x2a(const unsigned char *x, size_t len, char *s, const char conversion[16])
{
  unsigned i;

  for (i = 0; i < len; ++i) {
    unsigned n[2];

    n[0] = (x[i] >> 4) & 0x0f;
    n[1] = x[i] & 0x0f;
    s[2 * i + 0] = conversion[n[0]];
    s[2 * i + 1] = conversion[n[1]];
  }
  s[2 * len] = '\0';

  return s;
}

/* guaranteed creation */
void
_xpthread_create(pthread_t *thread, const pthread_attr_t *attr,
                 void *(*start_routine)(void *), void *arg, const char *caller)
{
  int rc;

  if ((rc = pthread_create(thread, attr, start_routine, arg))) {
    mlog(LOG_CRIT, "%s: pthread_create: %s", caller, strerror(rc));
    exit(1);
  }
}

/* guaranteed initialization */
void
_xpthread_mutex_init(pthread_mutex_t *mutexp, const pthread_mutexattr_t *attr,
                     const char *caller)
{
  int rc;

  if ((rc = pthread_mutex_init(mutexp, attr))) {
    mlog(LOG_CRIT, "%s: pthread_mutex_init: %s", caller, strerror(rc));
    exit(1);
  }
}

/* guaranteed trylock */
int
_xpthread_mutex_trylock(pthread_mutex_t *mutexp, const char *caller)
{
  int rc;

  rc = pthread_mutex_trylock(mutexp);
  if (rc && rc != EBUSY) {
    mlog(LOG_CRIT, "%s: pthread_mutex_trylock: %s", caller, strerror(rc));
    exit(1);
  }

  return rc;
}

/* guaranteed lock */
void
_xpthread_mutex_lock(pthread_mutex_t *mutexp, const char *caller)
{
  int rc;

  if ((rc = pthread_mutex_lock(mutexp))) {
    mlog(LOG_CRIT, "%s: pthread_mutex_lock: %s", caller, strerror(rc));
    exit(1);
  }
}

/* guaranteed unlock */
void
_xpthread_mutex_unlock(pthread_mutex_t *mutexp, const char *caller)
{
  int rc;

  if ((rc = pthread_mutex_unlock(mutexp))) {
    mlog(LOG_CRIT, "%s: pthread_mutex_unlock: %s", caller, strerror(rc));
    exit(1);
  }
}

/* guaranteed sempahore init */
void
_xsem_init(sem_t *sem, int pshared, unsigned value, const char *caller)
{
  if (sem_init(sem, pshared, value) == -1) {
    mlog(LOG_CRIT, "%s: sem_init: %s", caller, strerror(errno));
    exit(1);
  }
}

/* guaranteed socket allocation */
int
_xsocket(int domain, int type, int protocol, const char *caller)
{
  int s;

  if ((s = socket(domain, type, protocol)) == -1) {
    mlog(LOG_CRIT, "%s: socket: %s", caller, strerror(errno));
    exit(1);
  }
  return s;
}

/* guaranteed bind */
void
_xbind(int s, const struct sockaddr *addr, int namelen, const char *caller)
{
  if (bind(s, addr, namelen) == -1) {
    if (addr->sa_family == AF_UNIX)
      mlog(LOG_CRIT, "%s: bind(%s): %s", caller,
           ((struct sockaddr_un *) addr)->sun_path, strerror(errno));
    else
      mlog(LOG_CRIT, "%s: bind(%d): %s", caller, s, strerror(errno));
    exit(1);
  }
}

/* guaranteed listen */
void
_xlisten(int s, int backlog, const char *caller)
{
  if (listen(s, backlog) == -1) {
    mlog(LOG_CRIT, "%s: listen: %s\n", caller, strerror(errno));
    exit(1);
  }
}

/* guaranteed connect */
void
_xconnect(int s, const struct sockaddr *name, int namelen, const char *caller)
{
  if (connect(s, name, namelen) == -1) {
    if (name->sa_family == AF_INET)
      mlog(LOG_CRIT, "%s: connect(%s): %s", caller,
           inet_ntoa(((struct sockaddr_in *) name)->sin_addr), strerror(errno));
    else
      mlog(LOG_CRIT, "%s: connect(%d): %s", caller, s, strerror(errno));
    exit(1);
  }
}

/* guaranteed ioctl with int arg 1 */
void
_xioctl1(int fildes, int request, const char *caller)
{
  int one = 1;

  if (ioctl(fildes, request, &one) == -1) {
    mlog(LOG_CRIT, "%s: ioctl: %s", caller, strerror(errno));
    exit(1);
  }
}

/* guaranteed allocation */
void *
_xmalloc(size_t size)
{
  void *p = malloc(size);

  if (!p) {
    /* sketchy, since mlog might need to malloc (but not xmalloc()!) */
    mlog(LOG_CRIT, "malloc");
    exit(1);
  }
  return p;
}

/* guaranteed reallocation */
void *
_xrealloc(void *ptr, size_t size)
{
  void *p = realloc(ptr, size);

  if (!p) {
    /* sketchy, since mlog might need to malloc (but not xmalloc()!) */
    mlog(LOG_CRIT, "realloc");
    exit(1);
  }
  return p;
}

/* guaranteed unlink */
void
_xunlink(const char *path, const char *caller)
{
  if (unlink(path) == -1) {
    if (errno != ENOENT) {
      mlog(LOG_CRIT, "%s: unlink(%s): %s",
           caller, path, strerror(errno));
      exit(1);
    }
  }
}

/*
 * guaranteed full write
 * returns 1 on success, -1 on failure
 */
ssize_t
_xwrite(int fd, const char *buf, size_t len, const char *caller)
{
  size_t nwrote = 0;
  ssize_t n;

  while (nwrote < len) {
    if ((n = write(fd, &buf[nwrote], len - nwrote)) == -1) {
      if (errno == EINTR || errno == EPIPE) {
        continue;
      } else {
        mlog(LOG_ERR, "%s: write(%d): %s", caller, fd, strerror(errno));
        return -1;
      }
    }
    nwrote += n;
  }

  return 1;
}

/* guaranteed chdir */
void
_xchdir(const char *path, const char *caller)
{
  if (chdir(path) == -1) {
    mlog(LOG_CRIT, "%s: can't chdir to %s: %s", caller, path, strerror(errno));
    exit(1);
  }
}

/* guaranteed setuid */
void
_xsetuid(uid_t uid, const char *caller)
{
  if (setuid(uid) == -1) {
    if (errno == EPERM)
      mlog(LOG_CRIT, "%s: Permission denied", caller);
    else
      mlog(LOG_CRIT, "%s: setuid: %s", caller, strerror(errno));
    exit(1);
  }
}

/* guaranteed setgid */
void
_xsetgid(gid_t gid, const char *caller)
{
  if (setgid(gid) == -1) {
    if (errno == EPERM)
      mlog(LOG_CRIT, "%s: Permission denied", caller);
    else
      mlog(LOG_CRIT, "%s: setgid: %s", caller, strerror(errno));
    exit(1);
  }
}

/* guaranteed setsid */
void
_xsetsid(const char *caller)
{
  if (setsid() == (pid_t) -1) {
    mlog(LOG_CRIT, "%s: setsid: %s", caller, strerror(errno));
    exit(1);
  }
}

/* guaranteed strdup */
char *
_xstrdup(const char *s, const char *caller)
{
  char *t;

  if ((t = strdup(s)) == NULL) {
    mlog(LOG_CRIT, "%s: strdup failed", caller);
    exit(1);
  }
  return t;
}

#ifndef HAVE_CLOSEFROM
/* guaranteed getrlimit */
void
_xgetrlimit(int resource, struct rlimit *rlp, const char *caller)
{
  if (getrlimit(resource, rlp) == -1) {
    mlog(LOG_CRIT, "%s: getrlimit: %s", caller, strerror(errno));
    exit(1);
  }
}
#endif /* !HAVE_CLOSEFROM */

#ifdef __linux__
/* guaranteed time */
hrtime_t
_xgethrtime(const char *caller)
{
#ifdef HAVE_CLOCK_GETTIME
  struct timespec tp;
#else
  struct timeval tp;
#endif
  hrtime_t now;

#ifdef HAVE_CLOCK_GETTIME
  /* we try clock_gettime() because it may be faster */
  if (clock_gettime(CLOCK_REALTIME, &tp) == -1) {
    mlog(LOG_CRIT, "%s: clock_gettime: %s", caller, strerror(errno));
    exit(1);
  }
#else
  if (gettimeofday(&tp, NULL) == -1) {
    mlog(LOG_CRIT, "%s: gettimeofday: %s", caller, strerror(errno));
    exit(1);
  }
#endif

#ifdef HAVE_CLOCK_GETTIME
  now = tp.tv_sec * 1000000000LL + tp.tv_nsec;
#else
  now = tp.tv_sec * 1000000000LL + tp.tv_usec * 1000;
#endif

  return now;
}
#endif /* __linux__ */
