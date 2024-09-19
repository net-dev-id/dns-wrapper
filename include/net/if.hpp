/*
 * Copyright (c) 2024 Neeraj Jakhar
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

#include <iterator>

template <class T> class Interface {
public:
  Interface(T *addrs) : start(addrs), addrs(addrs) {}
  ~Interface() {}

  int Index() const;
  std::string Name() const;
  T *Start() { return start; }
  void Next();

  friend bool operator==(const Interface<T> &a, const Interface<T> &b) {
    return a.addrs == b.addrs;
  };

  friend bool operator!=(const Interface<T> &a, const Interface<T> &b) {
    return a.addrs != b.addrs;
  };

private:
  T *start;
  T *addrs;
};

template <class T> class NetInterface {
public:
  class iterator {
  public:
    using iterator_category = std::forward_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = Interface<T>;
    using pointer = Interface<T> *;
    using reference = Interface<T> &;

    iterator(T *t) : val(t), sentinel(nullptr) {}

    reference operator*() const { return &val; }

    pointer operator->() { return val; }

    iterator &operator++() {
      val.Next();
      return *this;
    }

    iterator operator++(int) {
      iterator tmp = *this;
      ++(*this);
      return tmp;
    }

    pointer begin() { return &val; }
    pointer end() { return &sentinel; }

    friend bool operator==(const iterator &a, const iterator &b) {
      return a.val == b.val;
    };
    friend bool operator!=(const iterator &a, const iterator &b) {
      return a.val != b.val;
    };

  private:
    value_type val, sentinel;
    static_assert(std::sentinel_for<decltype(&sentinel), decltype(&val)>);
  };

  NetInterface();
  ~NetInterface();

  iterator begin() { return iterator(iter.begin()->Start()); }
  iterator end() { return iterator(iter.end()->Start()); }

private:
  T *addrs;
  static_assert(std::input_iterator<iterator>);
  iterator iter;
};
