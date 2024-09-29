/*
 * Copyright (c) 2024 Neeraj Jakhar
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

#pragma once

#include "bookkeeping/ethmappings.hpp"
#include "bookkeeping/peer.hpp"
#include "bookkeeping/server.hpp"
#include "config.hpp"
#include "dnspacket.hpp"
#include "net/netcommon.h"
#include "rule/shm.hpp"

#include <boost/asio/generic/raw_protocol.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/udp.hpp>
#include <boost/asio/ip/v6_only.hpp>
#include <boost/compute/detail/lru_cache.hpp>
#include <boost/system/detail/error_code.hpp>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

#define ONE_HOUR 1 * 60 * 60;

using boost::asio::generic::raw_protocol;
using boost::asio::ip::udp;

class RuleEngine;

struct SocketData {
  SocketData(boost::asio::io_context &ioContext)
      : socket(ioContext), endpoint{}, recvBuffer{} {}

  raw_protocol::socket socket;
  raw_protocol::endpoint endpoint;
  RawPacketBuffer recvBuffer;
};

class DnsServer {
public:
  DnsServer(boost::asio::io_context &io_context, uint16_t port,
            const ConfigReader *configReader, ShmRuleEngine *ruleEngine);
  ~DnsServer();

  void Resolve(DnsPacket *p, const udp::endpoint *e, const bool i);

  void Redirect(DnsPacket *p, const udp::endpoint *e, const bool i,
                const union IpAddress &target);

private:
  void initUpstreamServers();
  void startRawSocketScan(boost::asio::io_context &io_context,
                          const uint16_t &port);
  void startDnsListeners(const uint16_t &port);

  void receive(boost::system::error_code ec, std::size_t, bool ipv4);
  void receiveRawData(boost::system::error_code ec, std::size_t, bool ipv4,
                      SocketData *d);

  bool processRequest(DnsPacket &packet, int &res,
                      const udp::endpoint &endpoint, bool ipv4);
  bool processUpstreamResponse(DnsPacket &packet, int &res,
                               udp::endpoint &endpoint);

  void resolve(DnsPacket &packet, const udp::endpoint &endpoint, bool ipv4);
  void sendPacket(const DnsPacket &packet, const udp::endpoint &endpoint,
                  bool ipv4);
  void updateErrorResponse(DnsPacket &packet, const uint8_t &errCode);
  void updateRedirectResponse(DnsPacket &packet,
                              const union IpAddress &target) const;

  void receive4();
  void receive6();
  void receive(SocketData *d);

private:
  PeerRequests peerRequests;
  EthMappings ethmappings;

  udp::socket socket4;
  udp::endpoint endpoint4;
  BytePacketBuffer recvBuffer4;
  udp::socket socket6;
  udp::endpoint endpoint6;
  BytePacketBuffer recvBuffer6;
  std::vector<std::unique_ptr<SocketData>> socketData;

  const ConfigReader *configReader;
  ShmRuleEngine *ruleEngine;
  std::list<std::unique_ptr<UpstreamServerInfo>> servers;
};
