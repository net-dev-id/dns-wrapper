/*
 * Copyright (c) 2024 Neeraj Jakhar
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

#pragma once

struct InPacket;
class DnsPacket;
class DnsServer;
struct SocketData;

struct Input {
  const InPacket *ipacket;
  DnsPacket *packet;
  DnsServer *server;
  const SocketData *socketData;
};
