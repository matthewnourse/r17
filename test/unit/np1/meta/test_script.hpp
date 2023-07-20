// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_TEST_UNIT_NP1_REL_RLANG_TEST_SCRIPT_HPP
#define NP1_TEST_UNIT_NP1_REL_RLANG_TEST_SCRIPT_HPP

#include "np1/io/named_temp_file.hpp"
#include "np1/str.hpp"
#define NP1_TEST_UNIT_NP1_REL_RLANG_TEST_SCRIPT_TEST_DIR "/tmp/np1_test_script/"

#include "np1/io/file.hpp"
#include "np1/rel/rlang/rlang.hpp"
#include "np1/meta/script.hpp"
#include "test/unit/helper.hpp"

namespace test {
namespace unit {
namespace np1 {
namespace meta {


void create_test_files(::np1::io::file &input, ::np1::io::file &output,
                        const rstd::string &test_data) {
  FILE *input_fp = tmpfile();
  input.from_handle(input_fp);
  input.write(test_data.c_str(), test_data.length());
  input.rewind();

  FILE *output_fp = tmpfile();
  output.from_handle(output_fp);
}


void run_script(const rstd::string &script, const rstd::string &test_data, const rstd::string &expected_output) {
  FILE *script_fp = tmpfile();
  ::np1::io::file script_file;
  script_file.from_handle(script_fp);
  script_file.write(script.c_str());
  script_file.hard_flush();
  script_file.rewind();

  ::np1::io::file input;
  ::np1::io::file output;

  create_test_files(input, output, test_data);

  //TODO: test with arguments
  typedef ::np1::rel::rlang::token token_type;
  ::rstd::vector<rstd::vector<token_type> > empty_arguments;
  ::np1::meta::script::run_from_stream(input, output, script_file, "[test]", empty_arguments);

  output.rewind();

  rstd::vector<char> actual_output;
  size_t expected_output_length = expected_output.length();
  actual_output.resize((expected_output_length + 1) * 2);
  size_t bytes_read = 0;
  output.read(&actual_output[0], actual_output.size(), &bytes_read);
  actual_output[bytes_read] = '\0';
//  fprintf(stderr, "EXPECTED\n'%s'\n\nACTUAL\n'%s'\n", expected_output.c_str(), &actual_output[0]);
  NP1_TEST_ASSERT(bytes_read == expected_output_length);
  NP1_TEST_ASSERT(memcmp(&actual_output[0], expected_output.c_str(), expected_output_length) == 0);
}


void check_split_file(const rstd::string &prefix, uint64_t n, const char *expected_result) {
  rstd::string file_name = prefix + ::np1::str::to_hex_str_pad_16(n) + ".gz";
  run_script("io.file.read('" + file_name + "') | rel.to_tsv();",
            "",
            expected_result);
}

void check_from_shapefile(const char *hex_shp, const char *expected_result) {
  ::np1::io::named_temp_file shp_tmp_file;
  ::np1::str::write_decoded_hex(hex_shp, shp_tmp_file.real_file());
  shp_tmp_file.real_file().hard_flush();
  shp_tmp_file.real_file().rewind();

  run_script(
      "rel.from_shapefile('" + shp_tmp_file.file_name() + "') | rel.to_tsv();",
      "",
      expected_result);
}

const char *basic_flintstones_data() {
  return
    "string:name\tuint:value1\tint:value2\n"
    "fred\t1\t2\n"
    "fred\t3\t4\n"
    "barney\t5\t6\n"
    "fred\t7\t8\n"
    "betty\t7\t8\n"
    "wilma\t100\t-1\n";
}

const char *test_duplicates_flintstones_data() {
  return
    "string:name\tuint:value1\tint:value2\n"
    "fred\t1\t2\n"
    "fred\t3\t4\n"
    "barney\t5\t6\n"
    "fred\t7\t8\n"
    "betty\t7\t8\n"
    "wilma\t100\t-1\n"
    "dino\t3\t4\n"
    "doozy\t3\t4\n"
    "pearl\t3\t4\n"
    "ed\t3\t4\n"
    "zeke\t3\t4\n"
    "bamm-bamm\t3\t4\n"
    "tex\t3\t4\n"
    "jemima\t3\t4\n"
    "arnold\t3\t4\n";
}


void test_single_stream_operator() {
  run_script(
    "# This is a comment.\n"
    "rel.from_tsv() | rel.select(name as character) | rel.to_tsv();",

    basic_flintstones_data(),
 
    "string:character\n"
    "fred\n"
    "fred\n"
    "barney\n"
    "fred\n"
    "betty\n"
    "wilma\n");
}

void test_multiple_stream_operators() {
  run_script(
    "rel.from_tsv() | rel.select(name as character) | rel.where(character = 'fred') | rel.to_tsv();",

    basic_flintstones_data(),
 
    "string:character\n"
    "fred\n"
    "fred\n"
    "fred\n");


  run_script(
    "rel.from_tsv() | rel.where(value1 = 7U) | rel.where(value2 = 8) | rel.where(name = 'fred') | rel.to_tsv();",

    basic_flintstones_data(),
 
    "string:name\tuint:value1\tint:value2\n"
    "fred\t7\t8\n");
}


void test_multiple_pipelines() {
  run_script(
    "rel.from_tsv() | rel.where(value1 = 7U) | io.file.overwrite('/tmp/this_is_a_very_boring_test_file_for_the_script_test.csv');\n"
    "io.file.read('/tmp/this_is_a_very_boring_test_file_for_the_script_test.csv') | rel.select(name) | rel.to_tsv();",
    
    basic_flintstones_data(),

    "string:name\n"
    "fred\n"
    "betty\n"
  );
}


void test_inline_script() {
  ::np1::io::file input;
  ::np1::io::file output;

  create_test_files(input, output, basic_flintstones_data());

  //TODO: test with non-empty arguments.
  typedef ::np1::rel::rlang::token token_type;
  ::rstd::vector<rstd::vector<token_type> > empty_arguments;
  ::np1::meta::script::run(input, output, "rel.from_tsv() | rel.where(name = 'fred') | rel.to_tsv();",
                           empty_arguments);

  const char *expected_output =
    "string:name\tuint:value1\tint:value2\n"
    "fred\t1\t2\n"
    "fred\t3\t4\n"
    "fred\t7\t8\n";

  output.rewind();

  rstd::vector<char> actual_output;
  size_t expected_output_length = strlen(expected_output);
  actual_output.resize((expected_output_length + 1) * 2);
  size_t bytes_read = 0;
  output.read(&actual_output[0], actual_output.size(), &bytes_read);
  actual_output[bytes_read] = '\0';
  //printf("EXPECTED\n%s\n\nACTUAL\n%s\n", expected_output, &actual_output[0]);
  NP1_TEST_ASSERT(bytes_read == expected_output_length);
  NP1_TEST_ASSERT(memcmp(&actual_output[0], expected_output, expected_output_length) == 0);
}


void test_variable_record_lengths() {
  static const size_t max_record_length = 1024;
  rstd::string input = "string:test\n";

  size_t i;
  for (i = 0; i < max_record_length; i += 17) {
    size_t j;
    for (j = 0; j < i; ++j) {
      input.push_back('a');
    }

    input.push_back('\n');
  }

  run_script(
    "rel.from_tsv() | rel.where(true) | rel.to_tsv();",
    input,
    input);
}


void test_python() {  
  run_script(
    "rel.from_tsv() | lang.python(@@@\nfor inputR in r17InputStream:\n    r17OutputStream.write(inputR)\n\n@@@) | rel.to_tsv();",

    "string:v1\tistring:v2\tint:v3\tuint:v4\tdouble:v5\tbool:v6\tipaddress:v7\n"
    "fred\twilma\t-1\t1\t1.1\ttrue\t192.168.1.1\n"
    "barney\tbetty\t1\t10\t-1.1\tfalse\t127.0.0.1\n",

    // Python has no "ipaddress", "istring" or "uint" types.
    "string:v1\tstring:v2\tint:v3\tint:v4\tdouble:v5\tbool:v6\tstring:v7\n"
    "fred\twilma\t-1\t1\t1.1\ttrue\t192.168.1.1\n"
    "barney\tbetty\t1\t10\t-1.1\tfalse\t127.0.0.1\n"
  );
}


void test_r() {
  const char *all_types_input =
    "string:v1\tistring:v2\tint:v3\tuint:v4\tdouble:v5\tbool:v6\tipaddress:v7\n"
    "fred\twilma\t-1\t1\t1.1\ttrue\t192.168.1.1\n"
    "barney\tbetty\t1\t10\t-1.1\tfalse\t127.0.0.1\n";

  run_script(
    "rel.from_tsv() "
      "| lang.R(@@@\n"
        "r17WriteTable(c(\"string:v1\", \"istring:v2\", \"int:v3\", \"uint:v4\", \"double:v5\", \"bool:v6\", \"ipaddress:v7\"), r17InputTable)\n"
      "@@@) "
      "| rel.to_tsv();",

    all_types_input,

    all_types_input
  );

  const char *no_bool_types_input =
    "string:v1\n"
    "fred\n";

  run_script(
    "rel.from_tsv() "
      "| lang.R(@@@\n"
        "r17WriteTable(c(\"string:v1\"), r17InputTable)\n"
      "@@@) "
      "| rel.to_tsv();",

    no_bool_types_input,

    no_bool_types_input
  ); 
}


void test_order_by(const rstd::string &op_name, const char *expected_results) {
  printf("  order_by op: %s\n", op_name.c_str());

  run_script(
    "rel.from_tsv() | " + op_name + " (value1, value2) | rel.to_tsv();",
    
    test_duplicates_flintstones_data(),

    expected_results);
}

void test_order_by_asc(const rstd::string &op_name) {
  test_order_by(
    op_name, 
    "string:name\tuint:value1\tint:value2\n"
    "fred\t1\t2\n"
    "fred\t3\t4\n"
    "dino\t3\t4\n"
    "doozy\t3\t4\n"
    "pearl\t3\t4\n"
    "ed\t3\t4\n"
    "zeke\t3\t4\n"
    "bamm-bamm\t3\t4\n"
    "tex\t3\t4\n"
    "jemima\t3\t4\n"
    "arnold\t3\t4\n"
    "barney\t5\t6\n"
    "fred\t7\t8\n"
    "betty\t7\t8\n"
    "wilma\t100\t-1\n"); 
}

void test_order_by_desc(const rstd::string &op_name) {
  test_order_by(
    op_name,
    "string:name\tuint:value1\tint:value2\n"
    "wilma\t100\t-1\n"
    "betty\t7\t8\n"
    "fred\t7\t8\n"    
    "barney\t5\t6\n"
    "arnold\t3\t4\n"
    "jemima\t3\t4\n"
    "tex\t3\t4\n"
    "bamm-bamm\t3\t4\n"
    "zeke\t3\t4\n"
    "ed\t3\t4\n"
    "pearl\t3\t4\n"
    "doozy\t3\t4\n"
    "dino\t3\t4\n"
    "fred\t3\t4\n"       
    "fred\t1\t2\n"); 
}


void test_order_by() {
  test_order_by_asc("rel.order_by");
  test_order_by_asc("rel.order_by.mergesort");
  test_order_by_asc("rel.order_by.quicksort");

  test_order_by_desc("rel.order_by.desc");
  test_order_by_desc("rel.order_by.mergesort.desc");
  test_order_by_desc("rel.order_by.quicksort.desc");
  
  printf("  order_by double\n");

  run_script(
    "rel.from_tsv() | rel.order_by(double1) | rel.to_tsv();",
    
    "string:name\tdouble:double1\n"
    "fred\t2.0\n"
    "wilma\t1.1\n",

    "string:name\tdouble:double1\n"    
    "wilma\t1.1\n"
    "fred\t2.0\n");
  
  printf("  big order_by\n");
  
  rstd::string test_data;
  make_very_large_test_data_record_string(test_data);

  run_script("rel.from_tsv() | rel.order_by.desc(mul1_int) | rel.order_by(mul1_int) | rel.to_tsv();",
             test_data,
             test_data);

  
  //TODO: test more big sorts!
}



void test_join() {
  //TODO: implement temp files to handle this kind of case.
  // Test joins where the file has no non-common fields.
  const char *join_file_name = "/tmp/this_is_a_very_boring_test_file_for_join.native";
  run_script("rel.from_tsv() | io.file.overwrite(\"" + rstd::string(join_file_name) + "\");",
              "string:name\nfred\nbarney\n",
              "");

  run_script(
    "rel.from_tsv() | rel.join.natural(\"" + rstd::string(join_file_name) + "\") | rel.to_tsv();",

    basic_flintstones_data(),

    "string:name\tuint:value1\tint:value2\n"
    "fred\t1\t2\n"
    "fred\t3\t4\n"
    "barney\t5\t6\n"
    "fred\t7\t8\n"
  );


  run_script(
    "rel.from_tsv() | rel.join.left(\"" + rstd::string(join_file_name) + "\") | rel.to_tsv();",

    basic_flintstones_data(),

    "string:name\tuint:value1\tint:value2\n"
    "fred\t1\t2\n"
    "fred\t3\t4\n"
    "barney\t5\t6\n"
    "fred\t7\t8\n"
    "betty\t7\t8\n"
    "wilma\t100\t-1\n"
  );


  run_script(
    "rel.from_tsv() | rel.join.anti(\"" + rstd::string(join_file_name) + "\") | rel.to_tsv();",

    basic_flintstones_data(),

    "string:name\tuint:value1\tint:value2\n"
    "betty\t7\t8\n"
    "wilma\t100\t-1\n"
  );


  // Consistent hash join.  TODO: how to test the correctness of this?  The expected output
  // is based on the current implementation, if the hash implementation changes then the
  // test will fail, but the output might still be legal.
  run_script(
    "rel.from_tsv() | rel.join.consistent_hash(\"" + rstd::string(join_file_name) + "\") | rel.to_tsv();",

    "int:value1\n0\n",

    "int:value1\tstring:name\n0\tbarney\n"
  );


  // Test joins where the file HAS non-common fields.
  run_script("rel.from_tsv() | io.file.overwrite(\"" + rstd::string(join_file_name) + "\");",
              "string:name\tint:value3\nfred\t10\nbarney\t20\n",
              "");

  run_script(
    "rel.from_tsv() | rel.join.natural(\"" + rstd::string(join_file_name) + "\") | rel.to_tsv();",

    basic_flintstones_data(),

    "string:name\tuint:value1\tint:value2\tint:value3\n"
    "fred\t1\t2\t10\n"
    "fred\t3\t4\t10\n"
    "barney\t5\t6\t20\n"
    "fred\t7\t8\t10\n"
  );


  run_script(
    "rel.from_tsv() | rel.join.left(\"" + rstd::string(join_file_name) + "\") | rel.to_tsv();",

    basic_flintstones_data(),

    "string:name\tuint:value1\tint:value2\tint:value3\n"
    "fred\t1\t2\t10\n"
    "fred\t3\t4\t10\n"
    "barney\t5\t6\t20\n"
    "fred\t7\t8\t10\n"
    "betty\t7\t8\t0\n"
    "wilma\t100\t-1\t0\n"
  );


  run_script(
    "rel.from_tsv() | rel.join.anti(\"" + rstd::string(join_file_name) + "\") | rel.to_tsv();",

    basic_flintstones_data(),

    "string:name\tuint:value1\tint:value2\n"
    "betty\t7\t8\n"
    "wilma\t100\t-1\n"
  );


  //TODO: MUCH more join testing!
}


void test_group() {
  // count
  run_script(
    "rel.from_tsv() | rel.group(count) | rel.to_tsv();",
    
    "string:name\n"
    "fred\n"
    "fred\n"
    "fred\n"
    "barney\n"
    "wilma\n"
    "wilma\n",
    
    "string:name\tuint:_count\n"   // NOTE the ordering here is hash-ordering so if the implementation changes
    "barney\t1\n"                  // this test will fail
    "wilma\t2\n"
    "fred\t3\n");
  
  // sum
  run_script(
    "rel.from_tsv() | rel.group(sum value) | rel.to_tsv();",
    
    "int:value\n"
    "1\n"
    "2\n"
    "3\n",
    
    "int:_sum\n"
    "6\n");

  run_script(
    "rel.from_tsv() | rel.group(sum value) | rel.to_tsv();",
    
    "double:value\n"
    "1.1\n"
    "2.2\n"
    "3.3\n",
    
    "double:_sum\n"
    "6.600000\n");

  // avg
  run_script(
    "rel.from_tsv() | rel.group(avg value) | rel.to_tsv();",
    
    "int:value\n"
    "1\n"
    "2\n"
    "3\n",
    
    "int:_avg\n"
    "2\n");

  run_script(
    "rel.from_tsv() | rel.group(avg value) | rel.to_tsv();",
    
    "double:value\n"
    "1.1\n"
    "2.2\n"
    "3.3\n",
    
    "double:_avg\n"
    "2.200000\n");

  // min
  run_script(
    "rel.from_tsv() | rel.group(min value) | rel.to_tsv();",
    
    "int:value\n"
    "1\n"
    "2\n"
    "3\n",
    
    "int:value\n"
    "1\n");

  run_script(
    "rel.from_tsv() | rel.group(min value) | rel.to_tsv();",
    
    "double:value\n"
    "1.1\n"
    "2.2\n"
    "3.3\n",
    
    "double:value\n"
    "1.1\n");

  // max
  run_script(
    "rel.from_tsv() | rel.group(max value) | rel.to_tsv();",
    
    "int:value\n"
    "1\n"
    "2\n"
    "3\n",
    
    "int:value\n"
    "3\n");

  run_script(
    "rel.from_tsv() | rel.group(max value) | rel.to_tsv();",
    
    "double:value\n"
    "1.1\n"
    "2.2\n"
    "3.3\n",
    
    "double:value\n"
    "3.3\n");

  // median, test 1
  run_script(
    "rel.from_tsv() | rel.group(median value) | rel.to_tsv();",

    "int:value\n"
    "2\n"
    "1\n"
    "3\n"
    "5\n",

    "int:_median\n"
    "2\n");

  // median, test 2
  run_script(
    "rel.from_tsv() | rel.group(median value) | rel.to_tsv();",

    "string:name\tint:value\n"
    "fred\t2\n"
    "barney\t1\n"
    "fred\t3\n"
    "barney\t3\n"
    "barney\t-1\n"
    "barney\t3\n"
    "barney\t10\n"
    "wilma\t5\n",

    "string:name\tint:_median\n"
    "barney\t3\n"
    "fred\t2\n"
    "wilma\t5\n");

  //TODO: much more group testing!
  
}


void test_unique() {
  run_script(
    "rel.from_tsv() | rel.unique() | rel.to_tsv();",
    
    "int:value\n"
    "1\n"
    "1\n"
    "1\n"
    "2\n"
    "2\n",
    
    "int:value\n"
    "1\n"
    "2\n"
  );
  
  //TODO: much more unique testing!
}


void test_select() {
  run_script(
    "rel.from_tsv() "
    "| rel.select(str.regex_match('fr.*', name) as match, str.regex_replace('(^be).*', name, '\\14444') as replaced) "
    "| rel.to_tsv();",
    
    basic_flintstones_data(),

    "bool:match\tstring:replaced\n"
    "true\tfred\n"
    "true\tfred\n"
    "false\tbarney\n"
    "true\tfred\n"
    "false\tbe4444\n"
    "false\twilma\n"
  );

  // Test 2 calls to a method that uses the heap, to check that the heap is appropriately reused.
  run_script(
    "rel.from_tsv() "
    "| rel.select(str.regex_replace('fred', name, 'not\\0') as replaced1, str.regex_replace('(^fr).*', name, '\\1 4444 \\0 is the original name') as replaced2) "
    "| rel.to_tsv();",
    
    basic_flintstones_data(),

    "string:replaced1\tstring:replaced2\n"
    "notfred\tfr 4444 fred is the original name\n"
    "notfred\tfr 4444 fred is the original name\n"
    "barney\tbarney\n"
    "notfred\tfr 4444 fred is the original name\n"
    "betty\tbetty\n"
    "wilma\twilma\n"
  );
  
  // Test reordered fast path columns.
  run_script(
    "rel.from_tsv() | rel.select(value1, value2, name) | rel.to_tsv();",
    
    basic_flintstones_data(),
    
    "uint:value1\tint:value2\tstring:name\n"
    "1\t2\tfred\n"
    "3\t4\tfred\n"
    "5\t6\tbarney\n"
    "7\t8\tfred\n"
    "7\t8\tbetty\n"
    "100\t-1\twilma\n");  
}


void test_record_count() {
  run_script(
    "rel.from_tsv() | rel.record_count();",
    basic_flintstones_data(),
    "6");
}


void test_record_split() {
  rstd::string prefix = "/tmp/np1_test_script/test_record_split_";

  run_script(
    "rel.from_tsv() | rel.record_split(1, '" + prefix + "');",
    basic_flintstones_data(),
    "");

  check_split_file(
    prefix, 0,
    "string:name\tuint:value1\tint:value2\n"
    "fred\t1\t2\n");

  check_split_file(
    prefix, 1,
    "string:name\tuint:value1\tint:value2\n"
    "fred\t3\t4\n");

  check_split_file(
    prefix, 2,
    "string:name\tuint:value1\tint:value2\n"
    "barney\t5\t6\n");

  check_split_file(
    prefix, 3,
    "string:name\tuint:value1\tint:value2\n"
    "fred\t7\t8\n");

  check_split_file(
    prefix, 4,
    "string:name\tuint:value1\tint:value2\n"
    "betty\t7\t8\n");

  check_split_file(
    prefix, 5,
    "string:name\tuint:value1\tint:value2\n"
    "wilma\t100\t-1\n");
}



void test_str_split() {
  run_script(
    "rel.from_tsv() | rel.str_split(name, 'e') | rel.to_tsv();",

    basic_flintstones_data(),

    "string:name\tuint:value1\tint:value2\tuint:_counter\n"
    "fr\t1\t2\t0\n"
    "d\t1\t2\t1\n"
    "fr\t3\t4\t0\n"
    "d\t3\t4\t1\n"
    "barn\t5\t6\t0\n"    
    "y\t5\t6\t1\n"    
    "fr\t7\t8\t0\n"
    "d\t7\t8\t1\n"
    "b\t7\t8\t0\n"
    "tty\t7\t8\t1\n"
    "wilma\t100\t-1\t0\n"
  );


  run_script(
    "rel.from_tsv() | rel.str_split(raw_response, '\\r\\n\\r\\n') | rel.to_tsv();",

    "string:raw_response\n"
    "HTTP/1.1 200 OK\\r\\nContent-Type: text/html\\r\\n\\r\\nContent\n",

    "string:raw_response\tuint:_counter\n"
    "HTTP/1.1 200 OK\\r\\nContent-Type: text/html\t0\n"
    "Content\t1\n"
  );
}


void test_from_tsv() {
  // from_tsv is mostly tested throughout the rest of this file.  Here we just check the auto-type-tag,
  // header name safe-ifying, explicit header name specification and escaping.
  run_script(
    "rel.from_tsv() | rel.to_tsv();",

    "name\tistring:value\tun&s Afe\tint:un safe\tnotatype:u n s a f e\n"
    "fred\tmary\tjane\t1\talphonse\n",

    "string:name\tistring:value\tstring:un_s_Afe\tint:un_safe\tstring:notatype_u_n_s_a_f_e\n"
    "fred\tmary\tjane\t1\talphonse\n"
  );
  
  run_script(
    "rel.from_tsv('istring:name', 'value') | rel.to_tsv();",
    
    "fred\tten\n"
    "mary\televen\n",
    
    "istring:name\tstring:value\n"
    "fred\tten\n"
    "mary\televen\n"
  );

  run_script(
    "rel.from_tsv() | rel.to_tsv();",
    
    "string:value\n"
    "\\n\\r\\t\\Xabc\n",
    
    "string:value\n"
    "\\n\\r\\t\\Xabc\n"    
  );
}


void test_from_csv() {
  // Test header name safe-ifying.
  run_script(
    "rel.from_csv() | rel.to_csv();",

    "name,istring:value,un&s Afe,int:un safe,notatype:u n s a f e\n"
    "fred,mary,jane,1,alphonse\n",

    "string:name,istring:value,string:un_s_Afe,int:un_safe,string:notatype_u_n_s_a_f_e\n"
    "fred,mary,jane,1,alphonse\n"
  );
    
  // Test auto-type-tag.
  run_script(
    "rel.from_csv('istring:name', 'value') | rel.to_csv();",
    
    "fred,ten\n"
    "mary,eleven\n",
    
    "istring:name,string:value\n"
    "fred,ten\n"
    "mary,eleven\n"
  );
    
  // Test that quote character handling is consistent across parsing and generating.
  // This test supplied by Gyuchang, the author of CSV parsing code.
  run_script(
    "rel.from_csv() | rel.to_csv();",

    "int:year,string:make,string:model,string:description,double:price\n"
    "1997,Ford,E350,\"ac, abs, moon\",3000.00\n"
    "1999,Chevy,\"Venture \"\"Extended Edition\"\"\",\"\",4900.00\n"
    "1999,Chevy,\"Venture \"\"Extended Edition, Very Large\"\"\",\"\",5000.00\n"
    "1996,Jeep,Grand Cherokee,\"MUST SELL!\\nair, moon roof, loaded\",4799.00\n",

    "int:year,string:make,string:model,string:description,double:price\n"
    "1997,Ford,E350,\"ac, abs, moon\",3000.00\n"
    "1999,Chevy,\"Venture \"\"Extended Edition\"\"\",,4900.00\n"
    "1999,Chevy,\"Venture \"\"Extended Edition, Very Large\"\"\",,5000.00\n"
    "1996,Jeep,Grand Cherokee,\"MUST SELL!\\nair, moon roof, loaded\",4799.00\n"
  );
  
  // Test varied quote placement and special characters.
  run_script(
    "rel.from_csv() | rel.to_csv();",
    
    "string:value1,string:value2\n"
    "\"fred\", $+*\n"
    "jane,\",,,,\"\n"
    "\"fred\",\"jane\"\n",
    
    "string:value1,string:value2\n"
    "fred, $+*\n"
    "jane,\",,,,\"\n"
    "fred,jane\n"    
  );

  // Test escaping
  run_script(
    "rel.from_csv() | rel.to_csv();",
    
    "string:value\n"
    "\\n\\r\\t\\Xabc\n",
    
    "string:value\n"
    "\\n\\r\\t\\Xabc\n"    
  );

}



void test_from_usv_and_to_usv() {
  run_script(
    "rel.from_tsv() | rel.to_usv() | rel.from_usv() | rel.to_tsv();",

    basic_flintstones_data(),

    basic_flintstones_data()
  );  
}



void test_from_text() {
  run_script(
    "rel.from_text('([^|]*?)\\|([^|]*?)\\|([^|]+)', 'string:first', 'istring:second', 'third') | rel.to_tsv();",

    "this|is a song|about a fish",

    "string:first\tistring:second\tstring:third\n"
  );

  run_script(
    "rel.from_text('([^|]*?)\\|([^|]*?)\\|([^|]+)', 'string:first', 'istring:second', 'third') | rel.to_tsv();",

    "this|is a song|about a fish\n",

    "string:first\tistring:second\tstring:third\n"
    "this\tis a song\tabout a fish\n"
  );


  run_script(
    "rel.from_text('([^|]*?)\\|([^|]*?)\\|([^|]+)', 'string:first', 'istring:second', 'third') | rel.to_tsv();",

    "this|is a song|about a fish\n"
    "the|fish|is mute\n",


    "string:first\tistring:second\tstring:third\n"
    "this\tis a song\tabout a fish\n"
    "the\tfish\tis mute\n"
  );

  //TODO: tests with bigger files.
}


void test_from_text_ignore_non_matching() {
  run_script(
    "rel.from_text_ignore_non_matching('([^|]*?)\\|([^|]*?)\\|([^|]+)', 'string:first', 'istring:second', 'third') | rel.to_tsv();",

    "this|is a song|about a fish",

    "string:first\tistring:second\tstring:third\n"
  );

  run_script(
    "rel.from_text_ignore_non_matching('([^|]*?)\\|([^|]*?)\\|([^|]+)', 'string:first', 'istring:second', 'third') | rel.to_tsv();",

    "this|is a song|about a fish\n",

    "string:first\tistring:second\tstring:third\n"
    "this\tis a song\tabout a fish\n"
  );


  run_script(
    "rel.from_text_ignore_non_matching('([^|]*?)\\|([^|]*?)\\|([^|]+)', 'string:first', 'istring:second', 'third') | rel.to_tsv();",

    "this|is a song|about a fish\n"
    "the|fish|is mute\n",


    "string:first\tistring:second\tstring:third\n"
    "this\tis a song\tabout a fish\n"
    "the\tfish\tis mute\n"
  );

  run_script(
    "rel.from_text_ignore_non_matching('([^|]*?)\\|(is a song)\\|([^|]+)', 'string:first', 'istring:second', 'third') | rel.to_tsv();",

    "this|is a song|about a fish\n"
    "the|fish|is mute\n",


    "string:first\tistring:second\tstring:third\n"
    "this\tis a song\tabout a fish\n"
  );


  //TODO: tests with bigger files.
}

void test_from_shapefile() {
  check_from_shapefile(
      "0000270a000000000000000000000000000000000000000000000040e803000001000000208cac45bb1d6ac0b10ba1233e0141c0208cac45bb1d6ac0b10ba1233e0141c00000000000000000000000000000000000000000000000000000000000000000000000000000000a01000000208cac45bb1d6ac0b10ba1233e0141c0", 
      "string:_geom\n"
      "POINT (-208.929110 -34.009709)\n");

  check_from_shapefile(
      "0000270a00000000000000000000000000000000000000000000004ee803000001000000a513469d881d6ac0f4048862300441c067d2f969771d6ac0542c6d9acc0341c00000000000000000000000000000000000000000000000000000000000000000000000000000000a01000000a513469d881d6ac0542c6d9acc0341c0000000010000000a0100000067d2f969771d6ac0f4048862300441c0", 
      "string:_geom\n"
      "POINT (-208.922927 -34.029681)\n"
      "POINT (-208.920827 -34.032727)\n");

  check_from_shapefile(
      "0000270a0000000000000000000000000000000000000000000000a0e803000005000000352fbdeb171e6ac02bb6a7b3da0241c0aff183d4981d6ac0a2e5f185880141c00000000000000000000000000000000000000000000000000000000000000000000000010000006a05000000352fbdeb171e6ac02bb6a7b3da0241c0aff183d4981d6ac0a2e5f185880141c0020000000a0000000000000005000000a22334bdfd1d6ac0a2e5f185880141c0aff183d4981d6ac04515ef548a0141c043fd0c03b31d6ac0fdecc915d70241c0352fbdeb171e6ac02bb6a7b3da0241c0a22334bdfd1d6ac0a2e5f185880141c0a5bb34bdf71d6ac0618d8e54340241c01f0d0703ef1d6ac03c0e5ca1c90141c07247c7ebb11d6ac0cb775670cb0141c03f650c03b91d6ac0b63ecadb7a0241c0a5bb34bdf71d6ac0618d8e54340241c0",
      "string:_geom\n"
      "MULTIPOLYGON((-208.937224 -34.011979,-208.924906 -34.012034,-208.928102 -34.022189,-208.940420 -34.022299,-208.937224 -34.011979),(-208.936492 -34.017222,-208.935426 -34.013966,-208.927969 -34.014021,-208.928834 -34.019374,-208.936492 -34.017222))\n");
}


void test_generate_sequence() {
  run_script(
    "rel.generate_sequence(-1, 10) | rel.to_tsv();",

    "",

    "int:_seq\n"
    "-1\n"
    "0\n"
    "1\n"
    "2\n"
    "3\n"
    "4\n"
    "5\n"
    "6\n"
    "7\n"
    "8\n"
    "9\n"
  );
}


void test_utf16_to_utf8() {
  const char *expected_result = "this is a test unicode file.\r\nit is extremely exciting.\r\n";

  // Little-endian.
  run_script(
    "text.utf16_to_utf8();",

    rstd::string("\xff\xfe\x74\x00\x68\x00\x69\x00\x73\x00\x20\x00\x69\x00\x73\x00\x20\x00\x61\x00\x20\x00\x74\x00\x65\x00\x73\x00\x74\x00\x20\x00\x75\x00\x6e\x00\x69\x00\x63\x00\x6f\x00\x64\x00\x65\x00\x20\x00\x66\x00\x69\x00\x6c\x00\x65\x00\x2e\x00\x0d\x00\x0a\x00\x69\x00\x74\x00\x20\x00\x69\x00\x73\x00\x20\x00\x65\x00\x78\x00\x74\x00\x72\x00\x65\x00\x6d\x00\x65\x00\x6c\x00\x79\x00\x20\x00\x65\x00\x78\x00\x63\x00\x69\x00\x74\x00\x69\x00\x6e\x00\x67\x00\x2e\x00\x0d\x00\x0a\x00", 116),

    expected_result
  );

  // Big-endian.
  run_script(
    "text.utf16_to_utf8();",

    rstd::string("\xfe\xff\x00\x74\x00\x68\x00\x69\x00\x73\x00\x20\x00\x69\x00\x73\x00\x20\x00\x61\x00\x20\x00\x74\x00\x65\x00\x73\x00\x74\x00\x20\x00\x75\x00\x6e\x00\x69\x00\x63\x00\x6f\x00\x64\x00\x65\x00\x20\x00\x66\x00\x69\x00\x6c\x00\x65\x00\x2e\x00\x0d\x00\x0a\x00\x69\x00\x74\x00\x20\x00\x69\x00\x73\x00\x20\x00\x65\x00\x78\x00\x74\x00\x72\x00\x65\x00\x6d\x00\x65\x00\x6c\x00\x79\x00\x20\x00\x65\x00\x78\x00\x63\x00\x69\x00\x74\x00\x69\x00\x6e\x00\x67\x00\x2e\x00\x0d\x00\x0a", 116),

    expected_result
  );
}


void test_strip_cr() {
  const char *expected_result = "this is some data with\nCRs removed\n";

  run_script(
    "text.strip_cr();",

    "\rthis is some dat\ra with\r\nCRs removed\r\n",

    expected_result
  );

  run_script(
    "text.strip_cr();",

    "\rthis is some dat\ra with\r\nCRs removed\r\n\r",

    expected_result
  );
}


void test_remote() {
  rstd::vector<rstd::string> hostnames;
  hostnames.push_back("127.0.0.1");
  hostnames.push_back("localhost");

  rstd::vector<rstd::string>::const_iterator i = hostnames.begin();
  rstd::vector<rstd::string>::const_iterator iz = hostnames.end();
  for (; i != iz; ++i) {
    run_script(
      "rel.from_tsv() | meta.remote('" + *i + "', rel.select(name as character) | rel.where(character = 'fred')) | rel.to_tsv();",
  
      basic_flintstones_data(),
   
      "string:character\n"
      "fred\n"
      "fred\n"
      "fred\n");
  }
}


void test_shell() {
  run_script(
    "meta.shell('cat');",

    basic_flintstones_data(),

    basic_flintstones_data()
  );
}


void test_parallel() {
  rstd::string file_prefix = "/tmp/np1_test_script/test_parallel_";

  // Make the test data and write it out to a bunch of files.
  rstd::string test_data;
  make_large_test_data_record_string(test_data);

  run_script("rel.from_tsv() | rel.record_split(100000, '" + file_prefix + "');", test_data, "");

  // Run the parallel test.
  run_script(
    "rel.from_tsv() | meta.parallel_explicit_mapping(rel.where(!str.starts_with(mul1_str, 'a')) | rel.where(mul1_int % 7 = 0) | rel.select('a' as dummy)) | rel.group(count) | rel.to_tsv();",

    "string:file_name\tstring:host_name\n"
    + file_prefix + ::np1::str::to_hex_str_pad_16(0) + ".gz\tlocalhost\n"
    + file_prefix + ::np1::str::to_hex_str_pad_16(1) + ".gz\t127.0.0.1\n"
    + file_prefix + ::np1::str::to_hex_str_pad_16(2) + ".gz\t127.0.0.1\n"
    + file_prefix + ::np1::str::to_hex_str_pad_16(3) + ".gz\t127.0.0.1\n"
    + file_prefix + ::np1::str::to_hex_str_pad_16(4) + ".gz\t127.0.0.1\n"
    + file_prefix + ::np1::str::to_hex_str_pad_16(5) + ".gz\tlocalhost\n"
    + file_prefix + ::np1::str::to_hex_str_pad_16(6) + ".gz\tlocalhost\n"
    + file_prefix + ::np1::str::to_hex_str_pad_16(7) + ".gz\tlocalhost\n"
    + file_prefix + ::np1::str::to_hex_str_pad_16(8) + ".gz\tlocalhost\n"
    + file_prefix + ::np1::str::to_hex_str_pad_16(9) + ".gz\tlocalhost\n",

    "string:dummy\tuint:_count\na\t142858\n");
}


void test_file_read() {
  rstd::string file_prefix = "/tmp/np1_test_script/test_file_read_";

  // Make the test data and write it out to a bunch of files.
  rstd::string test_data;
  make_large_test_data_record_string(test_data);

  run_script("rel.from_tsv() | rel.record_split(100000, '" + file_prefix + "');", test_data, "");

  rstd::string file_name_argument_list =
    "\"" + file_prefix + ::np1::str::to_hex_str_pad_16(0) + ".gz\", \""
    + file_prefix + ::np1::str::to_hex_str_pad_16(1) + ".gz\", \""
    + file_prefix + ::np1::str::to_hex_str_pad_16(2) + ".gz\", \""
    + file_prefix + ::np1::str::to_hex_str_pad_16(3) + ".gz\", \""
    + file_prefix + ::np1::str::to_hex_str_pad_16(4) + ".gz\", \""
    + file_prefix + ::np1::str::to_hex_str_pad_16(5) + ".gz\", \""
    + file_prefix + ::np1::str::to_hex_str_pad_16(6) + ".gz\", \""
    + file_prefix + ::np1::str::to_hex_str_pad_16(7) + ".gz\", \""
    + file_prefix + ::np1::str::to_hex_str_pad_16(8) + ".gz\", \""
    + file_prefix + ::np1::str::to_hex_str_pad_16(9) + ".gz\"";


  // Run the multiple-native-file-read test.
  run_script(
    "io.file.read(" + file_name_argument_list + ") | rel.where(!str.starts_with(mul1_str, 'a')) | rel.where(mul1_int % 7 = 0) | rel.select('a' as dummy) | rel.group(count) | rel.to_tsv();",

    "",

    "string:dummy\tuint:_count\na\t142858\n");
  

  //TODO: test reading non-native files.
}


void test_directory_list() {
  rstd::string test_root = "/tmp/np1_test_script/test_directory_list";
  rstd::string subdir_name = "test_subdir";
  rstd::string file1_name = "file1.txt";
  rstd::string file2_name = "file2.txt";
  rstd::string subdir_full_path = test_root + "/" + subdir_name;
  rstd::string file1_full_path = test_root + "/" + file1_name;
  rstd::string file2_full_path = subdir_full_path + "/" + file2_name;
  ::np1::io::file::mkdir(test_root.c_str());
  ::np1::io::file::mkdir(subdir_full_path.c_str());  

  ::np1::io::file file1;
  NP1_TEST_ASSERT(file1.create_or_open_wo_trunc(file1_full_path.c_str()));
  NP1_TEST_ASSERT(file1.write("this is a test", 14));
  file1.close();
  uint64_t file1_mtime = 0;
  NP1_TEST_ASSERT(::np1::io::file::get_mtime_usec(file1_full_path.c_str(), file1_mtime));

  ::np1::io::file file2;
  NP1_TEST_ASSERT(file2.create_or_open_wo_trunc(file2_full_path.c_str()));
  NP1_TEST_ASSERT(file2.write("hi mum", 6));
  file2.close();
  uint64_t file2_mtime = 0;
  NP1_TEST_ASSERT(::np1::io::file::get_mtime_usec(file2_full_path.c_str(), file2_mtime));

  uint64_t subdir_mtime = 0;
  uint64_t subdir_size = 0;  
  NP1_TEST_ASSERT(::np1::io::file::get_mtime_usec(subdir_full_path.c_str(), subdir_mtime));
  NP1_TEST_ASSERT(::np1::io::file::get_size(subdir_full_path.c_str(), subdir_size));

  // Non-recursive test.  
  run_script(
    "io.directory.list('" + test_root + "') | rel.order_by(file_name) | rel.to_tsv();",

    "",

    "string:directory_name\tstring:file_name\tstring:relative_path\tuint:size_bytes\tuint:mtime_usec\tbool:is_directory\n"    
    + test_root + "\t" + file1_name + "\t" + file1_full_path + "\t14\t" + ::np1::str::to_dec_str(file1_mtime) + "\tfalse\n"
    + test_root + "\t" + subdir_name + "\t" + subdir_full_path + "\t" + ::np1::str::to_dec_str(subdir_size) + "\t" + ::np1::str::to_dec_str(subdir_mtime) + "\ttrue\n");


  run_script(
    "io.ls('" + test_root + "') | rel.order_by(file_name) | rel.to_tsv();",

    "",

    "string:directory_name\tstring:file_name\tstring:relative_path\tuint:size_bytes\tuint:mtime_usec\tbool:is_directory\n"    
    + test_root + "\t" + file1_name + "\t" + file1_full_path + "\t14\t" + ::np1::str::to_dec_str(file1_mtime) + "\tfalse\n"
    + test_root + "\t" + subdir_name + "\t" + subdir_full_path + "\t" + ::np1::str::to_dec_str(subdir_size) + "\t" + ::np1::str::to_dec_str(subdir_mtime) + "\ttrue\n");


  // Recursive test.
  run_script(
    "io.directory.list_recurse('" + test_root + "') | rel.order_by(file_name) | rel.to_tsv();",

    "",

    "string:directory_name\tstring:file_name\tstring:relative_path\tuint:size_bytes\tuint:mtime_usec\tbool:is_directory\n"
    + test_root + "\t" + file1_name + "\t" + file1_full_path + "\t14\t" + ::np1::str::to_dec_str(file1_mtime) + "\tfalse\n"    
    + subdir_full_path + "\t" + file2_name + "\t" + file2_full_path + "\t6\t" + ::np1::str::to_dec_str(file2_mtime) + "\tfalse\n"    
    + test_root + "\t" + subdir_name + "\t" + subdir_full_path + "\t" + ::np1::str::to_dec_str(subdir_size) + "\t" + ::np1::str::to_dec_str(subdir_mtime) + "\ttrue\n");

  run_script(
    "io.ls_r('" + test_root + "') | rel.order_by(file_name) | rel.to_tsv();",

    "",

    "string:directory_name\tstring:file_name\tstring:relative_path\tuint:size_bytes\tuint:mtime_usec\tbool:is_directory\n"
    + test_root + "\t" + file1_name + "\t" + file1_full_path + "\t14\t" + ::np1::str::to_dec_str(file1_mtime) + "\tfalse\n"    
    + subdir_full_path + "\t" + file2_name + "\t" + file2_full_path + "\t6\t" + ::np1::str::to_dec_str(file2_mtime) + "\tfalse\n"    
    + test_root + "\t" + subdir_name + "\t" + subdir_full_path + "\t" + ::np1::str::to_dec_str(subdir_size) + "\t" + ::np1::str::to_dec_str(subdir_mtime) + "\ttrue\n");
}


void test_compound_operators() {
  rstd::string test_root = "/tmp/np1_test_script/test_compound_operators";
  rstd::string compound_op_name = "test_compound_operators_test_op";
  rstd::string compound_op_file_name = test_root + "/test_compound_operators_test_op";
  rstd::string compound_op_code = "$1($2);";  
  
  ::np1::io::file::mkdir(test_root.c_str());
  ::np1::io::file compound_op_file;
  NP1_TEST_ASSERT(compound_op_file.create_or_open_wo_trunc(compound_op_file_name.c_str()));
  NP1_TEST_ASSERT(compound_op_file.write(compound_op_code.c_str(), compound_op_code.length()));
  compound_op_file.close();
  
  rstd::string old_path = ::np1::environment::r17_path();
  ::np1::environment::r17_path(test_root);
  
  run_script(
    "rel.from_csv() | test_compound_operators_test_op(rel.where, name = 'fred') | rel.to_csv();",
    
    "string:name,int:value\n"
    "fred,1\n"
    "fred,2\n"
    "jane,1\n"
    "fred,3\n",
    
    "string:name,int:value\n"
    "fred,1\n"
    "fred,2\n"
    "fred,3\n"
  );
    
  ::np1::io::file::erase(compound_op_file_name.c_str());
  ::np1::environment::r17_path(old_path);
}




void test_script() {
  ::np1::io::file::mkdir(NP1_TEST_UNIT_NP1_REL_RLANG_TEST_SCRIPT_TEST_DIR);

  NP1_TEST_RUN_TEST(test_single_stream_operator);
  NP1_TEST_RUN_TEST(test_multiple_stream_operators);
  NP1_TEST_RUN_TEST(test_multiple_pipelines);
  NP1_TEST_RUN_TEST(test_inline_script);
  NP1_TEST_RUN_TEST(test_variable_record_lengths);

  NP1_TEST_RUN_TEST(test_python);
  NP1_TEST_RUN_TEST(test_r);

  //TODO: test more operators.
  NP1_TEST_RUN_TEST(test_order_by);
  NP1_TEST_RUN_TEST(test_join);
  NP1_TEST_RUN_TEST(test_group);  
  NP1_TEST_RUN_TEST(test_unique);
  NP1_TEST_RUN_TEST(test_select);
  NP1_TEST_RUN_TEST(test_record_count);
  NP1_TEST_RUN_TEST(test_record_split);
  NP1_TEST_RUN_TEST(test_str_split);
  NP1_TEST_RUN_TEST(test_from_tsv);
  NP1_TEST_RUN_TEST(test_from_csv); 
  NP1_TEST_RUN_TEST(test_from_usv_and_to_usv);
  NP1_TEST_RUN_TEST(test_from_text);
  NP1_TEST_RUN_TEST(test_from_text_ignore_non_matching); 
  NP1_TEST_RUN_TEST(test_from_shapefile);
  NP1_TEST_RUN_TEST(test_generate_sequence);
  NP1_TEST_RUN_TEST(test_utf16_to_utf8);
  NP1_TEST_RUN_TEST(test_strip_cr);
  NP1_TEST_RUN_TEST(test_remote);
  NP1_TEST_RUN_TEST(test_shell);
  NP1_TEST_RUN_TEST(test_parallel);
  NP1_TEST_RUN_TEST(test_file_read);
  NP1_TEST_RUN_TEST(test_directory_list);
  NP1_TEST_RUN_TEST(test_compound_operators);

  //TODO: add some more tests with some large data.

}

} // namespaces
}
}
}



#endif
