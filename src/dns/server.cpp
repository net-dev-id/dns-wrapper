/*
 * Copyright (c) 2024 Neeraj Jakhar
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "log.hpp"
#include "dns/server.hpp"
#include "bookkeeping/peer.hpp"
#include "bookkeeping/server.hpp"
#include "dns/dnscommon.hpp"
#include "dns/dnspacket.hpp"
#include "tp/sha256.hpp"
#include <boost/asio/ip/address_v4.hpp>
#include <boost/asio/ip/address_v6.hpp>
#include <boost/asio/ip/impl/address_v4.ipp>
#include <boost/asio/ip/impl/address_v6.ipp>
#include <boost/chrono.hpp>
#include <boost/thread/thread.hpp>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <ctime>
#include <ostream>

#define THREAD_SLEEP_AFTER_FAIL 100
#define UDP_RESIZE_PKG_SZ_AFTER                                                \
  60 /* How often to reset our idea of max packet size. */

DnsServer::DnsServer(boost::asio::io_context &io_context, uint16_t port,
                     const ConfigReader *configReader)
    : socket4(io_context), socket6(io_context), configReader(configReader) {
  using namespace boost::asio::ip;

  servers = nullptr;
  UpstreamServerInfo **t = &servers;
  for (auto server : configReader->servers) {
    if (server.protocol == Protocol::Tcp) {
      continue;
    }

    UpstreamServerInfo *usi = new UpstreamServerInfo;
    if (server.protocolVersion == IpProtocolVersion::Ipv4) {
      auto targetIP = make_address_v4(server.hostIp);
      usi->endpoint = udp::endpoint{targetIP, server.port};
      usi->ipv4 = true;
    } else {
      auto targetIP = make_address_v6(server.hostIp);
      usi->endpoint = udp::endpoint(targetIP, server.port);
      usi->ipv4 = false;
    }

    usi->next = nullptr;
    *t = usi;
    t = &usi->next;
  }

  socket4.open({udp::v4()});
  socket4.bind({udp::v4(), port});

  socket6.open({udp::v6()});
  boost::asio::ip::v6_only option(true);
  socket6.set_option(option);
  socket6.bind({udp::v6(), port});

  receive4();
  receive6();
}

DnsServer::~DnsServer() {
  socket4.close();
  socket6.close();
}

void DnsServer::receive(boost::system::error_code ec, std::size_t n,
                        bool ipv4) {
  if (ec) {
    LERROR << "Error calling async_receive_from [" << ipv4
           << "]: " << ec.message() << std::endl;
    // boost::this_thread::sleep_for(
    //     boost::chrono::milliseconds(THREAD_SLEEP_AFTER_FAIL));
  } else {
    if (n < sizeof(DnsHeader)) {
      LERROR << "Invalid packet recieved with size: " << n << std::endl;
      return;
    }

    udp::endpoint &endpoint = ipv4 ? endpoint4 : endpoint6;
    BytePacketBuffer &bpb = ipv4 ? recvBuffer4 : recvBuffer6;
    bool sent = false;

    if (endpoint.port() == 0) {
      LERROR << "Data received from port 0" << std::endl;
      return;
    }

    bpb.pos = 0;
    bpb.size = n;

    int res;
    DnsPacket packet;

    // DumpHex(bpb.buf, bpb.size);

    do {
      res = packet.Read(&bpb);
      if (res != E_NOERROR) {
        updateErrorResponse(packet, E_FORMERR);
        break;
      }

      if (packet.IsRequest()) { // Incoming request
        sent = processRequest(packet, res, endpoint, ipv4);
      } else { // Response from upstream DNS Server
        sent = processUpstreamResponse(packet, res, endpoint);
      }
    } while (false);

    if (!sent) {
      if (ipv4) {
        sendPacket(packet, endpoint, ipv4);
      } else {
        sendPacket(packet, endpoint, ipv4);
      }
    }
  }

  if (ipv4) {
    LDEBUG << "Listen ipv4" << std::endl;
    receive4();
  } else {
    LDEBUG << "Listen ipv6" << std::endl;
    receive6();
  }
}

bool DnsServer::processRequest(DnsPacket &packet, int &res,
                               const udp::endpoint &endpoint, bool ipv4) {
  bool sent = false;
  LDEBUG << "Incoming packet:" << std::endl << packet << std::endl;

  res = packet.Validate(PacketType::IncomingRequest);
  if (res != E_NOERROR) {
    updateErrorResponse(packet, (uint8_t)res);
    return sent;
  }

  if (ruleMatcher.IsMatch(packet, endpoint)) {
    LINFO << "Rule matched for " << endpoint << std::endl;
    updateRedirectResponse(packet);
  } else {
    LDEBUG << "Resolving using upstream servers for query from: " << endpoint
           << std::endl;
    sent = true;
    resolve(packet, endpoint, ipv4);
  }

  return sent;
}

static std::time_t getNow() {
  using std::chrono::system_clock;
  return system_clock::to_time_t(system_clock::now());
}

bool DnsServer::processUpstreamResponse(DnsPacket &packet, int &res,
                                        const udp::endpoint &endpoint) {
  bool sent = true;
  const std::time_t now = getNow();
  LDEBUG << "Incoming packet [@" << now << "]:" << std::endl
         << packet << std::endl;

  UpstreamServerInfo *server;
  for (server = servers; server; server = server->next) {
    if (server->endpoint == endpoint) {
      break;
    }
  }

  if (server == nullptr) {
    LERROR << "Upstream packet from " << endpoint
           << " is not from any known server: " << packet << std::endl;
    return sent;
  }

  res = packet.Validate(PacketType::IncomingResponse);
  if (res != E_NOERROR) {
    server->stats.invalids++;
    LERROR << "Upstream packet from " << endpoint << " is invalid: " << packet
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
    LINFO << "No source request found for reply from " << endpoint << ": "
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
    LDEBUG << "Endpoint: " << source->endpoint << ", " << source->ipv4
           << std::endl;
    packet.SetId(source->originalId);
    sendPacket(packet, source->endpoint, source->ipv4);
  }

  peerRequests.FreePeerRequestRecord(r);

  return sent;
}

void DnsServer::updateRedirectResponse(DnsPacket &packet) {
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

void DnsServer::updateErrorResponse(DnsPacket &packet, const uint8_t& errCode) {
  packet.SetResponseCode(errCode);
  packet.SetAsQueryResponse();
  packet.SetAsAuthoritativeAnswer(true);
  packet.SetRecursionAvailable(true);
}

void DnsServer::sendPacket(const DnsPacket &packet,
                           const udp::endpoint &endpoint, bool ipv4) {
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
  if (ipv4) {
    LDEBUG << "Sending ipv4 data: " << endpoint << std::endl;
    socket4.send_to(boost::asio::buffer(responseBuffer.buf, responseBuffer.pos),
                    endpoint, 0, ec);
  } else {
    LDEBUG << "Sending ipv6 data: " << endpoint << std::endl;
    socket6.send_to(boost::asio::buffer(responseBuffer.buf, responseBuffer.pos),
                    endpoint, 0, ec);
  }

  if (ec) {
    LERROR << "Error sending packet data: " << ec.message() << std::endl;
    LERROR << "The problematic packet: " << packet << std::endl;
  }
}

void DnsServer::receive4() {
  socket4.async_receive_from(boost::asio::buffer(recvBuffer4.buf), endpoint4,
                             [this](boost::system::error_code ec,
                                    std::size_t n) { receive(ec, n, true); });
}

void DnsServer::receive6() {
  socket6.async_receive_from(boost::asio::buffer(recvBuffer6.buf), endpoint6,
                             [this](boost::system::error_code ec,
                                    std::size_t n) { receive(ec, n, false); });
}

static void addSource(PeerRequests::PeerRequestRecord::PeerSource *s,
                      const DnsPacket &packet, const udp::endpoint &endpoint,
                      const bool &ipv4) {
  s->originalId = packet.GetId();
  s->endpoint = endpoint;
  s->ipv4 = ipv4;
}

void DnsServer::resolve(DnsPacket &packet, const udp::endpoint &endpoint,
                        bool ipv4) {
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
        if (s->ipv4 == ipv4 && s->originalId == packet.GetId() &&
            s->endpoint == endpoint) {
          break;
        }
      }

      if (s) { // This is repeat from client while another is in progress
        LDEBUG << "Repeat query from client: " << endpoint << std::endl;
        if (difftime(now, r->time) < 2) {
          LWARNING << "Repeate query within 2 seconds. Skipping" << std::endl;
          packet.SetResponseCode(E_REFUSED);
          sendPacket(packet, endpoint, ipv4);
        }

        return;
      } else {
        s = peerRequests.GetNewPeerSource(now);

        if (!s) { // Refuse the packet since we are maxed out
          packet.SetResponseCode(E_REFUSED);
          sendPacket(packet, endpoint, ipv4);
        } else {
          s->next = r->source.next;
          r->source.next = s;
          addSource(s, packet, endpoint, ipv4);
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
      sendPacket(packet, endpoint, ipv4);
      return;
    }

    addSource(&r->source, packet, endpoint, ipv4);
    r->newId = peerRequests.GetNewId();
    std::memcpy(r->hash, digest, HASH_SIZE);
    packet.SetId(r->newId);
    r->flags = fwdFlags;
  }

  r->sentTo = servers;
  for (auto server = servers; server; server = server->next) {
    server->stats.queries++;
    sendPacket(packet, server->endpoint, server->ipv4);
  }
}
