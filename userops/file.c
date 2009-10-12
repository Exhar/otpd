/*
 * $Id: //trid/release/otpd-3.1.0/userops/file.c#1 $
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
 * Copyright 2001,2002  Google, Inc.
 * Copyright 2005-2008 TRI-D Systems, Inc.
 */

#include "../ident.h"
RCSID("$Id: //trid/release/otpd-3.1.0/userops/file.c#1 $")

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "../extern.h"
#include "../cardops.h"
#include "file.h"

/*
 * allocate and fill in user from our database (key file)
 * returns 0 on success, -1 for user not found, -2 for other errors.
 */
static int
file_get(const char *username, user_t **user, const config_t *config,
         __unused__ time_t now)
{
  FILE *fp;
  char s[160];
  char *p, *q;
  struct stat st;
  size_t l;
  const char *passwd = config->file.passwd;
  user_t *tuser;	/* tmp user while filling in fields */

  int found;

  *user = NULL;		/* so put() can be called safely */

  /* Verify permissions first. */
  if (stat(passwd, &st) != 0) {
    mlog(LOG_ERR, "%s: stat(%s): %s", __func__, passwd, strerror(errno));
    return -2;
  }
  if ((st.st_mode & (S_IXUSR|S_IRWXG|S_IRWXO)) != 0) {
    mlog(LOG_ERR, "%s: %s: loose permissions", __func__, passwd);
    return -2;
  }

  if ((fp = fopen(passwd, "r")) == NULL) {
    mlog(LOG_ERR, "%s: fopen(%s): %s", __func__, passwd, strerror(errno));
    return -2;
  }

  /* Find the requested user. */
  found = 0;
  l = strlen(username);
  while (!feof(fp)) {
    if (fgets(s, sizeof(s), fp) == NULL) {
      if (!feof(fp)) {
        mlog(LOG_ERR, "%s: read(%s): %s", __func__, passwd, strerror(errno));
        (void) fclose(fp);
        return -2;
      }
    } else if (!strncmp(s, username, l) && s[l] == ':') {
      found = 1;
      break;
    }
  }
  (void) fclose(fp);
  if (!found) {
    mlog(LOG_NOTICE, "%s: %s: [%s] not found", __func__, passwd, username);
    return -1;
  }

  /* Found him, allocate a user_t */
  tuser = xmalloc(sizeof(*tuser));
  tuser->username = username;

  /* skip to next field (card) */
  if ((p = strchr(s, ':')) == NULL) {
    mlog(LOG_ERR, "%s: %s: invalid format for [%s]",
         __func__, passwd, username);
    free(tuser);
    return -2;
  }
  p++;
  if ((q = strchr(p, ':')) == NULL) {
    mlog(LOG_ERR, "%s: %s: invalid format for [%s]",
         __func__, passwd, username);
    free(tuser);
    return -2;
  }
  *q++ = '\0';
  /* p: card_type, q: key */

  /*
   * Unfortunately, we can't depend on having strl*, which would allow
   * us to check for buffer overflow and copy the string in the same step.
   * TODO: implement our own strlcpy().
   */
  if (strlen(p) > OTP_MAX_CARDNAME_LEN)
    mlog(LOG_ERR, "%s: %s: invalid format (tokenid) for [%s]",
         __func__, passwd, username);
  (void) strcpy(tuser->card, p);

  p = q;
  /* optional PIN field */
  if ((q = strchr(p, ':')) == NULL)
    tuser->pinstring[0] = '\0';
  else
    *q++ = '\0';
  /* p: key, q: PIN */

  l = strlen(p);
  /* OTP_MAX_KEY_LEN keys with trailing newline won't work */
  if (l > OTP_MAX_KEY_LEN * 2) {
    mlog(LOG_ERR, "%s: %s: invalid format (key) for [%s]",
         __func__, passwd, username);
    free(tuser);
    return -2;
  }
  (void) strcpy(tuser->keystring, p);
  /* strip possible trailing newline */
  if (l && tuser->keystring[l - 1] == '\n')
    tuser->keystring[--l] = '\0';
  /* check for empty key or odd len */
  if (!l || l & 1) {
    mlog(LOG_ERR, "%s: %s: invalid format (key) for [%s]",
         __func__, passwd, username);
    free(tuser);
    return -2;
  }

  if (q) {
    l = strlen(q);
    /* strip possible trailing newline */
    if (l && q[l - 1] == '\n')
      q[--l] = '\0';
    if ((l > OTP_MAX_PIN_LEN * 2) || (l & 1)) {
      mlog(LOG_ERR, "%s: %s: invalid format (PIN) for [%s]",
           __func__, passwd, username);
      free(tuser);
      return -2;
    }
    (void) strcpy(tuser->pinstring, q);
  }

  (void) strcpy(tuser->gsmd, config->file.gsmd); /* gsmd location          */
  tuser->timeissued = 0;	/* card issue time not stored in file db   */
  tuser->password = NULL;	/* static password override not in file db */
  tuser->rwindow_size = 0;	/* per-user rwindow not avail in file db   */
  tuser->encryptmode = config->file.encryptmode;
  tuser->keyid = config->file.keyid;

  *user = tuser;
  return 0;
} /* file_get() */

/* free the user_t allocated in get() */
static void
file_put(user_t *user)
{
  free(user);
}

static void
file_init1(const config_t *config)
{
  return;
}

/* constructor */
void
userops_file_init(void)
{
  userops[OTPD_USEROPS_FILE].init1 = file_init1;
  userops[OTPD_USEROPS_FILE].get   = file_get;
  userops[OTPD_USEROPS_FILE].put   = file_put;
}
