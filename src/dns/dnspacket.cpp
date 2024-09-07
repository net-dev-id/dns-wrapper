/*
 * Copyright (c) 2024 Neeraj Jakhar
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "dns/dnspacket.hpp"
#include "dns/dnscommon.hpp"
#include "log.hpp"
#include "tp/sha256.hpp"
#include <cstdint>

#define _READ_RECORD(what, class, field, typer)                                \
  if (header && header->what##Count > 0) {                                     \
    field = new Dns##class[header->what##Count];                               \
    for (int i = 0; i < header->what##Count; i++) {                            \
      typer;                                                                   \
      code = field[i].Read(bpb);                                               \
      if (code != E_NOERROR) {                                                 \
        return code;                                                           \
      }                                                                        \
    }                                                                          \
  }

#define READ_RECORD(what, field)                                               \
  _READ_RECORD(what, Record, field,                                            \
               field[i].RecordType = DnsRecord::DnsRecordType::what)
#define READ_QUESTION(what, field) _READ_RECORD(what, what, field, ;)

int DnsPacket::Read(BytePacketBuffer *bpb) {
  header = new DnsHeader;
  int code = header->Read(bpb);
  if (code != E_NOERROR) {
    return code;
  }

  READ_QUESTION(Question, questions)
  READ_RECORD(Answer, answers)
  READ_RECORD(Authority, authorities)
  READ_RECORD(Additional, additionals)

  if (bpb->pos < bpb->size) {
    LWARNING << "Packet contains extra data: " << *this << std::endl;
  }

  return E_NOERROR;
}

#define VALIDATE_RECORD(what, field)                                           \
  for (int i = 0; i < header->what##Count; i++) {                              \
    code = field[i].Validate(pt);                                              \
    if (code != E_NOERROR) {                                                   \
      return code;                                                             \
    }                                                                          \
  }

int DnsPacket::Validate(PacketType pt) const {
  int code = E_NOERROR;
  if (header) {
    if (!(code = header->Validate(pt))) {
      return code;
    };
  } else {
    return E_FORMERR;
  }

  if (pt == PacketType::IncomingRequest || pt == IncomingResponse) {
    VALIDATE_RECORD(Question, questions)
    VALIDATE_RECORD(Answer, answers)
    VALIDATE_RECORD(Authority, authorities)
    VALIDATE_RECORD(Additional, additionals)
  }

  int psuedoHeaderCount = 0;
  for (int i = 0; i < header->AdditionalCount; i++) {
    psuedoHeaderCount += (additionals[i].Type == QT_OPT);
  }

  if (psuedoHeaderCount > 1) {
    LWARNING << "More than one psuedo header found: " << psuedoHeaderCount
             << std::endl;
    return E_FORMERR;
  }

  return code;
}

#define WRITE_RECORD(what, field)                                              \
  if (header && header->what##Count > 0) {                                     \
    for (int i = 0; i < header->what##Count; i++) {                            \
      field[i].Write(bpb);                                                     \
    }                                                                          \
  }

int DnsPacket::Write(BytePacketBuffer *bpb) const {
  if (header) {
    header->Write(bpb);
  }

  WRITE_RECORD(Question, questions)
  WRITE_RECORD(Answer, answers)
  WRITE_RECORD(Authority, authorities)
  WRITE_RECORD(Additional, additionals)

  return E_NOERROR;
}

int DnsPacket::Hash(uint8_t *digest) const {
  SHA256_CTX ctx;

  sha256_init(&ctx);

  for (int i = 0; i < header->QuestionCount; i++) {
    auto question = questions[i];
    question.UpdateDigest(&ctx);
  }

  sha256_final(&ctx, (BYTE *)digest);

  return 0;
}

#define OUTPUT_RECORD(what, field)                                             \
  if (dp.header && dp.header->what##Count > 0) {                               \
    stream << #what "s: " << dp.header->what##Count << endl;                   \
    for (int i = 0; i < dp.header->what##Count; i++) {                         \
      stream << #what " #" << i + 1 << ": ";                                   \
      stream << dp.field[i];                                                   \
    }                                                                          \
  } else {                                                                     \
    stream << "No " #what "s available" << endl;                               \
  }

std::ostream &operator<<(std::ostream &stream, const DnsPacket &dp) {
  using namespace std;

  if (dp.header) {
    stream << *dp.header;
  }

  OUTPUT_RECORD(Question, questions)
  OUTPUT_RECORD(Answer, answers)
  OUTPUT_RECORD(Authority, authorities)
  OUTPUT_RECORD(Additional, additionals)

  return stream;
}

void DnsPacket::SetAsQueryRequest() { header->SetQueryResponse(false); }

void DnsPacket::SetAsQueryResponse() { header->SetQueryResponse(true); }

void DnsPacket::SetId(const uint16_t &id) { header->ID = id; }

void DnsPacket::SetAsAuthoritativeAnswer(const bool &value) {
  header->SetAuthoritativeAnswer(value);
}

void DnsPacket::SetRecursionAvailable(const bool &value) {
  header->SetRecursionAvailable(value);
}

void DnsPacket::SetCheckingDisabled(const bool &value) {
  header->SetCheckingDisabled(value);
}

void DnsPacket::SetResponseCode(const uint8_t &responseCode) {
  header->SetResponseCode(responseCode);
}

void DnsPacket::SetAnswers(void(writeAnswer)(DnsQuestion *, DnsRecord *)) {
  if (header->QuestionCount == 0 || !questions) {
    return;
  }

  answers = new DnsRecord[header->QuestionCount];
  for (int i = 0; i < header->QuestionCount; i++) {
    writeAnswer(questions + i, answers + i);
  }

  header->AnswerCount = header->QuestionCount;
}

#define _OPT_PROCESSOR(match, nomatch)                                         \
  for (int i = 0; i < header->AdditionalCount; i++) {                          \
    auto additional = additionals[i];                                          \
    if (additional.Type == QT_OPT) {                                           \
      match                                                                    \
    }                                                                          \
  }                                                                            \
  nomatch

#define OPT_GET(match, nomatch)                                                \
  _OPT_PROCESSOR(return (match);, return (nomatch);)

#define OPT_SET(match) _OPT_PROCESSOR((match);, return;)

bool DnsPacket::HasPsuedoHeader() const { OPT_GET(true, false); }

uint16_t DnsPacket::GetUdpPayloadSize() const {
  OPT_GET(additional.UdpSize, 0);
}

uint16_t DnsPacket::GetExtendedResponseCode() const {
  OPT_GET(header->GetResponseCode() | (additional.GetExtendedRcode() << 4),
          header->GetResponseCode());
}

bool DnsPacket::HasDoBit() const { OPT_GET(additional.GetDoBit(), false); }

bool DnsPacket::HasAdBit() const { OPT_GET(additional.GetAdBit(), false); }

void DnsPacket::SetUdpPayloadSize(const uint16_t &size) {
  OPT_SET(additional.UdpSize = size);
}
