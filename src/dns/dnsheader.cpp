/*
 * Created Date: Monday, August 26th 2024, 10:22:25 am
 * Author: Neeraj Jakhar
 * -----
 * Last Modified: Friday, 6th September 2024 8:45:21 am
 * Modified By: Neeraj Jakhar
 * -----
 * Copyright (c) 2024 Neeraj Jakhar
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * -----
 */

#include "dns/dnsheader.hpp"
#include "dns/dnscommon.hpp"
#include "log.hpp"

#define HEADER_SIZE 12

int DnsHeader::Read(BytePacketBuffer *bpb) {
  if (!PACKET_SZ_VALID(bpb, HEADER_SIZE)) {
    LERROR_X << "Header size is invalid (" << (bpb->pos + HEADER_SIZE) << " > "
             << bpb->size << ")" << std::endl;
    return E_FORMERR;
  }

  VREAD_U16(ID, bpb);
  VREAD_BYTE(hb3, bpb);
  VREAD_BYTE(hb4, bpb);
  VREAD_U16(QuestionCount, bpb);
  VREAD_U16(AnswerCount, bpb);
  VREAD_U16(AuthorityCount, bpb);
  VREAD_U16(AdditionalCount, bpb);

  return E_NOERROR;
}

int DnsHeader::Validate(PacketType pt) const {
  if (pt == PacketType::IncomingRequest) {
    if (GetQueryResponse()) {
      LERROR_X << "Expected query type to be 0" << std::endl;
      return E_FORMERR;
    }

    if (GetOpCode() != S_QUERY) {
      LERROR_X << "Expected standard query" << std::endl;
      return E_NOTIMP;
    }

    if (GetTruncatedMessage()) {
      LERROR_X << "Got truncated message" << std::endl;
      return E_FORMERR;
    }

    if (GetResponseCode() != E_NOERROR) {
      LERROR_X << "Expected response code to be 0" << std::endl;
      return E_FORMERR;
    }

    if (QuestionCount == 0) {
      LERROR_X << "Expected question count to be 0" << std::endl;
      return E_FORMERR;
    }

    if (AnswerCount > 0 || AuthorityCount > 0) {
      LERROR_X << "Expected counts to be 0" << std::endl;
      return E_FORMERR;
    }
  }

  if (pt == PacketType::IncomingResponse) {
    if (!GetQueryResponse()) {
      LERROR_X << "Expected query type to 1" << std::endl;
      return E_FORMERR;
    }
  }

  return E_NOERROR;
}

int DnsHeader::Write(BytePacketBuffer *bpb) const {
  WRITE_U16(bpb, ID)
  WRITE_BYTE(bpb, hb3)
  WRITE_BYTE(bpb, hb4)
  WRITE_U16(bpb, QuestionCount)
  WRITE_U16(bpb, AnswerCount)
  WRITE_U16(bpb, AuthorityCount)
  WRITE_U16(bpb, AdditionalCount)

  return SUCCESS;
}

std::ostream &operator<<(std::ostream &stream, const DnsHeader &dh) {
  using namespace std;

  stream << "ID: " << dh.ID << endl;
  stream << "QR: " << dh.GetQueryResponse() << endl;
  stream << "OPCODE: " << (int)dh.GetOpCode() << endl;
  stream << "AA: " << dh.GetAuthoritativeAnswer() << endl;
  stream << "TC: " << dh.GetTruncatedMessage() << endl;
  stream << "RD: " << dh.GetRecursionDesired() << endl;
  stream << "RA: " << dh.GetRecursionAvailable() << endl;
  stream << "RCODE: " << (int)dh.GetResponseCode() << endl;
  stream << "QDCOUNT: " << dh.QuestionCount << endl;
  stream << "ANCOUNT: " << dh.AnswerCount << endl;
  stream << "NSCOUNT: " << dh.AuthorityCount << endl;
  stream << "ARCOUNT: " << dh.AdditionalCount << endl;

  return stream;
}
