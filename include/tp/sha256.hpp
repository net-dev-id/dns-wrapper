/*
 * Copyright (c) 2024 Neeraj Jakhar
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

#pragma once

#include <cstddef>

#define SHA256_BLOCK_SIZE 32 /* SHA256 outputs a 32 byte digest */

typedef unsigned char _BYTE; /* 8-bit byte */
typedef unsigned int
  _WORD; /* 32-bit word, change to "long" for 16-bit machines */

struct SHA256_CTX {
  _BYTE data[64];
  _WORD datalen;
  unsigned long long bitlen;
  _WORD state[8];
};

void sha256_init(SHA256_CTX *ctx);
void sha256_update(SHA256_CTX *ctx, const _BYTE data[], size_t len);
void sha256_final(SHA256_CTX *ctx, _BYTE hash[]);
