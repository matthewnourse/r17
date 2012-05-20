// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_CONSISTENT_HASH_TABLE_HPP
#define NP1_CONSISTENT_HASH_TABLE_HPP


#include "np1/skip_list.hpp"
#include "np1/hash/fnv1a64.hpp"

namespace np1 {

/// See http://en.wikipedia.org/wiki/Consistent_hashing
template <typename V, typename Hash_Function>
class consistent_hash_table {
private:
  struct entry {
    entry() : m_point(0) {}
    entry(uint64_t p, const V &v) : m_point(p), m_value(v) {}
    uint64_t m_point; // point on consistent hash circle.
    V m_value;
  };

  struct less_than {
    bool operator()(const entry &e1, const entry &e2) const {
      return e1.m_point < e2.m_point;
    }

    bool operator()(uint64_t p, const entry &e2) const {
      return p < e2.m_point;
    }

    bool operator()(const entry &e1, uint64_t p) const {
      return e1.m_point < p;
    }
  };

  typedef skip_list<entry, less_than> skip_list_type;
  
public:
  // The iterator makes the hash table look like a circle.  end() is the
  // iteration starting point.
  class const_iterator {
  public:
    const_iterator() {}
    const_iterator(typename skip_list_type::const_iterator b,
                   typename skip_list_type::const_iterator e,
                   typename skip_list_type::const_iterator s)
      : m_begin(b), m_end(e), m_iteration_start_point(s), m_curr(s) {}
    const V &operator*() const { return m_curr->m_value; }
    const V *operator->() const { return &m_curr->m_value; }
    const_iterator &operator++() {
      if (m_curr == m_end) {
        return *this;    
      }

      ++m_curr;
      if (m_curr == m_iteration_start_point) {
        m_curr = m_end;
      } else if (m_curr == m_end) {
        m_curr = m_begin;
        if (m_begin == m_iteration_start_point) {
          m_curr = m_end;    
        }
      }

      return *this;
    }

    bool operator == (const const_iterator &o) { return m_curr == o.m_curr; }
    bool operator != (const const_iterator &o) { return m_curr != o.m_curr; }

  private:
    typename skip_list_type::const_iterator m_begin;
    typename skip_list_type::const_iterator m_end;
    typename skip_list_type::const_iterator m_iteration_start_point;
    typename skip_list_type::const_iterator m_curr;
  };

  class iterator : public const_iterator {
  public:
    iterator() {}
    iterator(typename skip_list_type::const_iterator b,
             typename skip_list_type::const_iterator e,
             typename skip_list_type::const_iterator s)
      : const_iterator(b, e, s) {}

    V &operator*() { return (V &)const_iterator::operator*(); }
    V *operator->() { return (V *)const_iterator::operator->(); }
    iterator &operator++() { const_iterator::operator++(); return *this; }
    bool operator == (const iterator &o) { return const_iterator::operator==(o); }
    bool operator != (const iterator &o) { return const_iterator::operator!=(o); }
  private:
  };


public:
  explicit consistent_hash_table(size_t number_duplicate_entries)
    : m_number_duplicate_entries(number_duplicate_entries) {}

  ~consistent_hash_table() {}

  // Insert into the consistent hash.  Note that this insert is not guaranteed
  // to succeed, it's just very very very likely to succeed :).
  bool insert(const V &v) {
    Hash_Function hf;

    // i needs to be a uint64_t so that it hashes to the same value on all
    // platforms.
    bool success = false;
    for (uint64_t i = 0; i < m_number_duplicate_entries; ++i) {
      if (m_entries.insert(entry(hf(v, i), v))) {
        success = true;
      }
    }

    return success;
  }

  
  // Insert into the consistent hash, hashing to a slightly different value if
  // an entry already exists for the value.
  void insert_allow_duplicates(const V &v) {
    Hash_Function hf;

    // i needs to be a uint64_t so that it hashes to the same value on all
    // platforms.
    uint64_t i = 0;
    while (true) {
      if (m_entries.insert(entry(hf(v, i), v))) {
        return;
      }

      ++i;
    }
  }

  // Get an iterator to the first entry that is greater than or equal to the
  // hash of k.  NOTE that this
  // insert(v)
  // lower_bound(v)
  // is _VERY_ unlikely to return an iterator that points to v.
  template <typename K>
  iterator lower_bound(const K &k) {
    Hash_Function hf;
    
    typename skip_list_type::iterator iter = m_entries.lower_bound(hf(k, 0));
    if (m_entries.end() == iter) {
      // As the hash table is one big circle, just return the first entry,
      // if any.
      return begin();    
    }

    return iterator(m_entries.begin(), m_entries.end(), iter);
  }

  // Remove all entries.
  void clear() { m_entries.clear(); }

  // Return the number of unique entries in the hash table.
  size_t size() const { return m_entries.size()/m_number_duplicate_entries; }

  // Iterator functions.
  iterator begin() { return iterator(m_entries.begin(), m_entries.end(), m_entries.begin()); }
  const_iterator begin() const {
    return const_iterator(m_entries.begin(), m_entries.end(), m_entries.begin());
  }

  iterator end() { return iterator(m_entries.end(), m_entries.end(), m_entries.end()); }
  const_iterator end() const {
    return const_iterator(m_entries.end(), m_entries.end(), m_entries.end());
  }

private:
  skip_list_type m_entries;
  size_t m_number_duplicate_entries;
};


} // namespaces



#endif
