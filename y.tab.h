/* A Bison parser, made by GNU Bison 2.1.  */

/* Skeleton parser for Yacc-like parsing with Bison,
   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.  */

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     OTPD = 258,
     USER = 259,
     LOG_FACILITY = 260,
     LOG_LEVEL = 261,
     PLUGIN_RP = 262,
     BACKEND = 263,
     TIMEOUT = 264,
     PREPEND_PIN = 265,
     HARDFAIL = 266,
     SOFTFAIL = 267,
     EWINDOW_SIZE = 268,
     RWINDOW_SIZE = 269,
     SITE_TRANSFORM = 270,
     USER_FILE = 271,
     PASSWD = 272,
     ENCRYPT = 273,
     CLEAR = 274,
     PIN = 275,
     PINMD5 = 276,
     STATE_SERVER = 277,
     USER_LDAP = 278,
     HOST = 279,
     PORT = 280,
     BINDDN = 281,
     BINDPW = 282,
     BASEDN = 283,
     FILTER = 284,
     SCOPE = 285,
     LDAP_TLS = 286,
     MODE = 287,
     VERIFY_CERT = 288,
     CACERTFILE = 289,
     CACERTDIR = 290,
     CERTFILE = 291,
     KEYFILE = 292,
     PASSWDKEY = 293,
     KEYID = 294,
     KEY = 295,
     STATE = 296,
     STATEDIR = 297,
     GPORT = 298,
     SERVER = 299,
     NAME = 300,
     PRIMARY = 301,
     BACKUP = 302,
     YES = 303,
     NO = 304,
     WORD = 305,
     INTEGER = 306
   };
#endif
/* Tokens.  */
#define OTPD 258
#define USER 259
#define LOG_FACILITY 260
#define LOG_LEVEL 261
#define PLUGIN_RP 262
#define BACKEND 263
#define TIMEOUT 264
#define PREPEND_PIN 265
#define HARDFAIL 266
#define SOFTFAIL 267
#define EWINDOW_SIZE 268
#define RWINDOW_SIZE 269
#define SITE_TRANSFORM 270
#define USER_FILE 271
#define PASSWD 272
#define ENCRYPT 273
#define CLEAR 274
#define PIN 275
#define PINMD5 276
#define STATE_SERVER 277
#define USER_LDAP 278
#define HOST 279
#define PORT 280
#define BINDDN 281
#define BINDPW 282
#define BASEDN 283
#define FILTER 284
#define SCOPE 285
#define LDAP_TLS 286
#define MODE 287
#define VERIFY_CERT 288
#define CACERTFILE 289
#define CACERTDIR 290
#define CERTFILE 291
#define KEYFILE 292
#define PASSWDKEY 293
#define KEYID 294
#define KEY 295
#define STATE 296
#define STATEDIR 297
#define GPORT 298
#define SERVER 299
#define NAME 300
#define PRIMARY 301
#define BACKUP 302
#define YES 303
#define NO 304
#define WORD 305
#define INTEGER 306




#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)
#line 102 "config.y"
typedef union YYSTYPE {
  int ival;
  char *sval;
} YYSTYPE;
/* Line 1447 of yacc.c.  */
#line 145 "y.tab.h"
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;



