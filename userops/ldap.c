/*
 * $Id: //trid/release/otpd-3.1.0/userops/ldap.c#1 $
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

#include "../ident.h"
RCSID("$Id: //trid/release/otpd-3.1.0/userops/ldap.c#1 $")

#include <errno.h>
#include <lber.h>
#include <ldap.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "../extern.h"
#include "../cardops.h"
#include "ldap.h"

static char *otpAttrs[13] = {
  "otpVendor",
  "otpModel",
  "otpKey1",
  "otpIPIN",
  "otpPIN",
  "otpTimeIssued",
  "otpStateLocationHint",
  "otpKeyEncryption",
  "otpTmpStaticPassword",
  "otpTmpStaticPasswordExpiry",
  "otpTmpRWindow",
  "otpTmpRWindowExpiry",
  NULL
};

static int		scope;
static struct berval	ccred;
static struct timeval	timeout;

static ldap_ld_t *ldp_head;
static pthread_mutex_t ldp_head_mutex = PTHREAD_MUTEX_INITIALIZER;

/*
 * fill in user from our database (ldap)
 * returns 0 on success, -1 for user not found, -2 for other errors.
 */
static int
ldap_get(const char *username, user_t **user, const config_t *config,
         time_t now)
{
  ldap_ld_t	*ldp;
  int		unbind = 0;
  int		rc = -2;
  int		ldaprc;
  hrtime_t	hrlast = xgethrtime();

  char			*filter;
  LDAPMessage		*result, *e;
  struct timeval	tv = timeout;	/* (tv) config->timeout */

  /* aliases for ldap returned values */
  struct berval **otpVendor = NULL;
  struct berval **otpModel  = NULL;
  struct berval **otpKey1   = NULL;
  struct berval **otpIPIN   = NULL;
  struct berval **otpPIN    = NULL;
  struct berval **otpTimeIssued        = NULL;
  struct berval **otpStateLocationHint = NULL;
  struct berval **otpKeyEncryption     = NULL;
  struct berval **otpTmpStaticPassword       = NULL;
  struct berval **otpTmpStaticPasswordExpiry = NULL;
  struct berval **otpTmpRWindow              = NULL;
  struct berval **otpTmpRWindowExpiry        = NULL;
  int encryptmode = EMODE_CLEAR;
  unsigned long keyid = 0;
  unsigned char h[20];	/* password hash */

  *user = NULL;		/* so ldap_put() can be called safely */

  filter = xmalloc(config->ldap.filter.len + strlen(username) + 1);
  (void) sprintf(filter, config->ldap.filter.val, username);

again:
  ldp = getldp(config, &hrlast, &tv);
  if (!ldp->ld)
    goto out_filter;

  /* search */
  ldaprc = ldap_search_ext_s(ldp->ld, config->ldap.basedn, scope, filter,
                             otpAttrs, 0, NULL, NULL,
                             &tv, 2 /* max #entries returned */, &result);
  if (ldaprc != LDAP_SUCCESS) {
    if (ldaprc == LDAP_SERVER_DOWN && ldp->used) {
      /*
       * This connection has been used successfully but now reports
       * server down.  This is probably due to an EPIPE when sending
       * the search request, which is probably due to an LDAP server
       * restart.  Try to get a new ldp.
       */
      putldp(ldp, 1);
      /* recalculate timeout */
      if ((ldaprc = update_timeout(&tv, &hrlast))) {
        unbind = 1;
        goto out_filter;
      }
      goto again;
    }
    mlog(LOG_ERR, "%s: ldap_search_ext_s: %s", __func__,
         ldap_err2string(ldaprc));
    unbind = 1;
    goto out_filter;
  }
  ldp->used = 1;

  /* make sure just one result */
  {
    int n;

    if ((n = ldap_count_entries(ldp->ld, result)) != 1) {
      if (n == 0) {
        mlog(LOG_NOTICE, "%s: [%s] not found", __func__, username);
        rc = -1;
      } else {
        mlog(LOG_NOTICE, "%s: [%s] multiple results", __func__, username);
      }
      goto out_result;
    }
  }

  /* extract attrs */
  e = ldap_first_entry(ldp->ld, result);
  {
    char *a;
    struct berval **v;
    BerElement *ber;

    for (a = ldap_first_attribute(ldp->ld, e, &ber); a;
         a = ldap_next_attribute(ldp->ld, e, ber)) {
      if (!(v = ldap_get_values_len(ldp->ld, e, a))) {
        /* attribute exists, but no value? */
        mlog(LOG_ERR, "%s: ldap_get_values_len: NULL", __func__);
        goto out_val;
      }
      if (ldap_count_values_len(v) != 1) {
        mlog(LOG_ERR, "%s: attribute %s not SINGLE-VALUEd for [%s]",
             __func__, a, username);
        goto out_val;
      }
      mlog(LOG_DEBUG3, "%s: user '%s', attribute %s: %s",
           __func__, username, a, (*v[0]).bv_val);

      if (!strcmp(&a[3], "Vendor")) {
        otpVendor = v;
      } else if (!strcmp(&a[3], "Model")) {
        otpModel = v;
      } else if (!strcmp(&a[3], "Key1")) {
        otpKey1 = v;
      } else if (!strcmp(&a[3], "IPIN")) {
        otpIPIN = v;
      } else if (!strcmp(&a[3], "PIN")) {
        otpPIN = v;
      } else if (!strcmp(&a[3], "TimeIssued")) {
        otpTimeIssued = v;
      } else if (!strcmp(&a[3], "StateLocationHint")) {
        otpStateLocationHint = v;
      } else if (!strcmp(&a[3], "KeyEncryption")) {
        otpKeyEncryption = v;
      } else if (!strcmp(&a[3], "TmpStaticPassword")) {
        otpTmpStaticPassword = v;
      } else if (!strcmp(&a[3], "TmpStaticPasswordExpiry")) {
        otpTmpStaticPasswordExpiry = v;
      } else if (!strcmp(&a[3], "TmpRWindow")) {
        otpTmpRWindow = v;
      } else if (!strcmp(&a[3], "TmpRWindowExpiry")) {
        otpTmpRWindowExpiry = v;
      } else {
        /* can't happen */
        mlog(LOG_ERR, "%s: unrequested attribute %s returned for [%s]",
             __func__, a, username);
        ldap_value_free_len(v);
        goto out_val;
      }
      ldap_memfree(a);
    }
    if (ber)
      ber_free(ber, 0);
  }

  /* sanity checks */
  if (!otpVendor || !otpModel || !otpKey1) {
    mlog(LOG_ERR, "%s: required attributes missing for [%s]", __func__,
         username);
    goto out_val;
  }
  if (strlen((*otpVendor[0]).bv_val) + 1 /* '-' */ +
      strlen((*otpModel[0]).bv_val) > OTP_MAX_CARDNAME_LEN) {
    mlog(LOG_ERR, "%s: invalid format (tokenid) for [%s]", __func__, username);
    goto out_val;
  }
  {
    size_t l;

    if ((l = strlen((*otpKey1[0]).bv_val)) > OTP_MAX_KEY_LEN * 2) {
      mlog(LOG_ERR, "%s: invalid format (key1) for [%s]", __func__, username);
      goto out_val;
    }
    if (!l || l & 1) {
      mlog(LOG_ERR, "%s: invalid format (key1) for [%s]", __func__, username);
      goto out_val;
    }
  }
  if (otpIPIN) {
    size_t l = strlen((*otpIPIN[0]).bv_val);

    if ((l > OTP_MAX_PIN_LEN * 2) || (l & 1)) {
      mlog(LOG_ERR, "%s: invalid format (IPIN) for [%s]", __func__, username);
      goto out_val;
    }
  }
  if (otpPIN) {
    size_t l = strlen((*otpPIN[0]).bv_val);

    if ((l > OTP_MAX_PIN_LEN * 2) || (l & 1)) {
      mlog(LOG_ERR, "%s: invalid format (PIN) for [%s]", __func__, username);
      goto out_val;
    }
  }
  if (otpStateLocationHint) {
    if (strlen((*otpStateLocationHint[0]).bv_val) > GSM_MAX_NAME_LEN) {
      mlog(LOG_ERR, "%s: invalid format (StateLocationHint) for [%s]",
           __func__, username);
      goto out_val;
    }
  }

  /* validate/convert otpKeyEncryption */
  if (otpKeyEncryption) {
    char *val;
    int l, i;

    val = (*otpKeyEncryption[0]).bv_val;
    l = strlen(val);

    if (!strcmp(val, "clear")) {
      encryptmode = EMODE_CLEAR;
    } else if (!strcmp(val, "pin")) {
      encryptmode = EMODE_PIN;
    } else if (!strcmp(val, "pin-md5")) {
      encryptmode = EMODE_PINMD5;
    } else {
      /*
       * We can't just call strtoul() because '25b' would become '25'
       * and we don't consider that valid.  We'd have to do some setup
       * to catch all cases of bad/incomplete conversion returned by
       * strtoul() so we are better off just running through the string.
       */
      for (i = 0; i < l; ++i)
        if (val[i] < '0' || val[i] > '9')
          break;
      if (i == l) {
        errno = 0;
        keyid = strtoul(val, NULL, 10);
        if (!(keyid == 0 && errno == EINVAL)) {
          if (keyid < config->nkeys && config->key[keyid]) {
            encryptmode = EMODE_KEYID;
          } else {
            mlog(LOG_ERR, "%s: keyid %lu for [%s] not configured",
                 __func__, keyid, username);
            goto out_val;
          }
        } else {
          mlog(LOG_ERR, "%s: keyid %s for [%s] invalid",
               __func__, val, username);
          goto out_val;
        } /* else (did not parse keyid) */
      } else {
        mlog(LOG_ERR, "%s: keyid %s for [%s] invalid",
             __func__, val, username);
        goto out_val;
      } /* else (val is not all digits) */
    } /* else (not pin mode) */
  } /* if (otpKeyEncryption) */

  /* validate static password */
  if (otpTmpStaticPassword) {
    char *p = (*otpTmpStaticPassword[0]).bv_val;

    if (!otpTmpStaticPasswordExpiry) {
      mlog(LOG_ERR, "%s: otpTmpStaticPassword for [%s] invalid (no expiry)",
           __func__, username);
      goto out_val;
    }
    if ((time_t) strtoul((*otpTmpStaticPasswordExpiry[0]).bv_val, NULL,
                         10) < now) {
      mlog(LOG_DEBUG1, "%s: otpTmpStaticPassword for [%s] expired",
           __func__, username);
      ldap_value_free_len(otpTmpStaticPassword);
      otpTmpStaticPassword = NULL;
      ldap_value_free_len(otpTmpStaticPasswordExpiry);
      otpTmpStaticPasswordExpiry = NULL;
      goto static_done;
    }

    if (p[0] == '{') {
      if (strncmp(p, "{MD5}", 5)) {
        p += 5;
        if (strlen(p) != 32 || a2nx(p, h, 16) != 16) {
          mlog(LOG_ERR, "%s: otpTmpStaticPassword for [%s] invalid",
               __func__, username);
          goto out_val;
        }
      } else if (strncmp(p, "{SHA}", 5)) {
        p += 5;
        if (strlen(p) != 40 || a2nx(p, h, 20) != 20) {
          mlog(LOG_ERR, "%s: otpTmpStaticPassword for [%s] invalid",
               __func__, username);
          goto out_val;
        }
      } else if (strncmp(p, "{SHA1}", 6)) {
        p += 6;
        if (strlen(p) != 40 || a2nx(p, h, 20) != 20) {
          mlog(LOG_ERR, "%s: otpTmpStaticPassword for [%s] invalid",
               __func__, username);
          goto out_val;
        }
      } else {
        mlog(LOG_ERR, "%s: otpTmpStaticPassword for [%s] invalid",
             __func__, username);
        goto out_val;
      }
    } /* if (password is encoded) */
  } /* if (otpTmpStaticPassword) */

static_done:
  if (otpTmpRWindow) {
    if (!otpTmpRWindow) {
      mlog(LOG_ERR, "%s: otpTmpRWindow for [%s] invalid (no expiry)",
           __func__, username);
      goto out_val;
    }
    if ((time_t) strtoul((*otpTmpRWindowExpiry[0]).bv_val, NULL, 10) < now) {
      mlog(LOG_DEBUG1, "%s: otpTmpRWindow for [%s] expired",
           __func__, username);
      ldap_value_free_len(otpTmpRWindow);
      otpTmpRWindow = NULL;
      ldap_value_free_len(otpTmpRWindowExpiry);
      otpTmpRWindowExpiry = NULL;
    }
    /* TODO: set upper bound on rwindow? */
  } /* if (otpTmpRWindow) */

  rc = 0;

  /* copy the data out */
  *user = xmalloc(sizeof(**user));
  (*user)->username = username;
  (void) sprintf((*user)->card, "%s-%s", (*otpVendor[0]).bv_val,
                 (*otpModel[0]).bv_val);
  (void) strcpy((*user)->keystring, (*otpKey1[0]).bv_val);
  if (otpIPIN)
    (void) strcpy((*user)->ipinstring, (*otpIPIN[0]).bv_val);
  else
    (*user)->ipinstring[0] = '\0';
  if (otpPIN)
    (void) strcpy((*user)->pinstring, (*otpPIN[0]).bv_val);
  else
    (*user)->pinstring[0] = '\0';
  if (otpTimeIssued) {
    (*user)->timeissued = strtoul((*otpTimeIssued[0]).bv_val, NULL, 10);
    if ((*user)->timeissued > now) {
      mlog(LOG_ERR, "%s: TimeIssued for [%s] is in the future, using 0",
           __func__, username);
      (*user)->timeissued = 0;
    }
  } else {
    (*user)->timeissued = 0;
  }
  if (otpStateLocationHint)
    (void) strcpy((*user)->gsmd, (*otpStateLocationHint[0]).bv_val);
  else
    (*user)->gsmd[0] = '\0';
  (*user)->encryptmode = encryptmode;
  (*user)->keyid       = keyid;
  if (otpTmpStaticPassword) {
    (*user)->password = xstrdup((*otpTmpStaticPassword[0]).bv_val);
    (void) memcpy((*user)->password_hash, h, sizeof(h));
  }
  else {
    (*user)->password = NULL;
  }
  if (otpTmpRWindow)
    (*user)->rwindow_size = strtoul((*otpTmpRWindow[0]).bv_val, NULL, 10);
  else
    (*user)->rwindow_size = 0;

out_val:
  /* and free the ldap values */
  ldap_value_free_len(otpVendor);
  ldap_value_free_len(otpModel);
  ldap_value_free_len(otpKey1);
  /* unfortunately, ldap_value_free_len() doesn't document if NULL is ok */
  if (otpIPIN)
    ldap_value_free_len(otpIPIN);
  if (otpPIN)
    ldap_value_free_len(otpPIN);
  if (otpTimeIssued)
    ldap_value_free_len(otpTimeIssued);
  if (otpStateLocationHint)
    ldap_value_free_len(otpStateLocationHint);
  if (otpKeyEncryption)
    ldap_value_free_len(otpKeyEncryption);
  if (otpTmpStaticPassword)
    ldap_value_free_len(otpTmpStaticPassword);
  if (otpTmpStaticPasswordExpiry)
    ldap_value_free_len(otpTmpStaticPasswordExpiry);
  if (otpTmpRWindow)
    ldap_value_free_len(otpTmpRWindow);
  if (otpTmpRWindowExpiry)
    ldap_value_free_len(otpTmpRWindowExpiry);

out_result:
  (void) ldap_msgfree(result);
out_filter:
  free(filter);
  putldp(ldp, unbind);

  return rc;
}

/* free the user_t allocated in get() */
static void
ldap_put(user_t *user)
{
  if (user->password)
    free(user->password);
  free(user);
}

/* get a bound ldap connection from the pool */
static ldap_ld_t *
getldp(const config_t *config, hrtime_t *hrlast, struct timeval *tv)
{
  ldap_ld_t *ldp;

  /* walk the connection pool looking for an available fd */
  for (ldp = ldp_head; ldp; ldp = ldp->next)
    if (!xpthread_mutex_trylock(&ldp->mutex))
      break;

  if (!ldp) {
    /* no ldp was available, add a new one */
    ldp = xmalloc(sizeof(*ldp));
    xpthread_mutex_init(&ldp->mutex, NULL);
    xpthread_mutex_lock(&ldp->mutex);
    /* insert new ldp at head */
    xpthread_mutex_lock(&ldp_head_mutex);
    ldp->next = ldp_head;
    ldp_head = ldp;
    xpthread_mutex_unlock(&ldp_head_mutex);
    ldp->ld = NULL;
  }

  /* connect and bind */
  if (!ldp->ld) {
    int ldaprc;
    int msgid;
    char *ldapfn;	/* for error reporting */
    LDAPMessage *result;

    xldap_initialize(&ldp->ld, config->ldap.url);

    if (config->ldap.tls.mode != TLSMODE_OFF) {
      /* this must be set after calling ldap_initialize() */
      if (config->ldap.tls.mode == TLSMODE_BINDONLY)
        xldap_set_option(ldp->ld, LDAP_OPT_X_SASL_SECPROPS, "maxssf=0");

      ldapfn = "ldap_start_tls";
      ldaprc = ldap_start_tls(ldp->ld, NULL, NULL, &msgid);
      /* ldap_start_tls() is undocumented but probably like ldap_sasl_bind() */
      if (msgid == -1)
        goto timeout_or_err;

      /* subtract connect() time */
      if ((ldaprc = update_timeout(tv, hrlast)))
        goto timeout_or_err;

      ldaprc = ldap_result(ldp->ld, msgid, 0, tv, &result);
      if (ldaprc == 0 || ldaprc == -1) {
        xldap_get_option(ldp->ld, LDAP_OPT_ERROR_NUMBER, &ldaprc);
        goto timeout_or_err;
      }
      if (ldaprc != LDAP_RES_EXTENDED) {
        (void) ldap_msgfree(result);
        ldaprc = LDAP_LOCAL_ERROR;	/* not sure what else to use */
        goto timeout_or_err;
      }
      if ((ldaprc = ldap_parse_extended_result(ldp->ld, result,
                                               NULL, NULL, 1)) != LDAP_SUCCESS)
        goto timeout_or_err;
      /*
       * It appears that ldap_start_tls(), ldap_result() and
       * ldap_parse_extended_result() only do part of the tls setup.
       * ldap_install_tls() (undocumented, of course) is needed to
       * complete it.  The problem is, whatever ldap_install_tls()
       * does has no timeout.  sigh.
       */
      if ((ldaprc = ldap_install_tls(ldp->ld)) != LDAP_SUCCESS)
        goto timeout_or_err;

      if ((ldaprc = update_timeout(tv, hrlast)))
        goto timeout_or_err;
    }

    ldapfn = "ldap_sasl_bind";
    /* binddn might be NULL, in that case this only connect()'s */
    ldaprc = ldap_sasl_bind(ldp->ld, config->ldap.binddn, LDAP_SASL_SIMPLE,
                            &ccred, NULL, NULL, &msgid);
    /* ldap_sasl_bind() is misdocumented */
    if (msgid == -1)
      goto timeout_or_err;

    /* subtract connect() time if we didn't do tls (tls already connected) */
    if (config->ldap.tls.mode != TLSMODE_OFF &&
        (ldaprc = update_timeout(tv, hrlast)))
      goto timeout_or_err;

    ldaprc = ldap_result(ldp->ld, msgid, 0, tv, &result);
    if (ldaprc == 0 || ldaprc == -1) {
      xldap_get_option(ldp->ld, LDAP_OPT_ERROR_NUMBER, &ldaprc);
      goto timeout_or_err;
    }
    if (ldaprc != LDAP_RES_BIND) {
      (void) ldap_msgfree(result);
      ldaprc = LDAP_LOCAL_ERROR;	/* not sure what else to use */
      goto timeout_or_err;
    }
    if ((ldaprc = ldap_parse_sasl_bind_result(ldp->ld, result, NULL, 1)) !=
        LDAP_SUCCESS)
      goto timeout_or_err;

    if ((ldaprc = update_timeout(tv, hrlast))) {
timeout_or_err:
      mlog(LOG_ERR, "%s: %s: %s", __func__, ldapfn, ldap_err2string(ldaprc));
      (void) ldap_unbind_ext_s(ldp->ld, NULL, NULL);
      ldp->ld = NULL;
    }

    ldp->used = 0;	/* new connection */
  }

  return ldp;
}

/* return an ldp to the pool */
static void
putldp(ldap_ld_t *ldp, int unbind)
{
  if (unbind) {
    (void) ldap_unbind_ext_s(ldp->ld, NULL, NULL);
    ldp->ld = NULL;
  }

  /* make connection available to another thread */
  xpthread_mutex_unlock(&ldp->mutex);
}

/*
 * update tv to subtract elapsed time
 * return 0 if time left, LDAP_TIMEOUT otherwise
 */
static int
update_timeout(struct timeval *tv, hrtime_t *hrlast)
{
  hrtime_t hret;

  /* on Solaris, gethrtime() and division will be faster than gettimeofday() */
  hret = xgethrtime() - *hrlast;
  tv->tv_sec  -= hret / 1000000000;
  tv->tv_usec -= hret % 1000000;
  if (tv->tv_usec < 0) {
    tv->tv_usec += 1000000;
    tv->tv_sec  -= 1;
  }
  *hrlast += hret;

  if ((tv->tv_sec | tv->tv_usec) < 0)
    return LDAP_TIMEOUT;
  else
    return 0;
}

/* guaranteed ldap_initialize */
static void
_xldap_initialize(LDAP **ldp, const char *url, const char *caller)
{
  int ldaprc;

  if ((ldaprc = ldap_initialize(ldp, url)) != LDAP_SUCCESS) {
    /* ldap_initalize() is undocumented but we hope it returns an error code */
    mlog(LOG_CRIT, "%s: ldap_initialize: %s", caller, ldap_err2string(ldaprc));
    exit(1);
  }
}

/* guaranteed ldap_set_option */
static void
_xldap_set_option(LDAP *ld, int option, void *optdata, const char *optname,
                  const char *caller)
{
  if (ldap_set_option(ld, option, optdata) != LDAP_SUCCESS) {
    mlog(LOG_CRIT, "%s: ldap_set_option: %s", caller, optname);
    exit(1);
  }
}

/* guaranteed ldap_get_option */
static void
_xldap_get_option(LDAP *ld, int option, void *optdata, const char *optname,
                  const char *caller)
{
  if (ldap_get_option(ld, option, optdata) != LDAP_SUCCESS) {
    mlog(LOG_CRIT, "%s: ldap_get_option: %s", caller, optname);
    exit(1);
  }
}

/*
 * some stuff we only have to do once
 * NOTE: This will have to be reworked if we support reconfig; we simply
 *       point to data in the config param, which is a static (file scope)
 *       var in config.y.  It is duplicated after config is done, then
 *       duplicated again for each work thread.  So the data we point
 *       to here is pretty old to begin with, and invalid on reconfig.
 */
static void
ldap_init1(const config_t *config)
{
  /* this can be NULL, in which case binddn is also NULL, which is ok */
  ccred.bv_val = config->ldap.bindpw.val;
  ccred.bv_len = config->ldap.bindpw.len;

  /* scope converstion to int was deferred to avoid ldap.h in config.y */
  if (!strcmp(config->ldap.scope, "sub")) {
    scope = LDAP_SCOPE_SUBTREE;
  } else if (!strcmp(config->ldap.scope, "one")) {
    scope = LDAP_SCOPE_ONELEVEL;
  } else {
    /* can't happen; config.y enforces correctness */
    mlog(LOG_CRIT, "%s: invalid scope", __func__);
    exit(1);
  }

  /* set starting timeout in global var so we do the math just once */
  {
    /* will not be rounded; orig. value is in ms converted to ns */
    int64_t usec = config->timeout / 1000;	/* ns -> us */

    timeout.tv_sec  = usec / 1000000;
    timeout.tv_usec = usec % 1000000;
  }

  /*
   * don't allow libldap to process LDAP_CONF_FILE or LDAP_USERRC_FILE
   * undocumented of course
   */
  (void) setenv("LDAPNOINIT", "1", 1);

  /*
   * These all HAVE to be set before calling ldap_initialize(),
   * or rather, with a NULL ld arg.  Calling ldap_set_option() 
   * with NULL sets global options, that are propagated to each
   * ld instance when it is initialized.  (Documented here since
   * OpenLDAP doesn't document this behavior.)  So we take advantage
   * of that fact to set these options only once.
   */
  if (config->ldap.tls.mode != TLSMODE_OFF) {
    int opt;

    if (config->ldap.tls.verify_cert) {
      opt = LDAP_OPT_X_TLS_DEMAND;
      xldap_set_option(NULL, LDAP_OPT_X_TLS_REQUIRE_CERT, &opt);
    } else {
      opt = LDAP_OPT_X_TLS_NEVER;
      xldap_set_option(NULL, LDAP_OPT_X_TLS_REQUIRE_CERT, &opt);
    }
    if (config->ldap.tls.verify_cert == 2) {
      opt = LDAP_OPT_X_TLS_CRL_ALL;
      xldap_set_option(NULL, LDAP_OPT_X_TLS_CRLCHECK, &opt);
    }

    if (config->ldap.tls.cacertfile)
      xldap_set_option(NULL, LDAP_OPT_X_TLS_CACERTFILE,
                       config->ldap.tls.cacertfile);
    if (config->ldap.tls.cacertdir)
      xldap_set_option(NULL, LDAP_OPT_X_TLS_CACERTDIR,
                       config->ldap.tls.cacertdir);
    if (config->ldap.tls.certfile)
      xldap_set_option(NULL, LDAP_OPT_X_TLS_CERTFILE,
                       config->ldap.tls.certfile);
    if (config->ldap.tls.keyfile)
      xldap_set_option(NULL, LDAP_OPT_X_TLS_KEYFILE,
                       config->ldap.tls.keyfile);
  } /* if (using tls) */

  /*
   * These can be set before or after calling ldap_initialize().
   * We set the here so we only have to do it once.
   */
  {
    int three = LDAP_VERSION3;
    xldap_set_option(NULL, LDAP_OPT_PROTOCOL_VERSION, &three);
  }
  /* connect() timeout */
  xldap_set_option(NULL, LDAP_OPT_NETWORK_TIMEOUT, &timeout);

  return;
}

/* constructor */
void
userops_ldap_init(void)
{
  userops[OTPD_USEROPS_LDAP].init1 = ldap_init1;
  userops[OTPD_USEROPS_LDAP].get   = ldap_get;
  userops[OTPD_USEROPS_LDAP].put   = ldap_put;
}
