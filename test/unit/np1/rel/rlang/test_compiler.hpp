// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_TEST_UNIT_NP1_REL_RLANG_TEST_COMPILER_HPP
#define NP1_TEST_UNIT_NP1_REL_RLANG_TEST_COMPILER_HPP



namespace test {
namespace unit {
namespace np1 {
namespace rel {
namespace rlang {


typedef ::np1::rel::rlang::token token_type;
typedef ::np1::rel::rlang::dt::data_type data_type;


template <typename Expected_Result>
void check_vm_info(compiler_type::vm_info &vm_info,
                    const record_type &record_values,
                    const record_type &other_record_values,
                    dt::data_type expected_typed_heading_type,
                    const char *expected_typed_heading_name,
                    dt::data_type expected_vm_return_type,
                    bool expected_refers_to_other_record,
                    Expected_Result expected_result) {
  rstd::string heading_name = vm_info.get_typed_heading_name();

  NP1_TEST_ASSERT(
    dt::mandatory_from_string(
    ::np1::rel::detail::helper::mandatory_get_heading_type_tag(heading_name))
        == expected_typed_heading_type);

  NP1_TEST_ASSERT(
    ::np1::str::cmp(
      ::np1::rel::detail::helper::get_heading_without_type_tag(heading_name),
      expected_typed_heading_name) == 0);

  NP1_TEST_ASSERT(vm_info.get_vm().return_type() == expected_vm_return_type);
  NP1_TEST_ASSERT(vm_info.get_vm().refers_to_other_record() == expected_refers_to_other_record);

  run_and_check_return_type(vm_info.get_vm(), record_values, other_record_values,
                            expected_result);    
}


template <typename Expected>
void execute_single_shot_test(const char *script,
                              const record_type &this_headings,
                              const record_type &other_headings,
                              const record_type &this_r,
                              const record_type &other_r,
                              bool expected_refers_to_other_record,
                              Expected expected_value) {
  NP1_TEST_UNIT_REL_RLANG_DEFINE_INPUT_STREAM(input, script);

  vm_type vm = compiler_type::compile_single_expression(
                  input, this_headings.ref(), other_headings.ref());

  NP1_TEST_ASSERT(vm.refers_to_other_record() == expected_refers_to_other_record);
  run_and_check_return_type(vm, this_r, other_r, expected_value);  
}

template <typename Expected>
void execute_single_shot_test(const char *script,
                              const record_type &this_headings,
                              const record_type &this_r,
                              Expected expected_value) {
  record_type empty_record;
  execute_single_shot_test(script, this_headings, empty_record, this_r,
                            empty_record, false, expected_value);
}


template <typename Expected>
void execute_single_shot_test_no_records(const char *script,
                                          Expected expected_value) {
  record_type empty_record;
  execute_single_shot_test(script, empty_record, empty_record, empty_record,
                            empty_record, false, expected_value);
}


void execute_boolean_test(const char *script,
                          const char *heading_name,
                          const char *value,
                          bool expected_result) {
  record_type headings(heading_name, 0);
  record_type value_record(value, 1);
  execute_single_shot_test(script, headings, value_record, expected_result);
}



void execute_string_function_test(const char *script,
                                    const char *heading_name,
                                    const char *value,
                                    const char *expected_result) {
  record_type headings(heading_name, 0);
  record_type value_record(value, 1);
  execute_single_shot_test(script, headings, value_record, expected_result);
}

template <typename T>
void execute_arithmetic_test(const char *script,
                              const char *heading_name,
                              const char *value,
                              T expected_result) {
  record_type headings(heading_name, 0);
  record_type value_record(value, 1);
  execute_single_shot_test(script, headings, value_record, expected_result);
}





void test_single_integer_value() {
  execute_single_shot_test_no_records("2", 2);
}

void test_single_double_value() {
  execute_single_shot_test_no_records("2.0", 2.0);
  execute_single_shot_test_no_records("4.0", 4.0);
}

void test_simple_arithmetic_operation_no_records() {
  execute_single_shot_test_no_records("2+5", 7);
}


void test_all_arithmetic_operations_no_records() {
  execute_single_shot_test_no_records("1%10+2+3*4/2--1", 10);
}

void test_simple_bitwise_operation_no_records() {
  execute_single_shot_test_no_records("6 & 2", 2);
}

void test_all_bitwise_operations_no_records() {
  execute_single_shot_test_no_records("~((13 & 5) | 1)", -6);
}

void test_record_value_get() {
  record_type headings("int:interesting", "int:inter", 0);
  record_type this_value("2", "5", 1);
  record_type other_value("3", "6", 1);

  execute_single_shot_test("interesting", headings, this_value, 2);
  execute_single_shot_test("this.interesting", headings, this_value, 2);
  execute_single_shot_test("interesting", headings, headings, this_value, other_value, false, 2);
  execute_single_shot_test("this.interesting", headings, headings, this_value, other_value, false, 2);
  execute_single_shot_test("other.interesting", headings, headings, this_value, other_value, true, 3);
  
  execute_single_shot_test("inter", headings, headings, this_value, other_value, false, 5);
}


void test_rownum() {
  record_type headings("int:interesting", 0);
  record_type this_value("2", 1);
  record_type other_value("3", 2);

  execute_single_shot_test("_rownum", headings, this_value, 1);
  execute_single_shot_test("this._rownum", headings, this_value, 1);
  execute_single_shot_test("other._rownum", headings, headings, this_value, other_value, true, 2);
}


void test_eq() {
  execute_boolean_test("interesting = \"fred\"", "string:interesting", "fred", true);
  execute_boolean_test("interesting = \"fred\"", "string:interesting", "Fred", false);

  execute_boolean_test("interesting = \"fred\"", "istring:interesting", "fred", true);
  execute_boolean_test("interesting = \"fred\"", "istring:interesting", "Fred", true);
  execute_boolean_test("interesting = \"fred\"", "istring:interesting", "jane", false);

  execute_boolean_test("interesting = 4", "int:interesting", "4", true);
  execute_boolean_test("interesting = 5", "int:interesting", "4", false);

  //TODO: uint-specific comparison tests.
  execute_boolean_test("interesting = 4U", "uint:interesting", "4", true);
  execute_boolean_test("interesting = 5U", "uint:interesting", "4", false);

  execute_boolean_test("interesting = true", "bool:interesting", "true", true);
  execute_boolean_test("interesting = false", "bool:interesting", "true", false);

  execute_boolean_test("interesting = \"192.168.1.23\"", "ipaddress:interesting", "192.168.1.23", true);
  execute_boolean_test("interesting = \"192.168.1.23\"", "ipaddress:interesting", "192.168.1.24", false);
}


void test_ne() {
  execute_boolean_test("interesting != \"fred\"", "string:interesting", "fred", false);
  execute_boolean_test("interesting != \"fred\"", "string:interesting", "Fred", true);
  execute_boolean_test("interesting <> \"fred\"", "string:interesting", "Fred", true);

  execute_boolean_test("interesting != \"fred\"", "istring:interesting", "fred", false);
  execute_boolean_test("interesting != \"fred\"", "istring:interesting", "Fred", false);
  execute_boolean_test("interesting != \"fred\"", "istring:interesting", "jane", true);

  execute_boolean_test("interesting != 4", "int:interesting", "4", false);
  execute_boolean_test("interesting != 5", "int:interesting", "4", true);

  //TODO: uint-specific comparison tests.
  execute_boolean_test("interesting != 4U", "uint:interesting", "4", false);
  execute_boolean_test("interesting != 5U", "uint:interesting", "4", true);

  execute_boolean_test("interesting != true", "bool:interesting", "true", false);
  execute_boolean_test("interesting != false", "bool:interesting", "true", true);

  execute_boolean_test("interesting != \"192.168.1.23\"", "ipaddress:interesting", "192.168.1.23", false);
  execute_boolean_test("interesting != \"192.168.1.23\"", "ipaddress:interesting", "192.168.1.24", true);
}


void test_lt() {
  execute_boolean_test("interesting < \"fred\"", "string:interesting", "fred", false);
  execute_boolean_test("interesting < \"fred\"", "string:interesting", "Fred", true);
  execute_boolean_test("interesting < \"alphonse\"", "string:interesting", "fred", false);
  execute_boolean_test("interesting < \"alphonse\"", "string:interesting", "Fred", true);
  execute_boolean_test("interesting < \"zog\"", "string:interesting", "fred", true);
  execute_boolean_test("interesting < \"zog\"", "string:interesting", "Fred", true);

  execute_boolean_test("interesting < \"fred\"", "istring:interesting", "fred", false);
  execute_boolean_test("interesting < \"fred\"", "istring:interesting", "Fred", false);
  execute_boolean_test("interesting < \"alphonse\"", "istring:interesting", "fred", false);
  execute_boolean_test("interesting < \"alphonse\"", "istring:interesting", "Fred", false);
  execute_boolean_test("interesting < \"zog\"", "istring:interesting", "fred", true);
  execute_boolean_test("interesting < \"zog\"", "istring:interesting", "Fred", true);

  execute_boolean_test("interesting < 4", "int:interesting", "4", false);
  execute_boolean_test("interesting < 3", "int:interesting", "4", false);
  execute_boolean_test("interesting < 5", "int:interesting", "4", true);

  //TODO: uint-specific comparison tests.
  execute_boolean_test("interesting < 4U", "uint:interesting", "4", false);
  execute_boolean_test("interesting < 3U", "uint:interesting", "4", false);
  execute_boolean_test("interesting < 5U", "uint:interesting", "4", true);
  
  execute_boolean_test("interesting < 4.0", "double:interesting", "4", false);
  execute_boolean_test("interesting < 3.0", "double:interesting", "4", false);
  execute_boolean_test("interesting < 5.0", "double:interesting", "4", true);

  execute_boolean_test("interesting < true", "bool:interesting", "true", false);
  execute_boolean_test("interesting < false", "bool:interesting", "true", false);
  execute_boolean_test("interesting < true", "bool:interesting", "false", true);

  execute_boolean_test("interesting < \"192.168.1.23\"", "ipaddress:interesting", "192.168.1.23", false);
  execute_boolean_test("interesting < \"192.168.1.23\"", "ipaddress:interesting", "192.168.1.24", false);
  execute_boolean_test("interesting < \"192.168.1.23\"", "ipaddress:interesting", "192.168.1.22", true);
}



void test_gt() {
  execute_boolean_test("interesting > \"fred\"", "string:interesting", "fred", false);
  execute_boolean_test("interesting > \"fred\"", "string:interesting", "Fred", false);
  execute_boolean_test("interesting > \"alphonse\"", "string:interesting", "fred", true);
  execute_boolean_test("interesting > \"alphonse\"", "string:interesting", "Fred", false);
  execute_boolean_test("interesting > \"zog\"", "string:interesting", "fred", false);
  execute_boolean_test("interesting > \"zog\"", "string:interesting", "Fred", false);

  execute_boolean_test("interesting > \"fred\"", "istring:interesting", "fred", false);
  execute_boolean_test("interesting > \"fred\"", "istring:interesting", "Fred", false);
  execute_boolean_test("interesting > \"alphonse\"", "istring:interesting", "fred", true);
  execute_boolean_test("interesting > \"alphonse\"", "istring:interesting", "Fred", true);
  execute_boolean_test("interesting > \"zog\"", "istring:interesting", "fred", false);
  execute_boolean_test("interesting > \"zog\"", "istring:interesting", "Fred", false);

  execute_boolean_test("interesting > 4", "int:interesting", "4", false);
  execute_boolean_test("interesting > 3", "int:interesting", "4", true);
  execute_boolean_test("interesting > 5", "int:interesting", "4", false);

  execute_boolean_test("interesting > 4.0", "double:interesting", "4", false);
  execute_boolean_test("interesting > 3.0", "double:interesting", "4", true);
  execute_boolean_test("interesting > 5.0", "double:interesting", "4", false);

  //TODO: uint-specific comparison tests.
  execute_boolean_test("interesting > 4U", "uint:interesting", "4", false);
  execute_boolean_test("interesting > 3U", "uint:interesting", "4", true);
  execute_boolean_test("interesting > 5U", "uint:interesting", "4", false);

  execute_boolean_test("interesting > true", "bool:interesting", "true", false);
  execute_boolean_test("interesting > false", "bool:interesting", "true", true);
  execute_boolean_test("interesting > true", "bool:interesting", "false", false);

  execute_boolean_test("interesting > \"192.168.1.23\"", "ipaddress:interesting", "192.168.1.23", false);
  execute_boolean_test("interesting > \"192.168.1.23\"", "ipaddress:interesting", "192.168.1.24", true);
  execute_boolean_test("interesting > \"192.168.1.23\"", "ipaddress:interesting", "192.168.1.22", false);
}


void test_le() {
  execute_boolean_test("interesting <= \"fred\"", "string:interesting", "fred", true);
  execute_boolean_test("interesting <= \"fred\"", "string:interesting", "Fred", true);
  execute_boolean_test("interesting <= \"alphonse\"", "string:interesting", "fred", false);
  execute_boolean_test("interesting <= \"alphonse\"", "string:interesting", "Fred", true);
  execute_boolean_test("interesting <= \"zog\"", "string:interesting", "fred", true);
  execute_boolean_test("interesting <= \"zog\"", "string:interesting", "Fred", true);

  execute_boolean_test("interesting <= \"fred\"", "istring:interesting", "fred", true);
  execute_boolean_test("interesting <= \"fred\"", "istring:interesting", "Fred", true);
  execute_boolean_test("interesting <= \"alphonse\"", "istring:interesting", "fred", false);
  execute_boolean_test("interesting <= \"alphonse\"", "istring:interesting", "Fred", false);
  execute_boolean_test("interesting <= \"zog\"", "istring:interesting", "fred", true);
  execute_boolean_test("interesting <= \"zog\"", "istring:interesting", "Fred", true);

  execute_boolean_test("interesting <= 4", "int:interesting", "4", true);
  execute_boolean_test("interesting <= 3", "int:interesting", "4", false);
  execute_boolean_test("interesting <= 5", "int:interesting", "4", true);

  //TODO: uint-specific comparison tests.
  execute_boolean_test("interesting <= 4U", "uint:interesting", "4", true);
  execute_boolean_test("interesting <= 3U", "uint:interesting", "4", false);
  execute_boolean_test("interesting <= 5U", "uint:interesting", "4", true);

  execute_boolean_test("interesting <= 4.0", "double:interesting", "4", true);
  execute_boolean_test("interesting <= 3.0", "double:interesting", "4", false);
  execute_boolean_test("interesting <= 5.0", "double:interesting", "4", true);

  execute_boolean_test("interesting <= true", "bool:interesting", "true", true);
  execute_boolean_test("interesting <= false", "bool:interesting", "true", false);
  execute_boolean_test("interesting <= true", "bool:interesting", "false", true);

  execute_boolean_test("interesting <= \"192.168.1.23\"", "ipaddress:interesting", "192.168.1.23", true);
  execute_boolean_test("interesting <= \"192.168.1.23\"", "ipaddress:interesting", "192.168.1.24", false);
  execute_boolean_test("interesting <= \"192.168.1.23\"", "ipaddress:interesting", "192.168.1.22", true);
}


void test_ge() {
  execute_boolean_test("interesting >= \"fred\"", "string:interesting", "fred", true);
  execute_boolean_test("interesting >= \"fred\"", "string:interesting", "Fred", false);
  execute_boolean_test("interesting >= \"alphonse\"", "string:interesting", "fred", true);
  execute_boolean_test("interesting >= \"alphonse\"", "string:interesting", "Fred", false);
  execute_boolean_test("interesting >= \"zog\"", "string:interesting", "fred", false);
  execute_boolean_test("interesting >= \"zog\"", "string:interesting", "Fred", false);

  execute_boolean_test("interesting >= \"fred\"", "istring:interesting", "fred", true);
  execute_boolean_test("interesting >= \"fred\"", "istring:interesting", "Fred", true);
  execute_boolean_test("interesting >= \"alphonse\"", "istring:interesting", "fred", true);
  execute_boolean_test("interesting >= \"alphonse\"", "istring:interesting", "Fred", true);
  execute_boolean_test("interesting >= \"zog\"", "istring:interesting", "fred", false);
  execute_boolean_test("interesting >= \"zog\"", "istring:interesting", "Fred", false);

  execute_boolean_test("interesting >= 4", "int:interesting", "4", true);
  execute_boolean_test("interesting >= 3", "int:interesting", "4", true);
  execute_boolean_test("interesting >= 5", "int:interesting", "4", false);

  //TODO: uint-specific comparison tests.
  execute_boolean_test("interesting >= 4U", "uint:interesting", "4", true);
  execute_boolean_test("interesting >= 3U", "uint:interesting", "4", true);
  execute_boolean_test("interesting >= 5U", "uint:interesting", "4", false);

  execute_boolean_test("interesting >= 4.0", "double:interesting", "4", true);
  execute_boolean_test("interesting >= 3.0", "double:interesting", "4", true);
  execute_boolean_test("interesting >= 5.0", "double:interesting", "4", false);

  execute_boolean_test("interesting >= true", "bool:interesting", "true", true);
  execute_boolean_test("interesting >= false", "bool:interesting", "true", true);
  execute_boolean_test("interesting >= true", "bool:interesting", "false", false);

  execute_boolean_test("interesting >= \"192.168.1.23\"", "ipaddress:interesting", "192.168.1.23", true);
  execute_boolean_test("interesting >= \"192.168.1.23\"", "ipaddress:interesting", "192.168.1.24", true);
  execute_boolean_test("interesting >= \"192.168.1.23\"", "ipaddress:interesting", "192.168.1.22", false);
}


void test_string_plus() {
  execute_string_function_test("\"fred\" + interesting", "string:interesting", " and jane", "fred and jane");
  execute_string_function_test("\"fred\" + interesting", "istring:interesting", " and jane", "fred and jane");
}

void test_plus() {
  execute_arithmetic_test("4 + interesting", "int:interesting", "5", 9);
  execute_arithmetic_test("4 + -interesting", "int:interesting", "5", -1);

  //TODO: uint-specific tests.
  execute_arithmetic_test("4U + interesting", "uint:interesting", "5", 9);
  
  execute_arithmetic_test("4.0 + interesting", "double:interesting", "5.0", 9.0);
  execute_arithmetic_test("4.0 + -interesting", "double:interesting", "5.0", -1.0);
}

void test_minus() {
  execute_arithmetic_test("4 - interesting", "int:interesting", "5", -1);
  execute_arithmetic_test("4 - -interesting", "int:interesting", "5", 9);

  //TODO: more uint-specific tests.
  execute_arithmetic_test("4U - interesting", "uint:interesting", "5", -1);
  execute_arithmetic_test("time.usec_to_sec(1000000U) - interesting", "uint:interesting", "1", 0);

  execute_arithmetic_test("4.0 - interesting", "double:interesting", "5.0", -1.0);
  execute_arithmetic_test("4.0 - -interesting", "double:interesting", "5.0", 9.0);
}


void test_multiply() {
  execute_arithmetic_test("4 * interesting", "int:interesting", "5", 20);
  execute_arithmetic_test("4 * -interesting", "int:interesting", "5", -20);

  //TODO: uint-specific tests.
  execute_arithmetic_test("4U * interesting", "uint:interesting", "5", 20);  

  execute_arithmetic_test("4.0 * interesting", "double:interesting", "5.0", 20.0);
  execute_arithmetic_test("4.0 * -interesting", "double:interesting", "5.0", -20.0);
}

void test_divide() {
  execute_arithmetic_test("4 / interesting", "int:interesting", "2", 2);
  execute_arithmetic_test("4 / -interesting", "int:interesting", "2", -2);

  //TODO: uint-specific tests.
  execute_arithmetic_test("4U / interesting", "uint:interesting", "2", 2);  

  execute_arithmetic_test("4.0 / interesting", "double:interesting", "2.0", 2.0);
  execute_arithmetic_test("4.0 / -interesting", "double:interesting", "2.0", -2.0);
}

void test_mod() {
  execute_arithmetic_test("4 % interesting", "int:interesting", "3", 1);
  execute_arithmetic_test("4 % -interesting", "int:interesting", "3", 1);
  execute_arithmetic_test("-4 % -interesting", "int:interesting", "3", -1);

  //TODO: uint-specific tests.
  execute_arithmetic_test("4U / interesting", "uint:interesting", "3", 1);  
}


void test_bitwise_and() {
  execute_arithmetic_test("4 & interesting", "int:interesting", "2", 0);
  execute_arithmetic_test("4 & -interesting", "int:interesting", "2", 4);

  //TODO: uint-specific tests.
  execute_arithmetic_test("4U & interesting", "uint:interesting", "2", 0);  
}

void test_bitwise_or() {
  execute_arithmetic_test("4 | interesting", "int:interesting", "2", 6);
  execute_arithmetic_test("4 | -interesting", "int:interesting", "2", -2);

  //TODO: uint-specific tests.
  execute_arithmetic_test("4U | interesting", "uint:interesting", "2", 6);  
}


void test_bitwise_not() {
  execute_arithmetic_test("~interesting", "int:interesting", "2", -3);
  execute_arithmetic_test("~-interesting", "int:interesting", "2", 1);

  //TODO: uint-specific tests.
  execute_arithmetic_test("~interesting", "uint:interesting", "2", -3);  
}



void test_logical_and() {
  execute_boolean_test("true && interesting", "bool:interesting", "true", true);
  execute_boolean_test("true and interesting", "bool:interesting", "true", true);
  execute_boolean_test("true && !interesting", "bool:interesting", "true", false);
  execute_boolean_test("true && interesting", "bool:interesting", "false", false);
  execute_boolean_test("!true && interesting", "bool:interesting", "false", false);
}

void test_logical_or() {
  execute_boolean_test("true || interesting", "bool:interesting", "true", true);
  execute_boolean_test("true or interesting", "bool:interesting", "true", true);
  execute_boolean_test("true || !interesting", "bool:interesting", "true", true);
  execute_boolean_test("true || interesting", "bool:interesting", "false", true);
  execute_boolean_test("!true || interesting", "bool:interesting", "true", true);
  execute_boolean_test("!true || interesting", "bool:interesting", "false", false);
}


void test_logical_not() {
  execute_boolean_test("!interesting", "bool:interesting", "true", false);
  execute_boolean_test("!interesting", "bool:interesting", "false", true);
}


void test_if() {
  execute_arithmetic_test("if (interesting = 6) then (7) else (8)", "int:interesting", "6", 7);
  execute_arithmetic_test("10 + (if (interesting = 6) then (7) else (8))", "int:interesting", "6", 17);
  execute_arithmetic_test("if (interesting = 6) then (7) else (8)", "int:interesting", "10", 8);
  execute_arithmetic_test("if (if (interesting = 6) then (true) else (false)) then (7) else (8)", "int:interesting", "6", 7);
  execute_arithmetic_test("if (interesting = 7) then (-1) else (if (interesting = 6) then (9) else (10))", "int:interesting", "6", 9);
  execute_arithmetic_test("if (interesting = 7) then (-1) else (if (interesting = 7) then (9) else (10))", "int:interesting", "6", 10);
  execute_arithmetic_test("if (interesting = 6) then (-1) else (if (interesting = 7) then (9) else (10))", "int:interesting", "6", -1);
  execute_arithmetic_test("10 + (if (interesting = 6) then (-1) else (if (interesting = 7) then (9) else (10)))", "int:interesting", "6", 9);
}

void test_to_string() {
  execute_string_function_test("to_string(interesting)", "string:interesting", "fred", "fred");
  execute_string_function_test("to_string(interesting)", "istring:interesting", "fred", "fred");
  execute_string_function_test("to_string(interesting)", "int:interesting", "123", "123");
  execute_string_function_test("to_string(interesting)", "uint:interesting", "123", "123");
  execute_string_function_test("to_string(interesting)", "double:interesting", "123.0", "123.000000");
  execute_string_function_test("to_string(interesting)", "bool:interesting", "false", "false");
  execute_string_function_test("to_string(interesting)", "bool:interesting", "true", "true");
  execute_string_function_test("to_string(interesting)", "ipaddress:interesting", "192.168.1.1", "192.168.1.1");
}

void test_to_istring() {
  execute_string_function_test("to_istring(interesting)", "string:interesting", "fred", "fred");
  execute_string_function_test("to_istring(interesting)", "istring:interesting", "fred", "fred");
  execute_string_function_test("to_istring(interesting)", "int:interesting", "123", "123");
  execute_string_function_test("to_istring(interesting)", "uint:interesting", "123", "123");
  execute_string_function_test("to_istring(interesting)", "double:interesting", "123.", "123.000000");
  execute_string_function_test("to_istring(interesting)", "bool:interesting", "false", "false");
  execute_string_function_test("to_istring(interesting)", "bool:interesting", "true", "true");
  execute_string_function_test("to_istring(interesting)", "ipaddress:interesting", "192.168.1.1", "192.168.1.1");
}

void test_str_to_upper_case() {
  execute_string_function_test("str.to_upper_case(interesting)", "string:interesting", "Fred", "FRED");
  execute_string_function_test("str.to_upper_case(interesting)", "istring:interesting", "freD", "FRED");
}

void test_str_to_lower_case() {
  execute_string_function_test("str.to_lower_case(interesting)", "string:interesting", "Fred", "fred");
  execute_string_function_test("str.to_lower_case(interesting)", "istring:interesting", "freD", "fred");
}


void test_str_regex_match() {
  //TODO: many more tests.  Is there standard tests for regexes?
  execute_boolean_test("str.regex_match(\".*fred.*\", interesting)", "string:interesting", "fred and jane", true);
  execute_boolean_test("str.regex_match(\"jane\", interesting)", "string:interesting", "fred and jane", true);
  execute_boolean_test("str.regex_match(\"chickens\", interesting)", "string:interesting", "fred and jane", false);
  execute_boolean_test("str.regex_match(\"fred9.*\", interesting)", "string:interesting", "fred9a", true);
  execute_boolean_test("str.regex_match(\"fred[1-3]\", interesting)", "string:interesting", "fred3", true);
  execute_boolean_test("str.regex_match(\"fred[1-3]\", interesting)", "string:interesting", "fred4", false);

  execute_boolean_test("str.regex_match(\".*fred.*\", interesting)", "istring:interesting", "Fred and jane", true);
  execute_boolean_test("str.regex_match(\"jane\", interesting)", "istring:interesting", "Fred and jane", true);
  execute_boolean_test("str.regex_match(\"chickens\", interesting)", "istring:interesting", "Fred and jane", false);
  execute_boolean_test("str.regex_match(\"fred9.*\", interesting)", "istring:interesting", "Fred9a", true);
  execute_boolean_test("str.regex_match(\"fred[1-3]\", interesting)", "istring:interesting", "Fred3", true);
  execute_boolean_test("str.regex_match(\"fred[1-3]\", interesting)", "istring:interesting", "Fred4", false);
}

void test_str_regex_replace() {
  //TODO: many more tests.  Is there standard tests for regexes?
  execute_string_function_test("str.regex_replace(\"(jane)\", interesting, \"\\1 and fred\")", "string:interesting", "fred and jane", "jane and fred");
  execute_string_function_test("str.regex_replace(\"([f][r][e][d]) and jane\", interesting, \"\\1\")", "string:interesting", "fred and jane", "fred");
  execute_string_function_test("str.regex_replace(\"(fred) and (jane)\", interesting, \"\\1 or \\2\")", "string:interesting", "fred and jane", "fred or jane");
  execute_string_function_test("str.regex_replace(\"(fred)\", interesting, \"\\1\")", "string:interesting", "wendy", "wendy");

  execute_string_function_test("str.regex_replace(\"(jane)\", interesting, \"\\1 and fred\")", "istring:interesting", "Fred and jane", "jane and fred");
  execute_string_function_test("str.regex_replace(\"([f][r][e][d]) and jane\", interesting, \"\\1\")", "istring:interesting", "fred and Jane", "fred");
  execute_string_function_test("str.regex_replace(\"(fred) and (jane)\", interesting, \"\\1 or \\2\")", "istring:interesting", "fred And jane", "fred or jane");
}

void test_str_regex_replace_empty_on_no_match() {
  //TODO: many more tests.  Is there standard tests for regexes?
  execute_string_function_test("str.regex_replace_empty_on_no_match(\"(jane)\", interesting, \"\\1 and fred\")", "string:interesting", "fred and jane", "jane and fred");
  execute_string_function_test("str.regex_replace_empty_on_no_match(\"([f][r][e][d]) and jane\", interesting, \"\\1\")", "string:interesting", "fred and jane", "fred");
  execute_string_function_test("str.regex_replace_empty_on_no_match(\"(fred) and (jane)\", interesting, \"\\1 or \\2\")", "string:interesting", "fred and jane", "fred or jane");
  execute_string_function_test("str.regex_replace_empty_on_no_match(\"(fred)\", interesting, \"\\1\")", "string:interesting", "wendy", "");

  execute_string_function_test("str.regex_replace_empty_on_no_match(\"(jane)\", interesting, \"\\1 and fred\")", "istring:interesting", "Fred and jane", "jane and fred");
  execute_string_function_test("str.regex_replace_empty_on_no_match(\"([f][r][e][d]) and jane\", interesting, \"\\1\")", "istring:interesting", "fred and Jane", "fred");
  execute_string_function_test("str.regex_replace_empty_on_no_match(\"(fred) and (jane)\", interesting, \"\\1 or \\2\")", "istring:interesting", "fred And jane", "fred or jane");
}

void test_str_regex_replace_all() {
  //TODO: many more tests.
  execute_string_function_test("str.regex_replace_all(\"(jane)\", interesting, \"\\1 and fred\")", "string:interesting", "fred and jane", "fred and jane and fred");
  execute_string_function_test("str.regex_replace_all(\"([f][r][e][d]) and jane\", interesting, \"\\1\")", "string:interesting", "fred and jane", "fred");
  execute_string_function_test("str.regex_replace_all(\"(fred) and (jane)\", interesting, \"\\1 or \\2\")", "string:interesting", "fred and jane", "fred or jane");
  execute_string_function_test("str.regex_replace_all(\"(fred)\", interesting, \"\\1\")", "string:interesting", "wendy", "wendy");

  execute_string_function_test("str.regex_replace_all(\"(jane)\", interesting, \"\\1 and fred\")", "istring:interesting", "Fred and jane", "Fred and jane and fred");
  execute_string_function_test("str.regex_replace_all(\"([f][r][e][d]) and jane\", interesting, \"\\1\")", "istring:interesting", "fred and Jane", "fred");
  execute_string_function_test("str.regex_replace_all(\"(fred) and (jane)\", interesting, \"\\1 or \\2\")", "istring:interesting", "fred And jane", "fred or jane");

  execute_string_function_test("str.regex_replace_all(\"(fred) and (jane)\", interesting, \"\\1 or \\2\")", "string:interesting", "fred and jane and fred and jane", "fred or jane and fred or jane");  
}




void test_str_starts_with() {
  execute_boolean_test("str.starts_with(interesting, \"fred\")", "string:interesting", "fred and jane", true);
  execute_boolean_test("str.starts_with(interesting, \"Fred\")", "string:interesting", "fred and jane", false);
  execute_boolean_test("str.starts_with(interesting, \"Fred\")", "istring:interesting", "fred and jane", true);
  execute_boolean_test("str.starts_with(interesting, \"fred\")", "istring:interesting", "jane and fred", false);
}

void test_str_ends_with() {
  execute_boolean_test("str.ends_with(interesting, \"jane\")", "string:interesting", "fred and jane", true);
  execute_boolean_test("str.ends_with(interesting, \"Jane\")", "string:interesting", "fred and jane", false);
  execute_boolean_test("str.ends_with(interesting, \"Jane\")", "istring:interesting", "fred and jane", true);
  execute_boolean_test("str.ends_with(interesting, \"jane\")", "istring:interesting", "jane and fred", false);
}

void test_str_contains() {
  execute_boolean_test("str.contains(interesting, \"fred\")", "string:interesting", "fred and jane", true);
  execute_boolean_test("str.contains(interesting, \"Fred\")", "string:interesting", "fred and jane", false);
  execute_boolean_test("str.contains(interesting, \"Fred\")", "istring:interesting", "fred and jane", true);
  execute_boolean_test("str.contains(interesting, \"fred\")", "istring:interesting", "jane and fred", true);

  execute_boolean_test("str.contains(interesting, \"jane\")", "string:interesting", "fred and jane", true);
  execute_boolean_test("str.contains(interesting, \"Jane\")", "string:interesting", "fred and jane", false);
  execute_boolean_test("str.contains(interesting, \"Jane\")", "istring:interesting", "fred and jane", true);
  execute_boolean_test("str.contains(interesting, \"jane\")", "istring:interesting", "jane and fred", true);

  execute_boolean_test("str.contains(interesting, \"jane and fred\")", "istring:interesting", "jane and fred", true);
  execute_boolean_test("str.contains(interesting, \"wally\")", "istring:interesting", "jane and fred", false);
}


void test_str_uuidgen() {
  NP1_TEST_UNIT_REL_RLANG_DEFINE_INPUT_STREAM(input, "str.uuidgen()");
  vm_heap_type heap;
  record_type empty;

  vm_type vm = compiler_type::compile_single_expression(
                  input, empty.ref(), empty.ref());

  NP1_TEST_ASSERT(vm.return_type() == dt::TYPE_STRING);
  vm_stack_type &stack = vm.run_heap_reset(heap, empty.ref(), empty.ref());
  dt::string actual_value;
  stack.pop(actual_value);
  
  // We've got no way of checking that the actual value is correct :).
}

void test_str_sha256() {
  execute_string_function_test("str.sha256(interesting)", "string:interesting", "hi mum", "645c91edb99b62699eff3c99508af3810dd9f78b3373c74db11cb494ea5f3227");
  execute_string_function_test("str.sha256(interesting)", "istring:interesting", "hi mum", "645c91edb99b62699eff3c99508af3810dd9f78b3373c74db11cb494ea5f3227");
  execute_string_function_test("str.sha256(interesting)", "int:interesting", "123", "4f319987a786107dc63b2b70115b3734cb9880b099b70c463c5e1b05521ab764");
  execute_string_function_test("str.sha256(interesting)", "uint:interesting", "123", "4f319987a786107dc63b2b70115b3734cb9880b099b70c463c5e1b05521ab764");
  execute_string_function_test("str.sha256(interesting)", "bool:interesting", "true", "4bf5122f344554c53bde2ebb8cd2b7e3d1600ad631c385a5d7cce23c7785459a");
  execute_string_function_test("str.sha256(interesting)", "bool:interesting", "false", "6e340b9cffb37a989ca544e6bb780a2c78901d3fb33738768511a30617afa01d");
  execute_string_function_test("str.sha256(interesting)", "ipaddress:interesting", "192.168.1.1", "c5eb5a4cc76a5cdb16e79864b9ccd26c3553f0c396d0a21bafb7be71c1efcd8c");
}

void test_math_rand64() {
  NP1_TEST_UNIT_REL_RLANG_DEFINE_INPUT_STREAM(input, "math.rand64()");
  vm_heap_type heap;
  record_type empty;

  vm_type vm = compiler_type::compile_single_expression(
                  input, empty.ref(), empty.ref());

  NP1_TEST_ASSERT(vm.return_type() == dt::TYPE_UINT);
  vm_stack_type &stack = vm.run_heap_reset(heap, empty.ref(), empty.ref());
  uint64_t actual_value;
  stack.pop(actual_value);
  
  // We've got no way of checking that the actual value is correct :).
}


void test_time_now_epoch_usec() {
  NP1_TEST_UNIT_REL_RLANG_DEFINE_INPUT_STREAM(input, "time.now_epoch_usec()");
  vm_heap_type heap;
  record_type empty;

  vm_type vm = compiler_type::compile_single_expression(
                  input, empty.ref(), empty.ref());

  NP1_TEST_ASSERT(vm.return_type() == dt::TYPE_UINT);
  vm_stack_type &stack = vm.run_heap_reset(heap, empty.ref(), empty.ref());
  uint64_t actual_value;
  stack.pop(actual_value);
  
  // We've got no way of checking that the actual value is correct :).
}

void test_time_usec_to_msec() {
  execute_arithmetic_test("time.usec_to_msec(interesting)", "int:interesting", "5", 0);
  execute_arithmetic_test("time.usec_to_msec(interesting)", "uint:interesting", "5", 0);
  execute_arithmetic_test("time.usec_to_msec(interesting)", "int:interesting", "2000", 2);
  execute_arithmetic_test("time.usec_to_msec(interesting)", "uint:interesting", "2000", 2);
}

void test_time_msec_to_usec() {
  execute_arithmetic_test("time.msec_to_usec(interesting)", "int:interesting", "5", 5000);
  execute_arithmetic_test("time.msec_to_usec(interesting)", "uint:interesting", "5", 5000);
  execute_arithmetic_test("time.msec_to_usec(interesting)", "int:interesting", "2000", 2000000);
  execute_arithmetic_test("time.msec_to_usec(interesting)", "uint:interesting", "2000", 2000000);
}


void test_time_usec_to_sec() {
  execute_arithmetic_test("time.usec_to_sec(interesting)", "int:interesting", "5", 0);
  execute_arithmetic_test("time.usec_to_sec(interesting)", "uint:interesting", "5", 0);
  execute_arithmetic_test("time.usec_to_sec(interesting)", "int:interesting", "2000000", 2);
  execute_arithmetic_test("time.usec_to_sec(interesting)", "uint:interesting", "2000000", 2);
}

void test_time_sec_to_usec() {
  execute_arithmetic_test("time.sec_to_usec(interesting)", "int:interesting", "5", 5000000);
  execute_arithmetic_test("time.sec_to_usec(interesting)", "uint:interesting", "5", 5000000);
  execute_arithmetic_test("time.sec_to_usec(interesting)", "int:interesting", "2000", 2000000000);
  execute_arithmetic_test("time.sec_to_usec(interesting)", "uint:interesting", "2000", 2000000000);
}

void test_time_extract_year() {
  execute_arithmetic_test("time.extract_year(interesting)", "int:interesting", "1287220895000000", 2010);
  execute_arithmetic_test("time.extract_year(interesting)", "uint:interesting", "1287220895000000", 2010);
}

void test_time_extract_month() {
  execute_arithmetic_test("time.extract_month(interesting)", "int:interesting", "1287220895000000", 10);
  execute_arithmetic_test("time.extract_month(interesting)", "uint:interesting", "1287220895000000", 10);
}

void test_time_extract_day() {
  execute_arithmetic_test("time.extract_day(interesting)", "int:interesting", "1287220895000000", 16);
  execute_arithmetic_test("time.extract_day(interesting)", "uint:interesting", "1287220895000000", 16);
}

void test_time_extract_hour() {
  execute_arithmetic_test("time.extract_hour(interesting)", "int:interesting", "1287220895000000", 9);
  execute_arithmetic_test("time.extract_hour(interesting)", "uint:interesting", "1287220895000000", 9);
}

void test_time_extract_minute() {
  execute_arithmetic_test("time.extract_minute(interesting)", "int:interesting", "1287220895000000", 21);
  execute_arithmetic_test("time.extract_minute(interesting)", "uint:interesting", "1287220895000000", 21);
}

void test_time_extract_second() {
  execute_arithmetic_test("time.extract_second(interesting)", "int:interesting", "1287220895000000", 35);
  execute_arithmetic_test("time.extract_second(interesting)", "uint:interesting", "1287220895000000", 35);
}

void test_time_parse() {
  execute_arithmetic_test("time.parse(interesting, '%d/%m/%Y %H:%M')", "string:interesting", "16/10/2010 10:21",  (int64_t)1287184860000000LL);
  execute_arithmetic_test("time.parse(interesting, '%d/%m/%Y %H:%M')", "istring:interesting", "16/10/2010 10:21", (int64_t)1287184860000000LL);
}

void test_time_format() {
  execute_string_function_test("time.format(interesting, '%d/%m/%Y %H:%M')", "uint:interesting", "1287184860000000", "16/10/2010 10:21");
}


void test_io_net_url_get() {
  //TODO: more servers, more URL types.
  execute_string_function_test("str.regex_replace(\".*(January 1994).*\", io.net.url.get(interesting), \"FOUND IT\")", "string:interesting", "http://www.nplus1.com.au/home.html", "FOUND IT");
}

void test_io_file_read() {
  //TODO: test with more & larger files etc.
  ::np1::io::file f;
  rstd::string test_file_name = "/tmp/np1_test_compiler_test_io_file_read.txt";
  f.create_or_open_wo_trunc(test_file_name.c_str());
  rstd::string test_string = "hi mum";
  NP1_TEST_ASSERT(f.write(test_string.c_str(), test_string.length()));
  f.close();
  
  execute_string_function_test("io.file.read(interesting)", "string:interesting", test_file_name.c_str(), test_string.c_str());
  ::np1::io::file::erase(test_file_name.c_str());
}


void test_io_file_erase() {
  ::np1::io::file f;
  rstd::string test_file_name = "/tmp/np1_test_compiler_test_io_file_erase.txt";
  f.create_or_open_wo_trunc(test_file_name.c_str());
  rstd::string test_string = "hi mum";
  NP1_TEST_ASSERT(f.write(test_string.c_str(), test_string.length()));
  f.close();
  
  execute_boolean_test("io.file.erase(interesting)", "string:interesting", test_file_name.c_str(), true);
  NP1_TEST_ASSERT(!::np1::io::file::erase(test_file_name.c_str()));

  // Check that erasing a nonexistent file fails.
  execute_boolean_test("io.file.erase(interesting)", "string:interesting", "I do not exist", false);
}


void test_meta_shell() {
  //TODO: test with larger command outputs.
  ::np1::io::file f;
  rstd::string test_file_name = "/tmp/np1_test_compiler_test_meta_shell.txt";
  f.create_or_open_wo_trunc(test_file_name.c_str());
  rstd::string test_string = "hi mum";
  NP1_TEST_ASSERT(f.write(test_string.c_str(), test_string.length()));
  f.close();
  
  execute_string_function_test("meta.shell('cat ' + interesting)", "string:interesting", test_file_name.c_str(), test_string.c_str());
  ::np1::io::file::erase(test_file_name.c_str());
}


void test_eval_to_string() {
  NP1_TEST_UNIT_REL_RLANG_DEFINE_INPUT_STREAM(input, "1+1");  
  rstd::vector<token_type> tokens;
  compiler_type::compile_single_expression_to_prefix(input, tokens);
  rstd::pair<rstd::string, data_type> result = compiler_type::eval_to_string(tokens);
  NP1_TEST_ASSERT(data_type::TYPE_INT == result.second);
  NP1_TEST_ASSERT(::np1::str::cmp(result.first, "2") == 0);  
}


void test_eval_to_string_only() {
  NP1_TEST_UNIT_REL_RLANG_DEFINE_INPUT_STREAM(input, "'fred'");  
  rstd::vector<token_type> tokens;
  compiler_type::compile_single_expression_to_prefix(input, tokens);
  rstd::string result = compiler_type::eval_to_string_only(tokens);
  NP1_TEST_ASSERT(::np1::str::cmp(result, "fred") == 0);  
}


void test_eval_to_strings() {
  NP1_TEST_UNIT_REL_RLANG_DEFINE_INPUT_STREAM(input, "1+1, 'cedric'");
  rstd::vector<token_type> tokens;
  compiler_type::compile_single_expression_to_prefix(input, tokens);
  rstd::vector<rstd::pair<rstd::string, data_type> > result = compiler_type::eval_to_strings(tokens);
  NP1_TEST_ASSERT(result.size() == 2);
  NP1_TEST_ASSERT(data_type::TYPE_INT == result[0].second);
  NP1_TEST_ASSERT(data_type::TYPE_STRING == result[1].second);
  NP1_TEST_ASSERT(::np1::str::cmp(result[0].first, "2") == 0);  
  NP1_TEST_ASSERT(::np1::str::cmp(result[1].first, "cedric") == 0);  
}


void test_eval_to_strings_only() {
  NP1_TEST_UNIT_REL_RLANG_DEFINE_INPUT_STREAM(input, "'fred', 'cedric'");
  rstd::vector<token_type> tokens;
  compiler_type::compile_single_expression_to_prefix(input, tokens);
  rstd::vector<rstd::string> result = compiler_type::eval_to_strings_only(tokens);
  NP1_TEST_ASSERT(result.size() == 2);
  NP1_TEST_ASSERT(::np1::str::cmp(result[0], "fred") == 0);  
  NP1_TEST_ASSERT(::np1::str::cmp(result[1], "cedric") == 0);  
}



void test_compile_select_no_type_tag() {
  NP1_TEST_UNIT_REL_RLANG_DEFINE_INPUT_STREAM(input, "1+2+3 as output");

  record_type empty_record;
  rstd::vector<compiler_type::vm_info> vm_infos;
  compiler_type::compile_select(input, empty_record.ref(), vm_infos);

  NP1_TEST_ASSERT(vm_infos.size() == 1);

  check_vm_info(vm_infos[0], empty_record, empty_record, dt::TYPE_INT, "output",
                dt::TYPE_INT, false, 6);

  rstd::vector<token_type> tokens;
  input.rewind();
  compiler_type::compile_single_expression_to_prefix(input, tokens);
  NP1_TEST_ASSERT(tokens.size() > 0);
  NP1_TEST_ASSERT(!compiler_type::any_references_to_other_record(tokens));  
}


void test_compile_select_with_type_tag() {
  NP1_TEST_UNIT_REL_RLANG_DEFINE_INPUT_STREAM(input, "1+2+3 as string:output");

  record_type empty_record;
  rstd::vector<compiler_type::vm_info> vm_infos;
  compiler_type::compile_select(input, empty_record.ref(), vm_infos);

  NP1_TEST_ASSERT(vm_infos.size() == 1);

  check_vm_info(vm_infos[0], empty_record, empty_record, dt::TYPE_STRING,
                "output", dt::TYPE_INT, false, 6);

  rstd::vector<token_type> tokens;
  input.rewind();
  compiler_type::compile_single_expression_to_prefix(input, tokens);
  NP1_TEST_ASSERT(tokens.size() > 0);
  NP1_TEST_ASSERT(!compiler_type::any_references_to_other_record(tokens));  
}



void test_compile_select_heading_only_no_type_tag() {
  NP1_TEST_UNIT_REL_RLANG_DEFINE_INPUT_STREAM(input, "output");

  record_type empty_record;
  record_type headings("string:output", 0);
  record_type value("fred", 1);
  rstd::vector<compiler_type::vm_info> vm_infos;
  compiler_type::compile_select(input, headings.ref(), vm_infos);

  check_vm_info(vm_infos[0], value, empty_record, dt::TYPE_STRING, "output",
                dt::TYPE_STRING, false, "fred");

  rstd::vector<token_type> tokens;
  input.rewind();
  compiler_type::compile_single_expression_to_prefix(input, tokens);
  NP1_TEST_ASSERT(tokens.size() > 0);
  NP1_TEST_ASSERT(!compiler_type::any_references_to_other_record(tokens));  
}


void test_compile_select_heading_only_with_type_tag() {
  NP1_TEST_UNIT_REL_RLANG_DEFINE_INPUT_STREAM(input, "istring:output");

  record_type empty_record;
  record_type headings("istring:output", 0);
  record_type value("fred", 1);
  rstd::vector<compiler_type::vm_info> vm_infos;
  compiler_type::compile_select(input, headings.ref(), vm_infos);

  NP1_TEST_ASSERT(vm_infos.size() == 1);

  check_vm_info(vm_infos[0], value, empty_record, dt::TYPE_ISTRING, "output",
                dt::TYPE_ISTRING, false, "fred");

  rstd::vector<token_type> tokens;
  input.rewind();
  compiler_type::compile_single_expression_to_prefix(input, tokens);
  NP1_TEST_ASSERT(tokens.size() > 0);
  NP1_TEST_ASSERT(!compiler_type::any_references_to_other_record(tokens));  
}



void test_compile_select_mixture() {
  NP1_TEST_UNIT_REL_RLANG_DEFINE_INPUT_STREAM(input, "1+2+3 as uint:num, istring:output, 4+5+6 as num2");

  record_type empty_record;
  record_type headings("istring:output", 0);
  record_type value("fred", 1);
  rstd::vector<compiler_type::vm_info> vm_infos;
  compiler_type::compile_select(input, headings.ref(), vm_infos);

  NP1_TEST_ASSERT(vm_infos.size() == 3);

  check_vm_info(vm_infos[0], value, empty_record, dt::TYPE_UINT, "num",
                dt::TYPE_INT, false, 6);

  check_vm_info(vm_infos[1], value, empty_record, dt::TYPE_ISTRING, "output",
                dt::TYPE_ISTRING, false, "fred");

  check_vm_info(vm_infos[2], value, empty_record, dt::TYPE_INT, "num2",
                dt::TYPE_INT, false, 15);

  rstd::vector<token_type> tokens;
  input.rewind();
  compiler_type::compile_single_expression_to_prefix(input, tokens);
  NP1_TEST_ASSERT(tokens.size() > 0);
  NP1_TEST_ASSERT(!compiler_type::any_references_to_other_record(tokens));  
}



void test_compile_select_mixture_with_other_record() {
  // Recall that prev is a synonym for other.
  NP1_TEST_UNIT_REL_RLANG_DEFINE_INPUT_STREAM(
    input,
    "1+2+3 as uint:num, istring:output, 4+5+6 as num2, prev.num + 1U as num3, other.num3 as num4");

  record_type headings("istring:output", 0);
  record_type value("fred", 1);
  record_type other_value("5", "", "0", "0", "0", 0);  // This will serve as the initial "prev" values.
  rstd::vector<compiler_type::vm_info> vm_infos;
  compiler_type::compile_select(input, headings.ref(), vm_infos);

  NP1_TEST_ASSERT(vm_infos.size() == 5);

  check_vm_info(vm_infos[0], value, other_value, dt::TYPE_UINT, "num",
                dt::TYPE_INT, false, 6);

  check_vm_info(vm_infos[1], value, other_value, dt::TYPE_ISTRING, "output",
                dt::TYPE_ISTRING, false, "fred");

  check_vm_info(vm_infos[2], value, other_value, dt::TYPE_INT, "num2",
                dt::TYPE_INT, false, 15);

  check_vm_info(vm_infos[3], value, other_value, dt::TYPE_UINT, "num3",
                dt::TYPE_UINT, true, 6);

  check_vm_info(vm_infos[4], value, other_value, dt::TYPE_UINT, "num4",
                dt::TYPE_UINT, true, 0);

  rstd::vector<token_type> tokens;
  input.rewind();
  compiler_type::compile_single_expression_to_prefix(input, tokens);
  NP1_TEST_ASSERT(tokens.size() > 0);
  NP1_TEST_ASSERT(compiler_type::any_references_to_other_record(tokens));  
}



void test_compiler() {
  // Single expression testing.
  // Test a few recordless operations just to get warmed up.
  NP1_TEST_RUN_TEST(test_single_integer_value);
  NP1_TEST_RUN_TEST(test_single_double_value);
  NP1_TEST_RUN_TEST(test_simple_arithmetic_operation_no_records);
  NP1_TEST_RUN_TEST(test_all_arithmetic_operations_no_records);
  NP1_TEST_RUN_TEST(test_simple_bitwise_operation_no_records);
  NP1_TEST_RUN_TEST(test_all_bitwise_operations_no_records);

  // Test that we can get a value out of a record
  NP1_TEST_RUN_TEST(test_record_value_get);
  NP1_TEST_RUN_TEST(test_rownum);

  // Now do a "proper" set of tests.
  NP1_TEST_RUN_TEST(test_eq);
  NP1_TEST_RUN_TEST(test_ne);
  NP1_TEST_RUN_TEST(test_lt);
  NP1_TEST_RUN_TEST(test_gt);
  NP1_TEST_RUN_TEST(test_le);
  NP1_TEST_RUN_TEST(test_ge); 

  NP1_TEST_RUN_TEST(test_string_plus);

  NP1_TEST_RUN_TEST(test_plus);
  NP1_TEST_RUN_TEST(test_minus);
  NP1_TEST_RUN_TEST(test_multiply);
  NP1_TEST_RUN_TEST(test_divide);
  NP1_TEST_RUN_TEST(test_mod);

  NP1_TEST_RUN_TEST(test_bitwise_and);
  NP1_TEST_RUN_TEST(test_bitwise_or);
  NP1_TEST_RUN_TEST(test_bitwise_not);

  NP1_TEST_RUN_TEST(test_logical_and);
  NP1_TEST_RUN_TEST(test_logical_or);
  NP1_TEST_RUN_TEST(test_logical_not);

  NP1_TEST_RUN_TEST(test_if);

  NP1_TEST_RUN_TEST(test_to_string);
  NP1_TEST_RUN_TEST(test_to_istring);

  NP1_TEST_RUN_TEST(test_str_to_upper_case);
  NP1_TEST_RUN_TEST(test_str_to_lower_case);

  NP1_TEST_RUN_TEST(test_str_regex_match);
  NP1_TEST_RUN_TEST(test_str_regex_replace);
  NP1_TEST_RUN_TEST(test_str_regex_replace_empty_on_no_match);
  NP1_TEST_RUN_TEST(test_str_regex_replace_all);

  NP1_TEST_RUN_TEST(test_str_starts_with);
  NP1_TEST_RUN_TEST(test_str_ends_with);
  NP1_TEST_RUN_TEST(test_str_contains);

  NP1_TEST_RUN_TEST(test_str_uuidgen);
  NP1_TEST_RUN_TEST(test_str_sha256);

  NP1_TEST_RUN_TEST(test_math_rand64);

  NP1_TEST_RUN_TEST(test_time_now_epoch_usec);
  NP1_TEST_RUN_TEST(test_time_usec_to_msec);
  NP1_TEST_RUN_TEST(test_time_msec_to_usec);
  NP1_TEST_RUN_TEST(test_time_usec_to_sec);
  NP1_TEST_RUN_TEST(test_time_sec_to_usec);
  NP1_TEST_RUN_TEST(test_time_extract_year);
  NP1_TEST_RUN_TEST(test_time_extract_month);
  NP1_TEST_RUN_TEST(test_time_extract_day);
  NP1_TEST_RUN_TEST(test_time_extract_hour);
  NP1_TEST_RUN_TEST(test_time_extract_minute);
  NP1_TEST_RUN_TEST(test_time_extract_second);
  NP1_TEST_RUN_TEST(test_time_parse);
  NP1_TEST_RUN_TEST(test_time_format);

  NP1_TEST_RUN_TEST(test_io_net_url_get);
  NP1_TEST_RUN_TEST(test_io_file_read);
  NP1_TEST_RUN_TEST(test_io_file_erase);
  NP1_TEST_RUN_TEST(test_meta_shell);

  //TODO: more permutations

  // Evaluate expression testing.
  NP1_TEST_RUN_TEST(test_eval_to_string);
  NP1_TEST_RUN_TEST(test_eval_to_string_only);
  NP1_TEST_RUN_TEST(test_eval_to_strings);
  NP1_TEST_RUN_TEST(test_eval_to_strings_only);

  // Select expression testing.
  NP1_TEST_RUN_TEST(test_compile_select_no_type_tag);
  NP1_TEST_RUN_TEST(test_compile_select_with_type_tag);
  NP1_TEST_RUN_TEST(test_compile_select_heading_only_no_type_tag);
  NP1_TEST_RUN_TEST(test_compile_select_heading_only_with_type_tag);
  NP1_TEST_RUN_TEST(test_compile_select_mixture);
  NP1_TEST_RUN_TEST(test_compile_select_mixture_with_other_record); 
}

} // namespaces
}
}
}
}


#endif
