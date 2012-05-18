// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_REL_RLANG_VM_STACK_HPP
#define NP1_REL_RLANG_VM_STACK_HPP


#include "np1/rel/rlang/dt.hpp"

namespace np1 {
namespace rel {
namespace rlang {


/// Helpers for the stack.
namespace detail {

template <size_t N> struct push_str;
template <size_t N> struct pop_str;

template <> struct push_str<sizeof(uint64_t)> {
  static inline void f(const str::ref &s, uint64_t *&stack_p) {
    uint64_t v = ((uint64_t)s.ptr()) << 32;
    v |= s.length();
    *stack_p++ = v;
  }
};


template <> struct push_str<sizeof(uint64_t)*2> {
  static inline void f(const str::ref &s, uint64_t *&stack_p) {
    *stack_p++ = (uint64_t)s.ptr();
    *stack_p++ = s.length();
  }
};


template <> struct pop_str<sizeof(uint64_t)> {
  static inline void f(str::ref &s, uint64_t *&stack_p) {
    uint64_t v = *--stack_p;
    s.set_ptr((const char *)(v >> 32));
    s.set_length((size_t)v);
  }
};

template <> struct pop_str<sizeof(uint64_t)*2> {
  static inline void f(str::ref &s, uint64_t *&stack_p) {
    s.set_length(*--stack_p);
    s.set_ptr((const char *)*--stack_p);
  }
};

static inline void eight_byte_copy(void *dest, const void *src) {
  unsigned char *dest_p = (unsigned char *)dest;
  const unsigned char *src_p = (const unsigned char *)src;
  *dest_p++ = *src_p++;
  *dest_p++ = *src_p++;
  *dest_p++ = *src_p++;
  *dest_p++ = *src_p++;
  *dest_p++ = *src_p++;
  *dest_p++ = *src_p++;
  *dest_p++ = *src_p++;
  *dest_p++ = *src_p++;  
}


} // namespace detail


/// The real stack for our virtual machine.
class vm_stack {
public:
  enum { MAX_STACK_BYTE_SIZE = 1024 };  

public:
  vm_stack() : m_ptr(m_data) {}

  inline void push(uint64_t ui) { *m_ptr++ = ui; }
  inline void pop(uint64_t &ui) { ui = *--m_ptr; }

  // We want to avoid a type conversion, we just want to copy the bytes.
  inline void push(double d) { detail::eight_byte_copy(m_ptr, &d); ++m_ptr; }
  inline void pop(double &d) { --m_ptr; detail::eight_byte_copy(&d, m_ptr); }

  inline void push(int64_t i) { *m_ptr++ = i; }
  inline void pop(int64_t &i) { i = *--m_ptr; }

  inline void push(bool b) { *m_ptr++ = (uint64_t)b; }
  inline void pop(bool &b) { b = (bool)*--m_ptr; }

  inline void push(const str::ref &s) { detail::push_str<sizeof(str::ref)>::f(s, m_ptr); }
  inline void pop(str::ref &s) { detail::pop_str<sizeof(str::ref)>::f(s, m_ptr); }

  inline void reset() { m_ptr = m_data; }

  inline bool empty() const { return m_ptr == m_data; }

  static size_t type_size(dt::data_type type) {
    switch (type) {
    case dt::TYPE_STRING:  
    case dt::TYPE_ISTRING:
    case dt::TYPE_IPADDRESS:
      return sizeof(str::ref);
  
    case dt::TYPE_INT:
    case dt::TYPE_UINT:
    case dt::TYPE_BOOL:
    case dt::TYPE_DOUBLE:
      NP1_PREPROC_STATIC_ASSERT(sizeof(double) == sizeof(uint64_t));
      return sizeof(uint64_t);
    }
  
    NP1_ASSERT(false, "Unreachable");
    return -1;
  }

  

private:
  // We should be ok with alignment issues because everything occupies at least
  // 8 bytes on all platforms.
  uint64_t *m_ptr;
  uint64_t m_data[MAX_STACK_BYTE_SIZE/sizeof(uint64_t)];
};



} // namespaces
}
}


#endif
