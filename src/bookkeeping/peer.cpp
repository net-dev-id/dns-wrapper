/*
 * Copyright (c) 2024 Neeraj Jakhar
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "bookkeeping/peer.hpp"
#include "log.hpp"
#include <cstdint>
#include <memory>
#include <utility>

#define MAX_FWD_QUERIES 150
#define TIMEOUT 10 /* drop UDP queries after TIMEOUT seconds */

bool PeerRequests::PeerRequestRecord::HasTimedOut() const {
  return time > TIMEOUT;
}

static void handleMaxFwdQueries(const std::time_t &now) {
  static std::time_t lastLog = 0;
  static uint32_t skipCount = 0;

  // Do not flood logs
  if ((int)difftime(now, lastLog) > 5) {
    lastLog = now;
    LWARNING << "Maximum number of concurrent DNS queries reached (max: "
             << MAX_FWD_QUERIES << ")";
    if (skipCount > 0) {
      LWARNING << skipCount << " messages of the above kind have been skipped.";
    }
  } else {
    skipCount++;
  }
}

void PeerRequests::FreePeerRequestRecord(PeerRequests::PeerRequestRecord *r) {
  PeerRequests::PeerRequestRecord::PeerSource *last;

  // Essentially once source remains with record and others are added to
  // PeerRequests::source for future use
  for (last = r->source.next.get(); last && last->next; last = last->next.get())
    ;
  if (last) {
    last->next = std::move(sources);
    sources = std::move(r->source.next);
  }

  r->source.next = nullptr;
  r->sentTo = nullptr;
  r->flags = 0;
}

PeerRequests::PeerRequestRecord *
PeerRequests::GetNewRecord(const std::time_t &now, bool force) {
  PeerRequests::PeerRequestRecord *r, *oldest = nullptr, *target = nullptr;
  int count = 0;

  /* look for free records, garbage collect old records and count number in
   * use by our server-group. */
  for (r = records.get(); r; r = r->next.get()) {
    if (!r->sentTo)
      target = r;
    else {
      if (std::difftime(now, r->time) >= 4 * TIMEOUT) {
        FreePeerRequestRecord(r);
        r->fromTimedOut = true;
        target = r;
      } else if (!oldest || std::difftime(r->time, oldest->time) <= 0) {
        oldest = r;
      }
    }

    if (r->sentTo && std::difftime(now, r->time) < TIMEOUT) {
      count++;
    }
  }

  if (!force && count >= MAX_FWD_QUERIES) {
    handleMaxFwdQueries(now);
    return nullptr;
  }

  /* can't find empty one, use oldest if there is one and it's older than
   * timeout */
  if (!target && oldest && ((int)difftime(now, oldest->time)) >= TIMEOUT) {
    FreePeerRequestRecord(oldest);
    oldest->fromTimedOut = true;
    target = oldest;
  }

  if (!target) {
    target = new PeerRequestRecord;
    target->fromTimedOut = false;
    target->source.next = nullptr;
    target->sentTo = nullptr;
    target->next = std::move(records);
    records = std::unique_ptr<PeerRequestRecord>(target);
  }

  if (target) {
    target->time = now;
  }

  return target;
}

void logQuery(uint16_t id, const unsigned char *hash) {
  auto *t = hash;
  std::stringstream ss;
  for (int i = 0; i < HASH_SIZE; i++) {
    ss << std::hex << (int)t[i] << " ";
  }
  LDEBUG << "Looking up for record: ID: " << id << ", Hash: " << ss.str()
         << std::endl;
}

PeerRequests::PeerRequestRecord *PeerRequests::Lookup(uint16_t id,
                                                      void *hash) const {
  PeerRequestRecord *r;

  if (hash) {
    for (r = records.get(); r; r = r->next.get()) {
      logQuery(r->newId, r->hash);
      if (r->sentTo && r->newId == id &&
          (memcmp(hash, r->hash, HASH_SIZE) == 0)) {
        return r;
      }
    }
  }

  return nullptr;
}

PeerRequests::PeerRequestRecord *
PeerRequests::LookupByQuery(void *hash, unsigned int flags,
                            unsigned int flagmask) const {
  PeerRequestRecord *f;
  logQuery(0, (const unsigned char *)hash);

  if (hash) {
    for (f = records.get(); f; f = f->next.get())
      if (f->sentTo && (f->flags & flagmask) == flags &&
          std::memcmp(hash, f->hash, HASH_SIZE) == 0)
        return f;
  }

  return nullptr;
}

PeerRequests::PeerRequestRecord::PeerSource *
PeerRequests::GetNewPeerSource(const std::time_t &now) {
  PeerRequests::PeerRequestRecord::PeerSource *s = nullptr;
  if (!sources && sourceCount < MAX_FWD_QUERIES) {
    sources = std::make_unique<PeerRequests::PeerRequestRecord::PeerSource>();
    sourceCount++;
    sources->next = nullptr;
  } else {
    handleMaxFwdQueries(now);
  }

  if (sources) {
    s = sources.get();
    sources = std::move(sources->next);
    s->next = nullptr;
  }

  return s;
}

uint16_t PeerRequests::GetNewId() {
  uint16_t ret = 0;

  while (1) {
    ret = distribute(rng);
    bool found = false;
    for (auto r = records.get(); r; r = r->next.get()) {
      if (r->sentTo && r->newId == ret) {
        found = true;
        break;
      }
    }

    if (!found) {
      break;
    }
  }

  return ret;
}
