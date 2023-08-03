// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_TEST_UNIT_NP1_JSON_TEST_ALL_HPP
#define NP1_TEST_UNIT_NP1_JSON_TEST_ALL_HPP


#include "test/unit/np1/json/test_tokenizer.hpp"
#include "test/unit/np1/json/test_parser.hpp"

namespace test {
namespace unit {
namespace np1 {
namespace json {

void test_all() {
  test_tokenizer();
  test_parser();
}

} // namespaces
}
}
}

#endif
