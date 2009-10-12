/*
 * $Id: //trid/release/otpd-3.1.0/otpauth.c#1 $
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
 * Copyright 2006,2007 TRI-D Systems, Inc.
 */

#include "ident.h"
RCSID("$Id: //trid/release/otpd-3.1.0/otpauth.c#1 $")

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/un.h>
#include <unistd.h>

#include "otp.h"

#define MAX_ERRLIST 7
static char *errlist[MAX_ERRLIST+1] = {
  "ok",
  "user unknown",
  "authinfo unavailable",
  "authentication error",
  "max tries exceeded",
  "service error",
  "next passcode required",
  "initial PIN error"
};

extern char *optarg;
extern int optind, opterr, optopt;

static void usage(void);
static int verify(const char *, const char *, const char *);
static int otp_verify(const char *, const otp_request_t *, otp_reply_t *);
static int otp_connect(const char *);
static int otp_read(int, char *, size_t);
static int otp_write(int, const char *, size_t);

static void
usage()
{
  (void) fprintf(stderr, "Usage: otpauth -u user -p password -s socket\n");
  exit(1);
}

int
main(int argc, char *argv[])
{
  char *username = NULL;
  char *password = NULL;
  char *otpd_socket = NULL;
  int rc;

  {
    int c;

    while ((c = getopt(argc, argv, ":u:p:s:")) != -1 ) {
      switch (c) {
      case 'u': username    = optarg; break;
      case 'p': password    = optarg; break;
      case 's': otpd_socket = optarg; break;
      case ':':
      case '?':
        usage();
      }
    }
    if (argc > optind)
      usage();
  }
  if (!username || !password || !otpd_socket)
    usage();

  rc = verify(otpd_socket, username, password);
  {
    char *errtext;

    if (rc < 0 || rc > MAX_ERRLIST)
      errtext = "invalid return code";
    else
      errtext = errlist[rc];
    printf("%d (%s)\n", rc, errtext);
  }

  exit(rc);
}

/*
 * Test for passcode validity by asking otpd.
 */
static int
verify(const char *otpd_socket, const char *username, const char *passcode)
{
  otp_request_t	otp_request;
  otp_reply_t	otp_reply;

  if (strlen(username) > OTP_MAX_USERNAME_LEN) {
    fprintf(stderr, "otpauth: username [%s] too long\n", username);
    return OTP_RC_AUTH_ERR;
  }
  /* we already know challenge is short enough */

  otp_request.version = 2;
  (void) strcpy(otp_request.username, username);
  otp_request.challenge[0] = '\0';
  otp_request.pwe.pwe = PWE_PAP;

  if (strlen(passcode) > OTP_MAX_PASSCODE_LEN) {
    fprintf(stderr, "otpauth: passcode for [%s] too long\n", username);
    return OTP_RC_AUTH_ERR;
  }
  (void) strcpy(otp_request.pwe.u.pap.passcode, passcode);

  /* last byte must also be a terminator so otpd can verify length easily */
  otp_request.username[OTP_MAX_USERNAME_LEN] = '\0';
  otp_request.challenge[OTP_MAX_CHALLENGE_LEN] = '\0';
  otp_request.pwe.u.pap.passcode[OTP_MAX_PASSCODE_LEN] = '\0';

  otp_request.allow_sync = 1;
  otp_request.allow_async = 0;
  otp_request.challenge_delay = 0;
  otp_request.resync = 1;

  return otp_verify(otpd_socket, &otp_request, &otp_reply);
}

/*
 * Verify an otp by asking otpd.
 * Returns an OTP_* code, or -1 on system failure.
 * Fills in reply.
 */
static int
otp_verify(const char *otpd_socket, const otp_request_t *request,
           otp_reply_t *reply)
{
  int fd, rc;

  if ((fd = otp_connect(otpd_socket)) == -1)
    return -1;

  if ((rc = otp_write(fd, (const char *) request, sizeof(*request))) != 0)
      return -1;

  if ((rc = otp_read(fd, (char *) reply, sizeof(*reply))) != sizeof(*reply))
      return -1;

  (void) close(fd);

  /* validate the reply */
  if (reply->version != 1) {
    fprintf(stderr, "otpauth: otpd reply for [%s] invalid "
                       "(version %d != 1)\n",
           request->username, reply->version);
    return -1;
  }

  if (reply->passcode[OTP_MAX_PASSCODE_LEN] != '\0') {
    fprintf(stderr, "otpauth: otpd reply for [%s] invalid (passcode)\n",
           request->username);
    return -1;
  }

  return reply->rc;
}

/* connect to otpd and return fd */
static int
otp_connect(const char *path)
{
  int fd;
  struct sockaddr_un sa;
  size_t sp_len;		/* sun_path length (strlen) */

  /* setup for unix domain socket */
  sp_len = strlen(path);
  if (sp_len > sizeof(sa.sun_path) - 1) {
    fprintf(stderr, "otpauth: rendezvous point name too long\n");
    return -1;
  }
  sa.sun_family = AF_UNIX;
  (void) strcpy(sa.sun_path, path);
    
  /* connect to otpd */
  if ((fd = socket(PF_UNIX, SOCK_STREAM, 0)) == -1) {
    fprintf(stderr, "otpauth: socket: %s\n", strerror(errno));
    return -1;
  }
  if (connect(fd, (struct sockaddr *) &sa,
              sizeof(sa.sun_family) + sp_len) == -1) {
    fprintf(stderr, "otpauth: connect(%s): %s\n", path, strerror(errno));
    (void) close(fd);
    return -1;
  }
  return fd;
}

/*
 * Full read with logging, and close on failure.
 * Returns nread on success, -1 on failures.
 */
static int
otp_read(int fd, char *buf, size_t len)
{
  ssize_t n;
  size_t nread = 0;	/* bytes read into buf */

  while (nread < len) {
    if ((n = read(fd, &buf[nread], len - nread)) == -1) {
      if (errno == EINTR) {
        continue;
      } else {
        fprintf(stderr, "otpauth: read: %s\n", strerror(errno));
        (void) close(fd);
        return -1;
      }
    }
    if (!n) {
      fprintf(stderr, "otpauth: disconnect\n");
      (void) close(fd);
      return -1;
    }
    nread += n;
  } /* while (more to read) */

  return nread;
}

/*
 * Full write with logging, and close on failure.
 * Returns 0 on success, -1 on failure.
 */
static int
otp_write(int fd, const char *buf, size_t len)
{
  size_t nleft = len;
  ssize_t nwrote;

  while (nleft) {
    if ((nwrote = write(fd, &buf[len - nleft], nleft)) == -1) {
      if (errno == EINTR || errno == EPIPE) {
        continue;
      } else {
        fprintf(stderr, "otpauth: write: %s\n", strerror(errno));
        (void) close(fd);
        return -1;
      }
    }
    nleft -= nwrote;
  }

  return 0;
}
