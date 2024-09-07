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
#include "dnscommon.hpp"
#include "dnspacket.hpp"
#include "rule/rule.hpp"

#include <boost/asio/io_context.hpp>
// #include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ip/udp.hpp>
#include <boost/asio/ip/v6_only.hpp>
#include <boost/compute/detail/lru_cache.hpp>
#include <boost/system/detail/error_code.hpp>
#include <cstddef>

#define ONE_HOUR 1 * 60 * 60;

// using boost::asio::ip::tcp;
using boost::asio::ip::udp;

class DnsServer {
public:
  DnsServer(boost::asio::io_context &io_context, uint16_t port,
            const ConfigReader *configReader);
  ~DnsServer();

private:
  void receive(boost::system::error_code ec, std::size_t, bool ipv4);

  bool processRequest(DnsPacket &packet, int &res,
                      const udp::endpoint &endpoint, bool ipv4);
  bool processUpstreamResponse(DnsPacket &packet, int &res,
                               const udp::endpoint &endpoint);

  void resolve(DnsPacket &packet, const udp::endpoint &endpoint, bool ipv4);
  void sendPacket(const DnsPacket &packet, const udp::endpoint &endpoint,
                  bool ipv4);
  void updateErrorResponse(DnsPacket &packet, int errCode);
  void updateRedirectResponse(DnsPacket &packet);

  void receive4();
  void receive6();

private:
  RuleMatcher ruleMatcher;
  PeerRequests peerRequests;

  udp::socket socket4;
  udp::endpoint endpoint4;
  BytePacketBuffer recvBuffer4;
  udp::socket socket6;
  udp::endpoint endpoint6;
  BytePacketBuffer recvBuffer6;

  const ConfigReader *configReader;
  UpstreamServerInfo *servers;
};
