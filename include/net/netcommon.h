/*
 * Copyright (c) 2024 Neeraj Jakhar
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

#pragma once

#include "common.h"
#include "log.hpp"

#include <array>
#include <cstddef>
#include <cstdint>

#define PACKETSZ 512
#define EDNS_PKTSZ                                                             \
  1232 /* default max EDNS.0 UDP packet from from  /dnsflagday.net/2020 */
#define SAFE_PKTSZ                                                             \
  1232 /* "go anywhere" UDP packet size, see https://dnsflagday.net/2020/ */
#define MAXDNAME 1025
#define RRFIXEDSZ 10 /* #/bytes of fixed data in r record */
#define MAX_PACKET_SZ EDNS_PKTSZ + MAXDNAME + RRFIXEDSZ
#define PACKET_SZ_MAX 65535

#define E_NOERROR 0  // DNS Query completed successfully
#define E_FORMERR 1  // DNS Query Format Error
#define E_SERVFAIL 2 // Server failed to complete the DNS request
#define E_NXDOMAIN 3 // Domain name does not exist
#define E_NOTIMP 4   // Function not implemented
#define E_REFUSED 5  // The server refused to answer for the query
#define E_YXDOMAIN 6 // Name that should not exist, does exist
#define E_XRRSET 7   // RRset that should not exist, does exist
#define E_NOTAUTH 8  // Server not authoritative for the zone
#define E_NOTZONE 9  // Name not in zone

#define PEEK_BYTE(bb, i) bb->buf[bb->pos + i]
#define READ_BYTE(bb) bb->buf[bb->pos++]
#define READ_U8(bb) uint8_t(READ_BYTE(bb))
#define READ_U16(bb)                                                           \
  uint16_t(PEEK_BYTE(bb, 0)) << 8 | PEEK_BYTE(bb, 1);                          \
  bb->pos += 2;
#define READ_U32(bb)                                                           \
  uint32_t(PEEK_BYTE(bb, 0)) << 24 | uint32_t(PEEK_BYTE(bb, 1)) << 16 |        \
      uint32_t(PEEK_BYTE(bb, 2)) << 8 | uint32_t(PEEK_BYTE(bb, 3));            \
  bb->pos += 4;

#define _VALIDATE(bb, n)                                                       \
  if (bb->pos + n > bb->size) {                                                \
    LERROR_X << "Expected " << n << " bytes: pos=" << bb->pos                  \
             << ", size=" << bb->size << std::endl;                            \
    return E_FORMERR;                                                          \
  }
#define VREAD_BYTE(v, bb)                                                      \
  _VALIDATE(bb, 1)                                                             \
  v = READ_BYTE(bb)
#define _VREAD(bb, v, n, b)                                                    \
  _VALIDATE(bb, n)                                                             \
  v = READ_U##b(bb);
#define VREAD_U8(v, bb) _VREAD(bb, v, 1, 8)
#define VREAD_U16(v, bb) _VREAD(bb, v, 2, 16)
#define VREAD_U32(v, bb) _VREAD(bb, v, 2, 32)

#define WRITE_BYTE(bb, b) bb->buf[bb->pos++] = b;
#define WRITE_U8(bb, b) WRITE_BYTE(bb, b)
#define WRITE_U16(bb, b) WRITE_BYTE(bb, b >> 8) WRITE_BYTE(bb, b & 0xff)
#define WRITE_U32(bb, b)                                                       \
  WRITE_BYTE(bb, b >> 24)                                                      \
  WRITE_BYTE(bb, b >> 16 & 0xff)                                               \
  WRITE_BYTE(bb, b >> 8 & 0xff) WRITE_BYTE(bb, b & 0xff)

#define PACKET_SZ_VALID(bb, bytes) (bb->pos + bytes <= bb->size)

template <std::size_t N> struct PacketBuffer {
  std::array<uint8_t, N> buf{};
  std::size_t pos{0};
  std::size_t size{0};
  std::size_t udp{0};
};

typedef PacketBuffer<MAX_PACKET_SZ> BytePacketBuffer;
typedef PacketBuffer<PACKET_SZ_MAX> RawPacketBuffer;

union EthAddress {
  uint8_t v[ETH_ADDR_LEN];
};

#define IPV6_SIZE 4

union IpAddress {
  uint32_t Ipv4;
  uint8_t Ipv4v[4];
  uint32_t Ipv6[IPV6_SIZE];
  uint8_t Ipv6v[4 * IPV6_SIZE];
};

typedef uint16_t Port;
