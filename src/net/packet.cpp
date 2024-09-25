/*
 * Copyright (c) 2024 Neeraj Jakhar
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "net/packet.hpp"
#include "net/netcommon.h"

#include <algorithm>
#include <cstdint>

#ifdef __linux
#include <linux/if_ether.h>
#include <netinet/in.h>
#else                       /* WIN32 */
#define ETH_P_IP 0x0800     /* Internet Protocol packet	*/
#define ETH_P_IPV6 0x86DD   /* IPv6 over bluebook		*/
#define ETH_P_8021Q 0x8100  /* 802.1Q VLAN Extended Header  */
#define ETH_P_8021AD 0x88A8 /* 802.1ad Service VLAN		*/
#define IPPROTO_UDP 17      /* User Datagram Protocol.  */
#include <winsock.h>
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

int InPacket::Read(RawPacketBuffer *bpb) {
  if (bpb->pos + ETH_HDR_SIZE > bpb->size) {
    return E_FORMERR;
  }

  std::copy(bpb->buf.begin() + bpb->pos,
            bpb->buf.begin() + bpb->pos + ETH_ADDR_LEN, EthDestination.v);
  bpb->pos += ETH_ADDR_LEN;
  std::copy(bpb->buf.begin() + bpb->pos,
            bpb->buf.begin() + bpb->pos + ETH_ADDR_LEN, EthSource.v);
  bpb->pos += ETH_ADDR_LEN;

  VREAD_U16(EthProtocol, bpb);

  for (int i = 0; i < VLAN_MAX_DEPTH; i++) {
    if (!isVlan(EthProtocol)) {
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
    if (bpb->pos + IPV4_HDR_SIZE > bpb->size) {
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

    uint8_t actualHeaderSize = (bpb->buf[bpb->pos] & 0x0f) * 4;
    if ((bpb->buf[bpb->pos] & 0xf0) != 0x40 || actualHeaderSize < 20 ||
        bpb->pos + actualHeaderSize > bpb->size) {
      return E_FORMERR;
    }

    Ipv4 = true;
    bpb->pos += 9;
    VREAD_U8(IpProtocol, bpb);
    bpb->pos += 2;
    VREAD_U32(IpSource.Ipv4, bpb);
    VREAD_U32(IpDestination.Ipv4, bpb);
    IpSource.Ipv4 = ntohl(IpSource.Ipv4);
    IpDestination.Ipv4 = ntohl(IpDestination.Ipv4);
    bpb->pos += actualHeaderSize - IPV4_HDR_SIZE;
  } else if (EthProtocol == ETH_P_IPV6) {
    if (bpb->pos + IPV6_HDR_SIZE > bpb->size) {
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
    for (int i = 0; i < 4; i++) {
      VREAD_U32(IpSource.Ipv6[i], bpb);
      IpSource.Ipv6[i] = ntohl(IpSource.Ipv6[i]);
    }
    for (int i = 0; i < 4; i++) {
      VREAD_U32(IpDestination.Ipv6[i], bpb);
      IpDestination.Ipv6[i] = ntohl(IpDestination.Ipv6[i]);
    }
  } else {
    return E_NOTIMP;
  }

  if (IpProtocol == IPPROTO_UDP) {
    if (bpb->pos + UDP_HDR_SIZE > bpb->size) {
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
