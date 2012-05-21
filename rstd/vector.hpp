// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_RSTD_VECTOR_HPP
#define NP1_RSTD_VECTOR_HPP


#include "np1/simple_types.hpp"
#include "rstd/detail/mem.hpp"
#include "rstd/swap.hpp"

namespace rstd {

/// A basic replacement for std::vector.
template <typename T>
class vector {
public:
  typedef T *iterator;
  typedef const T *const_iterator;

public:
  vector() : m_ptr(0), m_size(0), m_capacity(0) {}

  vector(const T *p, size_t sz) : m_ptr(0), m_size(0), m_capacity(0) {
    initialize(p, sz, sz);  
  }

  vector(const vector &other) : m_ptr(0), m_size(0), m_capacity(0) {
    initialize(other.m_ptr, other.m_size, other.m_size);
  }

  ~vector() { destroy(); }

  void resize(size_t new_sz) {
    if (new_sz < size()) {
      T *start_dead_zone = m_ptr + new_sz;
      detail::mem::destruct(start_dead_zone, size() - new_sz);
      m_size = new_sz;
    } else if (new_sz > size()) {
      if (m_capacity < new_sz) {
        realloc(new_sz);
      }

      T def = T();   //TODO: this should be initialized to zero for POD types...is this enough?
      rstd::detail::mem::fill_copy_construct_in_place(m_ptr + size(), def,
                                                      new_sz - size());
      m_size = new_sz;
    }
  }

  typedef enum {
    SEARCH_CAPACITY_FAIL = -1, SEARCH_CAPACITY_SUCCESS, SEARCH_CAPACITY_SUCCESS_WITH_RETRY
  } search_capacity_result_type;
  
  search_capacity_result_type search_capacity(size_t capacity_hint,
                                                size_t min_capacity) {
    destroy();

    if (capacity_hint < min_capacity) {
      capacity_hint = min_capacity;    
    }

    search_capacity_result_type result = SEARCH_CAPACITY_SUCCESS;

    // If we're running low on RAM/address space, try to degrade gracefully.
    while (!try_initialize(NULL, 0, capacity_hint)) {
      if (capacity_hint > min_capacity + 2) {
        capacity_hint = capacity_hint - ((capacity_hint - min_capacity)/2);
      } else if (capacity_hint <= min_capacity) {
        return SEARCH_CAPACITY_FAIL;
      } else {
        // One last try.
        capacity_hint = min_capacity;
      }

      result = SEARCH_CAPACITY_SUCCESS_WITH_RETRY;
    }


    return result;
  }

  void push_back(const T &v) {
    append(&v, 1);
  }

  void push_front(const T &v) {
    vector<T> new_vector;
    new_vector.push_back(v);
    new_vector.append(*this);
    swap(new_vector);
  }

  void pop_back() {
    NP1_ASSERT(m_size > 0, "Attempt to pop_back on an empty vector");
    --m_size;
    detail::mem::destruct(m_ptr + m_size);
  }

  void pop_front() {
    NP1_ASSERT(m_size > 0, "Attempt to pop_front on an empty vector");
    vector<T> new_vector(m_ptr+1, m_size-1);
    swap(new_vector);    
  }

  void append(const T *other, size_t sz) {
    size_t new_size = m_size + sz;
    if (new_size > m_capacity) {
      realloc(new_size);
    }
    
    detail::mem::copy_construct_in_place(m_ptr + m_size, other, sz);
    m_size = new_size;
  }


  void append(const vector &other) {
    append(other.begin(), other.m_size);  
  }

  void erase(iterator i) {
    NP1_ASSERT(((i >= begin()) && (i < end())),
                "vector::erase called on with invalid iterator");
    detail::mem::destruct(i);
    memmove(i, i+1, (end() - (i+1)) * sizeof(*i));  //TODO: is memmove ok here?
    --m_size;
  }

  void clear() {
    resize(0); 
  }

  size_t size() const { return m_size; }
  bool empty() const { return 0 == m_size; }
  size_t capacity() const { return m_capacity; }  
 
  iterator begin() { return m_ptr; }
  iterator end() { return m_ptr + m_size; }
  const_iterator begin() const { return m_ptr; }
  const_iterator end() const { return m_ptr + m_size; }
  T &front() { NP1_ASSERT(!empty(), "front() on empty vector"); return *begin(); }
  const T &front() const { NP1_ASSERT(!empty(), "front() on empty vector"); return *begin(); }
  T &back() { NP1_ASSERT(!empty(), "back() on empty vector"); return *(end()-1); }
  const T &back() const { NP1_ASSERT(!empty(), "back() on empty vector"); return *(end()-1); }

  void swap(vector &other) {
    rstd::swap(m_ptr, other.m_ptr);
    rstd::swap(m_size, other.m_size);
    rstd::swap(m_capacity, other.m_capacity);
  }

  vector &operator = (const vector &other) {
    vector v(other);
    swap(v);
    return *this;
  }

  T &operator[] (size_t offset) { return m_ptr[offset]; }
  const T &operator[] (size_t offset) const { return m_ptr[offset]; }

 

private:
  bool try_initialize(const T *p, size_t sz, size_t capacity) {
    if (capacity > 0) {
      m_ptr = (T *)detail::mem::try_alloc(capacity * sizeof(T));
      if (!m_ptr) {
        return false;
      }
      detail::mem::copy_construct_in_place(m_ptr, p, sz);
    }

    m_size = sz;
    m_capacity = capacity;
    return true;
  }

  void initialize(const T *p, size_t sz, size_t capacity) {
    NP1_ASSERT(try_initialize(p, sz, capacity),
                "vector::try_initialize failed- out of memory");
  }

  bool try_realloc(size_t min_capacity) {
    vector v;

    NP1_ASSERT(m_capacity <= NP1_SIZE_T_MAX/2, "Impossible m_capacity!");

    if (v.search_capacity(m_capacity * 2, min_capacity) == SEARCH_CAPACITY_FAIL) {
      return false;
    }

    detail::mem::copy_construct_in_place(v.m_ptr, m_ptr, m_size);        
    v.m_size = m_size;    
    swap(v);
    return true;
  }


  void realloc(size_t min_capacity) {
    NP1_ASSERT(try_realloc(min_capacity),
                "vector::try_realloca failed- out of memory");
  }

  void destroy() {
    detail::mem::destruct_and_free(m_ptr, m_size);
    m_size = m_capacity = 0;
  }


private:
  T *m_ptr;
  size_t m_size;
  size_t m_capacity;
};


} // namespaces


#endif
