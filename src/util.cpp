/*
 * Copyright (c) 2024 Neeraj Jakhar
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "util.hpp"
#include <iomanip>
#include <iostream>

#define ROW_SIZE 16

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
