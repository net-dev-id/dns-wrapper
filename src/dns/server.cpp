/*
 * Copyright (c) 2024 Neeraj Jakhar
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

#include <algorithm>
#ifdef __linux
#include <arpa/inet.h>
#include <ifaddrs.h>
#define IF_CLASS ifaddrs
#else /* WIN32 */
#include <iphlpapi.h>
#define IF_CLASS IP_ADAPTER_ADDRESSES
#include <ws2tcpip.h>
#endif /* __linux */

#include <boost/asio.hpp>
#include <boost/asio/ip/address_v4.hpp>
#include <boost/asio/ip/address_v6.hpp>
#include <boost/asio/ip/impl/address_v4.ipp>
#include <boost/asio/ip/impl/address_v6.ipp>
#include <boost/asio/ip/udp.hpp>
#include <boost/chrono.hpp>
#include <boost/format.hpp>
#include <boost/thread/thread.hpp>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <ctime>
#include <ostream>

#include "bookkeeping/peer.hpp"
#include "bookkeeping/server.hpp"
#include "dns/dnscommon.hpp"
#include "dns/dnspacket.hpp"
#include "dns/server.hpp"
#include "log.hpp"
#include "net/if.hpp"
#include "net/packet.hpp"
#include "rule/input.hpp"
#include "tp/sha256.hpp"

#define THREAD_SLEEP_AFTER_FAIL 100
#define UDP_RESIZE_PKG_SZ_AFTER                                                \
  60 /* How often to reset our idea of max packet size. */

using boost::asio::generic::raw_protocol;

DnsServer::DnsServer(boost::asio::io_context &io_context, uint16_t port,
                     const ConfigReader *configReader)
    : configReader(configReader) {
  using namespace boost::asio::ip;

  servers = nullptr;
  UpstreamServerInfo **t = &servers;
  for (auto server : configReader->servers) {
    if (server.protocol == Protocol::Tcp) {
      continue;
    }

    int af;
    boost::asio::ip::address targetIP;
    bool ipv4;
    UpstreamServerInfo *usi = new UpstreamServerInfo;
    if (server.protocolVersion == IpProtocolVersion::Ipv4) {
      af = AF_INET;
      targetIP = make_address_v4(server.hostIp);
      ipv4 = true;
      usi->ipv4 = true;
    } else {
      af = AF_INET6;
      targetIP = make_address_v6(server.hostIp);
      ipv4 = false;
    }

    if (inet_pton(af, server.hostIp.c_str(), &usi->address) != 1) {
      LERROR << "Unable to parse IP address: " << server.hostIp << std::endl;
      delete usi;
      continue;
    }
    usi->endpoint = udp::endpoint(targetIP, server.port);
    usi->ipv4 = ipv4;

    usi->next = nullptr;
    *t = usi;
    t = &usi->next;
  }

  NetInterface<class IF_CLASS> netInterface;
  for (auto it = netInterface.begin(); it != netInterface.end(); it++) {
    sockaddr_in sockaddr{};
    sockaddr.sin_family = AF_INET;
    sockaddr.sin_port = htons(port);
    sockaddr.sin_addr.s_addr = INADDR_ANY;
    std::memset(sockaddr.sin_zero, 0, sizeof(sockaddr.sin_zero));

    raw_protocol::endpoint endpoint(&sockaddr, sizeof(sockaddr), SOCK_RAW);
    SocketData data{socketData.size(),
                    new raw_protocol::socket(io_context, endpoint),
                    endpoint,
                    {}};

    data.socket->open({udp::v4()});
    data.socket->bind(endpoint);
    data.socket->non_blocking();
    socketData.push_back(data);
    receive(data);
  }
}

void DnsServer::receive(SocketData &d) {
  d.socket->async_receive_from(
      boost::asio::buffer(d.recvBuffer.buf), d.endpoint,
      [&](boost::system::error_code ec, std::size_t n) {
        receive(ec, n, true, d);
        receive(d);
      });
}

DnsServer::~DnsServer() {
  for (auto &s : socketData) {
    s.socket->close();
  }
}

void DnsServer::receive(boost::system::error_code ec, std::size_t n, bool ipv4,
                        SocketData &d) {
  if (ec) {
    LERROR << "Error calling async_receive_from [" << ipv4
           << "]: " << ec.message() << std::endl;
    // boost::this_thread::sleep_for(
    //     boost::chrono::milliseconds(THREAD_SLEEP_AFTER_FAIL));
  } else {
    BytePacketBuffer &bpb = d.recvBuffer;

    InPacket ipacket;
    int res = ipacket.Read(&bpb);

    if (res != E_NOERROR) {
      LERROR << "Invalid packet received: " << res << std::endl;
      return;
    }

    if (ipacket.SourcePort == 0) {
      LERROR << "Data received from port 0" << std::endl;
      return;
    }

    bpb.pos = 0;
    bpb.size = n;

    DnsPacket packet;

    // DumpHex(bpb.buf, bpb.size);

    res = packet.Read(&bpb);
    if (res != E_NOERROR) {
      updateErrorResponse(packet, E_FORMERR);
      return;
    }

    if (packet.IsRequest()) { // Incoming request
      processRequest(d, packet, res, ipacket);
    } else { // Response from upstream DNS Server
      processUpstreamResponse(packet, res, ipacket);
    }
  }
}

bool DnsServer::processRequest(const SocketData &d, DnsPacket &packet, int &res,
                               const InPacket &ipacket) {
  bool sent = false;
  LDEBUG << "Incoming packet:" << std::endl << packet << std::endl;

  res = packet.Validate(PacketType::IncomingRequest);
  if (res != E_NOERROR) {
    updateErrorResponse(packet, (uint8_t)res);
    return sent;
  }

  Input input{&ipacket, &packet, this, &d};
  ruleEngine.Evaluate(input);

  return sent;
}

static std::time_t getNow() {
  using std::chrono::system_clock;
  return system_clock::to_time_t(system_clock::now());
}

bool DnsServer::processUpstreamResponse(DnsPacket &packet, int &res,
                                        const InPacket &ipacket) {
  bool sent = true;
  const std::time_t now = getNow();
  LDEBUG << "Incoming packet [@" << now << "]:" << std::endl
         << packet << std::endl;

  UpstreamServerInfo *server;
  for (server = servers; server; server = server->next) {
    if (server->ipv4 != ipacket.Ipv4 || server->port != ipacket.SourcePort ||
        (server->ipv4 && server->address.Ipv4 != ipacket.IpSource.Ipv4) ||
        (!server->ipv4 &&
         std::ranges::equal(server->address.Ipv6, ipacket.IpSource.Ipv6))) {
      continue;
    }

    break;
  }

  if (server == nullptr) {
    LERROR << "Upstream packet from " << ipacket
           << " is not from any known server: " << packet << std::endl;
    return sent;
  }

  res = packet.Validate(PacketType::IncomingResponse);
  if (res != E_NOERROR) {
    server->stats.invalids++;
    LERROR << "Upstream packet from " << ipacket << " is invalid: " << packet
           << std::endl;
    return sent;
  }

  uint8_t digest[SHA256_BLOCK_SIZE];
  packet.Hash(digest);

  PeerRequests::PeerRequestRecord *r =
      peerRequests.Lookup(packet.GetId(), digest);
  if (r == nullptr) {
    server->stats.nosources++;
    // We send request to all configured servers for now. The record might
    // have been removed as we got request from first server that replied.
    LINFO << "No source request found for reply from " << ipacket << ": "
          << std::endl
          << packet << std::endl;
    return sent;
  }

  uint8_t responseCode = packet.GetResponseCode();
  if (responseCode != E_NOERROR) {
    server->stats.failed++;
  }

  /* calculate modified moving average of server latency */
  if (server->stats.Latency == 0)
    server->stats.MMA = (now - r->forwardTimestamp) * 128; /* init */
  else
    server->stats.MMA += now - r->forwardTimestamp - server->stats.Latency;
  /* denominator controls how many queries we average over. */
  server->stats.Latency = server->stats.MMA / 128;

  // Send reply
  packet.SetRecursionAvailable(true);

  for (auto source = &r->source; source; source = source->next) {
    LDEBUG << "Endpoint: " << ipacket << ", " << source->ipv4 << std::endl;
    packet.SetId(source->originalId);
    // The data for socket from which request came
    SocketData &d = socketData[source->index];
    sendPacket(d, packet, ipacket);
  }

  peerRequests.FreePeerRequestRecord(r);

  return sent;
}

void DnsServer::updateRedirectResponse(DnsPacket &packet) const {
  packet.SetAsQueryResponse();
  packet.SetAsAuthoritativeAnswer(true);
  packet.SetRecursionAvailable(true);

  packet.SetAnswers([](DnsQuestion *question, DnsRecord *answer) {
    answer->UpdateFromQuestion(question);

    answer->TTL = ONE_HOUR;
    switch (question->Type) {
    case QT_A:
      answer->Len = 4;
      answer->Address.Ipv4.n = 0;
      break;
    case QT_AAAA:
      answer->Len = 16;
      answer->Address.Ipv6.n1 = 0;
      answer->Address.Ipv6.n2 = 0;
      answer->Address.Ipv6.n3 = 0;
      answer->Address.Ipv6.n4 = 0;
      break;
    }
  });
}

void DnsServer::updateErrorResponse(DnsPacket &packet, const uint8_t &errCode) {
  packet.SetResponseCode(errCode);
  packet.SetAsQueryResponse();
  packet.SetAsAuthoritativeAnswer(true);
  packet.SetRecursionAvailable(true);
}

void DnsServer::sendPacket(const SocketData &d, const DnsPacket &packet,
                           const InPacket &ipacket) {
  BytePacketBuffer responseBuffer;
  responseBuffer.pos = 0;

  int res = packet.Write(&responseBuffer);
  if (res != E_NOERROR) {
    LERROR << "Unable to write data to response buffer: " << res << std::endl;
    return;
  }

  // DumpHex(responseBuffer.buf, responseBuffer.pos);
  LDEBUG << "Outgoing packet:" << std::endl << packet << std::endl;
  boost::system::error_code ec;
  if (ipacket.Ipv4) {
    LDEBUG << "Sending ipv4 data: " << ipacket << std::endl;
    d.socket->send_to(
        boost::asio::buffer(responseBuffer.buf, responseBuffer.pos), d.endpoint,
        0, ec);
  } else {
    LDEBUG << "Sending ipv6 data: " << ipacket << std::endl;
    d.socket->send_to(
        boost::asio::buffer(responseBuffer.buf, responseBuffer.pos), d.endpoint,
        0, ec);
  }

  if (ec) {
    LERROR << "Error sending packet data: " << ec.message() << std::endl;
    LERROR << "The problematic packet: " << packet << std::endl;
  }
}

static void addSource(PeerRequests::PeerRequestRecord::PeerSource *s,
                      const DnsPacket &packet,
                      const raw_protocol::endpoint &endpoint,
                      const bool &ipv4) {
  s->originalId = packet.GetId();
  s->endpoint = endpoint;
  s->ipv4 = ipv4;
}

void DnsServer::resolve(const SocketData &d, DnsPacket &packet,
                        const InPacket &ipacket) {
  if (!servers) {
    LWARNING << "No upstream server found: " << packet << std::endl;
    return;
  }

  using namespace boost::asio::ip;
  const std::time_t now = getNow();

  unsigned int fwdFlags = 0;

  uint8_t digest[SHA256_BLOCK_SIZE];
  packet.Hash(digest);

  auto r = peerRequests.LookupByQuery(
      digest, fwdFlags,
      PEER_CHECKING_DISABLED | PEER_AD_QUESTION | PEER_HAS_PSUEDO_HEADER);

  if (r) { // A upstream query for this already exists
    if (!r->HasTimedOut()) {
      auto s = &r->source;
      for (; s; s = s->next) {
        if (s->ipv4 == ipacket.Ipv4 && s->originalId == packet.GetId() &&
            s->ipPort == ipacket.SourcePort &&
            ((s->ipv4 && s->ipAddress.Ipv4 == ipacket.IpSource.Ipv4) ||
             (!s->ipv4 &&
              std::ranges::equal(s->ipAddress.Ipv6, ipacket.IpSource.Ipv6)))) {
          break;
        }
      }

      if (s) { // This is repeat from client while another is in progress
        LDEBUG << "Repeat query from client: " << ipacket << std::endl;
        if (difftime(now, r->time) < 2) {
          LWARNING << "Repeate query within 2 seconds. Skipping" << std::endl;
          packet.SetResponseCode(E_REFUSED);
          sendPacket(d, packet, ipacket);
        }

        return;
      } else {
        s = peerRequests.GetNewPeerSource(now);

        if (!s) { // Refuse the packet since we are maxed out
          packet.SetResponseCode(E_REFUSED);
          sendPacket(d, packet, ipacket);
        } else {
          s->index = d.index;
          s->next = r->source.next;
          r->source.next = s;
          addSource(s, packet, d.endpoint, ipacket.Ipv4);
        }

        return;
      }
    } else {
      peerRequests.FreePeerRequestRecord(r);
    }
  } else {
    r = peerRequests.GetNewRecord(now, false);
    if (!r) {
      packet.SetResponseCode(E_REFUSED);
      sendPacket(d, packet, ipacket);
      return;
    }

    addSource(&r->source, packet, d.endpoint, ipacket.Ipv4);
    r->newId = peerRequests.GetNewId();
    std::memcpy(r->hash, digest, HASH_SIZE);
    packet.SetId(r->newId);
    r->flags = fwdFlags;
  }

  r->sentTo = servers;
  for (auto server = servers; server; server = server->next) {
    server->stats.queries++;
    sendPacket(d, packet, ipacket);
  }
}

void DnsServer::Resolve(const SocketData *d, DnsPacket *p, const InPacket *ip) {
  resolve(*d, *p, *ip);
  sendPacket(*d, *p, *ip);
}

void DnsServer::Redirect(const SocketData *d, DnsPacket *p,
                         const InPacket *ip) {
  updateRedirectResponse(*p);
  sendPacket(*d, *p, *ip);
}
