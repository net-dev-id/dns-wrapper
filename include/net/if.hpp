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
    Interface<T> *_interface;

  public:
    Iterator(T *p) : _interface(new Interface<T>(p)) {}
    ~Iterator() {
      if (_interface) {
        delete _interface;
      }
    }
    Iterator(Iterator &b) {
      _interface = new Interface<T>(b._interface->Get());
    }
    Iterator(Iterator &&b) {
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

  NetInterface() : addrs(nullptr), start(nullptr), finish(nullptr) {}
  ~NetInterface() {}
  Iterator begin() { return start; }
  Iterator end() { return finish; }

private:
  T *addrs;
  Iterator start;
  Iterator finish;
};
