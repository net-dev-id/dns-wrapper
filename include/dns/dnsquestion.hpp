/*
 * Created Date: Monday, August 26th 2024, 10:17:24 am
 * Author: Neeraj Jakhar
 * -----
 * Last Modified: Tuesday, 3rd September 2024 12:12:49 am
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

#include "dnscommon.hpp"

#include <ostream>

struct SHA256_CTX;

class DnsQuestion : DnsObject {
public:
  DnsQuestion() {}
  virtual ~DnsQuestion() {}

  friend std::ostream &operator<<(std::ostream &stream, const DnsQuestion &);

  int Read(BytePacketBuffer *bpb);
  int Validate(PacketType pt) const;
  int Write(BytePacketBuffer *bpb) const;
  bool IsQuestionOfType(uint16_t type);

  int UpdateDigest(SHA256_CTX *) const;

protected:
  int ReadLabel(BytePacketBuffer *bpb, char *storage);
  int WriteLabel(const char *in, BytePacketBuffer *bpb) const;
  int updateDigest(SHA256_CTX *) const;

public:
  uint16_t Start;
  char Name[MAXDNAME];
  uint16_t Type;
  uint16_t Class;
};
