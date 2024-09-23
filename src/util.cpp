/*
 * Copyright (c) 2024 Neeraj Jakhar
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "util.hpp"
#include "net/netcommon.h"
#include <iomanip>
#include <iostream>
#include <regex>
#include <sstream>
#include <string>

#define ROW_SIZE 16

#define HEX "[0-9a-eA-E]"
#define HEXW HEX HEX "?"
#define IPV4 "(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)"
#define IPV6 HEXW

static std::regex ipRegex4(IPV4 "." IPV4 "." IPV4 "." IPV4);
static std::regex ipRegex6(IPV6 "(:" IPV6 "){15}");
static std::regex ethRegex(HEXW "(:" HEX "{5})");

void DumpHex(std::array<uint8_t, MAX_PACKET_SZ> data, const std::size_t n) {
  using namespace std;

  for (size_t j = 0; j < (n / ROW_SIZE) + 1; j++) {
    cout << "cc>> ";
    for (size_t i = 0; i < ROW_SIZE && j * ROW_SIZE + i < n; ++i)
      cout << setfill(' ') << setw(2)
           << (isprint(data[j * ROW_SIZE + i]) ? char(data[j * ROW_SIZE + i])
                                               : '.')
           << " ";
    cout << endl;

    cout << "0x>> ";
    for (size_t i = 0; i < ROW_SIZE && j * ROW_SIZE + i < n; ++i)
      cout << hex << setfill('0') << setw(2) << int(data[j * ROW_SIZE + i])
           << " ";
    cout << endl;
  }
}

std::time_t GetNow() {
  using std::chrono::system_clock;
  return system_clock::to_time_t(system_clock::now());
}

IpProtocolVersion GetIpAddressType(const std::string &host) {
  if (std::regex_match(host, ipRegex4)) {
    return IpProtocolVersion::Ipv4;
  } else if (std::regex_match(host, ipRegex6)) {
    return IpProtocolVersion::Ipv6;
  } else {
    return IpProtocolVersion::none;
  }
}

int ToEthAddress(const std::string &ethaddr, EthAddress *address) {
  if (std::regex_match(ethaddr, ethRegex)) {
    return -1;
  }

  std::stringstream ss(ethaddr);
  std::string token;
  int i =0;
  while (std::getline(ss, token, ':')) {
    address->v[i++] = std::stoi(token, nullptr, 16);
  }

  return 1;
}
