// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_RSTD_LIST
#define NP1_RSTD_LIST

#include "rstd/detail/mem.hpp"

namespace rstd {

/// A simple replacement for std::list.
template <typename T>
class list {
public:
  // For internal use only.
  struct entry {
    T payload;
    entry *next;
    entry *prev;
  };

public:
  /// Iterators.
  class const_iterator {
  public:
    const_iterator() {}
    explicit const_iterator(entry *e) : m_e(e) {}
    const T &operator*() const { return m_e->payload; }
    const T *operator->() const { return &m_e->payload; }
    const_iterator &operator++() { m_e = m_e->next; return *this; }
    const_iterator &operator--() { m_e = m_e->prev; return *this; }
    bool operator == (const const_iterator &o) { return m_e == o.m_e; }
    bool operator != (const const_iterator &o) { return m_e != o.m_e; }
  public:
    entry *m_e;
  };

  class iterator : public const_iterator {
  public:
    iterator() : const_iterator(0) {}
    explicit iterator(entry *e) : const_iterator(e) {}
    T &operator*() { return (T &)const_iterator::operator*(); }
    T *operator->() { return (T *)const_iterator::operator->(); }
    iterator &operator++() { const_iterator::operator++(); return *this; }
    bool operator == (const iterator &o) { return const_iterator::operator==(o); }
    bool operator != (const iterator &o) { return const_iterator::operator!=(o); }
  private:
  };
  

public:
  list() : m_head(0), m_tail(0) {}
  list(const list &other) : m_head(0), m_tail(0) {
    list new_list;
    const_iterator i = other.begin();
    const_iterator iz = other.end();
    for (; i != iz; ++i) {
      new_list.push_back(*i);
    }

    swap(new_list);
  }

  ~list() {
    while (!empty()) {
      pop_front();
    }
  }

  size_t size() const {
    const entry *p = m_head;
    size_t sz = 0;
    while (p) {
      ++sz;
      p = p->next;
    }

    return sz;
  }

  void push_back(const T &v) {
    entry *e = (entry *)detail::mem::alloc(sizeof(entry));
    detail::mem::copy_construct_in_place(&e->payload, v);
    e->next = 0;
    e->prev = m_tail;

    m_tail = e;
    if (!m_head) {
      m_head = e;
    }

    if (e->prev) {
      e->prev->next = e;
    }
  }

  void push_front(const T &v) {
    entry *e = (entry *)detail::mem::alloc(sizeof(entry));
    detail::mem::copy_construct_in_place(&e->payload, v);
    e->next = m_head;
    e->prev = 0;

    m_head = e;
    if (!m_tail) {
      m_tail = e;
    }
 
    if (e->next) {
      e->next->prev = e;
    }
  }

  void pop_front() {
    NP1_ASSERT(m_head, "pop_front() on empty list");

    entry *next = m_head->next;
    detail::mem::destruct_and_free(m_head);
    m_head = next;
    if (!next) {
      m_tail = next;      
    }
  }

  void erase(iterator iter) {
    NP1_ASSERT(!empty(), "erase() on empty list");
    entry *e = iter.m_e;
    entry *next = e->next;
    entry *prev = e->prev;
    if (next) {
      next->prev = prev;
    } else {
      m_tail = prev;
    }

    if (prev) {
      prev->next = next;
    } else {
      m_head = next;
    }

    detail::mem::destruct_and_free(e);
    iter.m_e = NULL;
  }

  void clear() {
    while (!empty()) {
      erase(begin());
    }
  }

  bool empty() const {
    return !m_head;
  }

  T &front() {
    NP1_ASSERT(m_head, "front() on empty list");
    return m_head->payload;
  }

  T &back() {
    NP1_ASSERT(m_tail, "back() on empty list");
    return m_tail->payload;
  }

  const T &front() const {
    NP1_ASSERT(m_head, "front() on empty list");
    return m_head->payload;
  }

  iterator begin() { return iterator(m_head); }
  iterator end() { return iterator(0); }
  iterator last() {
    NP1_ASSERT(m_tail, "last() on empty list");
    return iterator(m_tail);
  }

  const_iterator begin() const { return const_iterator(m_head); }
  const_iterator end() const { return const_iterator(0); }

  void swap(list &other) {
    rstd::swap(m_head, other.m_head);
    rstd::swap(m_tail, other.m_tail);
  }

  list &operator = (const list &other) {
    list new_list(other);
    swap(new_list);
    return *this;
  }

private:
  entry *m_head;
  entry *m_tail;
};


} // namespaces


#endif
