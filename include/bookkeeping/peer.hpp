/*
 * Copyright (c) 2024 Neeraj Jakhar
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

#pragma once

#include <boost/asio/ip/udp.hpp>
#include <cstdint>
#include <ctime>
#include <memory>
#include <random>
#include <sys/types.h>

#define HASH_SIZE 32

#define PEER_CHECKING_DISABLED 1
#define PEER_AD_QUESTION 2
#define PEER_HAS_PSUEDO_HEADER 4
#define PEER_USE_DEF_PKT_SZ 8

using boost::asio::ip::udp;

struct UpstreamServerInfo;

class PeerRequests {
public:
  struct PeerRequestRecord {
    // Same request can come from multiple sources
    struct PeerSource {
      udp::endpoint endpoint;
      bool ipv4;
      uint16_t originalId;
      std::unique_ptr<PeerSource> next;
    } source;

    // null means free. In future we will not send to all servers
    // and this will be list of servers we have sent requests to.
    UpstreamServerInfo *sentTo;
    uint16_t newId;
    // int forwardall;
    int flags;
    std::time_t time;
    std::time_t forwardTimestamp;
    unsigned char hash[HASH_SIZE];
    bool fromTimedOut;
    std::unique_ptr<PeerRequestRecord> next;

    bool HasTimedOut() const;
  };

public:
  PeerRequests()
      : distribute(1, 0xffff), records(nullptr), sources(nullptr),
        sourceCount(0) {
    std::random_device seed;
    rng.seed(seed());
  };

  PeerRequestRecord *GetNewRecord(const std::time_t &now, bool force);

  PeerRequestRecord *Lookup(uint16_t id, void *hash) const;

  PeerRequestRecord *LookupByQuery(void *hash, unsigned int flags,
                                   unsigned int flagmask) const;

  void FreePeerRequestRecord(PeerRequests::PeerRequestRecord *r);

  PeerRequestRecord::PeerSource *GetNewPeerSource(const std::time_t &now);

  uint16_t GetNewId();

private:
  std::mt19937 rng;
  std::uniform_int_distribution<uint16_t> distribute;
  std::unique_ptr<PeerRequestRecord> records;
  std::unique_ptr<PeerRequestRecord::PeerSource> sources;
  uint16_t sourceCount;
};
