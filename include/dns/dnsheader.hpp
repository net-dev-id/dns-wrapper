/*
 * Copyright (c) 2024 Neeraj Jakhar
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

#pragma once

#include "dnscommon.hpp"

#include <cstdint>
#include <iostream>
#include <ostream>

/*
 * Header format:
 * https://datatracker.ietf.org/doc/html/rfc1035
 * ^^^ Section 4.1.1    
*     
 *                                  1  1  1  1  1  1
 *    0  1  2  3  4  5  6  7  8  9  0  1  2  3  4  5
 *  +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
 *  |                      ID                       |
 *  +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
 *  |QR|   Opcode  |AA|TC|RD|RA|  |AD|CD|   RCODE   |
 *  +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
 *  |                    QDCOUNT                    |
 *  +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
 *  |                    ANCOUNT                    |
 *  +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
 *  |                    NSCOUNT                    |
 *  +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
 *  |                    ARCOUNT                    |
 *  +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
 *
 */

class DnsHeader : DnsObject {
public:
  DnsHeader() {}
  virtual ~DnsHeader() {}

  int Read(BytePacketBuffer *bpb);
  int Validate(PacketType pt) const;
  int Write(BytePacketBuffer *bpb) const;

  friend std::ostream &operator<<(std::ostream &stream, const DnsHeader &);

  bool GetQueryResponse() const { return hb3 & (1 << 7); }
  uint8_t GetOpCode() const { return (hb3 >> 4) & 0b111; }
  bool GetAuthoritativeAnswer() const { return hb3 & (1 << 2); }
  bool GetTruncatedMessage() const { return hb3 & (1 << 1); }
  bool GetRecursionDesired() const { return hb3 & 1; }
  bool GetRecursionAvailable() const { return hb4 & (1 << 7); }
  bool GetAuthenticData() const { return hb4 & 0x20; }
  bool GetCheckingDisabled() const { return hb4 & 0x10; }
  uint8_t GetResponseCode() const { return hb4 & 0b1111; }

  void SetQueryResponse(bool v) { hb3 |= (((int)v) << 7); }
  void SetOpCode(uint8_t v) { hb3 |= ((v & 0b1111) << 3); }
  void SetAuthoritativeAnswer(bool v) { hb3 |= (((int)v) << 2); }
  void SetTruncatedMessage(bool v) { hb3 |= (((int)v) << 1); }
  void SetRecursionDesired(bool v) { hb3 |= ((int)v); }
  void SetRecursionAvailable(bool v) { hb4 |= (((int)v) << 7); }
  void SetAuthenticData(bool v) { hb4 |= (((int)v) << 5); }
  void SetCheckingDisabled(bool v) { hb4 |= (((int)v) << 6); }
  void SetResponseCode(uint8_t v) { hb4 |= (v & 0b1111); }

  uint16_t ID; // header byte 1 & 2
  char hb3;    // header byte 3
  char hb4;    // header byte 4
  uint16_t QuestionCount;
  uint16_t AnswerCount;
  uint16_t AuthorityCount;
  uint16_t AdditionalCount;
};
