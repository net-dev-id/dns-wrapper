/*
 * Copyright (c) 2024 Neeraj Jakhar
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

#pragma once

#include "dns/dnscommon.hpp"
#include <array>
#include <cstddef>
#include <cstdint>

char *GetUserName(void);

void DumpHex(std::array<uint8_t, MAX_PACKET_SZ>, const std::size_t n);
