/*
 * Copyright (c) 2024 Neeraj Jakhar
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

#pragma once

#include "net/netcommon.h"

#include <cstdint>

struct InPacket {
  int Read(RawPacketBuffer *bpb);

  union EthAddress EthDestination;
  union EthAddress EthSource;
  uint16_t EthProtocol;

  bool Ipv4;

  union IpAddress IpSource;
  union IpAddress IpDestination;
  uint8_t IpProtocol;

  Port SourcePort;
  Port DestinationPort;

  friend std::ostream &operator<<(std::ostream &stream, const InPacket &);
};
