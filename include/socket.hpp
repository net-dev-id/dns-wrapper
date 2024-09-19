/*
 * Copyright (c) 2024 Neeraj Jakhar
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

#pragma once

#include <boost/asio/basic_raw_socket.hpp>
#include <boost/asio/ip/basic_endpoint.hpp>
#include <boost/asio/ip/basic_resolver.hpp>

namespace asio {
namespace ip {

class RawSocket {
public:
  /// The type of a RAW endpoint.
  typedef boost::asio::ip::basic_endpoint<RawSocket> endpoint;

  /// Construct to represent the IPv4 RAW protocol.
  static RawSocket v4() { return RawSocket(IPPROTO_UDP, PF_INET); }

  /// Construct to represent the IPv6 RAW protocol.
  static RawSocket v6() { return RawSocket(IPPROTO_UDP, PF_INET6); }

  // Default construction, compiler complains otherwise
  explicit RawSocket() : protocol_(IPPROTO_UDP), family_(PF_INET) {}

  /// Obtain an identifier for the type of the protocol.
  int type() const { return SOCK_RAW; }

  /// Obtain an identifier for the protocol.
  int protocol() const { return protocol_; }

  /// Obtain an identifier for the protocol family.
  int family() const { return family_; }

  /// The RAW socket type.
  typedef boost::asio::basic_raw_socket<RawSocket> socket;

  /// The RAW resolver type.
  typedef boost::asio::ip::basic_resolver<RawSocket> resolver;

  /// Compare two protocols for equality.
  friend bool operator==(const RawSocket &p1, const RawSocket &p2) {
    return p1.protocol_ == p2.protocol_ && p1.family_ == p2.family_;
  }

  /// Compare two protocols for inequality.
  friend bool operator!=(const RawSocket &p1, const RawSocket &p2) {
    return p1.protocol_ != p2.protocol_ || p1.family_ != p2.family_;
  }

private:
  explicit RawSocket(int protocol_id, int protocol_family)
      : protocol_(protocol_id), family_(protocol_family) {}

  int protocol_;
  int family_;
};

} // namespace ip
} // namespace asio
