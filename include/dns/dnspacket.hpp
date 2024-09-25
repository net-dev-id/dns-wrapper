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
  uint16_t GetQuestionCount() const { return header->QuestionCount; }
  uint16_t GetAnswerCount() const { return header->AnswerCount; }

  void SetAsQueryResponse();
  void SetAsQueryRequest();
  void SetId(const uint16_t &id);
  void SetAsAuthoritativeAnswer(const bool &value);
  void SetRecursionAvailable(const bool &value);
  void SetCheckingDisabled(const bool &value);
  void SetResponseCode(const uint8_t &responseCode);
  void SetAnswers(std::function<void(DnsQuestion *, DnsRecord *)> writeAnswer);

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
