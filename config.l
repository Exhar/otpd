%{
/*
 * $Id: //trid/release/otpd-3.1.0/config.l#1 $
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
RCSID("$Id: //trid/release/otpd-3.1.0/config.l#1 $")

#include "y.tab.h"
#include "extern.h"

#include <syslog.h>
#ifndef LOG_AUTHPRIV
#define LOG_AUTHPRIV LOG_AUTH
#endif
#ifndef LOG_FTP
#define LOG_FTP LOG_DAEMON
#endif
#ifndef LOG_AUDIT
#define LOG_AUDIT LOG_AUTH
#endif
%}

%option yylineno

%x UWORD
%x UWORD2
%x SQUOTE
%x DQUOTE

%%
otpd		return OTPD;

\{		return '{';
\}		return '}';
=		return '=';
yes		{ yylval.ival = 1; return YES; }
no		{ yylval.ival = 0; return NO;  }

user		{ BEGIN UWORD; return USER;         }
log_facility	{ BEGIN UWORD; return LOG_FACILITY; }
log_level	{ BEGIN UWORD; return LOG_LEVEL;    }
plugin_rp	{ BEGIN UWORD; return PLUGIN_RP;    }
backend		{ BEGIN UWORD; return BACKEND;      }
timeout		return TIMEOUT;
prepend_pin	return PREPEND_PIN;
hardfail	return HARDFAIL;
softfail	return SOFTFAIL;
ewindow_size	return EWINDOW_SIZE;
rwindow_size	return RWINDOW_SIZE;
site_transform	return SITE_TRANSFORM;
file		return USER_FILE;
passwd		{ BEGIN UWORD; return PASSWD;       }
encrypt		return ENCRYPT;
state_server	{ BEGIN UWORD; return STATE_SERVER; }
clear		return CLEAR;
pin		return PIN;
pin-md5		return PINMD5;
ldap		return USER_LDAP;
host		{ BEGIN UWORD; return HOST;         }
port		return PORT;
binddn		{ BEGIN UWORD; return BINDDN;       }
bindpw		{ BEGIN UWORD; return BINDPW;       }
basedn		{ BEGIN UWORD; return BASEDN;       }
filter		{ BEGIN UWORD; return FILTER;       }
scope		{ BEGIN UWORD; return SCOPE;        }
tls		return LDAP_TLS;
mode		{ BEGIN UWORD; return MODE;         }
verify_cert	{ BEGIN UWORD; return VERIFY_CERT;  }
cacertfile	{ BEGIN UWORD; return CACERTFILE;   }
cacertdir	{ BEGIN UWORD; return CACERTDIR;    }
certfile	{ BEGIN UWORD; return CERTFILE;     }
keyfile		{ BEGIN UWORD; return KEYFILE;      }
passwdkey	return PASSWDKEY;
id		return KEYID;
key		{ BEGIN UWORD; return KEY;          }
state		return STATE;
statedir	{ BEGIN UWORD; return STATEDIR;     }
gsmd_port	return GPORT;
server		return SERVER;
name		{ BEGIN UWORD; return NAME;    }
primary		{ BEGIN UWORD; return PRIMARY; }
backup		{ BEGIN UWORD; return BACKUP;  }

[[:digit:]]+	{ yylval.ival = atoi(yytext); return INTEGER; }

[ \t]+		/* ignore whitespace */;
\#.*		/* ignore comments */;
\n		/* ignore newlines */;
.		yyerror("syntax error");

<UWORD>[ \t]*=[ \t]*	{ BEGIN UWORD2; return '='; }
<UWORD2>\'		BEGIN SQUOTE;
<UWORD2>\"		BEGIN DQUOTE;
<UWORD2>[^'"][^ \t\n]*	{ yylval.sval = xstrdup(yytext);
			  BEGIN INITIAL; return WORD; }

<SQUOTE>\'	{ yytext[yyleng - 1] = '\0'; /* remove trailing quote */
		  yylval.sval = xstrdup(yytext); BEGIN INITIAL; return WORD; }
<SQUOTE>\n	{ yylineno--; (void) yyerror("unterminated quoted string"); }
<SQUOTE>.	yymore();
<DQUOTE>\"	{ yytext[yyleng - 1] = '\0'; /* remove trailing quote */
		  yylval.sval = xstrdup(yytext); BEGIN INITIAL; return WORD; }
<DQUOTE>\n	{ yylineno--; (void) yyerror("unterminated quoted string"); }
<DQUOTE>.	yymore();

%%
