/*
 * Created Date: Monday, August 26th 2024, 10:24:42 am
 * Author: Neeraj Jakhar
 * -----
 * Last Modified: Friday, 6th September 2024 9:25:49 am
 * Modified By: Neeraj Jakhar
 * -----
 * Copyright (c) 2024 Neeraj Jakhar
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * -----
 */

#include "dns/dnsquestion.hpp"
#include "dns/dnscommon.hpp"
#include "log.hpp"
#include "tp/sha256.hpp"
#include <cstring>

/*
 *                                  1  1  1  1  1  1
 *    0  1  2  3  4  5  6  7  8  9  0  1  2  3  4  5
 *  +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
 *  |                                               |
 *  /                     QNAME                     /
 *  /                                               /
 *  +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
 *  |                     QTYPE                     |
 *  +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
 *  |                     QCLASS                    |
 *  +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
 */

int DnsQuestion::ReadLabel(BytePacketBuffer *bpb, char *storage) {
  int jumps = 0;
  uint16_t pos = 0;

  Start = bpb->pos;
  int size = 0;
  uint8_t c, d;

  VREAD_U8(c, bpb);

  // Empty name is possible for psuedo header
  if (c == 0) {
    storage[0] = '\0';
    return E_NOERROR;
  }

  while (c && jumps < MAX_JUMPS) {
    if ((c & 0xc0) == 0xc0) {
      VREAD_U8(d, bpb);
      uint16_t t = (uint16_t(c ^ 0xc0) << 8 | d);
      if (t >= bpb->size) {
        LERROR_X << "Illegal label offset" << std::endl;
        return E_FORMERR;
      }
      pos = bpb->pos;
      bpb->pos = t;
      jumps += 1;
    } else if ((c & 0x40) == 0x40 || (c & 0x80) == 0x80) {
      return E_NOTIMP;
    } else {
      // In this case c is the number of bytes to be read
      if (bpb->pos + c >= bpb->size) {
        LERROR_X << "Illegal label size" << std::endl;
        return E_FORMERR;
      }

      for (uint8_t i = 0; i < c; i++) {
        storage[size++] = READ_BYTE(bpb);
      }
      storage[size++] = '.';
    }

    VREAD_U8(c, bpb);
  }

  if (jumps == MAX_JUMPS) {
    LERROR_X << "Too many label jumps" << std::endl;
    return E_FORMERR;
  }

  if (pos > 0) {
    bpb->pos = pos;
  }

  storage[--size] = '\0';
  return E_NOERROR;
}

int DnsQuestion::Read(BytePacketBuffer *bytePacketBuffer) {
  int code = ReadLabel(bytePacketBuffer, Name);
  if (code != SUCCESS) {
    return code;
  }

  VREAD_U16(Class, bytePacketBuffer);
  VREAD_U16(Type, bytePacketBuffer);

  return E_NOERROR;
}

int DnsQuestion::Validate(PacketType pt) const {
  if (pt == PacketType::IncomingRequest) {
    if (Type != QT_A && Type != QT_AAAA && Type != QT_NS && Type != QT_CNAME &&
        Type != QT_MX && Type != QT_OPT) {
      return E_NOTIMP;
    }

    if (Type != QT_OPT && Class != QTC_IN) {
      return E_NOTIMP;
    }
  }

  return E_NOERROR;
}

#define WRITE_LENGTH(bpb)                                                      \
  {                                                                            \
    uint16_t t = bpb->pos;                                                     \
    bpb->pos = pos;                                                            \
    WRITE_U8(bpb, len)                                                         \
    len = 0;                                                                   \
    pos = t;                                                                   \
    bpb->pos = ++t;                                                            \
  }

int DnsQuestion::WriteLabel(const char *in, BytePacketBuffer *bpb) const {
  uint16_t pos = bpb->pos++;
  int len = 0;

  for (const char *c = in; *c; c++) {
    if (*c == '.') {
      WRITE_LENGTH(bpb)
    } else {
      WRITE_BYTE(bpb, *c)
      len++;
    }
  }

  WRITE_LENGTH(bpb)
  bpb->buf[bpb->pos] = 0;
  return E_NOERROR;
}

int DnsQuestion::Write(BytePacketBuffer *bpb) const {
  int code = WriteLabel(Name, bpb);
  if (code != SUCCESS) {
    return code;
  }

  WRITE_U16(bpb, Class)
  WRITE_U16(bpb, Type)

  return E_NOERROR;
}

bool DnsQuestion::IsQuestionOfType(uint16_t type) { return Type == type; }

int DnsQuestion::UpdateDigest(SHA256_CTX *ctx) const {
  LDEBUG << "Hashing values:: Name: " << Name << ":" << strlen(Name)
         << ", Type: " << Type << ", Class: " << Class << std::endl;
  sha256_update(ctx, (const BYTE *)Name, strlen(Name));
  sha256_update(ctx, (const BYTE *)&Type, 2);
  sha256_update(ctx, (const BYTE *)&Class, 2);
  return 0;
}

std::ostream &operator<<(std::ostream &stream, const DnsQuestion &dq) {
  using namespace std;

  stream << "Name: " << dq.Name << ", Type: " << dq.Type
         << ", Class: " << dq.Class << endl;

  return stream;
}
