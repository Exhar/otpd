/*
 * $Id: //trid/release/otpd-3.1.0/extern.h#1 $
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

#ifndef EXTERN_H
#define EXTERN_H

#include "ident.h"
RCSIDH(extern_h, "$Id: //trid/release/otpd-3.1.0/extern.h#1 $")

/* config file */
#define OTPD_DEFAULT_CF "/etc/otpd.conf"

/* passwd file */
#define OTPD_PASSWD "/etc/otppasswd"

/* plugin rendezvous point */
#define OTPD_PLUGIN_RP "/var/run/otpd/socket"

/* statedir */
#define OTPD_STATEDIR "/etc/otpstate"

/* userops module indices */
#define OTPD_USEROPS_FILE 0
#define OTPD_USEROPS_LDAP 1
#define OTPD_USEROPS_MAX  2

/* key encryption modes */
#define EMODE_CLEAR  0
#define EMODE_PIN    1
#define EMODE_PINMD5 2
#define EMODE_KEYID  3

/* TLS encryption modes */
#define TLSMODE_OFF      0
#define TLSMODE_ON       1
#define TLSMODE_BINDONLY 2

/* state modes */
#define SMODE_LOCAL  0
#define SMODE_GLOBAL 1


#include <inttypes.h>
#include <sys/types.h>
#if defined(__linux__)
typedef long long hrtime_t;
#elif defined(__sun__)
#include <sys/time.h>
#endif

struct config_t;
struct user_t;
typedef struct userops_t {
  void	(*init1)(const struct config_t *);
  int	(*get)(const char *, struct user_t **, const struct config_t *, time_t);
  void	(*put)(struct user_t *);
} userops_t;
extern userops_t userops[OTPD_USEROPS_MAX];

#include <limits.h>
#include "gsm.h"
typedef struct config_t {
  unsigned	hardfail;
  unsigned	softfail;
  hrtime_t	timeout;
  int		prepend_pin;
  unsigned	ewindow_size;
  unsigned	rwindow_size;
  int		site_transform;

  userops_t	*userops;
  struct {
    char	passwd[PATH_MAX + 1];
    int		encryptmode;
    int		keyid;
    char	gsmd[GSM_MAX_NAME_LEN + 1];
  } file;

  struct {
    char	*url;
    char	*binddn;
    struct {
      char	*val;	/* wrong type; blame struct berval */
      unsigned	len;	/* cache strlen() result           */
    } bindpw;		/* our own struct to avoid lber.h  */
    char	*basedn;
    struct {
      char	*val;
      size_t	len;
    } filter;
    char	*scope;

    struct {
      int	mode;
      int	verify_cert;
      char	*cacertfile;
      char	*cacertdir;
      char	*certfile;
      char	*keyfile;
    } tls;
  } ldap;

  struct {
    size_t		len;
    unsigned char	*key;
  } **key;		/* otppasswd keys indexed by keyid           */
  unsigned	nkeys;	/* max keyid + 1, actually (**key is sparse) */

  struct {
    int		mode;
    char	statedir[PATH_MAX + 1];
  } state;

  int		s;	/* plugin socket (not actually configuration) */
} config_t;

#include "gsm.h"
typedef struct ulock_t {
  char			*username;
  hrtime_t		ltime;		/* time lock was granted */
  struct ulock_t	*prev;
  struct ulock_t	*next;
} ulock_t;

/* static (long term) helix context */
typedef struct helix_scontext_t {
  uint32_t	magic;
  uint32_t	l;		/* key len        */
  uint32_t	k[8];		/* working key    */
  uint32_t      x0[8];		/* key word x0    */
} helix_scontext_t;

/* dynamic (nonce-specific) helix context */
typedef struct helix_dcontext_t {
  uint32_t	n[8];		/* working nonce  */
  uint32_t	z[5];		/* working state  */
  uint32_t      x1[8];		/* key word x1    */
  uint64_t	iplus8;		/* stream counter */
  uint32_t	*iplus8hp;	/* pointer to high-order bits of iplus8 */
} helix_dcontext_t;

/* gsmd data */
#include <netinet/in.h>
#include <semaphore.h>
typedef struct gsmd_t {
  char			*name;
  sem_t			sem;	/* signalling mechanism       */
  int			n;	/* #hosts configured (1 or 2) */
  int			active;	/* primary (0) or backup (1)  */
  hrtime_t		fo_timeout; 	/* failover timeout   */
  hrtime_t		fb_timeout;	/* failback timeout  */
  struct sockaddr_in	sin[2];
  helix_scontext_t	*scontext;
  struct gsmd_t		*next;
} gsmd_t;

/* accept_thread.c */
extern void *accept_thread(void *);

/* cardops.c */
#include "otp.h"
extern int verify(config_t *, const otp_request_t *, otp_reply_t *);
extern void cardops_init(void);

/* config.y */
extern config_t *config_init(void);

/* failover_thread.c */
extern void *failover_thread(void *);

/* gsmd.c */
void gsmd_hash(gsmd_t *);
gsmd_t *gsmd_get(const char *);
gsmd_t *gsmd_next(gsmd_t *);

/* helix.c */
extern helix_scontext_t *helix_setkey(helix_scontext_t *,
                                      const unsigned char *, size_t);
extern void helix_nonce(const helix_scontext_t *, helix_dcontext_t *,
                        unsigned char [HELIX_NONCE_LEN]);
extern int helix_encrypt(const helix_scontext_t *, helix_dcontext_t *,
                         const unsigned char *, size_t, unsigned char *,
                         unsigned char [HELIX_MAC_LEN]);
extern int helix_decrypt(const helix_scontext_t *, helix_dcontext_t *,
                         const unsigned char *, size_t, unsigned char *,
                         unsigned char [HELIX_MAC_LEN]);

/* hotp.c */
extern int hotp_mac(const unsigned char [8], char [/*6..10*/],
                    const unsigned char *, size_t, unsigned);

/* lock.c */
extern ulock_t *lock_get(const char *);
extern void lock_put(ulock_t *lock);
extern void lock_init(void);

/* main.c */
#include <pthread.h>
extern pthread_attr_t attr_detached;
extern char *opt_c;
extern char *opt_u;
extern char *opt_d;
extern char *opt_p;
extern char *progname;

/* mlog.c */
#include <syslog.h>	/* LOG_DEBUG */
#if LOG_DEBUG != 7
#error LOG_DEBUG != 7
#endif
/* "super debug" levels */
#define LOG_DEBUG0	LOG_DEBUG
#define LOG_DEBUG1	8
#define LOG_DEBUG2	9
#define LOG_DEBUG3	10
#define LOG_DEBUG4	11
#define LOG_DEBUG5	12
#define LOG_DEBUG6	13
#define LOG_DEBUG7	14
#define LOG_DEBUG8	15

extern int log_level;
extern int log_facility;

#ifdef __GNUC__
extern void mlog(int, const char *, ...)
                 __attribute__ ((format (printf, 2, 3)));
#else
/*PRINTFLIKE2*/
extern void mlog(int, const char *, ...);
#endif
extern void mopenlog(const char *, int);

/* nonce.c */
extern uint32_t streamid[2];
extern uint32_t nonce32(void);
extern void nonce_init(void);

/* site.c */
#include "otp.h"
extern ssize_t challenge_transform(const char *,
                                   unsigned char [OTP_MAX_CHALLENGE_LEN],
                                   size_t);
/* state.c */
#include "cardops.h"
extern int state_get(const config_t *, const user_t *, state_t *);
extern int state_put(const config_t *, const user_t *, state_t *);
extern void state_close(void);

/* uhash.c */
#define UHASH_SIZE 64
extern int uhash(const char *);

/* x99.c */
extern int x99_mac(const unsigned char *, size_t, unsigned char [8],
                   const unsigned char *);

/* xfunc.c */
#include <pthread.h>
#include <semaphore.h>
#include <sys/socket.h>
#include <time.h>

/* Character maps for generic hex and vendor specific decimal modes */
extern const char x2a_hex_conversion[];
extern const char x2a_cc_dec_conversion[];
extern const char x2a_snk_dec_conversion[];
extern const char x2a_sc_friendly_conversion[];

extern ssize_t a2x(const char *, unsigned char []);
extern ssize_t a2nx(const char *, unsigned char [], size_t);
extern char *x2a(const unsigned char [], size_t, char *, const char [16]);

extern void _xpthread_create(pthread_t *, const pthread_attr_t *,
                             void *(*)(void *), void *, const char *);
extern void _xpthread_mutex_init(pthread_mutex_t *,
                                 const pthread_mutexattr_t *, const char *);
extern int _xpthread_mutex_trylock(pthread_mutex_t *, const char *);
extern void _xpthread_mutex_lock(pthread_mutex_t *, const char *);
extern void _xpthread_mutex_unlock(pthread_mutex_t *, const char *);

extern void _xsem_init(sem_t *, int, unsigned, const char *);

extern int _xsocket(int, int, int, const char *);
extern void _xbind(int, const struct sockaddr *, int, const char *);
extern void _xlisten(int, int, const char *);
extern void _xconnect(int, const struct sockaddr *, int, const char *);

extern void _xioctl1(int, int, const char *);
extern void *_xmalloc(size_t);
extern void *_xrealloc(void *, size_t);
extern void _xunlink(const char *, const char *);
extern ssize_t _xwrite(int, const char *, size_t, const char *);
extern void _xchdir(const char *, const char *);
extern void _xsetuid(uid_t, const char *);
extern void _xsetgid(gid_t, const char *);
extern void _xsetsid(const char *);
extern char *_xstrdup(const char *, const char *);
#ifndef HAVE_CLOSEFROM
#include <sys/resource.h>
extern void _xgetrlimit(int, struct rlimit *, const char *);
#endif

#if defined(__linux__)
extern hrtime_t _xgethrtime(const char *);
#endif

#define xpthread_create(a, b, c, d) _xpthread_create((a),(b),(c),(d), __func__)
#define xpthread_mutex_init(a, b) _xpthread_mutex_init((a), (b), __func__)
#define xpthread_mutex_trylock(a) _xpthread_mutex_trylock((a), __func__)
#define xpthread_mutex_lock(a) _xpthread_mutex_lock((a), __func__)
#define xpthread_mutex_unlock(a) _xpthread_mutex_unlock((a), __func__)

#define xsem_init(a, b, c) _xsem_init((a), (b), (c), __func__)

#define xsocket(a, b, c) _xsocket((a), (b), (c), __func__)
#define xbind(a, b, c) _xbind((a), (b), (c), __func__)
#define xlisten(a, b) _xlisten((a), (b), __func__)
#define xconnect(a, b, c) _xconnect((a), (b), (c), __func__)

#define xioctl1(a, b) _xioctl1((a), (b), __func__)
#define xmalloc(a) _xmalloc((a))
#define xrealloc(a, b) _xrealloc((a), (b))
#define xunlink(a) _xunlink((a), __func__)
#define xwrite(a, b, c) _xwrite((a), (b), (c), __func__)
#define xchdir(a) _xchdir((a), __func__)
#define xsetuid(a) _xsetuid((a), __func__)
#define xsetgid(a) _xsetgid((a), __func__)
#define xsetsid() _xsetsid(__func__)
#define xstrdup(a) _xstrdup((a), __func__)
#ifndef HAVE_CLOSEFROM
#define xgetrlimit(a, b) _xgetrlimit((a), (b), __func__)
#endif
#if defined(__linux__)
#define xgethrtime() _xgethrtime(__func__)
#elif defined(__sun__)
#define xgethrtime gethrtime
#endif

/* work_thread.c */
extern void *work_thread(void *);

/* GNU helpers */
#ifdef __GNUC__
#define __unused__ __attribute__ ((unused))
#else
#define __unused__
#endif

#endif /* EXTERN_H */
