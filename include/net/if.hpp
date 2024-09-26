/*
 * Copyright (c) 2024 Neeraj Jakhar
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

#pragma once

#include <cstddef>
#include <string>

#ifdef __linux
#include <ifaddrs.h>
#define IF_CLASS ifaddrs
#else /* WIN32 */
#define IF_CLASS IP_ADAPTER_ADDRESSES
#endif /* __linux */

class Interface {
private:
  IF_CLASS *ptr;

public:
  Interface(IF_CLASS *addrs) : ptr(addrs) {}
  ~Interface() {}

  int Index() const;
  std::string Name() const;
  std::string Type() const;
  bool HasIpv4() const;
  bool HasIpv6() const;
  IF_CLASS *Get() { return ptr; }
  Interface &operator++();
  bool operator==(const Interface &b) { return ptr == b.ptr; }
  bool operator!=(const Interface &b) { return ptr != b.ptr; }
};

class NetInterface {
public:
  class Iterator {
  public:
    using difference_type = std::ptrdiff_t;
    using element_type = Interface;
    using pointer = element_type *;
    using reference = element_type &;

  private:
    Interface *_interface;

  public:
    Iterator(IF_CLASS *p) : _interface(new Interface(p)) {}
    ~Iterator() {
      if (_interface) {
        delete _interface;
      }
    }
    Iterator(Iterator &b) { _interface = new Interface(b._interface->Get()); }
    Iterator(Iterator &&b) noexcept {
      _interface = b._interface;
      b._interface = nullptr;
    }
    pointer operator->() const { return _interface; }
    reference operator*() const { return *_interface; }
    Iterator &operator++() { // prefix
      ++(*_interface);
      return *this;
    }
    Iterator operator++(int) { // postfix
      Iterator tmp = *this;
      ++(*this);
      return tmp;
    }
    bool operator==(const Iterator &b) { return *_interface == *b._interface; }
    bool operator!=(const Iterator &b) { return *_interface != *b._interface; }
  };

  NetInterface();
  ~NetInterface();
  Iterator begin() { return start; }
  Iterator end() { return finish; }

private:
  IF_CLASS *addrs;
  Iterator start;
  Iterator finish;
};
