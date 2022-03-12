// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_TEST_UNIT_NP1_TEST_COMPRESSED_INT_HPP
#define NP1_TEST_UNIT_NP1_TEST_COMPRESSED_INT_HPP

#include "np1/compressed_int.hpp"

namespace test {
namespace unit {
namespace np1 {

typedef ::np1::compressed_int compressed_int_type;



uint64_t TEST_INTEGERS[] = {
  0, (uint64_t)-1, 1, (uint64_t)-2, 2, (uint64_t)-3, 3, (uint64_t)-126, 126, (uint64_t)-127, 127, (uint64_t)-128, 128,
  (uint64_t)-129, 129, (uint64_t)-254, 254, (uint64_t)-255, 255, (uint64_t)-256, 256, (uint64_t)-257, 257,
  (uint64_t)-65534, 65534, (uint64_t)-65535, 65535, (uint64_t)-65536, 65536, (uint64_t)-65537, 65537,
  (uint64_t)-9223372036854775806LL, 9223372036854775806LL, (uint64_t)-9223372036854775807LL, 9223372036854775807LL,
  (uint64_t)-9223372036854775808ULL
};

void test_compressed_int_assumptions() {
  uint64_t i = 0;
  NP1_TEST_ASSERT((!!i) == 0);
  NP1_TEST_ASSERT((!i) == 1);

  i = 1;
  NP1_TEST_ASSERT(!i == 0);
  NP1_TEST_ASSERT(!!i == 1);

  i = 0xFFFFFFFFFFFFFFFFLL;
  NP1_TEST_ASSERT(!i == 0);
  NP1_TEST_ASSERT(!!i == 1);

  i = 0xFFFFFFFF;
  NP1_TEST_ASSERT(!i == 0);
  NP1_TEST_ASSERT(!!i == 1);
}


void test_compressed_int_compress_decompress(uint64_t i) {
  uint64_t decompressed_i;
  unsigned char buffer[64];

  memset(buffer, 0, sizeof(buffer));

  unsigned char *end = compressed_int_type::compress(buffer, i);
  compressed_int_type::decompress(buffer, end, decompressed_i);
  NP1_TEST_ASSERT(i == decompressed_i);
}

void test_compressed_int_all_compress_decompress() {
  uint64_t *p = TEST_INTEGERS;
  uint64_t *end = p + sizeof(TEST_INTEGERS)/sizeof(TEST_INTEGERS[0]);
  for (; p < end; ++p) {
    test_compressed_int_compress_decompress(*p);
  }
}

void test_compressed_int_adjacent_compress_decompress(uint64_t i, uint64_t j) {
  unsigned char buffer[64];
  unsigned char *buf_p = buffer;

  memset(buffer, 0, sizeof(buffer));

  buf_p = compressed_int_type::compress(buf_p, i);
  buf_p = compressed_int_type::compress(buf_p, j);

  unsigned char *end_compressed = buf_p;
  NP1_TEST_ASSERT(end_compressed < buffer + sizeof(buffer));

  buf_p = buffer;
  uint64_t decompressed_i;
  uint64_t decompressed_j;
  buf_p = (unsigned char *)compressed_int_type::decompress(buf_p, end_compressed, decompressed_i);
  buf_p = (unsigned char *)compressed_int_type::decompress(buf_p, end_compressed, decompressed_j);

  NP1_TEST_ASSERT(i == decompressed_i);
  NP1_TEST_ASSERT(j == decompressed_j);
  NP1_TEST_ASSERT(buf_p == end_compressed);
}


void test_all_adjacent_compress_decompress() {
  uint64_t *p = TEST_INTEGERS;
  uint64_t *end = p + sizeof(TEST_INTEGERS)/sizeof(TEST_INTEGERS[0]);
  for (; p < end; ++p) {
    uint64_t *q = TEST_INTEGERS;
    for (; q < end; ++q) {
      test_compressed_int_adjacent_compress_decompress(*p, *q);
    }
  }
}


void test_compressed_int() {
  NP1_TEST_RUN_TEST(test_compressed_int_assumptions);
  NP1_TEST_RUN_TEST(test_compressed_int_all_compress_decompress);
  NP1_TEST_RUN_TEST(test_all_adjacent_compress_decompress);
}

} // namespaces
}
}

#endif
