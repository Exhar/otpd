/*
 * $Id: //trid/release/otpd-3.1.0/main.c#1 $
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

#include "ident.h"
RCSID("$Id: //trid/release/otpd-3.1.0/main.c#1 $")

#ifndef _POSIX_PTHREAD_SEMANTICS
#define _POSIX_PTHREAD_SEMANTICS	/* Solaris sigwait() */
#endif

#include <fcntl.h>
#include <libgen.h>		/* basename() */
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "extern.h"
#include "main.h"

extern char *optarg;
extern int optind, opterr, optopt;

pthread_attr_t attr_detached;		/* create all threads detached */

char *opt_c;				/* config file      */
char *opt_u;				/* user             */
char *opt_p;				/* plugin rp        */
char *opt_d;				/* log_level        */

char *progname = "otpd";

/* initialize signal handling */
static void
sig_init()
{
  sigset_t set;
  struct sigaction sa;

  /* block all signals */
  (void) sigfillset(&set);
  (void) pthread_sigmask(SIG_BLOCK, &set, NULL);

  /* ignore SIGPIPE */
  sa.sa_handler = SIG_IGN;
  sa.sa_flags = 0;
  (void) sigemptyset(&sa.sa_mask);	/* not really needed */
  (void) sigaction(SIGPIPE, &sa, NULL);
}

/*
 * Become a daemon, adapted from APUE 2nd Ed.
 * Keep fd open; it is our listening socket, opened before daemonizing
 * for better error reporting.  Use -1 to close all fd's.
 */
static void
daemonize(int listenfd)
{
  pid_t pid;
  int fd0, fd1, fd2;
  int i;
#ifndef HAVE_CLOSEFROM
  struct rlimit rl;

  /* get max fd */
  xgetrlimit(RLIMIT_NOFILE, &rl);
#endif

  /* become a session leader to lose controlling tty */
  if ((pid = fork()) == -1) {
    mlog(LOG_CRIT, "%s: fork", __func__);
    exit(1);
  } else if (pid != 0) {
    /* parent */
    _exit(0);
  }

  /*
   * first child
   */
  xsetsid();

  /* ensure future open()'s won't allocate controlling tty's */
  if ((pid = fork()) == -1) {
    mlog(LOG_CRIT, "%s: fork", __func__);
    exit(1);
  } else if (pid != 0) {
    /* parent (first child) */
    _exit(0);
  }

  /*
   * second child
   */

  /* change cwd to root to allow umount of current filesystem */
  xchdir("/");

  /* avoid listen socket using std* fd's */
  if (listenfd == 0 || listenfd == 1 || listenfd == 2) {
    mlog(LOG_CRIT, "%s: listen socket must be fd 3 or greater", __func__);
    exit(1);
  }
  /* close all open fd's except passed in arg */
  for (i = 0; i < listenfd; ++i)
    (void) close(i);
#ifdef HAVE_CLOSEFROM
  closefrom(listenfd + 1);
#else /* the sucky way */
  if (rl.rlim_max == RLIM_INFINITY)
    rl.rlim_max = 1024;
  for (i = listenfd + 1; i < (int) rl.rlim_max; ++i)
    (void) close(i);
#endif /* !HAVE_CLOSEFROM */

  /* open fd 0,1,2 as /dev/null, to head off problems */
  fd0 = open("/dev/null", O_RDWR);
  fd1 = dup(0);
  fd2 = dup(0);
  /* sanity check */
  if (fd0 != 0 || fd1 != 1 || fd2 != 2) {
    mlog(LOG_CRIT, "%s: unexpected fd's %d %d %d", __func__, fd0, fd1, fd2);
    exit(1);
  }

  /* initialize syslog */
  mopenlog(basename(progname), LOG_PID|LOG_NDELAY);
}

/* print usage */
static void
usage(int ec)
{
  fprintf(stderr,
"Usage: %s [options]\n"
"       -h           display this help message\n"
"       -v           display program version\n"
"       -c <file>    use config file <file>\n"
"       -u <user>    run as user <user> \n"
"       -p <rp>      listen to plugins at rendezvous point <rp> \n"
"       -f           run in foreground\n"
"       -d <level>   set debug level <level>\n"
"       -D           set max debug level\n",
          progname);
  exit(ec);
}

int
main(int argc, char *argv[])
{
  int dodaemon = 1;
  config_t *config;

  progname = argv[0];

  /*
   * argument parsing
   */
  {
    int c;

    while ((c = getopt(argc, argv, ":hvc:d:fDu:p:")) != -1) {
      switch (c) {
      case 'c':		/* config file */
        opt_c = optarg;
        break;

      case 'd':		/* debug */
        opt_d = optarg;
        break;
      case 'f':		/* foreground */
        dodaemon = 0;
        break;
      case 'D':		/* max debug + foreground */
        dodaemon = 0;
        opt_d = "debug8";
        break;

      case 'u':		/* user */
        opt_u = optarg;
        break;

      case 'p':		/* plugin rendezvous point */
        opt_p = optarg;
        break;

      case 'h':		/* help */
        usage(0);
      case 'v':		/* version */
        (void) fprintf(stderr, "%s version %s\n", basename(progname), VERSION);
        exit(0);
      case ':':		/* missing arg */
        usage(1);
      case '?':
        (void) fprintf(stderr, "%s: unknown option -%c\n", progname, optopt);
        exit(1);
      }
    }
    if (argc > optind)
      usage(1);
  } /* argument parsing */

  config = config_init();
  cardops_init();
  sig_init();
  if (config->state.mode == SMODE_LOCAL)
    lock_init();
  else
    nonce_init();

  if (dodaemon)
    daemonize(config->s);
  mlog(LOG_NOTICE, "%s %s starting", basename(progname), VERSION);

  /* all threads should be detached */
  (void) pthread_attr_init(&attr_detached);
  (void) pthread_attr_setdetachstate(&attr_detached, PTHREAD_CREATE_DETACHED);

  /* start failover threads */
  {
    gsmd_t *g = NULL;
    pthread_t tid;

    while ((g = gsmd_next(g)))
      xpthread_create(&tid, &attr_detached, failover_thread, g);
  }

#ifdef USE_SOCKET
  /* start the accept thread */
  {
    pthread_t tid;

    xpthread_create(&tid, &attr_detached, accept_thread, config);
  }
#endif /* USE_SOCKET */

  /* do nothing */
  for (;;) {
    sigset_t set;
    int sig;

    /* unblock all signals */
    (void) sigemptyset(&set);
    (void) pthread_sigmask(SIG_SETMASK, &set, NULL);

    /* small race: could get a signal between sigmask() and sigwait() */
    (void) sigfillset(&set);
    /* wait for any signal that would cause an exit */
    (void) sigwait(&set, &sig);	

    mlog(LOG_NOTICE, "%s %s stopping on signal %d",
         basename(progname), VERSION, sig);
    /* exit with correct code and possibly coredump */
    (void) raise(sig);
  }

  /*NOTREACHED*/
  return 0;
}
