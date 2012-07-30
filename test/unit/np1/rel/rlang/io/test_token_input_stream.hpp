// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_TEST_UNIT_NP1_REL_RLANG_IO_TEST_TOKEN_INPUT_STREAM_HPP
#define NP1_TEST_UNIT_NP1_REL_RLANG_IO_TEST_TOKEN_INPUT_STREAM_HPP



namespace test {
namespace unit {
namespace np1 {
namespace rel {
namespace rlang {
namespace io {

typedef ::np1::io::string_input_stream string_stream_type;
typedef ::np1::rel::rlang::io::token_input_stream<
                    string_stream_type,
                    ::np1::rel::rlang::fn::fn_table> input_stream_type;
typedef ::np1::rel::rlang::token token_type;

#define NP1_TEST_UNIT_REL_RLANG_IO_DEFINE_INPUT_STREAM(name__, str__) \
rstd::string name__##_string_token(str__); \
string_stream_type name__##_string_stream(name__##_string_token); \
input_stream_type name__(name__##_string_stream)


struct token_expected_results_definition {
  const char *m_text;
  token_type::type_type m_expected_type;
  const char *m_expected_text;
  size_t m_expected_line_number;
};


rstd::string make_test_string(const token_expected_results_definition *definition_p,
                              const token_expected_results_definition *definition_end,
                              const rstd::string &in_between) {
  rstd::string all_tokens;
  for (; definition_p < definition_end; ++definition_p) {
    all_tokens.append(definition_p->m_text);
    all_tokens.append(in_between);
  }

  return all_tokens;
}


void check_results(const rstd::string &input_string,
                    const token_expected_results_definition *definition_p,
                    const token_expected_results_definition *definition_end) {
  NP1_TEST_UNIT_REL_RLANG_IO_DEFINE_INPUT_STREAM(input, input_string);

  for (; definition_p < definition_end; ++definition_p) {
    token_type token;

    NP1_TEST_ASSERT(input.read(token));
    NP1_TEST_ASSERT(token.type() == definition_p->m_expected_type);

/*    printf("Expected: %s  Actual: %s\n",
            definition_p->m_expected_text ?
                        definition_p->m_expected_text : definition_p->m_text,
            token.text());
*/
    NP1_TEST_ASSERT(strcmp(token.text(),
                      definition_p->m_expected_text ?
                        definition_p->m_expected_text : definition_p->m_text) == 0);

    NP1_TEST_ASSERT(token.line_number() == definition_p->m_expected_line_number);    
  }

  token_type empty_token;
  NP1_TEST_ASSERT(!input.read(empty_token));
}



void test_all_legal_tokens() {

  static const token_expected_results_definition definitions[] = {
    { "\"plain string\"", token_type::TYPE_STRING, "plain string", 1},
    { "\"this is a \\\"string\\\" with a \\\\ in it\"", token_type::TYPE_STRING, "this is a \"string\" with a \\ in it", 1 },
    { "\"\\1\"", token_type::TYPE_STRING, "\\1", 1 },

    { "this.fred", token_type::TYPE_IDENTIFIER_VARIABLE, 0, 1 },
    { "other.fred", token_type::TYPE_IDENTIFIER_VARIABLE, 0, 1 },
    { "fred", token_type::TYPE_IDENTIFIER_VARIABLE, 0, 1 },
    { "fred_with_underscores", token_type::TYPE_IDENTIFIER_VARIABLE, 0, 1 },
    { "_fred_with_leading_underscore", token_type::TYPE_IDENTIFIER_VARIABLE, 0, 1 },
  
    { "0", token_type::TYPE_INT, 0, 1  },
    { "1", token_type::TYPE_INT, 0, 1 },
    { "01234567890", token_type::TYPE_INT, 0, 1 },

    { "0U", token_type::TYPE_UINT, "0", 1 },
    { "1U", token_type::TYPE_UINT, "1", 1 },
    { "01234567890U", token_type::TYPE_UINT, "01234567890", 1 },

    { "0.0", token_type::TYPE_DOUBLE, "0.0", 1 },
    { "1.0", token_type::TYPE_DOUBLE, "1.0", 1 },
    { "1.0E+4", token_type::TYPE_DOUBLE, "1.0E+4", 1 },

    { "true", token_type::TYPE_BOOL_TRUE, 0, 1 },
    { "false", token_type::TYPE_BOOL_FALSE, 0, 1 },

    { "(", token_type::TYPE_OPEN_PAREN, 0, 1 },
    { ")", token_type::TYPE_CLOSE_PAREN, 0, 1 },
    { ",", token_type::TYPE_COMMA, 0, 1 },
    { ";", token_type::TYPE_SEMICOLON, 0, 1 },
    { "=", token_type::TYPE_OPERATOR, 0, 1 },
    { "!=", token_type::TYPE_OPERATOR, 0, 1 },
    { ">", token_type::TYPE_OPERATOR, 0, 1 },
    { "<", token_type::TYPE_OPERATOR, 0, 1 },
    { "<=", token_type::TYPE_OPERATOR, 0, 1 },
    { ">=", token_type::TYPE_OPERATOR, 0, 1 },
    { "+", token_type::TYPE_OPERATOR, 0, 1 },
    { "-", token_type::TYPE_OPERATOR, 0, 1 },
    { "*", token_type::TYPE_OPERATOR, 0, 1 },
    { "/", token_type::TYPE_OPERATOR, 0, 1 },
    { "%", token_type::TYPE_OPERATOR, 0, 1 },
    { "mod", token_type::TYPE_OPERATOR, 0, 1 },
    { "&&", token_type::TYPE_OPERATOR, 0, 1 },
    { "and", token_type::TYPE_OPERATOR, 0, 1 },
    { "||", token_type::TYPE_OPERATOR, 0, 1 },
    { "or", token_type::TYPE_OPERATOR, 0, 1 },
    { "!", token_type::TYPE_OPERATOR, 0, 1 },
    { "not", token_type::TYPE_OPERATOR, 0, 1 },
    { "&", token_type::TYPE_OPERATOR, 0, 1 },
    { "|", token_type::TYPE_OPERATOR, 0, 1 },
    { "~", token_type::TYPE_OPERATOR, 0, 1 },

    { "@@@unparsed!@#@$%@^&*()_+:><>/';=-@@@", token_type::TYPE_UNPARSED_CODE_BLOCK, "unparsed!@#@$%@^&*()_+:><>/';=-", 1}
  };

  const token_expected_results_definition *definition_p = definitions;
  const token_expected_results_definition *definition_end =
    definition_p + (sizeof(definitions)/sizeof(definitions[0]));

  check_results(make_test_string(definition_p, definition_end, " "),
                definition_p,
                definition_end);
}



void test_arithmetic_expression() {

  static const token_expected_results_definition definitions[] = {
    { "this.fred", token_type::TYPE_IDENTIFIER_VARIABLE, 0, 1 },
    { "+", token_type::TYPE_OPERATOR, 0, 1 },
    { "other.fred", token_type::TYPE_IDENTIFIER_VARIABLE, 0, 1 },
    { "*", token_type::TYPE_OPERATOR, 0, 1 },
    { "22", token_type::TYPE_INT, 0, 1 }
  };

  const token_expected_results_definition *definition_p = definitions;
  const token_expected_results_definition *definition_end =
    definition_p + (sizeof(definitions)/sizeof(definitions[0]));

  check_results(make_test_string(definition_p, definition_end, ""),
                definition_p,
                definition_end);
}


void test_boolean_expression_with_newlines() {

  static const token_expected_results_definition definitions[] = {
    { "this.fred", token_type::TYPE_IDENTIFIER_VARIABLE, 0, 1 },
    { "and", token_type::TYPE_OPERATOR, 0, 2 },
    { "other.fred", token_type::TYPE_IDENTIFIER_VARIABLE, 0, 3 },
    { "||", token_type::TYPE_OPERATOR, 0, 4 },
    { "true", token_type::TYPE_BOOL_TRUE, 0, 5 }
  };

  const token_expected_results_definition *definition_p = definitions;
  const token_expected_results_definition *definition_end =
    definition_p + (sizeof(definitions)/sizeof(definitions[0]));

  check_results(make_test_string(definition_p, definition_end, "\n"),
                definition_p,
                definition_end);
}


void test_parentheses() {
  static const token_expected_results_definition definitions[] = {
    { "(", token_type::TYPE_OPEN_PAREN, 0, 1 },
    { "(", token_type::TYPE_OPEN_PAREN, 0, 1 },
    { "(", token_type::TYPE_OPEN_PAREN, 0, 1 },
    { ")", token_type::TYPE_CLOSE_PAREN, 0, 1 },
    { "(", token_type::TYPE_OPEN_PAREN, 0, 1 },
    { ")", token_type::TYPE_CLOSE_PAREN, 0, 1 },
    { ")", token_type::TYPE_CLOSE_PAREN, 0, 1 },
    { ")", token_type::TYPE_CLOSE_PAREN, 0, 1 }
  };

  const token_expected_results_definition *definition_p = definitions;
  const token_expected_results_definition *definition_end =
    definition_p + (sizeof(definitions)/sizeof(definitions[0]));

  check_results(make_test_string(definition_p, definition_end, ""),
                definition_p,
                definition_end);
}



void test_token_input_stream() {
  NP1_TEST_RUN_TEST(test_all_legal_tokens);
  NP1_TEST_RUN_TEST(test_arithmetic_expression);
  NP1_TEST_RUN_TEST(test_boolean_expression_with_newlines);
  NP1_TEST_RUN_TEST(test_parentheses);
}

} // namespaces
}
}
}
}
}


#endif
