/*
 * Created Date: Sunday, September 1st 2024, 10:49:39 pm
 * Author: Neeraj Jakhar
 * -----
 * Last Modified: Monday, 2nd September 2024 12:21:06 am
 * Modified By: Neeraj Jakhar
 * -----
 * Copyright (c) 2024 Neeraj Jakhar
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * -----
 */

#pragma once

#include <cstddef>

#define SHA256_BLOCK_SIZE 32 /* SHA256 outputs a 32 byte digest */

typedef unsigned char BYTE; /* 8-bit byte */
typedef unsigned int
    WORD; /* 32-bit word, change to "long" for 16-bit machines */

struct SHA256_CTX {
  BYTE data[64];
  WORD datalen;
  unsigned long long bitlen;
  WORD state[8];
};

void sha256_init(SHA256_CTX *ctx);
void sha256_update(SHA256_CTX *ctx, const BYTE data[], size_t len);
void sha256_final(SHA256_CTX *ctx, BYTE hash[]);
