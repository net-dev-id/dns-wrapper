/*
 * Copyright (c) 2024 Neeraj Jakhar
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "net/packet.hpp"

#include <algorithm>
#include <cstdint>
#include <netinet/in.h>

#ifdef __linux
#include <linux/if_ether.h>
#endif /* __linux */

#define ETH_HDR_SIZE 14
#define VLAN_HDR_SIZE 4

#ifndef VLAN_MAX_DEPTH
#define VLAN_MAX_DEPTH 2
#endif /* VLAN_MAX_DEPTH */

#define IPV4_HDR_SIZE 20
#define IPV6_HDR_SIZE 8 + 16 + 16

#define UDP_HDR_SIZE 8

bool isVlan(uint16_t protocol) {
  return protocol == ETH_P_8021Q || protocol == ETH_P_8021AD;
}

int InPacket::Read(BytePacketBuffer *bpb) {
  if (bpb->pos + ETH_HDR_SIZE > bpb->size) {
    return E_FORMERR;
  }

  std::copy(bpb->buf.begin() + bpb->pos,
            bpb->buf.begin() + bpb->pos + ETH_HDR_SIZE, EthDestination);
  bpb->pos += ETH_ADDR_LEN;
  std::copy(bpb->buf.begin() + bpb->pos,
            bpb->buf.begin() + bpb->pos + ETH_HDR_SIZE, EthSource);
  bpb->pos += ETH_ADDR_LEN;

  VREAD_U16(EthProtocol, bpb);

  for (int i = 0; i < VLAN_MAX_DEPTH; i++) {
    if (isVlan(EthProtocol)) {
      break;
    }

    if (bpb->pos + VLAN_HDR_SIZE > bpb->size) {
      return E_FORMERR;
    }

    bpb->pos += 2;
    VREAD_U16(EthProtocol, bpb);
  }

  // Done processing Ethernet header

  if (EthProtocol == ETH_P_IP) {
    if (bpb->pos + IPV4_HDR_SIZE > bpb->pos) {
      return E_FORMERR;
    }

    /*
     * 0                   1
     * 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5
     * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
     * |Version|  IHL  |Type of Service|
     * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
     * |          Total Length         |
     * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
     * |         Identification        |
     * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
     * |Flags|     Fragment Offset     |
     * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
     * |  Time to Live |    Protocol   |
     * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
     * |        Header Checksum        |
     * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
     * |                               |
     * +         Source Address        +
     * |                               |
     * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
     * |                               |
     * +      Destination Address      +
     * |                               |
     * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
     * |            Options            |
     * +               +-+-+-+-+-+-+-+-+
     * |               |    Padding    |
     * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
     */

    uint8_t actualHeaderSize = bpb->buf[bpb->pos] & 0x0f;
    if (bpb->pos + actualHeaderSize > bpb->size) {
      return E_FORMERR;
    }

    Ipv4 = true;
    bpb->pos += 9;
    VREAD_U8(IpProtocol, bpb);
    bpb->pos += 2;
    VREAD_U32(IpSource.Ipv4, bpb);
    VREAD_U32(IpDestination.Ipv4, bpb);
    bpb->pos += actualHeaderSize - IPV4_HDR_SIZE;
  } else if (EthProtocol == ETH_P_IPV6) {
    if (bpb->pos + IPV6_HDR_SIZE > bpb->pos) {
      return E_FORMERR;
    }

    /*
     * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
     * |Version| Traffic Class |               Flow Label              |
     * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
     * |         Payload Length        |  Next Header  |   Hop Limit   |
     * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
     * |                                                               |
     * +                                                               +
     * |                                                               |
     * +                         Source Address                        +
     * |                                                               |
     * +                                                               +
     * |                                                               |
     * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
     * |                                                               |
     * +                                                               +
     * |                                                               |
     * +                       Destination Address                     +
     * |                                                               |
     * +                                                               +
     * |                                                               |
     * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
     */

    Ipv4 = false;
    bpb->pos += 6;
    VREAD_U8(IpProtocol, bpb);
    bpb->pos += 1;
    VREAD_U32(IpSource.Ipv6[0], bpb);
    VREAD_U32(IpSource.Ipv6[1], bpb);
    VREAD_U32(IpSource.Ipv6[2], bpb);
    VREAD_U32(IpSource.Ipv6[3], bpb);
    VREAD_U32(IpDestination.Ipv6[0], bpb);
    VREAD_U32(IpDestination.Ipv6[1], bpb);
    VREAD_U32(IpDestination.Ipv6[2], bpb);
    VREAD_U32(IpDestination.Ipv6[3], bpb);
  } else {
    return E_NOTIMP;
  }

  if (IpProtocol == IPPROTO_UDP) {
    if (bpb->pos + UDP_HDR_SIZE > bpb->pos) {
      return E_FORMERR;
    }

    /*
     *  0                   1                   2                   3
     *  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
     * -----------------------------------------------------------------
     * |          Source Port          |        Destination Port       |
     * -----------------------------------------------------------------
     * |             Length            |            Checksum           |
     * -----------------------------------------------------------------
     */

    VREAD_U16(SourcePort, bpb);
    VREAD_U16(DestinationPort, bpb);
    VREAD_U16(bpb->udp, bpb);
    bpb->pos += 2;
  } else {
    return E_NOTIMP;
  }

  return E_NOERROR;
}

std::ostream &operator<<(std::ostream &stream, const InPacket &p) {
  if (p.Ipv4) {
    stream << p.IpSource.Ipv4 << ":" << p.SourcePort;
  } else {
    stream << p.IpSource.Ipv6[0] << ":" << p.IpSource.Ipv6[1] << ":"
           << p.IpSource.Ipv6[2] << ":" << p.IpSource.Ipv6[3] << ":"
           << p.IpSource.Ipv6[4] << ":" << p.IpSource.Ipv6[5];
  }
  return stream;
}
