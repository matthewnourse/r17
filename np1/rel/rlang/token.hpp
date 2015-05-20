// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_REL_RLANG_TOKEN_HPP
#define NP1_REL_RLANG_TOKEN_HPP


#include "np1/str.hpp"


// Special asserts that print out information available in the token.
#define NP1_TOKEN_ASSERT(cond__, tok__, message__) \
NP1_ASSERT((cond__), "line " + str::to_dec_str((tok__).line_number()) + ": " + rstd::string(message__) + "  token: '" + rstd::string((tok__).text()) + "'")

#define NP1_TOKEN_ASSERT_NO_TOKEN_TEXT(cond__, tok__, message__) \
NP1_ASSERT((cond__), "line " + str::to_dec_str((tok__).line_number()) + ": " + rstd::string(message__))

#define NP1_TOKEN_UNPARSED_CODE_BLOCK_DELIMITER "@@@"

namespace np1 {
namespace rel {
namespace rlang {

/// An input token.
class token {
public:
  typedef enum {
    TYPE_UNKNOWN = -1,
    TYPE_STRING,
    TYPE_IDENTIFIER_VARIABLE,
    TYPE_IDENTIFIER_FUNCTION,  // Never set by reader, used upstream.
    TYPE_INT,
    TYPE_UINT,
    TYPE_DOUBLE,
    TYPE_BOOL_TRUE,
    TYPE_BOOL_FALSE,
    TYPE_OPEN_PAREN,
    TYPE_CLOSE_PAREN,
    TYPE_COMMA,
    TYPE_SEMICOLON,
    TYPE_OPERATOR,
    TYPE_UNPARSED_CODE_BLOCK,
    TYPE_SCRIPT_ARGUMENT_REFERENCE,
  } type_type;

  typedef enum {
    INITIAL_CHAR_SCRIPT_ARGUMENT_REFERENCE = '$'
  } initial_char_type;
  
public:
  token() : m_line_number(0), m_type(TYPE_UNKNOWN), m_first_matching_sym_op_fn_id(-1) {}

  token(const char *text, type_type type)
    : m_line_number(0), m_type(type), m_first_matching_sym_op_fn_id(-1), m_text(text) {}

  void assert(bool condition, const char *message) const {
    NP1_TOKEN_ASSERT(condition, *this, message);
  }

  /// Get and set stuff.  
  size_t line_number() const { return m_line_number; }
  void line_number(size_t line_number) { m_line_number = line_number; }

  type_type type() const { return m_type; }
  void type(type_type type) { m_type = type; }

  int first_matching_sym_op_fn_id() const { return m_first_matching_sym_op_fn_id; }
  void first_matching_sym_op_fn_id(size_t first_matching_sym_op_fn_id) {
    m_first_matching_sym_op_fn_id = first_matching_sym_op_fn_id;
  }

  bool is_minus() const { return ((m_text.length() == 1) && ('-' == m_text[0])); }

  const char *text() const {
    const char *p = m_text.c_str();
    return p ? p : "";
  }
  
  const rstd::string &text_str() const { return m_text; }
  
  size_t text_length() const { return m_text.length(); }
  void text_remove_last() { m_text = m_text.substr(0, m_text.length() - 1); }
  rstd::string &writeable_text() { return m_text; }

private:
  size_t m_line_number;
  type_type m_type;
  int m_first_matching_sym_op_fn_id;
  rstd::string m_text;
};

} // namespaces
}
}


#endif
