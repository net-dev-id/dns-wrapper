/*
 * Copyright (c) 2024 Neeraj Jakhar
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

#include <cstddef>
#include <string>

template <class T> class Interface {
private:
  T *ptr;

public:
  Interface(T *addrs) : ptr(addrs) {}
  ~Interface() {}

  int Index() const;
  std::string Name() const;
  std::string Type() const;
  bool HasIpv4() const;
  bool HasIpv6() const;
  T *Get() { return ptr; }
  Interface<T> &operator++() {
    ++ptr;
    // prefix - this will be implemented only in specialization
    return *this;
  }
  bool operator==(const Interface<T> &b) { return ptr == b.ptr; }
  bool operator!=(const Interface<T> &b) { return ptr != b.ptr; }
};

template <class T> class NetInterface {
public:
  class Iterator {
  public:
    using difference_type = std::ptrdiff_t;
    using element_type = Interface<T>;
    using pointer = element_type *;
    using reference = element_type &;

  private:
    Interface<T> *interface;

  public:
    Iterator(T *p) : interface(new Interface<T>(p)) {}
    ~Iterator() {
      if (interface) {
        delete interface;
      }
    }
    Iterator(Iterator &b) { interface = new Interface<T>(b.interface->Get()); }
    Iterator(Iterator &&b) {
      interface = b.interface;
      b.interface = nullptr;
    }
    pointer operator->() const { return interface; }
    reference operator*() const { return *interface; }
    Iterator &operator++() { // prefix
      ++(*interface);
      return *this;
    }
    Iterator operator++(int) { // postfix
      Iterator tmp = *this;
      ++(*this);
      return tmp;
    }
    bool operator==(const Iterator &b) { return *interface == *b.interface; }
    bool operator!=(const Iterator &b) { return *interface != *b.interface; }
  };

  NetInterface() : addrs(nullptr), start(nullptr), finish(nullptr) {}
  ~NetInterface() {}
  Iterator begin() { return start; }
  Iterator end() { return finish; }

private:
  T *addrs;
  Iterator start;
  Iterator finish;
};
