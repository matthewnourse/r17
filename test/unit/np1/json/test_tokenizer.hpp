// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_TEST_UNIT_NP1_JSON_TEST_TOKENIZER_HPP
#define NP1_TEST_UNIT_NP1_JSON_TEST_TOKENIZER_HPP

#include "np1/io/static_buffer_output_stream.hpp"
#include "np1/json/tokenizer.hpp"
#include "np1/json/writer.hpp"
#include "test/unit/helper.hpp"

namespace test {
namespace unit {
namespace np1 {
namespace json {

void check_round_trip(const char *input, const char *expected) {
  typedef typename ::np1::io::static_buffer_output_stream<1024> sbos_type;
  sbos_type sbos;

  ::np1::json::tokenizer::raw_tokenize(::np1::str::ref(input), ::np1::json::writer::make_raw_token_writer_handler(sbos));
  //fprintf(stderr, "check_round_trip: expected=%s actual_size=%zu actual=%.*s\n", expected, sbos.size(), (int)sbos.size(), (const char *)sbos.ptr());
  NP1_TEST_ASSERT(::np1::str::cmp(expected, (const char *)sbos.ptr(), sbos.size()) == 0);
}

void check_round_trip(const char *input) {
  check_round_trip(input, input);
}

void test_special() {
  check_round_trip("true");
  check_round_trip("false");
  check_round_trip("null");
  check_round_trip(" true ", "true");
}

void test_number() {
  check_round_trip("1");
  check_round_trip("12");
  check_round_trip("-1");
  check_round_trip("1.2");
  check_round_trip("12.34");
  check_round_trip("-12.34");
  check_round_trip("   1.2  ", "1.2");
  check_round_trip("1.2e-1", "1.2e-1");
}

void test_string() {
  check_round_trip("\"\"");
  check_round_trip("\"a\"");
  check_round_trip("\"abc\"");
  check_round_trip("\"a\\\"bc\"");
  check_round_trip("\"\\\"a\\\"bc\\\"d\"");
  check_round_trip(" \"abc\"", "\"abc\"");
}

void test_object() {
  check_round_trip("{}");
  check_round_trip("{");  // The tokenizer doesn't check for closed objects, only tokens
  check_round_trip("  {  }  ", "{}");
  check_round_trip("{\"test\":true}");
  check_round_trip("{ \"test\" : false}", "{\"test\":false}");
  check_round_trip("{\"test1\":{\"test2\":1,\"test3\":2}}");
}

void test_array() {
  check_round_trip("[]");
  check_round_trip("[");
  check_round_trip(" [ ]   ", "[]");
  check_round_trip("[1]");
  check_round_trip("[1,2]");
  check_round_trip(" [  1, 2 ]  ", "[1,2]");
  check_round_trip("[{}]");
  check_round_trip("[{\"test\":1},[2,3]]");
}


void test_tokenizer() {
  NP1_TEST_RUN_TEST(test_special);
  NP1_TEST_RUN_TEST(test_number);
  NP1_TEST_RUN_TEST(test_string);
  NP1_TEST_RUN_TEST(test_object);
  NP1_TEST_RUN_TEST(test_array);
}

} // namespaces
}
}
}



#endif
