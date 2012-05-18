// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_TEST_UNIT_NP1_REL_RLANG_TEST_SHUNTING_YARD_HPP
#define NP1_TEST_UNIT_NP1_REL_RLANG_TEST_SHUNTING_YARD_HPP



namespace test {
namespace unit {
namespace np1 {
namespace rel {
namespace rlang {

void check_token_is(const parsed_token_info_type &parsed_token,
                    token_type::type_type type,
                    const char *value,
                    size_t number_arguments) {
  NP1_TEST_ASSERT(parsed_token.type() == type);
  NP1_TEST_ASSERT(strcmp(parsed_token.text(), value) == 0);
  if (number_arguments != (size_t)-1) {
    NP1_TEST_ASSERT(parsed_token.function_arg_count() == number_arguments);
  }
}


void test_single_expression() {
  NP1_TEST_UNIT_REL_RLANG_DEFINE_INPUT_TOKEN_VECTOR(input, "2");
  std::vector<parsed_token_info_type> postfix;

  shunting_yard_type::parse<fn_table_type>(input, postfix);

  NP1_TEST_ASSERT(postfix.size() == 1);
  check_token_is(postfix[0], token_type::TYPE_INT, "2", -1);
}

void test_binary_operation() {
  NP1_TEST_UNIT_REL_RLANG_DEFINE_INPUT_TOKEN_VECTOR(input, "this.fred+2");
  std::vector<parsed_token_info_type> postfix;

  shunting_yard_type::parse<fn_table_type>(input, postfix);

  NP1_TEST_ASSERT(postfix.size() == 3);
  check_token_is(postfix[0], token_type::TYPE_IDENTIFIER_VARIABLE, "this.fred", -1);
  check_token_is(postfix[1], token_type::TYPE_INT, "2", -1);
  check_token_is(postfix[2], token_type::TYPE_OPERATOR, "+", 2);
}


void test_arithmetic_precedence() {
  NP1_TEST_UNIT_REL_RLANG_DEFINE_INPUT_TOKEN_VECTOR(input, "this.fred+2*3");
  std::vector<parsed_token_info_type> postfix;

  shunting_yard_type::parse<fn_table_type>(input, postfix);

  NP1_TEST_ASSERT(postfix.size() == 5);
  check_token_is(postfix[0], token_type::TYPE_IDENTIFIER_VARIABLE, "this.fred", -1);
  check_token_is(postfix[1], token_type::TYPE_INT, "2", -1);
  check_token_is(postfix[2], token_type::TYPE_INT, "3", -1);
  check_token_is(postfix[3], token_type::TYPE_OPERATOR, "*", 2);
  check_token_is(postfix[4], token_type::TYPE_OPERATOR, "+", 2);
}


void test_arithmetic_precedence_with_parentheses() {
  NP1_TEST_UNIT_REL_RLANG_DEFINE_INPUT_TOKEN_VECTOR(input, "(this.fred+2)*3");
  std::vector<parsed_token_info_type> postfix;

  shunting_yard_type::parse<fn_table_type>(input, postfix);

  NP1_TEST_ASSERT(postfix.size() == 5);
  check_token_is(postfix[0], token_type::TYPE_IDENTIFIER_VARIABLE, "this.fred", -1);
  check_token_is(postfix[1], token_type::TYPE_INT, "2", -1);
  check_token_is(postfix[2], token_type::TYPE_OPERATOR, "+", 2);
  check_token_is(postfix[3], token_type::TYPE_INT, "3", -1);
  check_token_is(postfix[4], token_type::TYPE_OPERATOR, "*", 2);
}


void test_function_call_zero_arguments() {
  NP1_TEST_UNIT_REL_RLANG_DEFINE_INPUT_TOKEN_VECTOR(input, "fcall()");
  input[0].type(token_type::TYPE_IDENTIFIER_FUNCTION);
  std::vector<parsed_token_info_type> postfix;

  shunting_yard_type::parse<fn_table_type>(input, postfix);
  
  NP1_TEST_ASSERT(postfix.size() == 1);
  check_token_is(postfix[0], token_type::TYPE_IDENTIFIER_FUNCTION, "fcall", 0);  
}


void test_function_call_one_argument() {
  NP1_TEST_UNIT_REL_RLANG_DEFINE_INPUT_TOKEN_VECTOR(input, "fcall(1)");
  input[0].type(token_type::TYPE_IDENTIFIER_FUNCTION);
  std::vector<parsed_token_info_type> postfix;

  shunting_yard_type::parse<fn_table_type>(input, postfix);

  NP1_TEST_ASSERT(postfix.size() == 2);
  check_token_is(postfix[0], token_type::TYPE_INT, "1", -1);
  check_token_is(postfix[1], token_type::TYPE_IDENTIFIER_FUNCTION, "fcall", 1);
}


void test_function_call_two_arguments() {
  NP1_TEST_UNIT_REL_RLANG_DEFINE_INPUT_TOKEN_VECTOR(input, "fcall(1, 2)");
  input[0].type(token_type::TYPE_IDENTIFIER_FUNCTION);
  std::vector<parsed_token_info_type> postfix;

  shunting_yard_type::parse<fn_table_type>(input, postfix);

  NP1_TEST_ASSERT(postfix.size() == 3);
  check_token_is(postfix[0], token_type::TYPE_INT, "1", -1);
  check_token_is(postfix[1], token_type::TYPE_INT, "2", -1);
  check_token_is(postfix[2], token_type::TYPE_IDENTIFIER_FUNCTION, "fcall", 2);  
}


void test_unary_minus() {
  NP1_TEST_UNIT_REL_RLANG_DEFINE_INPUT_TOKEN_VECTOR(input, "1+-2");
  input[2].first_matching_sym_op_fn_id(fn_table_type::find_unary_minus());

  std::vector<parsed_token_info_type> postfix;

  shunting_yard_type::parse<fn_table_type>(input, postfix);

  NP1_TEST_ASSERT(postfix.size() == 4);
  check_token_is(postfix[0], token_type::TYPE_INT, "1", -1);
  check_token_is(postfix[1], token_type::TYPE_INT, "2", -1);
  check_token_is(postfix[2], token_type::TYPE_OPERATOR, "-", 1);
  check_token_is(postfix[3], token_type::TYPE_OPERATOR, "+", 2);
}

void test_unary_minus_on_complex_expression() {
  NP1_TEST_UNIT_REL_RLANG_DEFINE_INPUT_TOKEN_VECTOR(input, "2+3*4/2--1");
  input[8].first_matching_sym_op_fn_id(fn_table_type::find_unary_minus());

  std::vector<parsed_token_info_type> postfix;

  shunting_yard_type::parse<fn_table_type>(input, postfix);

  NP1_TEST_ASSERT(postfix.size() == 10);
  check_token_is(postfix[0], token_type::TYPE_INT, "2", -1);
  check_token_is(postfix[1], token_type::TYPE_INT, "3", -1);
  check_token_is(postfix[2], token_type::TYPE_INT, "4", -1);
  check_token_is(postfix[3], token_type::TYPE_OPERATOR, "*", 2);
  check_token_is(postfix[4], token_type::TYPE_INT, "2", -1);
  check_token_is(postfix[5], token_type::TYPE_OPERATOR, "/", 2);
  check_token_is(postfix[6], token_type::TYPE_OPERATOR, "+", 2);
  check_token_is(postfix[7], token_type::TYPE_INT, "1", -1);
  check_token_is(postfix[8], token_type::TYPE_OPERATOR, "-", 1);
  check_token_is(postfix[9], token_type::TYPE_OPERATOR, "-", 2);
}



void test_shunting_yard() {
  NP1_TEST_RUN_TEST(test_single_expression);
  NP1_TEST_RUN_TEST(test_binary_operation);
  NP1_TEST_RUN_TEST(test_arithmetic_precedence);
  NP1_TEST_RUN_TEST(test_arithmetic_precedence_with_parentheses);
  NP1_TEST_RUN_TEST(test_function_call_zero_arguments);
  NP1_TEST_RUN_TEST(test_function_call_one_argument);
  NP1_TEST_RUN_TEST(test_function_call_two_arguments);
  NP1_TEST_RUN_TEST(test_unary_minus);
  NP1_TEST_RUN_TEST(test_unary_minus_on_complex_expression);

  //TODO: more permutations
}

} // namespaces
}
}
}
}


#endif
