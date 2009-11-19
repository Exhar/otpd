/*
 * resynctool.c
 * find the counter value for trid event-based hotp tokens
 * prints a state file entry to stdout
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

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/hmac.h>
#include <unistd.h>

extern char *optarg;

static void
usage(const char *progname)
{
  fprintf(stderr, "usage: %s -1 <otp1> -2 <otp2> [-u <user>] [-k <key>] [-c <challenge>] [-i <counter>] [-f <counter>]\n", progname);
  exit(1);
}

static ssize_t
a2x(const char *s, unsigned char x[])
{
  unsigned i;
  size_t l = strlen(s);

  /*
   * We could just use sscanf, but we do this a lot, and have very
   * specific needs, and it's easy to implement, so let's go for it!
   */
  for (i = 0; i < l / 2; ++i) {
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

  return l/2;
}

/*
 * Convert an ASCII keystring to a keyblock.
 * Returns keylen on success, -1 otherwise.
 */
static int
keystring2keyblock(const char *keystring, unsigned char keyblock[])
{
  size_t l = strlen(keystring);

  /* 160-bit key with optional line ending */
  if ((l & ~1) != 40)
    return 1;

  return a2x(keystring, keyblock);
}

static char *
x2a(char s[17], unsigned char challenge[8])
{
  int i;

  for (i = 0; i < 8; i++)
    (void) sprintf(&s[i*2], "%02x", challenge[i]);
  return s;
}

static void
hotp(unsigned char challenge[], unsigned char keyblock[],
     unsigned char response[])
{
  uint32_t dbc;		/* "dynamic binary code" from HOTP draft */
  unsigned char md[20];
  unsigned md_len;

  /* 1. hmac */
  (void) HMAC(EVP_sha1(), keyblock, 20, challenge, 8, md, &md_len) ;

  /* 2. the truncate step is unnecessarily complex */
  {
    int offset;

    offset = md[19] & 0x0F;
    /* we can't just cast md[offset] because of alignment and endianness */
    dbc = (md[offset] & 0x7F) << 24 |
           md[offset + 1]     << 16 |
           md[offset + 2]     << 8  |
           md[offset + 3];
  }

  /* 3. int conversion and modulus (as string) */
  (void) sprintf((char *) response, "%06lu", dbc % 1000000L);
}

static void
c2c(uint64_t counter, unsigned char challenge[8])
{
  challenge[0] = counter >> 56;
  challenge[1] = counter >> 48;
  challenge[2] = counter >> 40;
  challenge[3] = counter >> 32;
  challenge[4] = counter >> 24;
  challenge[5] = counter >> 16;
  challenge[6] = counter >> 8;
  challenge[7] = counter;
}

int
main(int argc, char *argv[])
{
  char *username = "username";
  char *keystring = "0000000000000000000000000000000000000000";
  char *pass1 = NULL;
  char *pass2 = NULL;
  unsigned char challenge[8];
  unsigned char response[7];
  unsigned char keyblock[20];
  uint64_t counter;
  uint64_t initial = 0;
  uint64_t final = 0;

  int debug = 0;

  {
    int c;

    while ((c = getopt(argc, argv, ":d1:2:u:k:c:i:f:")) != -1) {
      switch (c) {
        case 'd': debug = 1; break;
        case '1': pass1 = optarg; break;
        case '2': pass2 = optarg; break;
        case 'u': username = optarg; break;
        case 'k': keystring = optarg; break;
        case 'c': initial = strtoull(optarg, NULL, 16); break;
        case 'i': initial = strtoull(optarg, NULL, 10); break;
        case 'f':
          if (optarg[0] == '+')	/* NOTE: must be given AFTER -i */
            final = initial + strtoull(&optarg[1], NULL, 10);
          else
            final = strtoull(optarg, NULL, 10);
          break;
        default:
          (void) fprintf(stderr, "%s: unknown option\n", argv[0]);
          exit(1);
      }
    }
  }
  if (!pass1 || !pass2)
    usage(argv[0]);
  if (strlen(pass1) != 6) {
    (void) fprintf(stderr, "%s: passcode 1 wrong length\n", argv[0]);
    exit(1);
  }
  if (strlen(pass2) != 6) {
    (void) fprintf(stderr, "%s: passcode 2 wrong length\n", argv[0]);
    exit(1);
  }

  if (!final)
    final = initial + 65536;

  (void) keystring2keyblock(keystring, keyblock);
  (void) memset(challenge, 0, 8);

  for (counter = initial; counter < final; ++counter) {
    c2c(counter, challenge);
    hotp(challenge, keyblock, response);
//#if 0
    (void) printf("%llu: %s\n", (long long unsigned int) counter, response);
//#endif
    if (!strcmp((char *) response, pass1)) {
      if (debug)
        (void) printf("matched %s at counter=%llu\n", pass1, (long long unsigned int) counter);

      /* matched first pass, look for 2nd */
      c2c(++counter, challenge);
      hotp(challenge, keyblock, response);
      if (!strcmp((char *) response, pass2)) {
        char s[17];

        /* token is now at the subsequent counter value */
        c2c(++counter, challenge);
        /* print the new state */
        (void) printf("5:%s:%s:::0:0:0:\n", username, x2a(s, challenge));
        exit(0);

      } else if (debug) {
        (void) fprintf(stderr,
                       "mismatch for counter=%llu, wanted: %s got: %s\n",
                       (long long unsigned int) counter, pass2, response);
        (void) fprintf(stderr, "continuing search\n");
      } /* else (pass2 does not match) */
    } /* if (pass1 matches) */
  } /* for (each event counter) */

  (void) fprintf(stderr, "%s: counter value not found\n", argv[0]);
  exit(1);
}
