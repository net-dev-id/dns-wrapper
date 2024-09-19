/*
 * Copyright (c) 2024 Neeraj Jakhar
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "rule/action.hpp"
#include "dns/server.hpp"

void DNSAction::Run(const Input &input) const {
  input.server->Resolve(input.socketData, input.packet, input.ipacket);
}

void RedirectAction::Run(const Input &input) const {
  input.server->Redirect(input.socketData, input.packet, input.ipacket);
}
