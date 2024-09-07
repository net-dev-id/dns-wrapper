/*
 * Copyright (c) 2024 Neeraj Jakhar
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

#pragma once

#include "dnsheader.hpp"
#include "dnsrecord.hpp"
#include <cstdint>

class DnsPacket : DnsObject {
public:
  DnsPacket()
      : header(nullptr), questions(nullptr), answers(nullptr),
        authorities(nullptr), additionals(nullptr) {}

  virtual ~DnsPacket() {
    if (questions)
      delete[] questions;

    if (answers)
      delete[] answers;

    if (authorities)
      delete[] authorities;

    if (additionals)
      delete[] additionals;

    if (header)
      delete header;
  }

  int Read(BytePacketBuffer *bpb);
  int Validate(PacketType pt) const;
  int Write(BytePacketBuffer *bpb) const;
  int Hash(uint8_t *digest) const;

  bool IsRequest() const { return !header->GetQueryResponse(); }
  bool IsResponse() const { return header->GetQueryResponse(); }
  bool IsTrucated() const { return header->GetTruncatedMessage(); }
  bool HasCheckingDisabled() const { return header->GetCheckingDisabled(); }
  uint16_t GetId() const { return header->ID; }
  uint8_t GetResponseCode() const { return header->GetResponseCode(); }

  void SetAsQueryResponse();
  void SetAsQueryRequest();
  void SetId(const uint16_t &id);
  void SetAsAuthoritativeAnswer(const bool &value);
  void SetRecursionAvailable(const bool &value);
  void SetCheckingDisabled(const bool &value);
  void SetResponseCode(const uint8_t &responseCode);
  void SetAnswers(void(writeAnswer)(DnsQuestion *, DnsRecord *));

  bool HasPsuedoHeader() const;
  uint16_t GetUdpPayloadSize() const;
  uint16_t GetExtendedResponseCode() const;
  bool HasDoBit() const;
  bool HasAdBit() const;
  void SetUdpPayloadSize(const uint16_t &size);

  friend std::ostream &operator<<(std::ostream &stream, const DnsPacket &);

private:
  DnsHeader *header;
  DnsQuestion *questions;
  DnsRecord *answers;
  DnsRecord *authorities;
  DnsRecord *additionals;
};

/*
The recursive mode occurs when a query with RD set arrives at a server
which is willing to provide recursive service; the client can verify
that recursive mode was used by checking that both RA and RD are set in
the reply.  Note that the name server should never perform recursive
service unless asked via RD, since this interferes with trouble shooting
of name servers and their databases.

If recursive service is requested and available, the recursive response
to a query will be one of the following:

   - The answer to the query, possibly preface by one or more CNAME
     RRs that specify aliases encountered on the way to an answer.
   - A name error indicating that the name does not exist.  This
     may include CNAME RRs that indicate that the original query
     name was an alias for a name which does not exist.
   - A temporary error indication.

If recursive service is not requested or is not available, the non-
recursive response will be one of the following:

   - An authoritative name error indicating that the name does not
     exist.
   - A temporary error indication.
   - Some combination of:
     RRs that answer the question, together with an indication
     whether the data comes from a zone or is cached.
     A referral to name servers which have zones which are closer
     ancestors to the name than the server sending the reply.
   - RRs that the name server thinks will prove useful to the
     requester.


*/