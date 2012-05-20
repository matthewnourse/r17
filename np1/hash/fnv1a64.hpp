// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_HASH_FNV1A64_HPP
#define NP1_HASH_FNV1A64_HPP


#include "np1/simple_types.hpp"

namespace np1 {
namespace hash {
namespace fnv1a64 {

/**
 * This hash function is a public-domain hash called "FNV-1a".
 * See http://www.isthe.com/chongo/tech/comp/fnv/ for more information.
 */
 
/// Add a byte into a non-crypto hash and return the hash value.
uint64_t add(uint8_t b, uint64_t hval) {
  /* xor the bottom with the current octet */
  hval ^= (uint64_t)b;

  /* multiply by the 64 bit FNV magic prime mod 2^64 */
  hval *= 0x100000001b3ULL;

  return hval;
}


/// Add an array of bytes into the hash value.
uint64_t add(const char *p, size_t len, uint64_t hval) {
  const char *end = p + len;
  for (; p < end; ++p) {
    hval = add(*p, hval);
  }
 
  return hval;
}
  
uint64_t add(const void *p, size_t len, uint64_t hval) {
  return add((const char *)p, len, hval);
}

// Do a hash when you have no previous hash value.
uint64_t init(uint64_t somedata = 0x12345) {
  return add((const char *)&somedata, sizeof(somedata), 0xcbf29ce484222325ULL);
}


} // namespaces
}
}


#endif
