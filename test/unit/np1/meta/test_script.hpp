// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_TEST_UNIT_NP1_REL_RLANG_TEST_SCRIPT_HPP
#define NP1_TEST_UNIT_NP1_REL_RLANG_TEST_SCRIPT_HPP



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

  ::np1::meta::script::run_from_stream(input, output, script_file, false, "[test]");

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


struct meta_worker_f {
  void operator()(const rstd::string &listen_endpoint) {
    rstd::string script = "meta.worker('" + listen_endpoint + "');";
    run_script(script, "", "");
  }
};



// No need to supply a test data argument, just use one set of huge test data.
void run_script_distributed(const rstd::string &script, const char *expected_output) {
  write_client_peer_strings_list();
  rstd::vector<rstd::string> worker_peer_list = write_worker_peer_strings_list();

  rstd::vector<pid_t> workers = fork_distributed_workers(worker_peer_list, meta_worker_f());

  reliable_storage_type::id input_recordset_id = reliable_storage_type::id::generate();

  printf("  Creating test data, input recordset id is %s\n", input_recordset_id.to_string().c_str());
  rstd::string test_data;
  make_large_test_data_record_string(test_data);
  ::np1::io::string_input_stream input(test_data);

  printf("  Writing test data to disk and running tests\n");

  rstd::string script_with_recordset_stuff =
    "rel.from_tsv() | rel.recordset.create('" + input_recordset_id.to_string()
    + "', 1024*1024); rel.recordset.read('" + input_recordset_id.to_string() + "') | "
    + script;

  run_script(script_with_recordset_stuff, test_data, expected_output);

  kill_distributed_workers(workers);
}


void check_split_file(const rstd::string &prefix, uint64_t n, const char *expected_result) {
  rstd::string file_name = prefix + ::np1::str::to_hex_str_pad_16(n) + ".gz";
  run_script("io.file.read('" + file_name + "') | rel.to_tsv();",
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

  ::np1::meta::script::run(input, output, "rel.from_tsv() | rel.where(name = 'fred') | rel.to_tsv();", false);

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
    "fred\twilma\t-1\t1\t1.0\ttrue\t192.168.1.1\n"
    "barney\tbetty\t1\t10\t0.0\tfalse\t127.0.0.1\n",

    // Python has no "ipaddress", "istring" or "uint" types.
    "string:v1\tstring:v2\tint:v3\tint:v4\tdouble:v5\tbool:v6\tstring:v7\n"
    "fred\twilma\t-1\t1\t1.0\ttrue\t192.168.1.1\n"
    "barney\tbetty\t1\t10\t0.0\tfalse\t127.0.0.1\n"
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
  rstd::string prefix = "/tmp/np1_test_reliable_storage_local_root/test_record_split_";

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
  // header name safe-ifying and explicit header name specification.
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
  rstd::string file_prefix = "/tmp/np1_test_reliable_storage_local_root/test_parallel_";

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
  rstd::string file_prefix = "/tmp/np1_test_reliable_storage_local_root/test_file_read_";

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
  rstd::string test_root = "/tmp/np1_test_reliable_storage_local_root/test_directory_list";
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


void test_recordset_create_from_data_stream() {
  //TODO: test with a bigger dataset
  reliable_storage_type::id recordset_id = reliable_storage_type::id::generate();

  printf("recordset id is %s\n", recordset_id.to_string().c_str());

  run_script(
    "rel.from_tsv() | rel.recordset.create('" + recordset_id.to_string() + "', 10 * 1);"
      "rel.recordset.read('" + recordset_id.to_string() + "') | rel.to_tsv();",

    basic_flintstones_data(),

    basic_flintstones_data()
  );
}



void test_distributed_where() {
  run_script_distributed(
    "rel.where(!str.starts_with(mul1_str, 'a')) | rel.where(mul1_int % 7 = 0) "
    "| rel.select('a' as dummy) | rel.group(count) | rel.to_tsv();",
    "string:dummy\tuint:_count\na\t142858\n"
  );
}

void test_distributed_str_split() {
  run_script_distributed(
    "rel.str_split(mul1_str, 'name_') | rel.where(mul1_str != '') | rel.where(mul1_int % 7 = 0) "
    "| rel.select('a' as dummy) | rel.group(count) | rel.to_tsv();",
    "string:dummy\tuint:_count\na\t142858\n"
  );
}

void test_distributed_group_count() {
  run_script_distributed(
    "rel.where(mul1_int % 7 = 0) | rel.select('a' as dummy) | rel.group(count) | rel.to_tsv();",
    "string:dummy\tuint:_count\na\t142858\n"
  );
}

void test_distributed_group_min() {
  run_script_distributed(
    "rel.select(mul1_int) | rel.group(min mul1_int) | rel.to_tsv();",
    "int:mul1_int\n0\n"
  );
}

void test_distributed_group_max() {
  run_script_distributed(
    "rel.select(mul1_int) | rel.group(max mul1_int) | rel.to_tsv();",
    "int:mul1_int\n999999\n"
  );
}

void test_distributed_group_avg() {
  run_script_distributed(
    "rel.select(mul1_int) | rel.group(avg mul1_int) | rel.to_tsv();",
    "int:_avg\n499999\n"
  );
}

void test_distributed_group_sum() {
  run_script_distributed(
    "rel.select(mul1_int) | rel.group(sum mul1_int) | rel.to_tsv();",
    "int:_sum\n499999500000\n"
  );
}




void test_script() {
  ::np1::io::file::mkdir(NP1_TEST_UNIT_NP1_RELIABLE_STORAGE_LOCAL_ROOT);

  NP1_TEST_RUN_TEST(test_single_stream_operator);
  NP1_TEST_RUN_TEST(test_multiple_stream_operators);
  NP1_TEST_RUN_TEST(test_multiple_pipelines);
  NP1_TEST_RUN_TEST(test_inline_script);
  NP1_TEST_RUN_TEST(test_variable_record_lengths);

  NP1_TEST_RUN_TEST(test_python);

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
  NP1_TEST_RUN_TEST(test_generate_sequence);
  NP1_TEST_RUN_TEST(test_utf16_to_utf8);
  NP1_TEST_RUN_TEST(test_strip_cr);
  NP1_TEST_RUN_TEST(test_remote);
  NP1_TEST_RUN_TEST(test_shell);
  NP1_TEST_RUN_TEST(test_parallel);
  NP1_TEST_RUN_TEST(test_file_read);
  NP1_TEST_RUN_TEST(test_directory_list);

  NP1_TEST_RUN_TEST(test_recordset_create_from_data_stream);
  //TODO: test recordset create from recordset stream.

  NP1_TEST_RUN_TEST(test_distributed_where);
  NP1_TEST_RUN_TEST(test_distributed_str_split);
  NP1_TEST_RUN_TEST(test_distributed_group_count);
  NP1_TEST_RUN_TEST(test_distributed_group_min);
  NP1_TEST_RUN_TEST(test_distributed_group_max);
  NP1_TEST_RUN_TEST(test_distributed_group_avg);
  NP1_TEST_RUN_TEST(test_distributed_group_sum);                                    

  //TODO: add some more tests with some large data.

}

} // namespaces
}
}
}



#endif
