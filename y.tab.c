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

/* Written by Richard Stallman by simplifying the original so called
   ``semantic'' parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "2.1"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Using locations.  */
#define YYLSP_NEEDED 0



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




/* Copy the first part of user declarations.  */
#line 1 "config.y"

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


/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* Enabling the token table.  */
#ifndef YYTOKEN_TABLE
# define YYTOKEN_TABLE 0
#endif

#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)
#line 102 "config.y"
typedef union YYSTYPE {
  int ival;
  char *sval;
} YYSTYPE;
/* Line 196 of yacc.c.  */
#line 293 "y.tab.c"
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



/* Copy the second part of user declarations.  */


/* Line 219 of yacc.c.  */
#line 305 "y.tab.c"

#if ! defined (YYSIZE_T) && defined (__SIZE_TYPE__)
# define YYSIZE_T __SIZE_TYPE__
#endif
#if ! defined (YYSIZE_T) && defined (size_t)
# define YYSIZE_T size_t
#endif
#if ! defined (YYSIZE_T) && (defined (__STDC__) || defined (__cplusplus))
# include <stddef.h> /* INFRINGES ON USER NAME SPACE */
# define YYSIZE_T size_t
#endif
#if ! defined (YYSIZE_T)
# define YYSIZE_T unsigned int
#endif

#ifndef YY_
# if YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

#if ! defined (yyoverflow) || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if defined (__STDC__) || defined (__cplusplus)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#     define YYINCLUDED_STDLIB_H
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning. */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2005 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM ((YYSIZE_T) -1)
#  endif
#  ifdef __cplusplus
extern "C" {
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if (! defined (malloc) && ! defined (YYINCLUDED_STDLIB_H) \
	&& (defined (__STDC__) || defined (__cplusplus)))
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if (! defined (free) && ! defined (YYINCLUDED_STDLIB_H) \
	&& (defined (__STDC__) || defined (__cplusplus)))
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifdef __cplusplus
}
#  endif
# endif
#endif /* ! defined (yyoverflow) || YYERROR_VERBOSE */


#if (! defined (yyoverflow) \
     && (! defined (__cplusplus) \
	 || (defined (YYSTYPE_IS_TRIVIAL) && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  short int yyss;
  YYSTYPE yyvs;
  };

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (short int) + sizeof (YYSTYPE))			\
      + YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined (__GNUC__) && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  YYSIZE_T yyi;				\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (0)
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack)					\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack, Stack, yysize);				\
	Stack = &yyptr->Stack;						\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (0)

#endif

#if defined (__STDC__) || defined (__cplusplus)
   typedef signed char yysigned_char;
#else
   typedef short int yysigned_char;
#endif

/* YYFINAL -- State number of the termination state. */
#define YYFINAL  4
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   222

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  55
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  17
/* YYNRULES -- Number of rules. */
#define YYNRULES  73
/* YYNRULES -- Number of states. */
#define YYNSTATES  174

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   306

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const unsigned char yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    54,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    52,     2,    53,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const unsigned short int yyprhs[] =
{
       0,     0,     3,     4,     8,    13,    15,    18,    22,    26,
      30,    34,    38,    42,    46,    50,    54,    58,    62,    66,
      71,    76,    81,    86,    87,    89,    92,    96,   100,   104,
     108,   112,   116,   118,   121,   125,   129,   133,   137,   141,
     145,   149,   154,   155,   157,   160,   164,   168,   172,   176,
     180,   184,   186,   189,   193,   197,   198,   200,   203,   207,
     211,   215,   219,   223,   228,   230,   233,   237,   241,   245,
     249,   253,   257,   259
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const yysigned_char yyrhs[] =
{
      56,     0,    -1,    -1,     3,    52,    53,    -1,     3,    52,
      57,    53,    -1,    58,    -1,    57,    58,    -1,     4,    54,
      50,    -1,     5,    54,    50,    -1,     6,    54,    50,    -1,
       7,    54,    50,    -1,     8,    54,    50,    -1,     9,    54,
      51,    -1,    10,    54,    71,    -1,    11,    54,    51,    -1,
      12,    54,    51,    -1,    13,    54,    51,    -1,    14,    54,
      51,    -1,    15,    54,    71,    -1,    16,    52,    59,    53,
      -1,    23,    52,    61,    53,    -1,    38,    52,    65,    53,
      -1,    41,    52,    67,    53,    -1,    -1,    60,    -1,    59,
      60,    -1,    17,    54,    50,    -1,    18,    54,    19,    -1,
      18,    54,    20,    -1,    18,    54,    21,    -1,    18,    54,
      51,    -1,    22,    54,    50,    -1,    62,    -1,    61,    62,
      -1,    24,    54,    50,    -1,    25,    54,    51,    -1,    26,
      54,    50,    -1,    27,    54,    50,    -1,    28,    54,    50,
      -1,    29,    54,    50,    -1,    30,    54,    50,    -1,    31,
      52,    63,    53,    -1,    -1,    64,    -1,    63,    64,    -1,
      32,    54,    50,    -1,    33,    54,    50,    -1,    34,    54,
      50,    -1,    35,    54,    50,    -1,    36,    54,    50,    -1,
      37,    54,    50,    -1,    66,    -1,    65,    66,    -1,    39,
      54,    51,    -1,    40,    54,    50,    -1,    -1,    68,    -1,
      67,    68,    -1,    32,    54,    50,    -1,    42,    54,    50,
      -1,     9,    54,    51,    -1,    43,    54,    51,    -1,    25,
      54,    51,    -1,    44,    52,    69,    53,    -1,    70,    -1,
      69,    70,    -1,    45,    54,    50,    -1,    46,    54,    50,
      -1,    47,    54,    50,    -1,     9,    54,    51,    -1,    43,
      54,    51,    -1,    40,    54,    50,    -1,    48,    -1,    49,
      -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned char yyrline[] =
{
       0,   126,   126,   127,   128,   130,   131,   133,   134,   135,
     136,   137,   138,   139,   140,   141,   142,   143,   144,   145,
     146,   147,   148,   150,   151,   152,   154,   155,   156,   157,
     158,   159,   161,   162,   164,   165,   166,   167,   168,   169,
     170,   171,   173,   174,   175,   177,   178,   179,   180,   181,
     182,   184,   185,   187,   188,   190,   191,   192,   194,   195,
     196,   197,   198,   199,   201,   202,   204,   205,   206,   207,
     208,   209,   211,   211
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals. */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "OTPD", "USER", "LOG_FACILITY",
  "LOG_LEVEL", "PLUGIN_RP", "BACKEND", "TIMEOUT", "PREPEND_PIN",
  "HARDFAIL", "SOFTFAIL", "EWINDOW_SIZE", "RWINDOW_SIZE", "SITE_TRANSFORM",
  "USER_FILE", "PASSWD", "ENCRYPT", "CLEAR", "PIN", "PINMD5",
  "STATE_SERVER", "USER_LDAP", "HOST", "PORT", "BINDDN", "BINDPW",
  "BASEDN", "FILTER", "SCOPE", "LDAP_TLS", "MODE", "VERIFY_CERT",
  "CACERTFILE", "CACERTDIR", "CERTFILE", "KEYFILE", "PASSWDKEY", "KEYID",
  "KEY", "STATE", "STATEDIR", "GPORT", "SERVER", "NAME", "PRIMARY",
  "BACKUP", "YES", "NO", "WORD", "INTEGER", "'{'", "'}'", "'='", "$accept",
  "otpd_section", "config", "config_item", "file_config",
  "file_config_item", "ldap_config", "ldap_config_item", "tls_config",
  "tls_config_item", "key_config", "key_config_item", "state_config",
  "state_config_item", "server_config", "server_config_item", "yesno", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const unsigned short int yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   123,   125,    61
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned char yyr1[] =
{
       0,    55,    56,    56,    56,    57,    57,    58,    58,    58,
      58,    58,    58,    58,    58,    58,    58,    58,    58,    58,
      58,    58,    58,    59,    59,    59,    60,    60,    60,    60,
      60,    60,    61,    61,    62,    62,    62,    62,    62,    62,
      62,    62,    63,    63,    63,    64,    64,    64,    64,    64,
      64,    65,    65,    66,    66,    67,    67,    67,    68,    68,
      68,    68,    68,    68,    69,    69,    70,    70,    70,    70,
      70,    70,    71,    71
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const unsigned char yyr2[] =
{
       0,     2,     0,     3,     4,     1,     2,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     4,
       4,     4,     4,     0,     1,     2,     3,     3,     3,     3,
       3,     3,     1,     2,     3,     3,     3,     3,     3,     3,
       3,     4,     0,     1,     2,     3,     3,     3,     3,     3,
       3,     1,     2,     3,     3,     0,     1,     2,     3,     3,
       3,     3,     3,     4,     1,     2,     3,     3,     3,     3,
       3,     3,     1,     1
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned char yydefact[] =
{
       2,     0,     0,     0,     1,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     3,     0,     5,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    23,     0,     0,    55,
       4,     6,     7,     8,     9,    10,    11,    12,    72,    73,
      13,    14,    15,    16,    17,    18,     0,     0,     0,     0,
      24,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      32,     0,     0,     0,    51,     0,     0,     0,     0,     0,
       0,     0,    56,     0,     0,     0,    19,    25,     0,     0,
       0,     0,     0,     0,     0,    42,    20,    33,     0,     0,
      21,    52,     0,     0,     0,     0,     0,     0,    22,    57,
      26,    27,    28,    29,    30,    31,    34,    35,    36,    37,
      38,    39,    40,     0,     0,     0,     0,     0,     0,     0,
      43,    53,    54,    60,    62,    58,    59,    61,     0,     0,
       0,     0,     0,     0,     0,    64,     0,     0,     0,     0,
       0,     0,    41,    44,     0,     0,     0,     0,     0,     0,
      63,    65,    45,    46,    47,    48,    49,    50,    69,    71,
      70,    66,    67,    68
};

/* YYDEFGOTO[NTERM-NUM]. */
static const short int yydefgoto[] =
{
      -1,     2,    22,    23,    59,    60,    69,    70,   129,   130,
      73,    74,    81,    82,   144,   145,    50
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -36
static const short int yypact[] =
{
      15,   -17,    38,    -4,   -36,    -7,    -1,     2,     4,     5,
       6,    11,    12,    14,    21,    30,    31,    34,    35,    39,
      40,   -36,    16,   -36,    53,    60,    61,    83,    84,    42,
     -35,    85,    86,    87,    88,   -35,    26,    90,   -24,    46,
     -36,   -36,   -36,   -36,   -36,   -36,   -36,   -36,   -36,   -36,
     -36,   -36,   -36,   -36,   -36,   -36,    68,    69,    81,    24,
     -36,    89,    91,    92,    93,    94,    96,    97,   100,    71,
     -36,    99,   101,    73,   -36,   102,   103,   104,   105,   106,
     109,     8,   -36,   112,    43,   113,   -36,   -36,   114,    98,
     115,   116,   117,   118,   119,    95,   -36,   -36,   120,   122,
     -36,   -36,   123,   124,   126,   127,   128,    36,   -36,   -36,
     -36,   -36,   -36,   -36,   -36,   -36,   -36,   -36,   -36,   -36,
     -36,   -36,   -36,   129,   130,   131,   132,   133,   134,    72,
     -36,   -36,   -36,   -36,   -36,   -36,   -36,   -36,   135,   136,
     137,   138,   139,   140,    27,   -36,   145,   146,   147,   148,
     149,   150,   -36,   -36,   151,   153,   154,   156,   157,   158,
     -36,   -36,   -36,   -36,   -36,   -36,   -36,   -36,   -36,   -36,
     -36,   -36,   -36,   -36
};

/* YYPGOTO[NTERM-NUM].  */
static const short int yypgoto[] =
{
     -36,   -36,   -36,   159,   -36,    82,   -36,    75,   -36,    13,
     -36,    67,   -36,   141,   -36,    10,   143
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -1
static const unsigned char yytable[] =
{
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    48,    49,    71,    72,    75,     1,    18,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    76,    19,     3,   138,    20,     4,    18,
      77,    56,    57,    56,    57,   138,    58,    24,    58,    21,
      78,    79,    80,    25,    19,    75,    26,    20,    27,    28,
      29,   108,   111,   112,   113,    30,    31,   139,    32,    40,
     140,    76,   141,   142,   143,    33,   139,    86,    77,   140,
     160,   141,   142,   143,    34,    35,    36,    37,    78,    79,
      80,    38,    39,    47,   114,    61,    62,    63,    64,    65,
      66,    67,    68,    42,   123,   124,   125,   126,   127,   128,
      43,    44,    71,    72,    61,    62,    63,    64,    65,    66,
      67,    68,    83,    84,    96,   152,   100,   123,   124,   125,
     126,   127,   128,    45,    46,    85,    51,    52,    53,    54,
     101,    87,   153,    88,    97,    89,    90,    91,    92,   117,
      93,    94,    95,    98,   161,    99,   102,   103,   104,   105,
     106,   107,   110,   115,   116,   118,   119,   120,   121,   122,
       0,   131,   132,     0,   133,   134,   135,   136,    55,   137,
       0,    41,     0,   146,   147,   148,   149,   150,   151,   154,
     155,   156,   157,   158,   159,   162,   163,   164,   165,   166,
     167,     0,   168,   169,     0,   170,   171,   172,   173,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   109
};

static const short int yycheck[] =
{
       4,     5,     6,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    48,    49,    39,    40,     9,     3,    23,
       4,     5,     6,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    25,    38,    52,     9,    41,     0,    23,
      32,    17,    18,    17,    18,     9,    22,    54,    22,    53,
      42,    43,    44,    54,    38,     9,    54,    41,    54,    54,
      54,    53,    19,    20,    21,    54,    54,    40,    54,    53,
      43,    25,    45,    46,    47,    54,    40,    53,    32,    43,
      53,    45,    46,    47,    54,    54,    52,    52,    42,    43,
      44,    52,    52,    51,    51,    24,    25,    26,    27,    28,
      29,    30,    31,    50,    32,    33,    34,    35,    36,    37,
      50,    50,    39,    40,    24,    25,    26,    27,    28,    29,
      30,    31,    54,    54,    53,    53,    53,    32,    33,    34,
      35,    36,    37,    50,    50,    54,    51,    51,    51,    51,
      73,    59,   129,    54,    69,    54,    54,    54,    54,    51,
      54,    54,    52,    54,   144,    54,    54,    54,    54,    54,
      54,    52,    50,    50,    50,    50,    50,    50,    50,    50,
      -1,    51,    50,    -1,    51,    51,    50,    50,    35,    51,
      -1,    22,    -1,    54,    54,    54,    54,    54,    54,    54,
      54,    54,    54,    54,    54,    50,    50,    50,    50,    50,
      50,    -1,    51,    50,    -1,    51,    50,    50,    50,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    81
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned char yystos[] =
{
       0,     3,    56,    52,     0,     4,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    23,    38,
      41,    53,    57,    58,    54,    54,    54,    54,    54,    54,
      54,    54,    54,    54,    54,    54,    52,    52,    52,    52,
      53,    58,    50,    50,    50,    50,    50,    51,    48,    49,
      71,    51,    51,    51,    51,    71,    17,    18,    22,    59,
      60,    24,    25,    26,    27,    28,    29,    30,    31,    61,
      62,    39,    40,    65,    66,     9,    25,    32,    42,    43,
      44,    67,    68,    54,    54,    54,    53,    60,    54,    54,
      54,    54,    54,    54,    54,    52,    53,    62,    54,    54,
      53,    66,    54,    54,    54,    54,    54,    52,    53,    68,
      50,    19,    20,    21,    51,    50,    50,    51,    50,    50,
      50,    50,    50,    32,    33,    34,    35,    36,    37,    63,
      64,    51,    50,    51,    51,    50,    50,    51,     9,    40,
      43,    45,    46,    47,    69,    70,    54,    54,    54,    54,
      54,    54,    53,    64,    54,    54,    54,    54,    54,    54,
      53,    70,    50,    50,    50,    50,    50,    50,    51,    50,
      51,    50,    50,    50
};

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */

#define YYFAIL		goto yyerrlab

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      yytoken = YYTRANSLATE (yychar);				\
      YYPOPSTACK;						\
      goto yybackup;						\
    }								\
  else								\
    {								\
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;							\
    }								\
while (0)


#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)				\
    do									\
      if (N)								\
	{								\
	  (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;	\
	  (Current).first_column = YYRHSLOC (Rhs, 1).first_column;	\
	  (Current).last_line    = YYRHSLOC (Rhs, N).last_line;		\
	  (Current).last_column  = YYRHSLOC (Rhs, N).last_column;	\
	}								\
      else								\
	{								\
	  (Current).first_line   = (Current).last_line   =		\
	    YYRHSLOC (Rhs, 0).last_line;				\
	  (Current).first_column = (Current).last_column =		\
	    YYRHSLOC (Rhs, 0).last_column;				\
	}								\
    while (0)
#endif


/* YY_LOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

#ifndef YY_LOCATION_PRINT
# if YYLTYPE_IS_TRIVIAL
#  define YY_LOCATION_PRINT(File, Loc)			\
     fprintf (File, "%d.%d-%d.%d",			\
              (Loc).first_line, (Loc).first_column,	\
              (Loc).last_line,  (Loc).last_column)
# else
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (YYLEX_PARAM)
#else
# define YYLEX yylex ()
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (0)

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)		\
do {								\
  if (yydebug)							\
    {								\
      YYFPRINTF (stderr, "%s ", Title);				\
      yysymprint (stderr,					\
                  Type, Value);	\
      YYFPRINTF (stderr, "\n");					\
    }								\
} while (0)

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yy_stack_print (short int *bottom, short int *top)
#else
static void
yy_stack_print (bottom, top)
    short int *bottom;
    short int *top;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (/* Nothing. */; bottom <= top; ++bottom)
    YYFPRINTF (stderr, " %d", *bottom);
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yy_reduce_print (int yyrule)
#else
static void
yy_reduce_print (yyrule)
    int yyrule;
#endif
{
  int yyi;
  unsigned long int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu), ",
             yyrule - 1, yylno);
  /* Print the symbols being reduced, and their result.  */
  for (yyi = yyprhs[yyrule]; 0 <= yyrhs[yyi]; yyi++)
    YYFPRINTF (stderr, "%s ", yytname[yyrhs[yyi]]);
  YYFPRINTF (stderr, "-> %s\n", yytname[yyr1[yyrule]]);
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (Rule);		\
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif



#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined (__GLIBC__) && defined (_STRING_H)
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
static YYSIZE_T
#   if defined (__STDC__) || defined (__cplusplus)
yystrlen (const char *yystr)
#   else
yystrlen (yystr)
     const char *yystr;
#   endif
{
  const char *yys = yystr;

  while (*yys++ != '\0')
    continue;

  return yys - yystr - 1;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined (__GLIBC__) && defined (_STRING_H) && defined (_GNU_SOURCE)
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
#   if defined (__STDC__) || defined (__cplusplus)
yystpcpy (char *yydest, const char *yysrc)
#   else
yystpcpy (yydest, yysrc)
     char *yydest;
     const char *yysrc;
#   endif
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      size_t yyn = 0;
      char const *yyp = yystr;

      for (;;)
	switch (*++yyp)
	  {
	  case '\'':
	  case ',':
	    goto do_not_strip_quotes;

	  case '\\':
	    if (*++yyp != '\\')
	      goto do_not_strip_quotes;
	    /* Fall through.  */
	  default:
	    if (yyres)
	      yyres[yyn] = *yyp;
	    yyn++;
	    break;

	  case '"':
	    if (yyres)
	      yyres[yyn] = '\0';
	    return yyn;
	  }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

#endif /* YYERROR_VERBOSE */



#if YYDEBUG
/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yysymprint (FILE *yyoutput, int yytype, YYSTYPE *yyvaluep)
#else
static void
yysymprint (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  /* Pacify ``unused variable'' warnings.  */
  (void) yyvaluep;

  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);


# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# endif
  switch (yytype)
    {
      default:
        break;
    }
  YYFPRINTF (yyoutput, ")");
}

#endif /* ! YYDEBUG */
/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yymsg, yytype, yyvaluep)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  /* Pacify ``unused variable'' warnings.  */
  (void) yyvaluep;

  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  switch (yytype)
    {

      default:
        break;
    }
}


/* Prevent warnings from -Wmissing-prototypes.  */

#ifdef YYPARSE_PARAM
# if defined (__STDC__) || defined (__cplusplus)
int yyparse (void *YYPARSE_PARAM);
# else
int yyparse ();
# endif
#else /* ! YYPARSE_PARAM */
#if defined (__STDC__) || defined (__cplusplus)
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */



/* The look-ahead symbol.  */
int yychar;

/* The semantic value of the look-ahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;



/*----------.
| yyparse.  |
`----------*/

#ifdef YYPARSE_PARAM
# if defined (__STDC__) || defined (__cplusplus)
int yyparse (void *YYPARSE_PARAM)
# else
int yyparse (YYPARSE_PARAM)
  void *YYPARSE_PARAM;
# endif
#else /* ! YYPARSE_PARAM */
#if defined (__STDC__) || defined (__cplusplus)
int
yyparse (void)
#else
int
yyparse ()
    ;
#endif
#endif
{
  
  int yystate;
  int yyn;
  int yyresult;
  /* Number of tokens to shift before error messages enabled.  */
  int yyerrstatus;
  /* Look-ahead token as an internal (translated) token number.  */
  int yytoken = 0;

  /* Three stacks and their tools:
     `yyss': related to states,
     `yyvs': related to semantic values,
     `yyls': related to locations.

     Refer to the stacks thru separate pointers, to allow yyoverflow
     to reallocate them elsewhere.  */

  /* The state stack.  */
  short int yyssa[YYINITDEPTH];
  short int *yyss = yyssa;
  short int *yyssp;

  /* The semantic value stack.  */
  YYSTYPE yyvsa[YYINITDEPTH];
  YYSTYPE *yyvs = yyvsa;
  YYSTYPE *yyvsp;



#define YYPOPSTACK   (yyvsp--, yyssp--)

  YYSIZE_T yystacksize = YYINITDEPTH;

  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;


  /* When reducing, the number of symbols on the RHS of the reduced
     rule.  */
  int yylen;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss;
  yyvsp = yyvs;

  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed. so pushing a state here evens the stacks.
     */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack. Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	short int *yyss1 = yyss;


	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow (YY_("memory exhausted"),
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),

		    &yystacksize);

	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	short int *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyexhaustedlab;
	YYSTACK_RELOCATE (yyss);
	YYSTACK_RELOCATE (yyvs);

#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;


      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

/* Do appropriate processing given the current state.  */
/* Read a look-ahead token if we need one and don't already have one.  */
/* yyresume: */

  /* First try to decide what to do without reference to look-ahead token.  */

  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a look-ahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid look-ahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yyn == 0 || yyn == YYTABLE_NINF)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Shift the look-ahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the token being shifted unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  *++yyvsp = yylval;


  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  yystate = yyn;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 7:
#line 133 "config.y"
    { set_user((yyvsp[0].sval), 1);        }
    break;

  case 8:
#line 134 "config.y"
    { set_log_facility((yyvsp[0].sval));   }
    break;

  case 9:
#line 135 "config.y"
    { set_log_level((yyvsp[0].sval), 1);   }
    break;

  case 10:
#line 136 "config.y"
    { set_plugin_rp((yyvsp[0].sval), 1);   }
    break;

  case 11:
#line 137 "config.y"
    { set_backend((yyvsp[0].sval));        }
    break;

  case 12:
#line 138 "config.y"
    { set_auth_timeout((yyvsp[0].ival));   }
    break;

  case 13:
#line 139 "config.y"
    { set_prepend_pin((yyvsp[0].ival));    }
    break;

  case 14:
#line 140 "config.y"
    { set_hardfail((yyvsp[0].ival));       }
    break;

  case 15:
#line 141 "config.y"
    { set_softfail((yyvsp[0].ival));       }
    break;

  case 16:
#line 142 "config.y"
    { set_ewindow_size((yyvsp[0].ival));   }
    break;

  case 17:
#line 143 "config.y"
    { set_rwindow_size((yyvsp[0].ival));   }
    break;

  case 18:
#line 144 "config.y"
    { set_site_transform((yyvsp[0].ival)); }
    break;

  case 19:
#line 145 "config.y"
    { finish_file();          }
    break;

  case 20:
#line 146 "config.y"
    { finish_ldap();          }
    break;

  case 21:
#line 147 "config.y"
    { finish_key();           }
    break;

  case 22:
#line 148 "config.y"
    { finish_state();         }
    break;

  case 26:
#line 154 "config.y"
    { set_file_passwd((yyvsp[0].sval));    }
    break;

  case 27:
#line 155 "config.y"
    { set_file_encrypt(EMODE_CLEAR, 0);  }
    break;

  case 28:
#line 156 "config.y"
    { set_file_encrypt(EMODE_PIN, 0);    }
    break;

  case 29:
#line 157 "config.y"
    { set_file_encrypt(EMODE_PINMD5, 0); }
    break;

  case 30:
#line 158 "config.y"
    { set_file_encrypt(EMODE_KEYID, (yyvsp[0].ival)); }
    break;

  case 31:
#line 159 "config.y"
    { set_file_server((yyvsp[0].sval));            }
    break;

  case 34:
#line 164 "config.y"
    { set_ldap_host((yyvsp[0].sval));      }
    break;

  case 35:
#line 165 "config.y"
    { set_ldap_port((yyvsp[0].ival));      }
    break;

  case 36:
#line 166 "config.y"
    { set_ldap_binddn((yyvsp[0].sval));    }
    break;

  case 37:
#line 167 "config.y"
    { set_ldap_bindpw((yyvsp[0].sval));    }
    break;

  case 38:
#line 168 "config.y"
    { set_ldap_basedn((yyvsp[0].sval));    }
    break;

  case 39:
#line 169 "config.y"
    { set_ldap_filter((yyvsp[0].sval));    }
    break;

  case 40:
#line 170 "config.y"
    { set_ldap_scope((yyvsp[0].sval));     }
    break;

  case 41:
#line 171 "config.y"
    { finish_tls();           }
    break;

  case 45:
#line 177 "config.y"
    { set_tls_mode((yyvsp[0].sval));       }
    break;

  case 46:
#line 178 "config.y"
    { set_tls_verify_cert((yyvsp[0].sval)); }
    break;

  case 47:
#line 179 "config.y"
    { set_tls_cacertfile((yyvsp[0].sval)); }
    break;

  case 48:
#line 180 "config.y"
    { set_tls_cacertdir((yyvsp[0].sval));  }
    break;

  case 49:
#line 181 "config.y"
    { set_tls_certfile((yyvsp[0].sval));   }
    break;

  case 50:
#line 182 "config.y"
    { set_tls_keyfile((yyvsp[0].sval));    }
    break;

  case 53:
#line 187 "config.y"
    { set_passwd_keyid((yyvsp[0].ival));   }
    break;

  case 54:
#line 188 "config.y"
    { set_passwd_key((yyvsp[0].sval));     }
    break;

  case 58:
#line 194 "config.y"
    { set_state_mode((yyvsp[0].sval));     }
    break;

  case 59:
#line 195 "config.y"
    { set_statedir((yyvsp[0].sval));       }
    break;

  case 60:
#line 196 "config.y"
    { set_state_timeout((yyvsp[0].ival));  }
    break;

  case 61:
#line 197 "config.y"
    { set_state_gport((yyvsp[0].ival));    }
    break;

  case 62:
#line 198 "config.y"
    { set_state_gport((yyvsp[0].ival));    }
    break;

  case 63:
#line 199 "config.y"
    { finish_server();        }
    break;

  case 66:
#line 204 "config.y"
    { set_server_name((yyvsp[0].sval));    }
    break;

  case 67:
#line 205 "config.y"
    { set_server_primary((yyvsp[0].sval)); }
    break;

  case 68:
#line 206 "config.y"
    { set_server_backup((yyvsp[0].sval));  }
    break;

  case 69:
#line 207 "config.y"
    { set_server_timeout((yyvsp[0].ival)); }
    break;

  case 70:
#line 208 "config.y"
    { set_server_gport((yyvsp[0].ival));   }
    break;

  case 71:
#line 209 "config.y"
    { set_server_key((yyvsp[0].sval));     }
    break;


      default: break;
    }

/* Line 1126 of yacc.c.  */
#line 1700 "y.tab.c"

  yyvsp -= yylen;
  yyssp -= yylen;


  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;


  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if YYERROR_VERBOSE
      yyn = yypact[yystate];

      if (YYPACT_NINF < yyn && yyn < YYLAST)
	{
	  int yytype = YYTRANSLATE (yychar);
	  YYSIZE_T yysize0 = yytnamerr (0, yytname[yytype]);
	  YYSIZE_T yysize = yysize0;
	  YYSIZE_T yysize1;
	  int yysize_overflow = 0;
	  char *yymsg = 0;
#	  define YYERROR_VERBOSE_ARGS_MAXIMUM 5
	  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
	  int yyx;

#if 0
	  /* This is so xgettext sees the translatable formats that are
	     constructed on the fly.  */
	  YY_("syntax error, unexpected %s");
	  YY_("syntax error, unexpected %s, expecting %s");
	  YY_("syntax error, unexpected %s, expecting %s or %s");
	  YY_("syntax error, unexpected %s, expecting %s or %s or %s");
	  YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s");
#endif
	  char *yyfmt;
	  char const *yyf;
	  static char const yyunexpected[] = "syntax error, unexpected %s";
	  static char const yyexpecting[] = ", expecting %s";
	  static char const yyor[] = " or %s";
	  char yyformat[sizeof yyunexpected
			+ sizeof yyexpecting - 1
			+ ((YYERROR_VERBOSE_ARGS_MAXIMUM - 2)
			   * (sizeof yyor - 1))];
	  char const *yyprefix = yyexpecting;

	  /* Start YYX at -YYN if negative to avoid negative indexes in
	     YYCHECK.  */
	  int yyxbegin = yyn < 0 ? -yyn : 0;

	  /* Stay within bounds of both yycheck and yytname.  */
	  int yychecklim = YYLAST - yyn;
	  int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
	  int yycount = 1;

	  yyarg[0] = yytname[yytype];
	  yyfmt = yystpcpy (yyformat, yyunexpected);

	  for (yyx = yyxbegin; yyx < yyxend; ++yyx)
	    if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	      {
		if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
		  {
		    yycount = 1;
		    yysize = yysize0;
		    yyformat[sizeof yyunexpected - 1] = '\0';
		    break;
		  }
		yyarg[yycount++] = yytname[yyx];
		yysize1 = yysize + yytnamerr (0, yytname[yyx]);
		yysize_overflow |= yysize1 < yysize;
		yysize = yysize1;
		yyfmt = yystpcpy (yyfmt, yyprefix);
		yyprefix = yyor;
	      }

	  yyf = YY_(yyformat);
	  yysize1 = yysize + yystrlen (yyf);
	  yysize_overflow |= yysize1 < yysize;
	  yysize = yysize1;

	  if (!yysize_overflow && yysize <= YYSTACK_ALLOC_MAXIMUM)
	    yymsg = (char *) YYSTACK_ALLOC (yysize);
	  if (yymsg)
	    {
	      /* Avoid sprintf, as that infringes on the user's name space.
		 Don't have undefined behavior even if the translation
		 produced a string with the wrong number of "%s"s.  */
	      char *yyp = yymsg;
	      int yyi = 0;
	      while ((*yyp = *yyf))
		{
		  if (*yyp == '%' && yyf[1] == 's' && yyi < yycount)
		    {
		      yyp += yytnamerr (yyp, yyarg[yyi++]);
		      yyf += 2;
		    }
		  else
		    {
		      yyp++;
		      yyf++;
		    }
		}
	      yyerror (yymsg);
	      YYSTACK_FREE (yymsg);
	    }
	  else
	    {
	      yyerror (YY_("syntax error"));
	      goto yyexhaustedlab;
	    }
	}
      else
#endif /* YYERROR_VERBOSE */
	yyerror (YY_("syntax error"));
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse look-ahead token after an
	 error, discard it.  */

      if (yychar <= YYEOF)
        {
	  /* Return failure if at end of input.  */
	  if (yychar == YYEOF)
	    YYABORT;
        }
      else
	{
	  yydestruct ("Error: discarding", yytoken, &yylval);
	  yychar = YYEMPTY;
	}
    }

  /* Else will try to reuse look-ahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (0)
     goto yyerrorlab;

yyvsp -= yylen;
  yyssp -= yylen;
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (yyn != YYPACT_NINF)
	{
	  yyn += YYTERROR;
	  if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
	    {
	      yyn = yytable[yyn];
	      if (0 < yyn)
		break;
	    }
	}

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
	YYABORT;


      yydestruct ("Error: popping", yystos[yystate], yyvsp);
      YYPOPSTACK;
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  *++yyvsp = yylval;


  /* Shift the error token. */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#ifndef yyoverflow
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEOF && yychar != YYEMPTY)
     yydestruct ("Cleanup: discarding lookahead",
		 yytoken, &yylval);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
		  yystos[*yyssp], yyvsp);
      YYPOPSTACK;
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
  return yyresult;
}


#line 213 "config.y"


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

