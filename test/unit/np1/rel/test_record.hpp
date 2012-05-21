// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_TEST_UNIT_NP1_REL_TEST_RECORD_HPP
#define NP1_TEST_UNIT_NP1_REL_TEST_RECORD_HPP


namespace test {
namespace unit {
namespace np1 {
namespace rel {

typedef ::np1::rel::record record_type;
typedef ::np1::io::static_buffer_output_stream<4096> buffer_output_stream_type;


void test_initialize_from_string_vector_then_write_record_1() {
  rstd::vector<rstd::string> fields;
  fields.push_back("string:Path_ID");
  fields.push_back("string:Other_Data_Brand");
  fields.push_back("string:Other_Data_Event");
  fields.push_back("string:Looked_Up_Product_Category");
  fields.push_back("int:_sum");

  record_type headings(fields, 0);
  buffer_output_stream_type output;
  headings.write(output);
  NP1_TEST_ASSERT(output.size() == 116);
  NP1_TEST_ASSERT(
    memcmp(
      output.ptr(),
      "\xf3\x85\x8estring:Path_ID\x97string:Other_Data_Brand\x97string:Other_Data_Event\xa1string:Looked_Up_Product_Category\x88int:_sum\0\0\0\0\0\0\0\0",
      output.size()) == 0);
}


void test_initialize_from_string_vector_then_write_record_2() {
  // This simulates a real-world problem.
  rstd::vector<rstd::string> fields;
  fields.push_back("string:User_ID");
  fields.push_back("string:Path_ID");
  fields.push_back("string:Other_Data_Brand");
  fields.push_back("string:Other_Data_Event");
  fields.push_back("string:Looked_Up_Product_Category");
  fields.push_back("int:_sum");

  record_type headings(fields, 0);
  buffer_output_stream_type output;
  headings.write(output);
  NP1_TEST_ASSERT(output.size() == 132);
  NP1_TEST_ASSERT(
    memcmp(
      output.ptr(),
      "\x02\x81\x86\x8estring:User_ID\x8estring:Path_ID\x97string:Other_Data_Brand\x97string:Other_Data_Event\xa1string:Looked_Up_Product_Category\x88int:_sum\0\0\0\0\0\0\0\0",
      output.size()) == 0);
}


void test_record() {
  NP1_TEST_RUN_TEST(test_initialize_from_string_vector_then_write_record_1);
  NP1_TEST_RUN_TEST(test_initialize_from_string_vector_then_write_record_2);
}

} // namespaces
}
}
}

#endif
