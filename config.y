%{
/*
 * $Id: //trid/release/otpd-3.1.0/config.y#1 $
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

#include "ident.h"
RCSID("$Id: //trid/release/otpd-3.1.0/config.y#1 $")

extern int yylex(void);
extern int yyerror(char *);

#if 0
/* need this first, like LFS; should really be in CPPFLAGS but it's hard */
#if defined(__linux__) && !defined(_GNU_SOURCE)
#define _GNU_SOURCE	/* RTLD_DEFAULT */
#endif
#include <dlfcn.h>
#endif

#include "extern.h"	/* need EMODE_* early */

#define BACKEND_FILE 0
#define BACKEND_LDAP 1

static void set_user(char *, int);
static void set_log_facility(char *);
static void set_log_level(char *, int);
static void set_plugin_rp(char *, int);

/* the rest of these could be done more elegantly, but whatever */
static void set_backend(char *);
static void set_auth_timeout(int);
static void set_prepend_pin(int);
static void set_hardfail(int);
static void set_softfail(int);
static void set_ewindow_size(int);
static void set_rwindow_size(int);
static void set_site_transform(int);

static void set_file_passwd(char *);
static void set_file_encrypt(int, int);
static void set_file_server(char *);
static void set_ldap_host(char *);
static void set_ldap_port(int);
static void set_ldap_binddn(char *);
static void set_ldap_bindpw(char *);
static void set_ldap_basedn(char *);
static void set_ldap_filter(char *);
static void set_ldap_scope(char *);

static void set_tls_mode(char *);
static void set_tls_verify_cert(char *);
static void set_tls_cacertfile(char *);
static void set_tls_cacertdir(char *);
static void set_tls_certfile(char *);
static void set_tls_keyfile(char *);

static void set_passwd_keyid(int);
static void set_passwd_key(char *);

static void set_state_mode(char *);
static void set_statedir(char *);
static void set_state_timeout(int);
static void set_state_gport(int);

static void finish_server(void);
static void set_server_name(char *);
static void set_server_primary(char *);
static void set_server_backup(char *);
static void set_server_timeout(int);
static void set_server_gport(int);
static void set_server_key(char *);

static void finish_file(void);
static void finish_ldap(void);
static void finish_tls(void);
static void finish_key(void);
static void finish_state(void);
static void finish_config(void);

static void userops_init(void);
%}

%union {
  int ival;
  char *sval;
}

/* ignore shift/reduce conflicts we know of */
%expect 15

%token OTPD
%token USER LOG_FACILITY LOG_LEVEL PLUGIN_RP BACKEND TIMEOUT
%token PREPEND_PIN HARDFAIL SOFTFAIL
%token EWINDOW_SIZE RWINDOW_SIZE SITE_TRANSFORM
%token USER_FILE PASSWD ENCRYPT CLEAR PIN PINMD5 STATE_SERVER
%token USER_LDAP HOST PORT BINDDN BINDPW BASEDN FILTER SCOPE
%token LDAP_TLS MODE VERIFY_CERT CACERTFILE CACERTDIR CERTFILE KEYFILE
%token PASSWDKEY KEYID KEY
%token STATE STATEDIR GPORT SERVER NAME PRIMARY BACKUP

%token <ival> YES NO
%token <sval> WORD
%token <ival> INTEGER
%type <ival> yesno
%%

otpd_section:	/* empty */
	|	OTPD '{' '}'
	|	OTPD '{' config '}'

config:		config_item
	|	config config_item

config_item:	USER           '=' WORD		{ set_user($3, 1);        }
	|	LOG_FACILITY   '=' WORD		{ set_log_facility($3);   }
	|	LOG_LEVEL      '=' WORD		{ set_log_level($3, 1);   }
	|	PLUGIN_RP      '=' WORD		{ set_plugin_rp($3, 1);   }
	|	BACKEND        '=' WORD		{ set_backend($3);        }
	|	TIMEOUT        '=' INTEGER	{ set_auth_timeout($3);   }
	|	PREPEND_PIN    '=' yesno	{ set_prepend_pin($3);    }
	|	HARDFAIL       '=' INTEGER	{ set_hardfail($3);       }
	|	SOFTFAIL       '=' INTEGER	{ set_softfail($3);       }
	|	EWINDOW_SIZE   '=' INTEGER	{ set_ewindow_size($3);   }
	|	RWINDOW_SIZE   '=' INTEGER	{ set_rwindow_size($3);   }
	|	SITE_TRANSFORM '=' yesno	{ set_site_transform($3); }
	|	USER_FILE '{' file_config  '}'	{ finish_file();          }
	|	USER_LDAP '{' ldap_config  '}'	{ finish_ldap();          }
	|	PASSWDKEY '{' key_config   '}'	{ finish_key();           }
	|	STATE     '{' state_config '}'	{ finish_state();         }

file_config:	/* empty */
	|	file_config_item
	|	file_config file_config_item

file_config_item:	PASSWD '=' WORD		{ set_file_passwd($3);    }
	|	ENCRYPT '=' CLEAR	{ set_file_encrypt(EMODE_CLEAR, 0);  }
	|	ENCRYPT '=' PIN		{ set_file_encrypt(EMODE_PIN, 0);    }
	|	ENCRYPT '=' PINMD5	{ set_file_encrypt(EMODE_PINMD5, 0); }
	|	ENCRYPT '=' INTEGER	{ set_file_encrypt(EMODE_KEYID, $3); }
	|	STATE_SERVER '=' WORD	{ set_file_server($3);            }

ldap_config:	ldap_config_item
	|	ldap_config ldap_config_item

ldap_config_item:	HOST '=' WORD		{ set_ldap_host($3);      }
	|	PORT     '=' INTEGER		{ set_ldap_port($3);      }
	|	BINDDN   '=' WORD		{ set_ldap_binddn($3);    }
	|	BINDPW   '=' WORD		{ set_ldap_bindpw($3);    }
	|	BASEDN   '=' WORD		{ set_ldap_basedn($3);    }
	|	FILTER   '=' WORD		{ set_ldap_filter($3);    }
	|	SCOPE    '=' WORD		{ set_ldap_scope($3);     }
	|	LDAP_TLS '{' tls_config '}'	{ finish_tls();           }

tls_config:	/* empty */
	|	tls_config_item
	|	tls_config tls_config_item

tls_config_item:	MODE '=' WORD		{ set_tls_mode($3);       }
	|	VERIFY_CERT '=' WORD		{ set_tls_verify_cert($3); }
	|	CACERTFILE  '=' WORD		{ set_tls_cacertfile($3); }
	|	CACERTDIR   '=' WORD		{ set_tls_cacertdir($3);  }
	|	CERTFILE    '=' WORD		{ set_tls_certfile($3);   }
	|	KEYFILE     '=' WORD		{ set_tls_keyfile($3);    }

key_config:	key_config_item
	|	key_config key_config_item

key_config_item:	KEYID '=' INTEGER	{ set_passwd_keyid($3);   }
	|	KEY '=' WORD			{ set_passwd_key($3);     }

state_config:	/* empty */
	|	state_config_item
	|	state_config state_config_item

state_config_item:	MODE '=' WORD		{ set_state_mode($3);     }
	|	STATEDIR '=' WORD		{ set_statedir($3);       }
	|	TIMEOUT  '=' INTEGER		{ set_state_timeout($3);  }
	|	GPORT    '=' INTEGER		{ set_state_gport($3);    }
	|	PORT     '=' INTEGER		{ set_state_gport($3);    }
	|	SERVER '{' server_config '}'	{ finish_server();        }

server_config:	server_config_item
	|	server_config server_config_item

server_config_item:	NAME '=' WORD		{ set_server_name($3);    }
	|	PRIMARY '=' WORD		{ set_server_primary($3); }
	|	BACKUP  '=' WORD		{ set_server_backup($3);  }
	|	TIMEOUT '=' INTEGER		{ set_server_timeout($3); }
	|	GPORT   '=' INTEGER		{ set_server_gport($3);   }
	|	KEY     '=' WORD		{ set_server_key($3);     }

yesno:		YES | NO

%%

#include <ctype.h>
#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <locale.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/nameser.h>	/* hstrerror(), must be after netinet/in.h */
#include <pthread.h>
#include <pwd.h>
#include <resolv.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <sys/socket.h>
#include <sys/un.h>	/* sockaddr_un */
#include <sys/types.h>
#include <unistd.h>
#include <grp.h>

#include "gsm.h"

extern FILE *yyin;
extern int yylineno;

#ifdef USE_SOCKET
static size_t rp_maxlen = sizeof(((struct sockaddr_un *) 0)->sun_path) - 1;
#endif

static config_t config;

static int seen_user;		/* seen user option?           */
static int seen_log_facility;	/* seen log_facility option?   */
static int seen_log_level;	/* seen log_level option?      */
static char *seen_plugin_rp;	/* doubles as seen value       */
static int seen_backend;	/* seen backend option?        */
static int backend;		/* backend in use              */
static int seen_auth_timeout;	/* seen timeout option?        */
static int seen_prepend_pin;	/* seen prepend_pin option?    */
static int seen_hardfail;	/* seen hardfail option?       */
static int seen_softfail;	/* seen softfail option?       */
static int seen_ewindow_size;	/* seen ewindow_size option?   */
static int seen_rwindow_size;	/* seen rwindow_size option?   */
static int seen_site_transform;	/* seen site_transform option? */
static int seen_file;		/* seen file section?          */
static int seen_ldap;		/* seen ldap section?          */

static int seen_file_passwd;	/* seen file passwd option?    */
static int seen_file_encrypt;	/* seen file encrypt option?   */
static int seen_file_server;	/* seen file server option?    */
static char *seen_ldap_host;	/* doubles as seen value       */
static int seen_ldap_port;	/* doubles as seen value       */
static int seen_ldap_binddn;	/* seen ldap binddn option?    */
static int seen_ldap_bindpw;	/* seen ldap bindpw option?    */
static int seen_ldap_basedn;	/* seen ldap basedn option?    */
static int seen_ldap_filter;	/* seen ldap filter option?    */
static int seen_ldap_scope;	/* seen ldap scope option?     */

static int seen_tls_mode;	/* seen tls mode option?       */
static int seen_tls_verify_cert; /* seen tls verify_cert option? */
static int seen_tls_cacertfile;	/* seen tls cacertfile option? */
static int seen_tls_cacertdir;	/* seen tls cacertdir option?  */
static int seen_tls_certfile;	/* seen tls certfile option?   */
static int seen_tls_keyfile;	/* seen tls keyfile option?    */

static int seen_passwd_keyid;	/* seen keyid option?          */
static int seen_passwd_key;	/* seen key option?            */

static int seen_state_mode;	/* seen state mode option?     */
static int seen_statedir;	/* seen state statedir option? */
static int64_t seen_gtimeout;	/* doubles as current value    */
static int seen_gport;		/* seen state port option?     */
static char *seen_server_name;	/* doubles as current value    */
static char *seen_server_primary;	/* doubles as current value */
static char *seen_server_backup;	/* doubles as current value */
static int seen_server_timeout;	/* doubles as current value    */
static int seen_server_gport;	/* doubles as current value    */
static char *seen_server_key;	/* doubles as current value    */

static int filekeylineno;	/* deferred error reporting    */
static int gtimeoutlineno;	/* deferred error reporting    */

static int primarylineno;	/* deferred error reporting    */
static int backuplineno;	/* deferred error reporting    */
static int timeoutlineno;	/* deferred error reporting    */
static int gsmdkeylineno;	/* deferred error reporting    */

static int gport;	/* default gsmd port, network order  */
static struct {
  char	*name;
  char	*primary;
  char	*backup;
  int64_t timeout;
  int	primarylineno;	/* deferred error reporting          */
  int	backuplineno;	/* deferred error reporting          */
  int	timeoutlineno;	/* deferred error reporting          */
  int	gport;		/* holding space until port is known */
  char	*key;		/* holding space until port is known */
  int	keylineno;	/* deferred error reporting          */
} **gsmd;
static int ngsmd;	/* number of gsmd's                  */

static int using_tls;		/* using tls config            */

static int passwd_keyid;
static int passwd_keylen;
static unsigned char *passwd_key;

static int using_state;		/* state configured            */

userops_t userops[OTPD_USEROPS_MAX];	/* initialized by userops ctors */

/*
 * parse the config file and validate command line options
 * returns an allocated config_t
 */
config_t *
config_init(void)
{
  static int once = 0;	/* runs in main thread only, no lock needed */

  if (!once) {
    (void) setlocale(LC_ALL, "");	/* required for yyparse() */

    if (rp_maxlen > PATH_MAX)
      rp_maxlen = PATH_MAX;

    /*
     * verify command line options and set defaults
     * done here to keep it (validation) all in one place
     */
    if (!opt_c)
      opt_c = OTPD_DEFAULT_CF;

    if (opt_d)
      set_log_level(opt_d, 0);

    if (opt_u)
      set_user(opt_u, 0);

    if (opt_p)
      set_plugin_rp(xstrdup(opt_p), 0);

    userops_init();

    once = 1;
  } /* if (!once) */

  /* open config file */
  if (!(yyin = fopen(opt_c, "r"))) {
    mlog(LOG_CRIT, "%s: fopen(%s): %s", __func__, opt_c, strerror(errno));
    exit(1);
  }

  /* initialize defaults */
  (void) strcpy(config.file.passwd, OTPD_PASSWD);
  config.file.encryptmode  = EMODE_CLEAR;
  config.file.gsmd[0]      = '\0';
  config.ldap.url          = NULL;
  config.ldap.binddn       = NULL;
  config.ldap.bindpw.val   = NULL;
  config.ldap.basedn       = NULL;
  config.ldap.filter.val   = NULL;
  config.ldap.scope        = NULL;

  config.ldap.tls.mode        = 0;
  config.ldap.tls.verify_cert = 0;
  config.ldap.tls.cacertfile  = NULL;
  config.ldap.tls.cacertdir   = NULL;
  config.ldap.tls.certfile    = NULL;

  config.state.mode     = SMODE_LOCAL;
  (void) strcpy(config.state.statedir, OTPD_STATEDIR);

  config.timeout        = 11700000000LL;	/* 11.7s */
  config.prepend_pin    = 1;
  config.hardfail       = 0;
  config.softfail       = 5;
  config.ewindow_size   = 5;
  config.rwindow_size   = 25;
  config.site_transform = 1;
  config.userops        = NULL;

  {
    struct servent *sp;

    sp = getservbyname("gsmd", "udp");
    if (sp)
      gport = sp->s_port;
    else
      gport = htons(GSM_DEFAULT_GPORT);
  }

  /* parse config */
  seen_user           = 0;
  seen_log_facility   = 0;
  seen_log_level      = 0;
  seen_plugin_rp      = NULL;
  seen_backend        = 0;
  backend             = BACKEND_FILE;
  seen_auth_timeout   = 0;
  seen_prepend_pin    = 0;
  seen_hardfail       = 0;
  seen_softfail       = 0;
  seen_ewindow_size   = 0;
  seen_rwindow_size   = 0;
  seen_site_transform = 0;
  seen_file           = 0;
  seen_ldap           = 0;

  using_tls           = 0;
  using_state         = 0;

  seen_file_passwd    = 0;
  seen_file_encrypt   = 0;
  seen_file_server    = 0;
  seen_ldap_host      = NULL;
  seen_ldap_port      = 0;
  seen_ldap_binddn    = 0;
  seen_ldap_bindpw    = 0;
  seen_ldap_basedn    = 0;
  seen_ldap_filter    = 0;
  seen_ldap_scope     = 0;

  seen_tls_mode        = TLSMODE_OFF;
  seen_tls_verify_cert = 0;
  seen_tls_cacertfile  = 0;
  seen_tls_cacertdir   = 0;
  seen_tls_certfile    = 0;
  seen_tls_keyfile     = 0;

  seen_passwd_keyid    = 0;
  seen_passwd_key      = 0;

  seen_state_mode      = 0;
  seen_statedir        = 0;
  seen_gport           = 0;
  seen_server_name     = NULL;
  seen_server_primary  = NULL;
  seen_server_backup   = NULL;
  seen_server_gport    = 0;
  seen_server_timeout  = 0;
  seen_server_key      = NULL;
  gsmd                 = NULL;
  ngsmd                = 0;

  (void) yyparse();
  finish_config();

  /* validate interdependent parameters */
  if (config.hardfail && config.hardfail <= config.softfail) {
    /*
     * This is noise if the admin leaves softfail alone, so it gets
     * the default value of 5, and sets hardfail <= to that ... but
     * in practice that will never happen.  Anyway, it is easily
     * overcome with a softfail setting of 0.
     *
     * This is because we can't be bothered to tell the difference
     * between a default [softfail] value and an admin-configured one.
     */
    mlog(LOG_ERR, "%s: hardfail (%d) is less than softfail (%d), "
                  "effectively disabling softfail",
         opt_c, config.hardfail, config.softfail);
  }

  if (config.rwindow_size && (config.rwindow_size < config.ewindow_size)) {
    config.rwindow_size = 0;
    mlog(LOG_ERR, "%s: rwindow_size must be at least as large as "
                  "ewindow_size, disabling", opt_c);
  }

  /* config is ok */
  {
    config_t *inst;

    inst = xmalloc(sizeof(config));
    (void) memcpy(inst, &config, sizeof(config));
    return inst;
  }
} /* config_init() */

static void
set_user(char *arg, int config_file)
{
  unsigned i;
  int isuid = 1;	/* is this a uid (or name) */
  struct passwd *pw;

  if (config_file) {
    if (seen_user) {
      mlog(LOG_NOTICE,
           "%s:%d: user option already set, ignoring", opt_c, yylineno);
      free(arg);
      return;
    }
    seen_user = 1;
  }

  if (config_file && opt_u) {
    mlog(LOG_INFO, "%s:%d: -u given on command line, ignoring user option",
         opt_c, yylineno);
    free(arg);
    return;
  }

  /* if arg is all digits, this is a uid */
  for (i = 0; i < strlen(arg); ++i) {
    if (!isdigit(arg[i])) {
      isuid = 0;
      break;
    }
  }

  errno = 0;	/* getpw* returns NULL on both not found and error */
  if (isuid)
    pw = getpwuid(atoi(arg));
  else
    pw = getpwnam(arg);

  if (!pw) {
    if (errno) {
      if (config_file)
        mlog(LOG_CRIT, "%s:%d: user lookup error: %s",
             opt_c, yylineno, strerror(errno));
      else
        mlog(LOG_CRIT, "-u: user lookup error: %s", strerror(errno));
    } else {
      if (config_file)
        mlog(LOG_CRIT, "%s:%d: user unknown", opt_c, yylineno);
      else
        mlog(LOG_CRIT, "-u: user unknown");
    }
    exit(1);
  }

  if (config_file)
    free(arg);

  /* setgid() first, while we have privs */
  xsetgid(pw->pw_gid);
  (void) setgroups(0, NULL);
  xsetuid(pw->pw_uid);
}

#ifndef LOG_AUTHPRIV
#define LOG_AUTHPRIV LOG_AUTH
#endif
#ifndef LOG_FTP
#define LOG_FTP LOG_DAEMON
#endif
#ifndef LOG_AUDIT
#define LOG_AUDIT LOG_AUTH
#endif

static struct {
  const char *name;
  int facility;
} fac_names[] = {
  { .name = "user",	.facility = LOG_USER     },
  { .name = "mail",	.facility = LOG_MAIL     },
  { .name = "daemon",	.facility = LOG_DAEMON   },
  { .name = "ftp",	.facility = LOG_FTP      },
  { .name = "auth",	.facility = LOG_AUTH     },
  { .name = "authpriv",	.facility = LOG_AUTHPRIV },
  { .name = "lpr",	.facility = LOG_LPR      },
  { .name = "news",	.facility = LOG_NEWS     },
  { .name = "uucp",	.facility = LOG_UUCP     },
  { .name = "cron",	.facility = LOG_CRON     },
  { .name = "audit",	.facility = LOG_AUDIT    },
  { .name = "local0",	.facility = LOG_LOCAL0   },
  { .name = "local1",	.facility = LOG_LOCAL1   },
  { .name = "local2",	.facility = LOG_LOCAL2   },
  { .name = "local3",	.facility = LOG_LOCAL3   },
  { .name = "local4",	.facility = LOG_LOCAL4   },
  { .name = "local5",	.facility = LOG_LOCAL5   },
  { .name = "local6",	.facility = LOG_LOCAL6   },
  { .name = "local7",	.facility = LOG_LOCAL7   },
  { }
};

/* set log_facility option */
static void
set_log_facility(char *arg)
{
  int i, facility = -1;

  if (seen_log_facility) {
    mlog(LOG_NOTICE,
         "%s:%d: log_facility option already set, ignoring", opt_c, yylineno);
    return;
  }
  seen_log_facility = 1;

  /* convert facility name to int */
  for (i = 0; fac_names[i].name; ++i) {
    if (!strcmp(fac_names[i].name, arg)) {
      facility = fac_names[i].facility;
      break;
    }
  }

  if (facility >= 0) {
    log_facility = facility;
    return;
  }

  /* no match */
  mlog(LOG_CRIT, "%s:%d: invalid log_facility", opt_c, yylineno);
  exit(1);
}

static struct {
  const char *name;
  int priority;
} pri_names[] = {
  { .name = "emerg",	.priority = LOG_EMERG   },
  { .name = "alert",	.priority = LOG_ALERT   },
  { .name = "crit",	.priority = LOG_CRIT    },
  { .name = "err",	.priority = LOG_ERR     },
  { .name = "warning",	.priority = LOG_WARNING },
  { .name = "notice",	.priority = LOG_NOTICE  },
  { .name = "info",	.priority = LOG_INFO    },
  { .name = "debug",	.priority = LOG_DEBUG   },
  { .name = "debug0",	.priority = LOG_DEBUG0  },
  { .name = "debug1",	.priority = LOG_DEBUG1  },
  { .name = "debug2",	.priority = LOG_DEBUG2  },
  { .name = "debug3",	.priority = LOG_DEBUG3  },
  { .name = "debug4",	.priority = LOG_DEBUG4  },
  { .name = "debug5",	.priority = LOG_DEBUG5  },
  { .name = "debug6",	.priority = LOG_DEBUG6  },
  { .name = "debug7",	.priority = LOG_DEBUG7  },
  { .name = "debug8",	.priority = LOG_DEBUG8  },
  { }
};

/* set log_level option */
static void
set_log_level(char *arg, int config_file)
{
  int i, level = -1;

  if (config_file) {
    if (seen_log_level) {
      mlog(LOG_NOTICE,
           "%s:%d: log_level option already set, ignoring", opt_c, yylineno);
      free(arg);
      return;
    }
    seen_log_level = 1;
  }

  /* convert priority name to int */
  for (i = 0; pri_names[i].name; ++i) {
    if (!strcmp(pri_names[i].name, arg)) {
      level = pri_names[i].priority;
      break;
    }
  }
  if (config_file)
    free(arg);

  if (level >= 0) {
    if (config_file && opt_d)
      mlog(LOG_INFO,
           "%s:%d: -d given on command line, ignoring log_level option",
           opt_c, yylineno);
    else
      log_level = level;
    return;
  }

  /* no match */
  if (config_file)
    mlog(LOG_CRIT, "%s:%d: invalid log_level", opt_c, yylineno);
  else
    mlog(LOG_CRIT, "-d: invalid log_level");
  exit(1);
}

/* set plugin_rp option */
static void
set_plugin_rp(char *arg, int config_file)
{
  if (config_file) {
    if (seen_plugin_rp) {
      mlog(LOG_NOTICE,
           "%s:%d: plugin_rp option already set, ignoring", opt_c, yylineno);
      free(arg);
      return;
    }
  }

  if (arg[0] != '/') {
    mlog(LOG_CRIT, "%s:%d: plugin_rp name not absolute", opt_c, yylineno);
    exit(1);
  }

  if (strlen(arg) > rp_maxlen) {
    if (config_file)
      mlog(LOG_CRIT, "%s:%d: plugin_rp name too long", opt_c, yylineno);
    else
      mlog(LOG_CRIT, "-p: plugin_rp name too long");
    exit(1);
  }

  /* could have done this earlier, but we always want the sanity checks */
  if (config_file && opt_p) {
    mlog(LOG_INFO, "%s:%d: -p given on command line, ignoring plugin_rp option",
         opt_c, yylineno);
    free(arg);
    return;
  }

  seen_plugin_rp = arg;
}

static void
set_backend(char *arg)
{
  if (seen_backend) {
    mlog(LOG_NOTICE,
        "%s:%d: backend option already set, ignoring", opt_c, yylineno);
    return;
  }
  seen_backend = 1;

  if (!strcmp(arg, "file")) {
    backend = BACKEND_FILE;
  } else if (!strcmp(arg, "ldap")) {
    backend = BACKEND_LDAP;
  } else {
    mlog(LOG_CRIT, "%s:%d: invalid backend option", opt_c, yylineno);
    exit(1);
  }
}

/* set timeout option */
static void
set_auth_timeout(int arg)
{
  if (seen_auth_timeout) {
    mlog(LOG_NOTICE,
        "%s:%d: timeout option already set, ignoring", opt_c, yylineno);
    return;
  }
  seen_auth_timeout = 1;

  if (arg < 3000) {
    mlog(LOG_CRIT, "%s:%d: minimum timeout is 3000", opt_c, yylineno);
    exit(1);
  }

  config.timeout = arg * 1000000;	/* arg is in ms, timeout is in ns */
}

/* set prepend_pin option */
static void
set_prepend_pin(int arg)
{
  if (seen_prepend_pin) {
    mlog(LOG_NOTICE,
        "%s:%d: prepend_pin option already set, ignoring", opt_c, yylineno);
    return;
  }
  seen_prepend_pin = 1;

  config.prepend_pin = arg;
}

/* set hardfail option */
static void
set_hardfail(int arg)
{
  if (seen_hardfail) {
    mlog(LOG_NOTICE,
        "%s:%d: hardfail option already set, ignoring", opt_c, yylineno);
    return;
  }
  seen_hardfail = 1;

  config.hardfail = arg;
}

/* set softfail option */
static void
set_softfail(int arg)
{
  if (seen_softfail) {
    mlog(LOG_NOTICE,
        "%s:%d: softfail option already set, ignoring", opt_c, yylineno);
    return;
  }
  seen_softfail = 1;

  config.softfail = arg;
}

/* set ewindow_size option */
static void
set_ewindow_size(int arg)
{
  if (seen_ewindow_size) {
    mlog(LOG_NOTICE,
        "%s:%d: ewindow_size option already set, ignoring", opt_c, yylineno);
    return;
  }
  seen_ewindow_size = 1;

  config.ewindow_size = arg;
}

/* set rwindow_size option */
static void
set_rwindow_size(int arg)
{
  if (seen_rwindow_size) {
    mlog(LOG_NOTICE,
        "%s:%d: rwindow_size option already set, ignoring", opt_c, yylineno);
    return;
  }
  seen_rwindow_size = 1;

  config.rwindow_size = arg;
}

/* set site_transform option */
static void
set_site_transform(int arg)
{
  if (seen_site_transform) {
    mlog(LOG_NOTICE,
        "%s:%d: site_transform option already set, ignoring", opt_c, yylineno);
    return;
  }
  seen_site_transform = 1;

  config.site_transform = arg;
}

/* set file passwd option */
static void
set_file_passwd(char *arg)
{
  if (seen_file_passwd) {
    mlog(LOG_NOTICE,
         "%s:%d: file passwd option already set, ignoring", opt_c, yylineno);
    free(arg);
    return;
  }
  seen_file_passwd = 1;

  if (arg[0] != '/') {
    mlog(LOG_CRIT, "%s:%d: file passwd name not absolute", opt_c, yylineno);
    exit(1);
  }

  if (access(arg, F_OK) == -1) {
    mlog(LOG_CRIT, "%s:%d: %s: %s", opt_c, yylineno, arg, strerror(errno));
    exit(1);
  }

  (void) strcpy(config.file.passwd, arg);
  free(arg);
}

/* set file encrypt option */
static void
set_file_encrypt(int mode, int id)
{
  if (seen_file_encrypt) {
    mlog(LOG_NOTICE,
         "%s:%d: file encrypt option already set, ignoring", opt_c, yylineno);
    return;
  }
  seen_file_encrypt = 1;

  /* see finish_key() */
  if (id > 127) {
    mlog(LOG_CRIT, "%s:%d: key id %d too large", opt_c, yylineno, id);
    exit(1);
  }

  config.file.encryptmode = mode;
  config.file.keyid = id;
  filekeylineno = yylineno;
}

/* set file server option */
static void
set_file_server(char *arg)
{
  if (seen_file_server) {
    mlog(LOG_NOTICE,
         "%s:%d: file server option already set, ignoring", opt_c, yylineno);
    return;
  }
  seen_file_server = 1;

  if (strlen(arg) > GSM_MAX_NAME_LEN) {
    mlog(LOG_CRIT, "%s:%d: file server name too long", opt_c, yylineno);
    exit(1);
  }

  (void) strcpy(config.file.gsmd, arg);
}

/* set ldap host option */
static void
set_ldap_host(char *arg)
{
  if (seen_ldap_host) {
    mlog(LOG_NOTICE,
         "%s:%d: ldap host option already set, ignoring", opt_c, yylineno);
    free(arg);
    return;
  }

  /*
   * Attempt to validate the hostname.
   * We can't be 100% sure that ldap_initialize() (which doesn't actually
   * connect()) does the same thing, but it's hard to imagine much different.
   */
  {
    struct hostent *hp;

    if ((hp = gethostbyname(arg)) == NULL) {
      mlog(LOG_CRIT, "%s:%d: ldap host name is invalid: %s",
           opt_c, yylineno, hstrerror(h_errno));
      exit(1);
    }
  }

  seen_ldap_host = arg;
}

/* set ldap port option */
static void
set_ldap_port(int arg)
{
  if (seen_ldap_port) {
    mlog(LOG_NOTICE,
         "%s:%d: ldap port option already set, ignoring", opt_c, yylineno);
    return;
  }

  if (arg == 0 || arg > 65535) {
    mlog(LOG_CRIT, "%s:%d: invalid ldap port", opt_c, yylineno);
    exit(1);
  }

  seen_ldap_port = arg;
}

/* set ldap binddn option */
static void
set_ldap_binddn(char *arg)
{
  if (seen_ldap_binddn) {
    mlog(LOG_NOTICE,
         "%s:%d: ldap binddn option already set, ignoring", opt_c, yylineno);
    free(arg);
    return;
  }
  seen_ldap_binddn = 1;

  config.ldap.binddn = arg;
}

/* set ldap bindpw option */
static void
set_ldap_bindpw(char *arg)
{
  if (seen_ldap_bindpw) {
    mlog(LOG_NOTICE,
         "%s:%d: ldap bindpw option already set, ignoring", opt_c, yylineno);
    free(arg);
    return;
  }
  seen_ldap_bindpw = 1;

  config.ldap.bindpw.val = arg;
  config.ldap.bindpw.len = strlen(arg);
}

/* set ldap basedn option */
static void
set_ldap_basedn(char *arg)
{
  if (seen_ldap_basedn) {
    mlog(LOG_NOTICE,
         "%s:%d: ldap basedn option already set, ignoring", opt_c, yylineno);
    free(arg);
    return;
  }
  seen_ldap_basedn = 1;

  config.ldap.basedn = arg;
}

/* set ldap filter option */
static void
set_ldap_filter(char *arg)
{
  size_t l;
  unsigned i, seen_u = 0;

  if (seen_ldap_filter) {
    mlog(LOG_NOTICE,
         "%s:%d: ldap filter option already set, ignoring", opt_c, yylineno);
    free(arg);
    return;
  }
  seen_ldap_filter = 1;

  /* make sure at most 1 '%' and it is '%u' */
  l = strlen(arg);
  for (i = 0; i < l; ++i) {
    if (arg[i] == '%') {
      if (seen_u++ || arg[i + 1] != 'u' || i == l - 1) {
        mlog(LOG_CRIT, "%s:%d: ldap filter invalid", opt_c, yylineno);
        exit(1);
      }
      arg[i + 1] = 's';	/* change '%u' to '%s' */
    }
  }
  if (!seen_u) {
    mlog(LOG_CRIT, "%s:%d: ldap filter invalid", opt_c, yylineno);
    exit(1);
  }

  config.ldap.filter.val = arg;
  config.ldap.filter.len = l - 2;	/* not incl. '%s' */
}

/* set ldap scope option */
static void
set_ldap_scope(char *arg)
{
  if (seen_ldap_scope) {
    mlog(LOG_NOTICE,
         "%s:%d: ldap scope option already set, ignoring", opt_c, yylineno);
    free(arg);
    return;
  }
  seen_ldap_scope = 1;

  if (strcmp(arg, "one") && strcmp(arg, "sub")) {
    mlog(LOG_CRIT, "%s:%d: ldap scope invalid", opt_c, yylineno);
    exit(1);
  }

  /* set to a string to avoid ldap.h */
  config.ldap.scope = arg;
}

/* set tls mode option */
static void
set_tls_mode(char *arg)
{
  if (seen_tls_mode) {
    mlog(LOG_NOTICE,
         "%s:%d: tls mode option already set, ignoring", opt_c, yylineno);
    free(arg);
    return;
  }

  if (!strcmp(arg, "off")) {
    config.ldap.tls.mode = TLSMODE_OFF;
  } else if (!strcmp(arg, "on")) {
    config.ldap.tls.mode = TLSMODE_ON;
  } else if (!strcmp(arg, "bind-only")) {
    config.ldap.tls.mode = TLSMODE_BINDONLY;
  } else {
    mlog(LOG_CRIT, "%s:%d: tls mode invalid", opt_c, yylineno);
    exit(1);
  }

  seen_tls_mode = 1;
}

/* set tls verify_cert option */
static void
set_tls_verify_cert(char *arg)
{
  if (seen_tls_verify_cert) {
    mlog(LOG_NOTICE, "%s:%d: tls verify_cert option already set, ignoring",
         opt_c, yylineno);
    free(arg);
    return;
  }
  seen_tls_verify_cert = 1;

  if (!strcmp(arg, "ignore")) {
    config.ldap.tls.verify_cert = 0;
  } else if (!strcmp(arg, "check")) {
    config.ldap.tls.verify_cert = 1;
  } else if (!strcmp(arg, "crl")) {
    config.ldap.tls.verify_cert = 2;
  } else {
    mlog(LOG_CRIT, "%s:%d: tls verify_cert invalid", opt_c, yylineno);
    exit(1);
  }
}

/* set tls cacertfile option */
static void
set_tls_cacertfile(char *arg)
{
  if (seen_tls_cacertfile) {
    mlog(LOG_NOTICE, "%s:%d: tls cacertfile option already set, ignoring",
         opt_c, yylineno);
    free(arg);
    return;
  }
  seen_tls_cacertfile = 1;

  if (access(arg, F_OK) == -1) {
    mlog(LOG_CRIT, "%s:%d: %s: %s", opt_c, yylineno, arg, strerror(errno));
    exit(1);
  }

  config.ldap.tls.cacertfile = arg;
}

/* set tls cacertdir option */
static void
set_tls_cacertdir(char *arg)
{
  if (seen_tls_cacertdir) {
    mlog(LOG_NOTICE, "%s:%d: tls cacertdir option already set, ignoring",
         opt_c, yylineno);
    free(arg);
    return;
  }
  seen_tls_cacertdir = 1;

  if (access(arg, F_OK) == -1) {
    mlog(LOG_CRIT, "%s:%d: %s: %s", opt_c, yylineno, arg, strerror(errno));
    exit(1);
  }

  config.ldap.tls.cacertdir = arg;
}

/* set tls certfile option */
static void
set_tls_certfile(char *arg)
{
  if (seen_tls_certfile) {
    mlog(LOG_NOTICE, "%s:%d: tls certfile option already set, ignoring",
         opt_c, yylineno);
    free(arg);
    return;
  }
  seen_tls_certfile = 1;

  if (access(arg, F_OK) == -1) {
    mlog(LOG_CRIT, "%s:%d: %s: %s", opt_c, yylineno, arg, strerror(errno));
    exit(1);
  }

  config.ldap.tls.certfile = arg;
}

/* set tls keyfile option */
static void
set_tls_keyfile(char *arg)
{
  if (seen_tls_keyfile) {
    mlog(LOG_NOTICE, "%s:%d: tls keyfile option already set, ignoring",
         opt_c, yylineno);
    free(arg);
    return;
  }
  seen_tls_keyfile = 1;

  if (access(arg, F_OK) == -1) {
    mlog(LOG_CRIT, "%s:%d: %s: %s", opt_c, yylineno, arg, strerror(errno));
    exit(1);
  }

  config.ldap.tls.keyfile = arg;
}

/* set passwd keyid option */
static void
set_passwd_keyid(int arg)
{
  if (seen_passwd_keyid) {
    mlog(LOG_NOTICE, "%s:%d: keyid option already set, ignoring",
         opt_c, yylineno);
    return;
  }
  seen_passwd_keyid = 1;

  passwd_keyid = arg;
  return;
}

/* set passwd key option */
static void
set_passwd_key(char *arg)
{
  if (seen_passwd_key) {
    mlog(LOG_NOTICE, "%s:%d: key option already set, ignoring",
         opt_c, yylineno);
    free(arg);
    return;
  }
  seen_passwd_key = 1;

  if (strlen(arg) & 1) {
    mlog(LOG_CRIT, "%s:%d: key invalid", opt_c, yylineno);
    exit(1);
  }
  passwd_key = xmalloc(strlen(arg) / 2);
  if ((passwd_keylen = a2nx(arg, passwd_key, strlen(arg) / 2)) == -1) {
    mlog(LOG_CRIT, "%s:%d: key invalid", opt_c, yylineno);
    exit(1);
  }

  return;
}

/* set state mode option */
static void
set_state_mode(char *arg)
{
  if (seen_state_mode) {
    mlog(LOG_NOTICE,
         "%s:%d: state mode option already set, ignoring", opt_c, yylineno);
    free(arg);
    return;
  }
  seen_state_mode = 1;

  if (!strcmp(arg, "local")) {
    config.state.mode = SMODE_LOCAL;
  } else if (!strcmp(arg, "global")) {
    config.state.mode = SMODE_GLOBAL;
  } else {
    mlog(LOG_CRIT, "%s:%d: state mode invalid", opt_c, yylineno);
    exit(1);
  }
}

/* set state statedir option */
static void
set_statedir(char *arg)
{
  if (seen_statedir) {
    mlog(LOG_NOTICE,
         "%s:%d: statedir option already set, ignoring", opt_c, yylineno);
    free(arg);
    return;
  }
  seen_statedir = 1;

  if (arg[0] != '/') {
    mlog(LOG_CRIT, "%s:%d: statedir name not absolute", opt_c, yylineno);
    exit(1);
  }

  if (strlen(arg) > PATH_MAX - OTP_MAX_USERNAME_LEN - 8 /* tmp files */) {
    mlog(LOG_CRIT, "%s:%d: statedir name too long", opt_c, yylineno);
    exit(1);
  }

  (void) strcpy(config.state.statedir, arg);
  free(arg);
}

/* set state timeout option */
static void
set_state_timeout(int arg)
{
  if (seen_gtimeout) {
    mlog(LOG_NOTICE, "%s:%d: state timeout option already set, ignoring",
         opt_c, yylineno);
    return;
  }
  seen_gtimeout = arg * 1000000;	/* arg is in ms, timeout is in ns */

  gtimeoutlineno = yylineno;
}

/* set state gport option */
static void
set_state_gport(int arg)
{
  if (seen_gport) {
    mlog(LOG_NOTICE, "%s:%d: state port option already set, ignoring",
         opt_c, yylineno);
    return;
  }
  seen_gport = 1;

  if (arg == 0 || arg > 65535) {
    mlog(LOG_CRIT, "%s:%d: invalid port", opt_c, yylineno);
    exit(1);
  }

  gport = htons(arg);
}

/* state server name option */
static void
set_server_name(char *arg)
{
  int i;

  if (seen_server_name) {
    mlog(LOG_NOTICE,
         "%s:%d: server name option already set, ignoring", opt_c, yylineno);
    free(arg);
    return;
  }

  if (strlen(arg) > GSM_MAX_NAME_LEN) {
    mlog(LOG_CRIT, "%s:%d: server name too long", opt_c, yylineno);
    exit(1);
  }

  for (i = 0; i < ngsmd; ++i) {
    if (!strcmp(gsmd[i]->name, arg)) {
      mlog(LOG_CRIT, "%s:%d: server '%s' already configured",
           opt_c, yylineno, arg);
      exit(1);
    }
  }

  seen_server_name = arg;
}

/* state server primary option */
static void
set_server_primary(char *arg)
{
  if (seen_server_primary) {
    mlog(LOG_NOTICE,
         "%s:%d: server primary option already set, ignoring", opt_c, yylineno);
    free(arg);
    return;
  }

  seen_server_primary = arg;
  primarylineno = yylineno;
}

/* state server backup option */
static void
set_server_backup(char *arg)
{
  if (seen_server_backup) {
    mlog(LOG_NOTICE,
         "%s:%d: server backup option already set, ignoring", opt_c, yylineno);
    free(arg);
    return;
  }

  seen_server_backup = arg;
  backuplineno = yylineno;
}

/* state server timeout option */
static void
set_server_timeout(int arg)
{
  if (seen_server_timeout) {
    mlog(LOG_NOTICE, "%s:%d: server timeout option already set, ignoring",
         opt_c, yylineno);
    return;
  }

  seen_server_timeout = arg;
  timeoutlineno = yylineno;
}

/* state server gport option */
static void
set_server_gport(int arg)
{
  if (seen_server_gport) {
    mlog(LOG_NOTICE, "%s:%d: server port option already set, ignoring",
         opt_c, yylineno);
    return;
  }

  if (arg == 0 || arg > 65535) {
    mlog(LOG_CRIT, "%s:%d: invalid port", opt_c, yylineno);
    exit(1);
  }

  seen_server_gport = htons(arg);
}

/* state server key option */
static void
set_server_key(char *arg)
{
  if (seen_server_key) {
    mlog(LOG_NOTICE,
         "%s:%d: key option already set, ignoring", opt_c, yylineno);
    free(arg);
    return;
  }

  /* must use 256-bit keys */
  if (strlen(arg) != 64) {
    mlog(LOG_CRIT, "%s:%d: key must be 256 bits", opt_c, yylineno);
    exit(1);
  }

  seen_server_key = arg;
  gsmdkeylineno = yylineno;
}

/* finish up a server config */
static void
finish_server(void)
{
  if (!seen_server_name) {
    mlog(LOG_CRIT, "%s:%d: server requires a name", opt_c, yylineno);
    exit(1);
  }

  if (!seen_server_primary) {
    mlog(LOG_CRIT, "%s:%d: server requires a primary host", opt_c, yylineno);
    exit(1);
  }

  if (!seen_server_key) {
    mlog(LOG_CRIT, "%s:%d: server requires a key", opt_c, yylineno);
    exit(1);
  }

  /* copy server config into a new gsmd_t */
  gsmd = xrealloc(gsmd, sizeof(*gsmd) * (ngsmd + 1));
  gsmd[ngsmd] = xmalloc(sizeof(**gsmd));
  gsmd[ngsmd]->name          = seen_server_name;
  gsmd[ngsmd]->primary       = seen_server_primary;
  gsmd[ngsmd]->backup        = seen_server_backup;
  gsmd[ngsmd]->timeout       = seen_server_timeout;
  gsmd[ngsmd]->gport         = seen_server_gport;
  gsmd[ngsmd]->key           = seen_server_key;
  gsmd[ngsmd]->primarylineno = primarylineno;
  gsmd[ngsmd]->backuplineno  = backuplineno;
  gsmd[ngsmd]->timeoutlineno = timeoutlineno;
  gsmd[ngsmd]->keylineno     = gsmdkeylineno;
  ngsmd++;

  /* we COULD configure the key now, but we'll do it all together */

  /* reset for next server (NOTE: don't need to reset *lineno) */
  seen_server_name    = NULL;
  seen_server_primary = NULL;
  seen_server_backup  = NULL;
  seen_server_timeout = 0;
  seen_server_gport   = 0;
  seen_server_key     = NULL;
}

/* finish up file config */
static void
finish_file(void)
{
  if (!userops[OTPD_USEROPS_FILE].init1) {
    mlog(LOG_CRIT, "%s: userops file module not loaded", opt_c);
    exit(1);
  }
  if (seen_file) {
    mlog(LOG_CRIT, "%s: only 1 file section allowed", opt_c);
    exit(1);
  }
  seen_file = 1;
}

/* finish up ldap config */
static void
finish_ldap(void)
{
  if (!userops[OTPD_USEROPS_LDAP].init1) {
    mlog(LOG_CRIT, "%s: userops ldap module not loaded", opt_c);
    exit(1);
  }
  if (seen_ldap) {
    mlog(LOG_CRIT, "%s: only 1 ldap section allowed", opt_c);
    exit(1);
  }
  seen_ldap = 1;

  if (!seen_ldap_host)
    seen_ldap_host = xstrdup("localhost");

  if (!seen_ldap_port) {
    struct servent *sp;

    sp = getservbyname("ldap", "tcp");
    if (sp)
      seen_ldap_port = ntohs(sp->s_port);
    else
      seen_ldap_port = 389;
  }

  config.ldap.url = xmalloc(7 + strlen(seen_ldap_host) + 1 + 5 + 1);
  (void) sprintf(config.ldap.url, "ldap://%s:%d", seen_ldap_host,
                 seen_ldap_port);

  if (config.ldap.binddn && !config.ldap.bindpw.val) {
    mlog(LOG_CRIT, "%s: ldap bindpw option not set", opt_c);
    exit(1);
  }

  if (!config.ldap.basedn) {
    mlog(LOG_CRIT, "%s: ldap basedn option not set", opt_c);
    exit(1);
  }
  if (!config.ldap.filter.val) {
    mlog(LOG_CRIT, "%s: ldap filter option not set", opt_c);
    exit(1);
  }
  if (!config.ldap.scope)
    config.ldap.scope = xstrdup("sub");
}

/* finish up tls config */
static void
finish_tls(void)
{
  if (using_tls) {
    mlog(LOG_CRIT, "%s: only 1 tls section allowed", opt_c);
    exit(1);
  }
  using_tls = 1;
}

/* finish up passwd key config */
static void
finish_key(void)
{
  if (!seen_passwd_keyid || !seen_passwd_key) {
    mlog(LOG_CRIT, "%s:%d: key configuration incomplete", opt_c, yylineno);
    exit(1);
  }
  seen_passwd_keyid = 0;
  seen_passwd_key = 0;

  /* avoid wasting a lot of memory */
  if (passwd_keyid > 127) {
    mlog(LOG_CRIT, "%s:%d: key id %d too large", opt_c, yylineno, passwd_keyid);
    exit(1);
  }

  /* since we limit nkeys to 128 maybe a fixed size array is fine */
  if (!config.key) {
    config.key = xmalloc((passwd_keyid + 1) * sizeof(*config.key));
    (void) memset(config.key, 0, (passwd_keyid + 1) * sizeof(*config.key));
    config.nkeys = passwd_keyid + 1;
  } else if (passwd_keyid >= (int) config.nkeys) {
    config.key = xrealloc(config.key, (passwd_keyid + 1) * sizeof(*config.key));
    (void) memset(&config.key[passwd_keyid], 0,
                  ((passwd_keyid + 1) - config.nkeys) * sizeof(*config.key));
    config.nkeys = passwd_keyid + 1;
  }

  if (config.key[passwd_keyid]) {
    mlog(LOG_CRIT, "%s:%d: key id %d already configured",
         opt_c, yylineno, passwd_keyid);
    exit(1);
  }

  config.key[passwd_keyid] = xmalloc(sizeof(**config.key));
  config.key[passwd_keyid]->len = passwd_keylen;
  config.key[passwd_keyid]->key = passwd_key;
}

/* finish up state config */
static void
finish_state(void)
{
  if (using_state) {
    mlog(LOG_CRIT, "%s: only 1 state section allowed", opt_c);
    exit(1);
  }
  using_state = 1;
}

/*
 * Listen to plugins.
 * We couldn't do this earlier because we didn't know the rendezvous point.
 */
static void
finish_config(void)
{
  int i;
  int unfound_gsmd;

  /* use files by default */
  if (!seen_file && !seen_ldap)
    finish_file();
  unfound_gsmd = seen_file;		/* will count down later */

  if (backend == BACKEND_LDAP && !seen_ldap) {
    mlog(LOG_CRIT, "%s: ldap backend selected but not configured", opt_c);
    exit(1);
  }

  if (backend == BACKEND_FILE) {
    config.userops = &userops[OTPD_USEROPS_FILE];
    userops[OTPD_USEROPS_FILE].init1(&config);
  } else {
    config.userops = &userops[OTPD_USEROPS_LDAP];
    userops[OTPD_USEROPS_LDAP].init1(&config);
  }

  /*
   * In file mode, only a single keyid can be used.  Make sure the key
   * was configured.  (In ldap mode, userops has to check for each user.)
   */
  if (backend == BACKEND_FILE) {
    if (config.file.encryptmode == EMODE_KEYID &&
        (!config.key || !config.key[config.file.keyid])) {
      mlog(LOG_CRIT, "%s:%d: keyid %d not configured",
           opt_c, filekeylineno, config.file.keyid);
      exit(1);
    }
  }

  /*
   * Listen on local rendezvous point.
   * It's important to do this now, as opposed to in the accept thread,
   * to report errors before we daemonize.
   */
  {
    struct sockaddr_un sa;

    /* setup for socket() call */
    sa.sun_family = AF_UNIX;
    if (seen_plugin_rp) {
      (void) strcpy(sa.sun_path, seen_plugin_rp); /* -p or configured rp */
      free(seen_plugin_rp);
    } else {
      (void) strcpy(sa.sun_path, OTPD_PLUGIN_RP); /* default rp */
    }

    /* must use SOCK_STREAM for reliability */
    config.s = xsocket(PF_UNIX, SOCK_STREAM, 0);
    /* try to open our listen socket */
    xunlink(sa.sun_path);
    xbind(config.s, (struct sockaddr *) &sa,
          sizeof(sa.sun_family) + strlen(sa.sun_path));
    xlisten(config.s, 5);
  }

  if (config.state.mode == SMODE_LOCAL) {
    if (gsmd) {
      /* running in local mode; just free allocated mem */
      for (i = 0; i < ngsmd; ++i) {
        free(gsmd[i]->name);
        free(gsmd[i]->primary);
        free(gsmd[i]->backup);
        (void) memset(gsmd[i]->key, 0, strlen(gsmd[i]->key)); /* tin foil */
        free(gsmd[i]->key);
        free(gsmd[i]);
      }
      free(gsmd);
    }
    if (access(config.state.statedir, R_OK|W_OK|X_OK) == -1) {
      mlog(LOG_CRIT, "%s: statedir '%s': %s", opt_c, config.state.statedir,
           strerror(errno));
      exit(1);
    }
    return;
  }

  /*
   * running in global mode
   */

  /* make sure there is at least 1 server configured */
  if (!gsmd) {
    mlog(LOG_CRIT, "%s: state mode is global, but no servers configured",
         opt_c);
    exit(1);
  }

  if (!seen_gtimeout) {
    seen_gtimeout = config.timeout * 2 + 1000000000LL /* 1s */;
    if (seen_gtimeout < 30000000000LL)	/* 30s */
      seen_gtimeout = 30000000000LL;
  }

  /* check gsmd timeout */
  if (seen_gtimeout < config.timeout + 1000000000LL /* 1s */) {
    mlog(LOG_CRIT, "%s:%d: state timeout value too low", opt_c, gtimeoutlineno);
    exit(1);
  }
  /* failover thread is only signalled after config.timeout */
  seen_gtimeout -= config.timeout;

  for (i = 0; i < ngsmd; ++i) {
    gsmd_t *g;

    /* allocate a new gsmd */
    g = xmalloc(sizeof(*g));
    g->name = gsmd[i]->name;	/* NOTE: not free()'d */
    g->n      = 1;
    g->active = 0;		/* primary active by default */
    xsem_init(&g->sem, 0, 0);

    if (gsmd[i]->timeout == 0) {
      g->fo_timeout = seen_gtimeout;
    } else {
      if (gsmd[i]->timeout < config.timeout + 1000000000LL /* 1s */) {
        mlog(LOG_CRIT, "%s:%d: state timeout value too low", opt_c,
             gsmd[i]->timeoutlineno);
        exit(1);
      }
      g->fo_timeout = gsmd[i]->timeout - config.timeout;
    }
    g->fb_timeout = config.timeout + 1000000000LL;	/* +1s */

    /* loop over all file configs later */
    if (backend == BACKEND_FILE)
      if (!strcmp(g->name, config.file.gsmd))
        unfound_gsmd--;
      
    /*
     * fill in g->sin
     * we do this now to avoid hostname lookup latency later
     */
    {
      struct hostent *hp;

      if ((hp = gethostbyname(gsmd[i]->primary)) == NULL) {
        mlog(LOG_CRIT, "%s:%d: primary hostname is invalid: %s",
             opt_c, gsmd[i]->primarylineno, hstrerror(h_errno));
        exit(1);
      }
      free(gsmd[i]->primary);

      g->sin[0].sin_family = AF_INET;
      (void) memcpy(&g->sin[0].sin_addr, *(hp->h_addr_list),
                    sizeof(struct in_addr));
      if (gsmd[i]->gport == 0)
        g->sin[0].sin_port = gport;
      else
        g->sin[0].sin_port = gsmd[i]->gport;

      if (gsmd[i]->backup) {
        if ((hp = gethostbyname(gsmd[i]->backup)) == NULL) {
          mlog(LOG_CRIT, "%s:%d: backup hostname is invalid: %s",
               opt_c, gsmd[i]->backuplineno, hstrerror(h_errno));
          exit(1);
        }
        free(gsmd[i]->backup);

        g->sin[1].sin_family = AF_INET;
        (void) memcpy(&g->sin[1].sin_addr, *(hp->h_addr_list),
                      sizeof(struct in_addr));
        if (gsmd[i]->gport == 0)
          g->sin[1].sin_port = gport;
        else
          g->sin[1].sin_port = gsmd[i]->gport;

        g->n++;
      } /* if (backup configured) */
    } /* g->sin */

    /* fill in g->scontext */
    {
      union {
        unsigned char key[32];
        uint32_t pad;	/* force uint32_t alignment for helix_setkey() */
      } u;

      /* convert ASCII key to hex; we already know len is ok, verify format */
      if (a2x(gsmd[i]->key, u.key) == -1) {
        mlog(LOG_CRIT, "%s:%d: key is invalid", opt_c, gsmd[i]->keylineno);
        exit(1);
      }

      g->scontext = helix_setkey(NULL, u.key, sizeof(u.key));
    } /* g->scontext */

    (void) memset(gsmd[i]->key, 0, strlen(gsmd[i]->key)); /* tin foil */
    free(gsmd[i]->key);
    free(gsmd[i]);

    /* put new gsmd on the hash chain */
    gsmd_hash(g);
  }
  free(gsmd);

  if (unfound_gsmd) {
    mlog(LOG_CRIT, "%s: file/gsmd configuration mismatch", opt_c);
    exit(1);
  }
}

/*
 * Initialize userops modules.
 * It's done here because it has to be done after config is parsed
 * but before finish_{file,ldap}() is called.
 */
static void
userops_init(void)
{
#if 0
  void (*init)(void);

  /*
   * In the Cyclades ACS environment (2.3.0 tested), the runtime linker
   * apparently does not run static constructors in ELF .ctors sections.
   * Since that is how we initialize userops modules, we have an ugly
   * workaround here.  Our other choice is to implement userops modules
   * as full-fledged shared libraries, which is just too much work.
   */
  if (!userops[OTPD_USEROPS_FILE].init1)
    if ((init = dlsym(RTLD_DEFAULT, "userops_file_init")))
      (*init)();
  if (!userops[OTPD_USEROPS_LDAP].init1)
    if ((init = dlsym(RTLD_DEFAULT, "userops_ldap_init")))
      (*init)();
#endif
}

/* report parse error and exit */
int
yyerror(char *s)
{
  mlog(LOG_CRIT, "%s:%d: %s", opt_c, yylineno, s);
  exit(1);
}

/* don't try to continue on EOF */
int
yywrap(void)
{
  return 1;
}
