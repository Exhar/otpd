/*
 * $Id: //trid/release/otpd-3.1.0/helix.c#1 $
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
 * Copyright 2005,2006,2008 TRI-D Systems, Inc.
 */

#include "ident.h"
RCSID("$Id: //trid/release/otpd-3.1.0/helix.c#1 $")

#include <inttypes.h>
#include <string.h>
#include <sys/types.h>

#include "extern.h"
#include "bitmanip.h"
#include "helix.h"


/*
 * WARNING: All char * args (helix_setkey(), helix_nonce(), helix_encrypt(),
 * helix_decrypt()) must be uint32 (4 byte) aligned.  The macros defined in
 * helix.h are optimized for little-endian architectures to assume uint32
 * alignment!
 */


/* helix block function */
static uint32_t
helix_block(const helix_scontext_t *scontext, helix_dcontext_t *dcontext,
            uint32_t p)
{
  uint32_t s;
  uint32_t x0 = scontext->x0[MOD8(dcontext->iplus8)];
  uint32_t x1 = dcontext->x1[MOD8(dcontext->iplus8)] + dcontext->iplus8;

  /*
   * Helix has 5 state words because x86 has 5 available registers.
   * Let's make sure the compiler knows our intent.
   */
  register uint32_t z0 = dcontext->z[0];
  register uint32_t z1 = dcontext->z[1];
  register uint32_t z2 = dcontext->z[2];
  register uint32_t z3 = dcontext->z[3];
  register uint32_t z4 = dcontext->z[4];

  /* output of a block is the initial z[0] */
  s = z0;

  /*
   * Add bits 31..62 of iplus8 to x1.  See _Helix_, sec. 3.3.
   *
   * x1 includes 2 factors related to i, which we must add in for each block.
   * One of these factors is simply i itself, which we directly add in.  The
   * other factor, x'i, encodes the input key (u) length or bits 31..62 of
   * i + 8, depending on the value of i % 4.
   *
   * To optimize the x'i factor, we store iplus8 instead of just i.  This
   * saves us from adding in the +8 constant part of x'i, as well as the +8
   * addition to the upper bits.
   *
   * We also precompute the static part of x1 and i by subtracting 2^31 and
   * adding 2^31, respectively.  These cancel each other out when adding the
   * iplus8 factor to x1, and encode bits 31..62 (instead of 32..63) directly
   * into the high-order 32-bits of iplus8.
   *
   * One 32-bit hardware, this doesn't present any advantage since 64-bit
   * math is done in two 32-bit parts anyway, so we could just keep iplus8
   * in two 32-bit parts ourselves, and increment the high-order part
   * "one bit early".
   *
   * But on 64-bit hardware, we want to use native 64-bit math (iplus8++).
   * This then leaves us with the problem of how to access the high-order
   * part directly.  See helix_setkey() for that.
   *
   * We could use double-indirect referencing and avoid the test+branch
   * (create iplus8hp[8] and point the mod4=3 indices to iplus8 and the
   * other indices to a 0 value), but that ranges from slightly slower on
   * x86 hardware to much slower on USII hardware.
   */
  if (MOD4(dcontext->iplus8) == 3)
    x1 += *dcontext->iplus8hp;

  /*
   * Now we perform the block function to update z.
   * Each of the 20 rounds is an addition and/or xor, and a rotate.
   */
  z0 += z3;		z3 = rol(z3, 15);
  z1 += z4;		z4 = rol(z4, 25);
  z2 ^= z0;		z0 = rol(z0, 9);
  z3 ^= z1;		z1 = rol(z1, 10);
  z4 += z2;		z2 = rol(z2, 17);

  z0 ^= z3 + x0;	z3 = rol(z3, 30);
  z1 ^= z4;		z4 = rol(z4, 13);
  z2 += z0;		z0 = rol(z0, 20);
  z3 += z1;		z1 = rol(z1, 11);
  z4 ^= z2;		z2 = rol(z2, 5);

  z0 += z3 ^ p;		z3 = rol(z3, 15);
  z1 += z4;		z4 = rol(z4, 25);
  z2 ^= z0;		z0 = rol(z0, 9);
  z3 ^= z1;		z1 = rol(z1, 10);
  z4 += z2;		z2 = rol(z2, 17);

  z0 ^= z3 + x1;	z3 = rol(z3, 30);
  z1 ^= z4;		z4 = rol(z4, 13);
  z2 += z0;		z0 = rol(z0, 20);
  z3 += z1;		z1 = rol(z1, 11);
  z4 ^= z2;		z2 = rol(z2, 5);

  dcontext->iplus8++;

  /* copy local vars back to context */
  dcontext->z[0] = z0;
  dcontext->z[1] = z1;
  dcontext->z[2] = z2;
  dcontext->z[3] = z3;
  dcontext->z[4] = z4;

  /* return the initial z[0] as the keystream word */
  return s;
}

/* setup working key and x0 key schedule */
helix_scontext_t *
helix_setkey(helix_scontext_t *scontextp, const unsigned char *u, size_t l)
{
  uint32_t k[40];
  int i;

  unsigned char u32[32];
  unsigned char *up;

  helix_scontext_t *scontext;
  helix_dcontext_t dcontext;

  /* zero-extend key, if necessary */
  if (l < 32) {
    (void) memset(u32, 0, sizeof(u32));
    (void) memcpy(u32, u, l);
    up = u32;
  } else if (l > 32) {
    return NULL;
  } else {
    up = (unsigned char *) u;
  }

  if (scontextp)
    scontext = scontextp;
  else
    scontext = xmalloc(sizeof (helix_scontext_t));

  /* convert little-endian key bytes to host-endian words k[32]..k[39] */
#ifdef _LITTLE_ENDIAN
  (void) memcpy(&k[32], up, 32);
#else
  LEUCHAR2HEUINT32(k[32], &up[0]);
  LEUCHAR2HEUINT32(k[33], &up[4]);
  LEUCHAR2HEUINT32(k[34], &up[8]);
  LEUCHAR2HEUINT32(k[35], &up[12]);
  LEUCHAR2HEUINT32(k[36], &up[16]);
  LEUCHAR2HEUINT32(k[37], &up[20]);
  LEUCHAR2HEUINT32(k[38], &up[24]);
  LEUCHAR2HEUINT32(k[39], &up[28]);
#endif

  (void) memset(scontext, 0, sizeof(*scontext));
  (void) memset(&dcontext, 0, sizeof(dcontext));

  /* setup pointer to high-order 32 bits of iplus8 */
  dcontext.iplus8hp = (uint32_t *) &dcontext.iplus8;
#ifdef _LITTLE_ENDIAN
  dcontext.iplus8hp++;
#endif

  /* setup x1 to be constant (0) for key mixing */
  for (i = 0; i < 8; ++i)
   dcontext.x1[i] = -i;

  /* mix the key */
  for (i = 7; i >= 0; --i) {
    dcontext.z[0] = k[4 * i + 4];
    dcontext.z[1] = k[4 * i + 5];
    dcontext.z[2] = k[4 * i + 6];
    dcontext.z[3] = k[4 * i + 7];
    dcontext.z[4] = l + 64;
    (void) helix_block(scontext, &dcontext, 0);	/* NOTE: x0 = 0, x1 = 0 */
    k[4 * i + 0] = dcontext.z[0] ^ k[4 * i + 8];
    k[4 * i + 1] = dcontext.z[1] ^ k[4 * i + 9];
    k[4 * i + 2] = dcontext.z[2] ^ k[4 * i + 10];
    k[4 * i + 3] = dcontext.z[3] ^ k[4 * i + 11];
  }

  /* k[0] .. k[7] form the working key */
  (void) memcpy(scontext->k, k, 32);

  scontext->l = l;

  /* x0 key schedule */
  (void) memcpy(scontext->x0, scontext->k, 32);

  scontext->magic = HELIX_MAGIC;
  return scontext;
}

/* setup working nonce and x1 key schedule, and initialize stream */
void
helix_nonce(const helix_scontext_t *scontext, helix_dcontext_t *dcontext,
            unsigned char n[HELIX_NONCE_LEN])
{
  int i;

  /* convert little-endian nonce bytes to host-endian words n[0]..n[3] */
#ifdef _LITTLE_ENDIAN
  (void) memcpy(dcontext->n, n, 16);
#else
  LEUCHAR2HEUINT32(dcontext->n[0], &n[0]);
  LEUCHAR2HEUINT32(dcontext->n[1], &n[4]);
  LEUCHAR2HEUINT32(dcontext->n[2], &n[8]);
  LEUCHAR2HEUINT32(dcontext->n[3], &n[12]);
#endif
  /* extend nonce to 8 words */
  for (i = 0; i < 4; ++i)
    dcontext->n[i + 4] = i - dcontext->n[i];

  /* setup x1 key schedule */
  for (i = 0; i < 8; ++i)
    dcontext->x1[i] = scontext->k[MOD8(i + 4)] +
                      dcontext->n[i] +
                      ((MOD4(i % 4) == 1) ? 4 * scontext->l : 0) -
                     0x80000000;	/* see helix_block() */

  /* initialize the stream */
  dcontext->z[0] = scontext->k[3] ^ dcontext->n[0];
  dcontext->z[1] = scontext->k[4] ^ dcontext->n[1];
  dcontext->z[2] = scontext->k[5] ^ dcontext->n[2];
  dcontext->z[3] = scontext->k[6] ^ dcontext->n[3];
  dcontext->z[4] = scontext->k[7];
  dcontext->iplus8 = 0;			/* start at i = -8   */
  dcontext->iplus8 += 0x80000000;	/* see helix_block() */
  /* setup pointer to high-order 32 bits of iplus8 */
  dcontext->iplus8hp = (uint32_t *) &dcontext->iplus8;
#ifdef _LITTLE_ENDIAN
  dcontext->iplus8hp++;
#endif
  for (i = 0; i < 8; ++i)
    (void) helix_block(scontext, dcontext, 0);
}

/*
 * encrypt (in-place ok)
 *
 * Unlike typical stream ciphers, helix_encrypt() cannot be called
 * repeatedly, because a final partial plaintext word changes the state
 * differently than a full word, and MAC generation also changes the state.
 * So, helix_encrypt("a"); helix_encrypt("b"); does not produce the same
 * result as helix_encrypt("ab").
 *
 * helix_encrypt() and helix_nonce() must always be called as a pair.
 *
 * We *could* squirrel away partial state and allow multiple calls, but
 * current usage doesn't require this.
 */
int
helix_encrypt(const helix_scontext_t *scontext, helix_dcontext_t *dcontext,
              const unsigned char *p, size_t l, unsigned char *c,
              unsigned char m[HELIX_MAC_LEN])
{
  uint32_t s;
  uint32_t word;
  size_t i, j;

  if (scontext->magic != HELIX_MAGIC)
    return -1;

  /* encrypt the full words of p */
  for (i = 0; l - i >= 4; i += 4) {
    /* convert little-endian plaintext bytes to a host-endian word */
    LEUCHAR2HEUINT32(word, &p[i]);
    /* mix in plaintext and extract a keystream word */
    s = helix_block(scontext, dcontext, word);
    /* produce a ciphertext word */
    LEUCHARXORHEUINT32(&c[i], &p[i], s);
  }

  /* encrypt a leftover partial plaintext word */
  if (i < l) {
    /* zero-extend little-endian plaintext bytes to a host-endian word */
    LEUCHAR2HEUINT32ZE(word, &p[i], l - i);
    /* mix in plaintext and extract a keystream word */
    s = helix_block(scontext, dcontext, word);
    /* produce a ciphertext word */
    word ^= s;
    /* fill in last block of ciphertext */
    HEUINT32P2LEUCHAR(&c[i], word, l - i);
  }

  /* generate the MAC */
  dcontext->z[0] ^= HELIX_MAGIC;
  for (j = 0; j < 8; ++j)
    (void) helix_block(scontext, dcontext, l - i);
  for (j = 0; j < 4; ++j) {
    s = helix_block(scontext, dcontext, l - i);
    HEUINT32TOLEUCHAR(&m[4 * j], s);
  }

  return 0;
}

/*
 * decrypt (in-place ok)
 *
 * See comment in helix_encrypt() about repeated calls.
 */
int
helix_decrypt(const helix_scontext_t *scontext, helix_dcontext_t *dcontext,
              const unsigned char *c, size_t l, unsigned char *p,
              unsigned char m[HELIX_MAC_LEN])
{
  uint32_t s;
  uint32_t word;
  size_t i, j;

  if (scontext->magic != HELIX_MAGIC)
    return -1;

  /* decrypt the full words of c */
  for (i = 0; l - i >= 4; i += 4) {
    /* mix in ciphertext and extract a keystream word */
    LEUCHAR2HEUINT32(word, &c[i]);
    word ^= dcontext->z[0];
    s = helix_block(scontext, dcontext, word);
    /* produce a plaintext word */
    LEUCHARXORHEUINT32(&p[i], &c[i], s);
  }

  /* decrypt a leftover partial ciphertext word */
  if (i < l) {
    /* decrypt the last partial ciphertext word */
    LEUCHARXORHEUINT32P(&p[i], &c[i], dcontext->z[0], l - i);
    /*
     * Decryption is done.
     * Run the last partial word through helix_block() to setup for MAC.
     */
    LEUCHAR2HEUINT32ZE(word, &p[i], l - i);
    (void) helix_block(scontext, dcontext, word);
  }

  /* generate the MAC */
  dcontext->z[0] ^= HELIX_MAGIC;
  for (j = 0; j < 8; ++j)
    (void) helix_block(scontext, dcontext, l - i);
  for (j = 0; j < 4; ++j) {
    s = helix_block(scontext, dcontext, l - i);
    HEUINT32TOLEUCHAR(&m[4 * j], s);
  }

  return 0;
}
