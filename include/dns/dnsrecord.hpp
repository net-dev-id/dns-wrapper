/*
 * Copyright (c) 2024 Neeraj Jakhar
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

#pragma once

#include "dns/dnscommon.hpp"
#include "dnsquestion.hpp"
#include <cstdint>

struct Ipv4Address {
  uint32_t n;
};

struct Ipv6Address {
  uint32_t n1;
  uint32_t n2;
  uint32_t n3;
  uint32_t n4;
};

class DnsRecord : public DnsQuestion {
public:
  enum DnsRecordType { Answer, Authority, Additional };

  DnsRecord() = default;
  virtual ~DnsRecord() {
    if (Type == QT_OPT && Address.Opt.octet) {
      delete[] Address.Opt.octet;
    }
  }

  int Read(BytePacketBuffer *bpb);
  int Validate(PacketType pt) const;
  int Write(BytePacketBuffer *bpb) const;
  void UpdateFromQuestion(const DnsQuestion *);

  friend std::ostream &operator<<(std::ostream &stream, const DnsRecord &);

  DnsRecordType RecordType;
  uint16_t &UdpSize = Class;

  uint8_t GetExtendedRcode() const { return TTL >> 24; }
  bool GetDoBit() const { return (TTL & 0x8000) > 1; }
  bool GetAdBit() const { return (TTL & 0x2000) > 1; }

  uint32_t TTL;
  uint16_t Len;
  union {
    Ipv4Address Ipv4;     // A
    char NS[MAXDNAME];    // NS
    char CName[MAXDNAME]; // CNAME
    struct {              // MX
      uint16_t priority;
      char host[MAXDNAME];
    } MailExchange;
    Ipv6Address Ipv6; // AAAA
    struct {          // OPT
      uint8_t *octet;
    } Opt;
  } Address;
};
