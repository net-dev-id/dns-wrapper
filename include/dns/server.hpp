/*
 * Copyright (c) 2024 Neeraj Jakhar
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

#pragma once

#include "bookkeeping/peer.hpp"
#include "bookkeeping/server.hpp"
#include "config.hpp"
#include "dnspacket.hpp"
#include "net/packet.hpp"
#include "rule/engine.hpp"

#include <boost/asio/io_context.hpp>
#include <boost/asio/generic/raw_protocol.hpp>
#include <boost/asio/ip/udp.hpp>
#include <boost/asio/ip/v6_only.hpp>
#include <boost/compute/detail/lru_cache.hpp>
#include <boost/system/detail/error_code.hpp>
#include <cstddef>
#include <vector>

#define ONE_HOUR 1 * 60 * 60;

using boost::asio::generic::raw_protocol;

class RuleEngine;

struct SocketData {
  std::size_t index;
  raw_protocol::socket *socket;
  raw_protocol::endpoint endpoint;
  BytePacketBuffer recvBuffer;
};

class DnsServer {
public:
  DnsServer(boost::asio::io_context &io_context, uint16_t port,
            const ConfigReader *configReader);
  ~DnsServer();

  void Resolve(const SocketData *d, DnsPacket *p, const InPacket *ip);

  void Redirect(const SocketData *d, DnsPacket *p, const InPacket *ip);

private:
  void receive(boost::system::error_code ec, std::size_t, bool ipv4,
               SocketData &d);

  bool processRequest(const SocketData &d, DnsPacket &packet, int &res,
                      const InPacket &ipacket);
  bool processUpstreamResponse(DnsPacket &packet, int &res,
                               const InPacket &ipacket);

  void resolve(const SocketData &d, DnsPacket &packet, const InPacket &ipacket);
  void sendPacket(const SocketData &d, const DnsPacket &packet,
                  const InPacket &ipacket);
  void updateErrorResponse(DnsPacket &packet, const uint8_t &errCode);
  void updateRedirectResponse(DnsPacket &packet) const;
  void receive(SocketData &d);

private:
  RuleEngine ruleEngine;
  PeerRequests peerRequests;

  std::vector<SocketData> socketData;

  const ConfigReader *configReader;
  UpstreamServerInfo *servers;
};
