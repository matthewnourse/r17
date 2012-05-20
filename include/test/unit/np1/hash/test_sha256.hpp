// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_TEST_UNIT_NP1_HASH_TEST_SHA256_HPP
#define NP1_TEST_UNIT_NP1_HASH_TEST_SHA256_HPP


#include "np1/hash/sha256.hpp"
#include "np1/str.hpp"



namespace test {
namespace unit {
namespace np1 {
namespace hash {


void test_sha256_fips180_2() {
  // standard FIPS-180-2 test vectors
 
  static const char *input[] = {
    "abc",
    "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq"
  };
  
  static const char *expected[] = {
    "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad",
    "248d6a61d20638b8e5c026930c3e6039a33ce45964ff2167f6ecedd419db06c1",
    "cdc76e5c9914fb9281a1c7e284d73e67f1809a48a497200e046d39ccc7112cd0"
  };

  const char **input_p = input;
  const char **input_end = input_p + sizeof(input)/sizeof(input[0]);
  const char **expected_p = expected;

  for (; input_p < input_end; ++input_p, ++expected_p) {
    unsigned char result_bytes[32];
    char result_str[65];
    ::np1::hash::sha256::hash((const unsigned char *)*input_p, strlen(*input_p), result_bytes);
    ::np1::str::to_hex_str_pad_64(result_str, result_bytes);
    NP1_TEST_ASSERT(::np1::str::cmp(result_str, *expected_p) == 0);
  }
}

void test_sha256() {
  NP1_TEST_RUN_TEST(test_sha256_fips180_2);
}



} // namespaces
}
}
}

#endif
