// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_RSTD_DETAIL_MEM_HPP
#define NP1_RSTD_DETAIL_MEM_HPP

#include "np1/simple_types.hpp"
#include "np1/assert.hpp"

#ifdef _WIN32
#include <malloc.h>
#else
#include <stdlib.h>
#endif


// Placement new.
void * operator new (size_t, void * p) throw() { return p; }



/* Memory management functions. */
namespace rstd {
namespace detail {
namespace mem {

/// Allocate a raw chunk of RAM.
void *try_alloc(size_t sz) {
  /*TODO: malloc doesn't return NULL on Linux unless we run out of address
    space.  Do Something about this. */
  return malloc(sz);
}

void *alloc(size_t sz) {
  void *p = try_alloc(sz);
  NP1_ASSERT(p, "Out of memory");
  return p;
}

/// Free a raw chunk of RAM.
void free(void *p) {
  if (p) {
    ::free(p);
  }
}

void free(FILE **p) { free((void *)p); }


/// Construct into the supplied pointer.
//TODO: more overloads or cheeky metaprogramming to cover all POD types.
void copy_construct_in_place(char *target, const char &src) {
  *target = src;
}

void copy_construct_in_place(unsigned char *target, const unsigned char &src) {
  *target = src;
}


template <typename T>
void copy_construct_in_place(T *target, const T &src) {
  new (target) T(src);
}


/// Copy from one area to another, assuming that the target area does not need
/// destructing.
//TODO: more overloads or cheeky metaprogramming to cover all POD types.
void copy_construct_in_place(char *target, const char *src, size_t sz) {
  if (0 == sz) {
    return;
  }
  memcpy(target, src, sz);
}

void copy_construct_in_place(unsigned char *target, const unsigned char *src,
                              size_t sz) {
  if (0 == sz) {
    return;
  }
  memcpy(target, src, sz);
}

template <typename T>
void copy_construct_in_place(T *target, const T *src, size_t sz) {
  const T *src_end = src + sz;
  for (; src < src_end; ++src, ++target) {
    copy_construct_in_place(target, *src);  
  }
}

template <typename T>
void fill_copy_construct_in_place(T *target, const T &single_src, size_t sz) {
  T *target_end = target + sz;
  for (; target < target_end; ++target) {
    copy_construct_in_place(target, single_src);  
  }
}



template <typename T, typename... Arguments>
T *alloc_construct(const Arguments& ...arguments) {
  T *p = (T *)alloc(sizeof(T));
  new (p) T(arguments...);
  return p;
}

template <typename T, typename... Arguments>
T *alloc_construct(Arguments& ...arguments) {
  T *p = (T *)alloc(sizeof(T));
  new (p) T(arguments...);
  return p;
}


template <typename T>
T *clone(const T *src, size_t sz) {
  T *target = (T *)alloc(sz * sizeof(T));
  copy_construct_in_place(target, src, sz);
}



template <typename T>
T *clone(const T &src) {
  return clone(&src, 1);
}



/// Destruct the object without freeing the pointer.
//TODO: more overloads or cheeky metaprogramming to cover all POD types.
void destruct(char *p) {}
void destruct(unsigned char *p) {}

template <typename T>
void destruct(T *p) { if (p) { p->~T(); } }

/// Destruct and free the pointer.
template <typename T>
void destruct_and_free(T *p) { destruct(p); free(p); }



/// Destruct all the objects in the buffer without freeing the buffer.
//TODO: more overloads or cheeky metaprogramming to cover all POD types.
void destruct(char *p, size_t length) {}
void destruct(unsigned char *p, size_t length) {}

template <typename T>
void destruct(T *ptr, size_t length) {
  T *p = ptr;  
  T *end = ptr + length;
  for (; p < end; ++p) {
    destruct(p);
  }
}

/// Destruct all the objects in the buffer, then free the buffer.
template <typename T>
void destruct_and_free(T *ptr, size_t length) {
  destruct(ptr, length);
  free(ptr);
}



} // namespaces
}
}

#endif
