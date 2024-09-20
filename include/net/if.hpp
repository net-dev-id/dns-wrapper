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
  T *start, *ptr;

public:
  Interface(T *addrs) : start(addrs), ptr(addrs) {}
  ~Interface() {}

  int Index() const;
  std::string Name() const;
  T *Start() const { return start; }
  T *Ptr() const { return ptr; }

  Interface<T> &operator++() {
    // prefix - this will be implemented only in specialization
    return *this;
  }

  Interface<T> operator++(int) {
    // postfix - this will be implemented only in specialization
    return *this;
  }
  bool operator==(const Interface<T> &b) const { return ptr == b.ptr; }
  bool operator!=(const Interface<T> &b) const { return ptr != b.ptr; }
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
    element_type addrs;

  public:
    Iterator(T *p) : addrs(Interface<T>(p)) {}
    reference operator*() const { return addrs; }
    Iterator &operator++() {
      addrs++;
      return *this;
    }
    Iterator operator++(int) {
      Iterator tmp = *this;
      ++addrs;
      return tmp;
    }
    bool operator==(const Iterator &b) { return addrs == b.addrs; }
    bool operator!=(const Iterator &b) { return addrs != b.addrs; }
  };

  NetInterface() : start(nullptr), finish(nullptr) {}

  Iterator begin() { return start; }

  Iterator end() { return finish; }

private:
  Iterator start;
  Iterator finish;
};
