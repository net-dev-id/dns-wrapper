/*
 * Copyright (c) 2024 Neeraj Jakhar
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "args.hpp"
#include "net/netcommon.h"
#include "util.hpp"
#include <algorithm>
#include <boost/asio/generic/raw_protocol.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/smart_ptr/make_unique.hpp>
#include <memory>
#include <utility>
#ifdef __linux
#include <arpa/inet.h>
#include <net/ethernet.h>
#include <netpacket/packet.h>
#else /* WIN32 */
#include <iphlpapi.h>

#include <Windows.h>
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
#include <cstring>
#include <ctime>

#include "bookkeeping/peer.hpp"
#include "bookkeeping/server.hpp"
#include "dns/dnscommon.hpp"
#include "dns/dnspacket.hpp"
#include "dns/server.hpp"
#include "log.hpp"
#include "net/packet.hpp"
#include "rule/input.hpp"
#include "tp/sha256.hpp"

#define THREAD_SLEEP_AFTER_FAIL 100
#define UDP_RESIZE_PKG_SZ_AFTER                                                \
  60 /* How often to reset our idea of max packet size. */

using boost::asio::generic::raw_protocol;

DnsServer::DnsServer(boost::asio::io_context &io_context, uint16_t port,
                     const ConfigReader *configReader,
                     ShmRuleEngine *ruleEngine)
    : socket4(io_context), socket6(io_context), configReader(configReader),
      ruleEngine(ruleEngine) {
  initUpstreamServers();
  startRawSocketScan(io_context, port);
  startDnsListeners(port);
}

void DnsServer::initUpstreamServers() {
  using namespace boost::asio::ip;

  for (auto &server : configReader->servers) {
    if (server.protocol == Protocol::Tcp) {
      continue;
    }

    boost::asio::ip::address targetIP;
    bool ipv4;
    std::unique_ptr<UpstreamServerInfo> usi =
        std::make_unique<UpstreamServerInfo>();
    usi->displayAddress = server.hostIp;
    if (server.protocolVersion == IpProtocolVersion::Ipv4) {
      targetIP = make_address_v4(server.hostIp);
      ipv4 = true;
      usi->ipv4 = true;
    } else {
      targetIP = make_address_v6(server.hostIp);
      ipv4 = false;
    }

    if (ToIpAddress(server.hostIp, ipv4, &usi->address) != 1) {
      LERROR << "Unable to parse IP address: " << server.hostIp << std::endl;
      continue;
    }

    usi->endpoint = udp::endpoint(targetIP, server.port);
    usi->ipv4 = ipv4;
    servers.push_back(std::move(usi));
  }
}

void DnsServer::startRawSocketScan(
    [[maybe_unused]] boost::asio::io_context &io_context,
    [[maybe_unused]] const uint16_t &port) {
  if (!Args::Get()->useRawSockets) {
    return;
  }

#ifdef __linux
  std::unique_ptr<SocketData> updata = std::make_unique<SocketData>(io_context);
  SocketData *data = updata.get();
  raw_protocol protocol(PF_PACKET, htons(ETH_P_ALL));
  data->socket.open(protocol);
  socketData.push_back(std::move(updata));
  receive(data);
#endif /* __linux */
}

void DnsServer::startDnsListeners(const uint16_t &port) {
  socket4.open({udp::v4()});
  socket4.bind({udp::v4(), port});

  socket6.open({udp::v6()});
  boost::asio::ip::v6_only option(true);
  socket6.set_option(option);
  socket6.bind({udp::v6(), port});

  receive4();
  receive6();
}

void DnsServer::receive(SocketData *d) {
  d->socket.async_receive_from(
      boost::asio::buffer(d->recvBuffer.buf), d->endpoint,
      [&, d](boost::system::error_code ec, [[maybe_unused]] std::size_t n) {
        if (ec) {
          LERROR << "Error calling async_receive_from raw socket: "
                 << ec.message() << std::endl;
        } else {
          d->recvBuffer.pos = 0;
          d->recvBuffer.udp = 0;
          d->recvBuffer.size = n;
          receiveRawData(ec, n, true, d);
        }
        receive(d);
      });
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

    if (endpoint.port() == 0) {
      LERROR << "Data received from port 0" << std::endl;
      return;
    }

    bpb.pos = 0;
    bpb.size = n;

    int res;
    DnsPacket packet;

    // DumpHex(bpb.buf, bpb.size);

    res = packet.Read(&bpb);
    if (res != E_NOERROR) {
      updateErrorResponse(packet, E_FORMERR);
      return;
    }

    if (packet.IsRequest()) { // Incoming request
      processRequest(packet, res, endpoint, ipv4);
    } else { // Response from upstream DNS Server
      processUpstreamResponse(packet, res, endpoint);
    }
  }
}

static std::string toDisplayAddress(const bool &ipv4,
                                    const union IpAddress &ipaddr,
                                    const union EthAddress &ethaddr) {
  std::stringstream ss;
  std::string address;
  IpAddressToString(ipaddr, ipv4, address);
  ss << address << " --> " << EthAddressToString(ethaddr);
  return ss.str();
}

void DnsServer::receiveRawData(boost::system::error_code ec, std::size_t n,
                               bool ipv4, SocketData *d) {
  if (ec) {
    LERROR << "Error calling async_receive_from [" << ipv4
           << "]: " << ec.message() << std::endl;
    // boost::this_thread::sleep_for(
    //     boost::chrono::milliseconds(THREAD_SLEEP_AFTER_FAIL));
  } else {
    d->recvBuffer.pos = 0;
    d->recvBuffer.size = n;

    InPacket ipacket;
    int res = ipacket.Read(&d->recvBuffer);

    if (res == E_NOTIMP) {
      return;
    }

    if (res != E_NOERROR) {
      LERROR << "Invalid packet received: " << res << std::endl;
      return;
    }

    if (ipacket.DestinationPort != configReader->dnsPort) {
      // We are only interested in port 53 or whatever is configured.
      return;
    }

    if (d->recvBuffer.pos + 2 > d->recvBuffer.size ||
        (d->recvBuffer.buf[d->recvBuffer.pos + 2] & 0b10000000) != 0) {
      // This is not a DNS query
      LDEBUG << "Skipping DNS query response from mac mappings" << std::endl;
      return;
    }

    if (!ethmappings.Add(ipacket.EthSource, ipv4, ipacket.IpSource)) {
      LERROR << "Error adding mapping for ethernet address" << std::endl;
    }
  }
}

bool DnsServer::processRequest(DnsPacket &packet, int &res,
                               const udp::endpoint &endpoint, bool ipv4) {
  bool sent = false;
  LDEBUG << "Incoming packet:: destination: " << endpoint
         << " Id: " << packet.GetId() << " QC: " << packet.GetQuestionCount()
         << " AC: " << packet.GetAnswerCount() << std::endl;

  res = packet.Validate(PacketType::IncomingRequest);
  if (res != E_NOERROR) {
    updateErrorResponse(packet, (uint8_t)res);
    return sent;
  }

  union EthAddress ethaddr {};
  union IpAddress ipaddr {};
  if (ipv4) {
    std::ranges::copy(endpoint.address().to_v4().to_bytes(), ipaddr.Ipv4v);
  } else {
    std::ranges::copy(endpoint.address().to_v6().to_bytes(), ipaddr.Ipv6v);
  }

  const EthMappings::MacMappingRecord *r = ethmappings.LookUp(ipaddr, ipv4);
  if (!r) {
    LERROR << "Mac address not found for: " << endpoint << std::endl;
  } else {
    LDEBUG << "Mac address found: "
           << toDisplayAddress(ipv4, r->ipaddress, r->ethaddress) << std::endl;
    std::ranges::copy(r->ethaddress.v, ethaddr.v);
  }

  Input input{&packet, this, &endpoint, ipv4, ethaddr, ipaddr};
  ruleEngine->Evaluate(input);

  return sent;
}

bool DnsServer::processUpstreamResponse(DnsPacket &packet, int &res,
                                        udp::endpoint &endpoint) {
  bool sent = true;
  const std::time_t now = GetNow();
  LDEBUG << "Incoming packet:: destination: " << endpoint
         << " Id: " << packet.GetId() << " QC: " << packet.GetQuestionCount()
         << " AC: " << packet.GetAnswerCount() << std::endl;

  UpstreamServerInfo *server = nullptr;
  for (auto &s : servers) {
    if (s->endpoint == endpoint) {
      server = s.get();
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
    LINFO << "No source request found for reply:: destination: " << endpoint
          << " Id: " << packet.GetId() << " QC: " << packet.GetQuestionCount()
          << " AC: " << packet.GetAnswerCount() << std::endl;

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

  for (auto source = &r->source; source; source = source->next.get()) {
    LDEBUG << "Endpoint: " << endpoint << ", " << source->ipv4 << std::endl;
    packet.SetId(source->originalId);
    sendPacket(packet, source->endpoint, source->ipv4);
  }

  peerRequests.FreePeerRequestRecord(r);

  return sent;
}

void DnsServer::updateRedirectResponse(DnsPacket &packet,
                                       const union IpAddress &target) const {
  packet.SetAsQueryResponse();
  packet.SetAsAuthoritativeAnswer(true);
  packet.SetRecursionAvailable(true);

  packet.SetAnswers([&target](DnsQuestion *question, DnsRecord *answer) {
    answer->UpdateFromQuestion(question);

    answer->TTL = ONE_HOUR;
    switch (question->Type) {
    case QT_A:
      answer->Len = 4;
      answer->Address.Ipv4.n = target.Ipv4;
      break;
    case QT_AAAA:
      answer->Len = 16;
      answer->Address.Ipv6.n1 = target.Ipv6[0];
      answer->Address.Ipv6.n2 = target.Ipv6[1];
      answer->Address.Ipv6.n3 = target.Ipv6[2];
      answer->Address.Ipv6.n4 = target.Ipv6[3];
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
  LDEBUG << "Outgoing packet:: destination: " << endpoint
         << " Id: " << packet.GetId() << " QC: " << packet.GetQuestionCount()
         << " AC: " << packet.GetAnswerCount() << std::endl;
  boost::system::error_code ec;
  if (ipv4) {
    socket4.send_to(boost::asio::buffer(responseBuffer.buf, responseBuffer.pos),
                    endpoint, 0, ec);
  } else {
    socket6.send_to(boost::asio::buffer(responseBuffer.buf, responseBuffer.pos),
                    endpoint, 0, ec);
  }

  if (ec) {
    LERROR << "Error sending packet data: " << ec.message() << std::endl;
    LERROR << "The problematic packet: " << packet << std::endl;
  }
}
void DnsServer::receive4() {
  socket4.async_receive_from(
      boost::asio::buffer(recvBuffer4.buf), endpoint4,
      [this](boost::system::error_code ec, std::size_t n) {
        receive(ec, n, true);
        receive4();
      });
}

void DnsServer::receive6() {
  socket6.async_receive_from(
      boost::asio::buffer(recvBuffer6.buf), endpoint6,
      [this](boost::system::error_code ec, std::size_t n) {
        receive(ec, n, false);
        receive6();
      });
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
  if (servers.empty()) {
    LWARNING << "No upstream server found: " << packet << std::endl;
    return;
  }

  using namespace boost::asio::ip;
  const std::time_t now = GetNow();

  unsigned int fwdFlags = 0;

  uint8_t digest[SHA256_BLOCK_SIZE];
  packet.Hash(digest);

  auto r = peerRequests.LookupByQuery(
      digest, fwdFlags,
      PEER_CHECKING_DISABLED | PEER_AD_QUESTION | PEER_HAS_PSUEDO_HEADER);

  if (r) { // A upstream query for this already exists
    if (!r->HasTimedOut()) {
      auto s = &r->source;
      for (; s; s = s->next.get()) {
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
          s->next = std::move(r->source.next);
          r->source.next =
              std::unique_ptr<PeerRequests::PeerRequestRecord::PeerSource>(s);
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

  for (auto &server : servers) {
    r->sentTo = server.get();
    server->stats.queries++;
    LINFO << "Forwarding request to upstream server: " << server->displayAddress
          << ":" << server->port << std::endl;
    sendPacket(packet, server->endpoint, server->ipv4);
  }
}

void DnsServer::Resolve(DnsPacket *p, const udp::endpoint *e, const bool i) {
  resolve(*p, *e, i);
}

void DnsServer::Redirect(DnsPacket *p, const udp::endpoint *e, const bool i,
                         const union IpAddress &target) {
  updateRedirectResponse(*p, target);
  sendPacket(*p, *e, i);
}
