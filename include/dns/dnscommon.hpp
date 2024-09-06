/*
 * Created Date: Monday, August 26th 2024, 10:08:38 am
 * Author: Neeraj Jakhar
 * -----
 * Last Modified: Friday, 6th September 2024 8:55:09 am
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

#include "log.hpp"
#include <array>

#define PACKETSZ 512
#define EDNS_PKTSZ                                                             \
  1232 /* default max EDNS.0 UDP packet from from  /dnsflagday.net/2020 */
#define SAFE_PKTSZ                                                             \
  1232 /* "go anywhere" UDP packet size, see https://dnsflagday.net/2020/ */
#define MAXDNAME 1025
#define RRFIXEDSZ 10 /* #/bytes of fixed data in r record */
#define MAX_PACKET_SZ EDNS_PKTSZ + MAXDNAME + RRFIXEDSZ
#define MAX_JUMPS 5

#define SUCCESS 0
#define ERROR_INVALID_PACKET 1

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

#define S_QUERY 0 // Standard DNS Query

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

/* RFC-8914 extended errors, negative values are our definitions */
#define EDE_UNSET -1         /* No extended DNS error available */
#define EDE_OTHER 0          /* Other */
#define EDE_USUPDNSKEY 1     /* Unsupported DNSKEY algo */
#define EDE_USUPDS 2         /* Unsupported DS Digest */
#define EDE_STALE 3          /* Stale answer */
#define EDE_FORGED 4         /* Forged answer */
#define EDE_DNSSEC_IND 5     /* DNSSEC Indeterminate  */
#define EDE_DNSSEC_BOGUS 6   /* DNSSEC Bogus */
#define EDE_SIG_EXP 7        /* Signature Expired */
#define EDE_SIG_NYV 8        /* Signature Not Yet Valid  */
#define EDE_NO_DNSKEY 9      /* DNSKEY missing */
#define EDE_NO_RRSIG 10      /* RRSIGs missing */
#define EDE_NO_ZONEKEY 11    /* No Zone Key Bit Set */
#define EDE_NO_NSEC 12       /* NSEC Missing  */
#define EDE_CACHED_ERR 13    /* Cached Error */
#define EDE_NOT_READY 14     /* Not Ready */
#define EDE_BLOCKED 15       /* Blocked */
#define EDE_CENSORED 16      /* Censored */
#define EDE_FILTERED 17      /* Filtered */
#define EDE_PROHIBITED 18    /* Prohibited */
#define EDE_STALE_NXD 19     /* Stale NXDOMAIN */
#define EDE_NOT_AUTH 20      /* Not Authoritative */
#define EDE_NOT_SUP 21       /* Not Supported */
#define EDE_NO_AUTH 22       /* No Reachable Authority */
#define EDE_NETERR 23        /* Network error */
#define EDE_INVALID_DATA 24  /* Invalid Data */
#define EDE_SIG_E_B_V 25     /* Signature Expired before Valid */
#define EDE_TOO_EARLY 26     /* To Early */
#define EDE_UNS_NS3_ITER 27  /* Unsupported NSEC3 Iterations Value */
#define EDE_UNABLE_POLICY 28 /* Unable to conform to policy */
#define EDE_SYNTHESIZED 29   /* Synthesized */

#define QT_Unknown 0
#define QT_A 1
#define QT_NS 2
#define QT_CNAME 5
#define QT_MX 15
#define QT_AAAA 28
#define QT_OPT 41

#define QTC_IN 1

enum PacketType {
  IncomingRequest = 1,
  IncomingResponse,
  OutgoingRequest,
  OutgoingResponse
};

struct BytePacketBuffer {
  std::array<uint8_t, MAX_PACKET_SZ> buf{};
  uint16_t pos{0};
  uint16_t size{0};
};

class DnsObject {
public:
  virtual int Read(BytePacketBuffer *bpb) = 0;
  virtual int Validate(PacketType pt) const = 0;
  virtual int Write(BytePacketBuffer *bpb) const = 0;
};
