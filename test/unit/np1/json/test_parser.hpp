// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_TEST_UNIT_NP1_JSON_TEST_PARSER_HPP
#define NP1_TEST_UNIT_NP1_JSON_TEST_PARSER_HPP

#include "np1/json/parser.hpp"
#include "np1/rel/rlang/vm_heap.hpp"
#include "test/unit/helper.hpp"
#include "test/unit/np1/json/test_tokenizer.hpp"

namespace test {
namespace unit {
namespace np1 {
namespace json {

static void check_validate_and_normalize(const char *input, const char *expected) {
  ::np1::rel::rlang::vm_heap heap;
  ::np1::str::ref output = ::np1::json::parser::validate_and_normalise(heap, ::np1::str::ref(input));
  // fprintf(stderr, "Actual: '%s' Expected: '%s'\n", output.to_string().c_str(), expected); 
  NP1_TEST_ASSERT(::np1::str::cmp(output, expected) == 0);
}

static void check_validate_and_normalize(const char *input) {
  check_validate_and_normalize(input, input);
}

static void check_get_member(const char *input, const char *member_name, const char *expected) {
  ::np1::rel::rlang::vm_heap heap;
  ::np1::str::ref output = ::np1::json::parser::get_member(heap, ::np1::str::ref(input), ::np1::str::ref(member_name));
  // fprintf(stderr, "Actual: '%s' Expected: '%s'\n", output.to_string().c_str(), expected);
  NP1_TEST_ASSERT(::np1::str::cmp(output, expected) == 0);
}

static void test_validate_and_normalize() {
  check_validate_and_normalize("true");
  check_validate_and_normalize("false");
  check_validate_and_normalize("null");
  check_validate_and_normalize("1");
  check_validate_and_normalize("1.1");
  check_validate_and_normalize("\"\"");
  check_validate_and_normalize("\"fred\"");

  check_validate_and_normalize("{}");
  check_validate_and_normalize("{\"test\":0}");
  check_validate_and_normalize("{\"test\":true}");
  check_validate_and_normalize("{\"test\":\"fred\"}");
  check_validate_and_normalize("{\"test\":{}}");
  check_validate_and_normalize("{\"test\":{\"test2\":0}}");
  check_validate_and_normalize("{\"test\":0,\"test2\":1,\"test3\":{\"test4\":22}}");

  check_validate_and_normalize("[]");
  check_validate_and_normalize("[1]");
  check_validate_and_normalize("[1,2]");
  check_validate_and_normalize("[1,\"fred\"]");
  check_validate_and_normalize("[[]]");
  check_validate_and_normalize("[[1]]");
  check_validate_and_normalize("[1,2,[3,[4,5]]]");

  check_validate_and_normalize("[{}]");
  check_validate_and_normalize("[{\"test\":0}]");
  check_validate_and_normalize("[1,{\"test\":0,\"test2\":1},2]");
  check_validate_and_normalize("[1,{\"test\":0,\"test2\":[3,4]},2]");

  check_validate_and_normalize(" [1, 2]  ", "[1,2]");
}

static void test_get_member() {
  check_get_member("{}", "test", "");
  check_get_member("{\"test\":0}", "test", "0");
  check_get_member("{\"test\":\"fred\"}", "test", "\"fred\"");
  check_get_member("{\"test\":[1,2]}", "test", "[1,2]");
  check_get_member("{\"test\":[1,2,{\"something\":3}]}", "test", "[1,2,{\"something\":3}]");
  check_get_member("{\"test\":{\"test2\":1}", "test", "{\"test2\":1}");
  check_get_member("{\"test1\":0,\"test2\":1}", "test2", "1");

  check_get_member("[{\"test\":0}]", "test", "");
  check_get_member("{\"outer\":{\"inner\":1}}", "inner", "");
  check_get_member("{\"outer1\":{\"inner\":1},\"outer2\":true}", "outer2", "true");
}

static void test_parser() {
  NP1_TEST_RUN_TEST(test_validate_and_normalize);
  NP1_TEST_RUN_TEST(test_get_member);
}



} // namespaces
}
}
}

#endif


