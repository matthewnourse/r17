// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_TEST_UNIT_NP1_REL_TEST_ALL_HPP
#define NP1_TEST_UNIT_NP1_REL_TEST_ALL_HPP


#include "test/unit/np1/rel/test_record_ref.hpp"
#include "test/unit/np1/rel/test_record.hpp"
#include "test/unit/np1/rel/rlang/test_all.hpp"

namespace test {
namespace unit {
namespace np1 {
namespace rel {

void test_all() {
  test_record_ref();
  test_record();
  rlang::test_all();
}

} // namespaces
}
}
}

#endif
