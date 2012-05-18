// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_TEST_UNIT_NP1_HASH_TEST_ALL_HPP
#define NP1_TEST_UNIT_NP1_HASH_TEST_ALL_HPP


#include "test/unit/np1/hash/test_sha256.hpp"



namespace test {
namespace unit {
namespace np1 {
namespace hash {


void test_all() {
  test_sha256();
}



} // namespaces
}
}
}

#endif
