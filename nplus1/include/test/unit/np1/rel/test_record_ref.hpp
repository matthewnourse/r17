// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_TEST_UNIT_NP1_REL_TEST_RECORD_REF_HPP
#define NP1_TEST_UNIT_NP1_REL_TEST_RECORD_REF_HPP


namespace test {
namespace unit {
namespace np1 {
namespace rel {

typedef ::np1::rel::record_ref record_ref_type;
typedef ::np1::rel::record record_type;
typedef ::np1::io::static_buffer_output_stream<4096> buffer_output_stream_type;


void test_number_fields() {
  const unsigned char *one_field_data = (const unsigned char *)"\x8e\x81\x84test\0\0\0\0\0\0\0\0";
  record_ref_type one_field(one_field_data, one_field_data + 15, 1);
  NP1_TEST_ASSERT(one_field.number_fields() == 1);  
  
  const unsigned char *two_fields_data = (const unsigned char *)"\x95\x82\x85test1\x85test2\0\0\0\0\0\0\0\0";
  record_ref_type two_fields(two_fields_data, two_fields_data + 22, 1);
  NP1_TEST_ASSERT(two_fields.number_fields() == 2);  
}

void test_field() {
  const unsigned char *one_field_data = (const unsigned char *)"\x8e\x81\x84test\0\0\0\0\0\0\0\0";
  record_ref_type one_field(one_field_data, one_field_data + 15, 1);
  NP1_TEST_ASSERT(::np1::str::cmp(one_field.field(0), "test") == 0);
  NP1_TEST_ASSERT(one_field.field(1).is_null());

  
  const unsigned char *two_fields_data = (const unsigned char *)"\x95\x82\x85test1\x85test2\0\0\0\0\0\0\0\0";
  record_ref_type two_fields(two_fields_data, two_fields_data + 22, 1);
  NP1_TEST_ASSERT(::np1::str::cmp(two_fields.field(0), "test1") == 0);
  NP1_TEST_ASSERT(::np1::str::cmp(two_fields.field(1), "test2") == 0);
  NP1_TEST_ASSERT(two_fields.field(2).is_null());
}

void test_find_field() {
  const unsigned char *one_field_data = (const unsigned char *)"\x8e\x81\x84test\0\0\0\0\0\0\0\0";
  record_ref_type one_field(one_field_data, one_field_data + 15, 1);
  NP1_TEST_ASSERT(one_field.find_field("test") == 0);
  NP1_TEST_ASSERT(one_field.find_field("test1") == (size_t)-1);
  NP1_TEST_ASSERT(one_field.find_field("tes") == (size_t)-1);

  
  const unsigned char *two_fields_data = (const unsigned char *)"\x95\x82\x85test1\x85test2\0\0\0\0\0\0\0\0";
  record_ref_type two_fields(two_fields_data, two_fields_data + 22, 1);
  NP1_TEST_ASSERT(two_fields.find_field("test1") == 0);
  NP1_TEST_ASSERT(two_fields.find_field("test2") == 1);
  NP1_TEST_ASSERT(two_fields.find_field("test") == (size_t)-1);
  NP1_TEST_ASSERT(two_fields.find_field("test3") == (size_t)-1);
  NP1_TEST_ASSERT(two_fields.find_field("test12") == (size_t)-1);
}


void test_find_heading() {
  const unsigned char *one_heading_data = (const unsigned char *)"\x95\x81\x8bstring:test\0\0\0\0\0\0\0\0";
  record_ref_type one_heading(one_heading_data, one_heading_data + 22, 1);
  NP1_TEST_ASSERT(one_heading.find_heading("string:test") == 0);
  NP1_TEST_ASSERT(one_heading.find_heading("test") == 0);
  NP1_TEST_ASSERT(one_heading.find_heading("int:test") == (size_t)-1);
  NP1_TEST_ASSERT(one_heading.find_heading("tes") == (size_t)-1);

  const unsigned char *two_headings_data = (const unsigned char *)"\xa0\x82\x8cstring:test1\x89int:test2\0\0\0\0\0\0\0\0";
  record_ref_type two_headings(two_headings_data, two_headings_data + 33, 1);
  NP1_TEST_ASSERT(two_headings.find_heading("test1") == 0);
  NP1_TEST_ASSERT(two_headings.find_heading("test2") == 1);
  NP1_TEST_ASSERT(two_headings.find_heading("string:test1") == 0);
  NP1_TEST_ASSERT(two_headings.find_heading("int:test2") == 1);
  NP1_TEST_ASSERT(two_headings.find_heading("int:test1") == (size_t)-1);
  NP1_TEST_ASSERT(two_headings.find_heading("string:test2") == (size_t)-1);
  NP1_TEST_ASSERT(two_headings.find_heading("tes") == (size_t)-1);
}


void test_fields() {
  const unsigned char *one_field_data = (const unsigned char *)"\x8e\x81\x84test\0\0\0\0\0\0\0\0";
  record_ref_type one_field(one_field_data, one_field_data + 15, 1);
  std::vector<std::string> one_field_fields = one_field.fields();
  NP1_TEST_ASSERT(one_field_fields.size() == 1);
  NP1_TEST_ASSERT(::np1::str::cmp(one_field_fields[0], "test") == 0);
 
  const unsigned char *two_fields_data = (const unsigned char *)"\x95\x82\x85test1\x85test2\0\0\0\0\0\0\0\0";
  record_ref_type two_fields(two_fields_data, two_fields_data + 22, 1);
  std::vector<std::string> two_fields_fields = two_fields.fields();
  NP1_TEST_ASSERT(two_fields_fields.size() == 2);
  NP1_TEST_ASSERT(::np1::str::cmp(two_fields_fields[0], "test1") == 0);
  NP1_TEST_ASSERT(::np1::str::cmp(two_fields_fields[1], "test2") == 0);
}


void test_get_record_end() {
  const unsigned char *one_field_data = (const unsigned char *)"\x8e\x81\x84test\0\0\0\0\0\0\0\0";
  size_t length;
  for (length = 0; length < 15; ++length) {
    NP1_TEST_ASSERT(!record_ref_type::get_record_end(one_field_data, length));
  }

  NP1_TEST_ASSERT(record_ref_type::get_record_end(one_field_data, 15) == one_field_data + 15);

 
  const unsigned char *two_fields_data = (const unsigned char *)"\x95\x82\x85test1\x85test2\0\0\0\0\0\0\0\0";
  for (length = 0; length < 22; ++length) {
    NP1_TEST_ASSERT(!record_ref_type::get_record_end(two_fields_data, length));
  }

  NP1_TEST_ASSERT(record_ref_type::get_record_end(two_fields_data, 22) == two_fields_data + 22);
}


void test_contains_record() {
  const unsigned char *one_field_data = (const unsigned char *)"\x8e\x81\x84test\0\0\0\0\0\0\0\0";
  size_t length;
  uint64_t number_fields = 0xdeadbeef;
  for (length = 0; length < 15; ++length) {
    NP1_TEST_ASSERT(!record_ref_type::contains_record(one_field_data, length, number_fields));
  }

  NP1_TEST_ASSERT(record_ref_type::contains_record(one_field_data, 15, number_fields));
  NP1_TEST_ASSERT(1 == number_fields);

 
  const unsigned char *two_fields_data = (const unsigned char *)"\x95\x82\x85test1\x85test2\0\0\0\0\0\0\0\0";
  for (length = 0; length < 22; ++length) {
    NP1_TEST_ASSERT(!record_ref_type::contains_record(two_fields_data, length, number_fields));
  }

  NP1_TEST_ASSERT(record_ref_type::contains_record(two_fields_data, 22, number_fields));
  NP1_TEST_ASSERT(2 == number_fields);


  const char *plain_text = "this is some fabulous text";
  size_t plain_text_length = strlen(plain_text);
  for (length = 0; length <= plain_text_length; ++length) {
    NP1_TEST_ASSERT(!record_ref_type::contains_record((const unsigned char *)plain_text, length, number_fields));
  }
}


void test_byte_size() {
  const unsigned char *one_field_data = (const unsigned char *)"\x8e\x81\x84test\0\0\0\0\0\0\0\0";
  record_ref_type one_field(one_field_data, one_field_data + 15, 0);
  NP1_TEST_ASSERT(one_field.byte_size() == 15);
 
  const unsigned char *two_fields_data = (const unsigned char *)"\x95\x82\x85test1\x85test2\0\0\0\0\0\0\0\0";
  record_ref_type two_fields(two_fields_data, two_fields_data + 22, 0);
  NP1_TEST_ASSERT(two_fields.byte_size() == 22);
}


void test_write_argv_one_field() {
  buffer_output_stream_type output;
  const char *argv[] = {
    "test"  
  };

  record_ref_type::write(output, 1, argv);
  NP1_TEST_ASSERT(output.size() == 15);
  NP1_TEST_ASSERT(memcmp(output.ptr(), "\x8e\x81\x84test\0\0\0\0\0\0\0\0", output.size()) == 0);
}


void test_write_argv_two_fields() {
  buffer_output_stream_type output;
  const char *argv[] = {
    "test1",
    "test2"
  };

  record_ref_type::write(output, 2, argv);
  NP1_TEST_ASSERT(output.size() == 22);
  NP1_TEST_ASSERT(memcmp(output.ptr(), "\x95\x82\x85test1\x85test2\0\0\0\0\0\0\0\0", output.size()) == 0);
}


void test_write_argv_ten_fields() {
  buffer_output_stream_type output;
  const char *argv[] = {
    "very_exciting_long_test_test0",
    "very_exciting_long_test_test1",
    "very_exciting_long_test_test2",
    "very_exciting_long_test_test3",
    "very_exciting_long_test_test4",
    "very_exciting_long_test_test5",
    "very_exciting_long_test_test6",
    "very_exciting_long_test_test7",
    "very_exciting_long_test_test8",
    "very_exciting_long_test_test9",
  };

  record_ref_type::write(output, 10, argv);
  NP1_TEST_ASSERT(output.size() == 311);
  NP1_TEST_ASSERT(memcmp(output.ptr(), "\x35\x82\x8a\x9dvery_exciting_long_test_test0\x9dvery_exciting_long_test_test1\x9dvery_exciting_long_test_test2\x9dvery_exciting_long_test_test3\x9dvery_exciting_long_test_test4\x9dvery_exciting_long_test_test5\x9dvery_exciting_long_test_test6\x9dvery_exciting_long_test_test7\x9dvery_exciting_long_test_test8\x9dvery_exciting_long_test_test9\0\0\0\0\0\0\0\0", output.size()) == 0);
}


void test_write_two_records() {
  const unsigned char *one_field_data = (const unsigned char *)"\x8e\x81\x84test\0\0\0\0\0\0\0\0";
  record_ref_type one_field(one_field_data, one_field_data + 15, 0);
  NP1_TEST_ASSERT(one_field.byte_size() == 15);
 
  std::vector<std::string> two_fields;
  two_fields.push_back("test1");
  two_fields.push_back("test2");

  buffer_output_stream_type output;
  record_ref_type::write(output, one_field, two_fields);
  NP1_TEST_ASSERT(output.size() == 27);
  NP1_TEST_ASSERT(memcmp(output.ptr(), "\x9a\x83\x84test\x85test1\x85test2\0\0\0\0\0\0\0\0", 27) == 0);  
}


void test_is_equal() {
  record_type one_rec("one", "two", 0);
  record_type two_rec("one", "two", 0);
  NP1_TEST_ASSERT(one_rec.ref().is_equal(two_rec.ref()));
}


void test_record_ref() {
  NP1_TEST_RUN_TEST(test_number_fields);
  NP1_TEST_RUN_TEST(test_field);
  NP1_TEST_RUN_TEST(test_find_field);
  NP1_TEST_RUN_TEST(test_find_heading);
  NP1_TEST_RUN_TEST(test_fields);
  NP1_TEST_RUN_TEST(test_get_record_end);
  NP1_TEST_RUN_TEST(test_contains_record);
  NP1_TEST_RUN_TEST(test_byte_size);
  NP1_TEST_RUN_TEST(test_write_argv_one_field);
  NP1_TEST_RUN_TEST(test_write_argv_two_fields);
  NP1_TEST_RUN_TEST(test_write_argv_ten_fields);
  NP1_TEST_RUN_TEST(test_write_two_records);
  NP1_TEST_RUN_TEST(test_is_equal);
}

} // namespaces
}
}
}

#endif
