/*
 * Copyright (c) 2024 Neeraj Jakhar
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

#pragma once

#include "net/netcommon.h"

#define MAX_JUMPS 5

#define SUCCESS 0
#define ERROR_INVALID_PACKET 1

#define S_QUERY 0 // Standard DNS Query

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

class DnsObject {
public:
  virtual int Read(BytePacketBuffer *bpb) = 0;
  virtual int Validate(PacketType pt) const = 0;
  virtual int Write(BytePacketBuffer *bpb) const = 0;
};
