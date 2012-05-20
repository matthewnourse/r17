// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_TEST_UNIT_NP1_REL_RLANG_TEST_ALL_HPP
#define NP1_TEST_UNIT_NP1_REL_RLANG_TEST_ALL_HPP


#include "test/unit/np1/rel/rlang/io/test_all.hpp"
#include "test/unit/np1/rel/rlang/helper.hpp"
#include "test/unit/np1/rel/rlang/test_vm_stack.hpp"
#include "test/unit/np1/rel/rlang/test_shunting_yard.hpp"
#include "test/unit/np1/rel/rlang/test_compiler.hpp"

namespace test {
namespace unit {
namespace np1 {
namespace rel {
namespace rlang {

void test_all() {
  io::test_all();
  test_vm_stack();
  test_shunting_yard();
  test_compiler();
}

} // namespaces
}
}
}
}

#endif
