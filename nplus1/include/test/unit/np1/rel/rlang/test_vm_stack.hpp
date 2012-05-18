// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_TEST_UNIT_NP1_REL_RLANG_TEST_VM_STACK_HPP
#define NP1_TEST_UNIT_NP1_REL_RLANG_TEST_VM_STACK_HPP


namespace test {
namespace unit {
namespace np1 {
namespace rel {
namespace rlang {


void test_uint64_push_pop() {
  ::np1::rel::rlang::vm_stack stk;
  uint64_t val1;
  uint64_t val2;

  NP1_TEST_ASSERT(stk.empty());
  
  val1 = 0;
  stk.push(val1);
  NP1_TEST_ASSERT(!stk.empty());
  stk.pop(val2);
  NP1_TEST_ASSERT(stk.empty());
  NP1_TEST_ASSERT(val1 == val2);
  
  val1 = 1;
  stk.push(val1);
  NP1_TEST_ASSERT(!stk.empty());
  stk.pop(val2);
  NP1_TEST_ASSERT(stk.empty());
  NP1_TEST_ASSERT(val1 == val2);

  val1 = 0xffffffffffffffffULL;
  stk.push(val1);
  NP1_TEST_ASSERT(!stk.empty());
  stk.pop(val2);
  NP1_TEST_ASSERT(stk.empty());
  NP1_TEST_ASSERT(val1 == val2);

  val1 = 2;
  stk.push(val1);
  NP1_TEST_ASSERT(!stk.empty());
  val1 = 3;
  stk.push(val1);
  NP1_TEST_ASSERT(!stk.empty());
  stk.pop(val2);
  NP1_TEST_ASSERT(!stk.empty());
  NP1_TEST_ASSERT(3 == val2);
  stk.pop(val2);
  NP1_TEST_ASSERT(stk.empty());
  NP1_TEST_ASSERT(2 == val2);  
}


void test_double_push_pop() {
  ::np1::rel::rlang::vm_stack stk;
  double val1;
  double val2;

  NP1_TEST_ASSERT(stk.empty());
  
  val1 = 0;
  stk.push(val1);
  NP1_TEST_ASSERT(!stk.empty());
  stk.pop(val2);
  NP1_TEST_ASSERT(stk.empty());
  NP1_TEST_ASSERT(val1 == val2);
  
  val1 = 4;
  stk.push(val1);
  NP1_TEST_ASSERT(!stk.empty());
  stk.pop(val2);
  NP1_TEST_ASSERT(stk.empty());
  NP1_TEST_ASSERT(val1 == val2);

  val1 = -1;
  stk.push(val1);
  NP1_TEST_ASSERT(!stk.empty());
  stk.pop(val2);
  NP1_TEST_ASSERT(stk.empty());
  NP1_TEST_ASSERT(val1 == val2);

  val1 = 2;
  stk.push(val1);
  NP1_TEST_ASSERT(!stk.empty());
  val1 = 3;
  stk.push(val1);
  NP1_TEST_ASSERT(!stk.empty());
  stk.pop(val2);
  NP1_TEST_ASSERT(!stk.empty());
  NP1_TEST_ASSERT(3.0 == val2);
  stk.pop(val2);
  NP1_TEST_ASSERT(stk.empty());
  NP1_TEST_ASSERT(2.0 == val2);  
}


void test_vm_stack() {
  NP1_TEST_RUN_TEST(test_uint64_push_pop);
  NP1_TEST_RUN_TEST(test_double_push_pop);
}

} // namespaces
}
}
}
}

#endif
