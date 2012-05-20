// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_TEST_UNIT_NP1_REL_RLANG_HELPER_HPP
#define NP1_TEST_UNIT_NP1_REL_RLANG_HELPER_HPP


namespace test {
namespace unit {
namespace np1 {
namespace rel {
namespace rlang {

typedef ::np1::io::string_input_stream string_stream_type;
typedef ::np1::rel::rlang::token token_type;
typedef ::np1::rel::rlang::shunting_yard shunting_yard_type;
typedef ::np1::rel::rlang::vm vm_type;
typedef ::np1::rel::rlang::vm_stack vm_stack_type;
typedef ::np1::rel::rlang::vm_heap vm_heap_type;
typedef ::np1::rel::rlang::compiler compiler_type;
namespace dt =  ::np1::rel::rlang::dt;
typedef ::np1::rel::record record_type;
typedef shunting_yard_type::parsed_token_info parsed_token_info_type;
typedef ::np1::rel::rlang::fn::fn_table fn_table_type;


template <typename Input_Stream>
void read_source(Input_Stream &input, std::vector<token_type> &output) {
  ::np1::rel::rlang::io::token_input_stream<Input_Stream, fn_table_type> token_input(input);

  token_type tok;

  output.clear();

  while (token_input.read(tok)) {
    output.push_back(tok);
  }
}

#define NP1_TEST_UNIT_REL_RLANG_DEFINE_INPUT_STREAM(name__, str__) \
std::string name__##_string(str__); \
string_stream_type name__(name__##_string)


#define NP1_TEST_UNIT_REL_RLANG_DEFINE_INPUT_TOKEN_VECTOR(name__, str__) \
NP1_TEST_UNIT_REL_RLANG_DEFINE_INPUT_STREAM(name__##_string_stream, str__); \
std::vector<token_type> name__; \
read_source(name__##_string_stream, name__)


void run_and_check_return_type(vm_type &vm, const record_type &this_r,
                                const record_type &other_r,
                                int64_t expected_value) {
  NP1_TEST_ASSERT((vm.return_type() == dt::TYPE_INT)
                    || (vm.return_type() == dt::TYPE_UINT));
  vm_heap_type heap;
  vm_stack_type &stack = vm.run_heap_reset(heap, this_r.ref(), other_r.ref());
  int64_t actual_value;
  stack.pop(actual_value);
  //printf("Expected: %lld  Actual: %lld\n", expected_value, actual_value);
  NP1_TEST_ASSERT(expected_value == actual_value);
}

void run_and_check_return_type(vm_type &vm, const record_type &this_r,
                                const record_type &other_r,
                                double expected_value) {
  NP1_TEST_ASSERT(vm.return_type() == dt::TYPE_DOUBLE);
  vm_heap_type heap;
  vm_stack_type &stack = vm.run_heap_reset(heap, this_r.ref(), other_r.ref());
  double actual_value;
  stack.pop(actual_value);
//  printf("Expected: %g  Actual: %g\n", expected_value, actual_value);
  NP1_TEST_ASSERT(fabs(expected_value - actual_value) < 0.0000001);
}


void run_and_check_return_type(vm_type &vm, const record_type &this_r,
                                const record_type &other_r,
                                int expected_value) {
  run_and_check_return_type(vm, this_r, other_r, (int64_t)expected_value);
}


void run_and_check_return_type(vm_type &vm, const record_type &this_r,
                                const record_type &other_r,
                                bool expected_value) {
  NP1_TEST_ASSERT(vm.return_type() == dt::TYPE_BOOL);
  vm_heap_type heap;
  vm_stack_type &stack = vm.run_heap_reset(heap, this_r.ref(), other_r.ref());
  bool actual_value;
  stack.pop(actual_value);
  NP1_TEST_ASSERT(expected_value == actual_value);
}


void run_and_check_return_type(vm_type &vm, const record_type &this_r,
                                const record_type &other_r,
                                const char *expected_value) {
  NP1_TEST_ASSERT((vm.return_type() == dt::TYPE_STRING)
                  || (vm.return_type() == dt::TYPE_ISTRING));
  vm_heap_type heap;
  vm_stack_type &stack = vm.run_heap_reset(heap, this_r.ref(), other_r.ref());
  ::np1::str::ref actual_value;
  stack.pop(actual_value);
//  fprintf(stderr, "Expected: %s  Actual: %s\n", expected_value,
//            std::string(actual_value.ptr(), actual_value.length()).c_str()); 
  NP1_TEST_ASSERT(::np1::str::cmp(actual_value, expected_value) == 0);
}


}
}
}
}
}


#endif
