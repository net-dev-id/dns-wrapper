/*
 * Copyright (c) 2024 Neeraj Jakhar
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "dns/dnsrecord.hpp"
#include "dns/dnscommon.hpp"
#include <cstdint>
#include <ostream>

void DnsRecord::UpdateFromQuestion(const DnsQuestion *question) {
  Start = question->Start;
  Type = question->Type;
  Class = question->Class;

  const char *c = question->Name;
  char *d = Name;
  while (*c) {
    *d++ = *c++;
  }
  *d = 0;
}

int DnsRecord::Read(BytePacketBuffer *bpb) {
  int code = DnsQuestion::Read(bpb);
  if (code != E_NOERROR) {
    return code;
  }

  VREAD_U32(TTL, bpb);
  VREAD_U16(Len, bpb);

  switch (Type) {
  case QT_A:
    VREAD_U32(Address.Ipv4.n, bpb);
    break;
  case QT_NS:
    code = ReadLabel(bpb, Address.NS);
    break;
  case QT_CNAME:
    code = ReadLabel(bpb, Address.CName);
    break;
  case QT_MX:
    VREAD_U16(Address.MailExchange.priority, bpb);
    code = ReadLabel(bpb, Address.MailExchange.host);
    break;
  case QT_AAAA:
    VREAD_U32(Address.Ipv6.n1, bpb);
    VREAD_U32(Address.Ipv6.n2, bpb);
    VREAD_U32(Address.Ipv6.n3, bpb);
    VREAD_U32(Address.Ipv6.n4, bpb);
    break;
  case QT_OPT:
    // Psuedo record is only allowed as additional record.
    if (RecordType != DnsRecordType::Additional) {
      LERROR_X << "Psuedo record not allowed here " << RecordType << std::endl;
      return E_FORMERR;
    }
    if (Len > 0) {
      Address.Opt.octet = new uint8_t[Len];
      for (int i = 0; i < Len; i++) {
        VREAD_BYTE(Address.Opt.octet[i], bpb);
      }
    } else {
      Address.Opt.octet = nullptr;
    }
    break;
  default:
    return E_SERVFAIL;
  }

  return code;
}

int DnsRecord::Validate(PacketType pt) const {
  int code = DnsQuestion::Validate(pt);
  if (code != E_NOERROR) {
    return code;
  }

  switch (Type) {
  case QT_A:
    if (Len != 4) {
      LERROR_X << "Invalid length for record: " << Len << std::endl;
      return E_FORMERR;
    }
    break;
  case QT_AAAA:
    if (Len != 16) {
      LERROR_X << "Invalid length for record: " << Len << std::endl;
      return E_FORMERR;
    }
    break;
    // TODO add other validations
  }

  return E_NOERROR;
}

int DnsRecord::Write(BytePacketBuffer *bpb) const {
  int code = DnsQuestion::Write(bpb);
  if (code != E_NOERROR) {
    return code;
  }

  WRITE_U32(bpb, TTL)
  WRITE_U16(bpb, Len);
  switch (Type) {
  case QT_A:
    WRITE_U32(bpb, Address.Ipv4.n)
    break;
  case QT_NS:
    code = WriteLabel(Address.NS, bpb);
    break;
  case QT_CNAME:
    code = WriteLabel(Address.CName, bpb);
    break;
  case QT_MX:
    WRITE_U16(bpb, Address.MailExchange.priority);
    code = WriteLabel(Address.MailExchange.host, bpb);
    break;
  case QT_AAAA:
    WRITE_U32(bpb, Address.Ipv6.n1);
    WRITE_U32(bpb, Address.Ipv6.n2);
    WRITE_U32(bpb, Address.Ipv6.n3);
    WRITE_U32(bpb, Address.Ipv6.n4);
    break;
  }

  return code;
}

std::ostream &operator<<(std::ostream &stream, const Ipv4Address &addr) {
  using namespace std;

  stream << ((addr.n & 0xff000000) >> 24) << "." << ((addr.n & 0xff0000) >> 16)
         << "." << ((addr.n & 0xff00) >> 8) << "." << (addr.n & 0xff);

  return stream;
}

#define IPV6_OUT(n)                                                            \
  ((addr.n & 0xff000000) >> 24)                                                \
      << ":" << ((addr.n & 0xff0000) >> 16) << ":" << ((addr.n & 0xff00) >> 8) \
      << ":" << (addr.n & 0xff)

std::ostream &operator<<(std::ostream &stream, const Ipv6Address &addr) {
  using namespace std;

  stream << IPV6_OUT(n1) << ":" << IPV6_OUT(n2) << ":" << IPV6_OUT(n3) << ":"
         << IPV6_OUT(n4);

  return stream;
}

std::ostream &operator<<(std::ostream &stream, const DnsRecord &dp) {
  using namespace std;

  stream << "Name: " << dp.Name << ", Type: " << dp.Type
         << ", Class: " << dp.Class << ", TTL: " << dp.TTL
         << ", Len: " << dp.Len << ", Address: ";

  switch (dp.Type) {
  case QT_A:
    stream << dp.Address.Ipv4;
    break;
  case QT_AAAA:
    stream << dp.Address.Ipv6;
    break;
  default:
    stream << " Unsupported record type : " << dp.Type;
  }

  stream << endl;

  return stream;
}
