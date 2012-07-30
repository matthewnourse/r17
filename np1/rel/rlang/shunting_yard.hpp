// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_REL_RLANG_SHUNTING_YARD_HPP
#define NP1_REL_RLANG_SHUNTING_YARD_HPP


#include "np1/rel/rlang/io/token_input_stream.hpp"
#include "np1/rel/rlang/token.hpp"


namespace np1 {
namespace rel {
namespace rlang {

/// Dijkstra's Shunting Yard algorithm as described in
/// http://en.wikipedia.org/wiki/Shunting_yard_algorithm
class shunting_yard {
public:
  class parsed_token_info {
  public:
    explicit parsed_token_info(const token &tok,
                                size_t initial_function_arg_count = -1,
                                size_t function_start_postfix_offset = -1)
      : m_token(tok), m_function_arg_count(initial_function_arg_count),
        m_function_start_postfix_offset(function_start_postfix_offset),
        m_function_end_postfix_offset(-1) {}

  const token &tok() const { return m_token; }
  size_t function_arg_count() const { return m_function_arg_count; }
  void incr_function_arg_count() { ++m_function_arg_count; }
  size_t function_start_postfix_offset() const { return m_function_start_postfix_offset; }
  size_t function_end_postfix_offset() const { return m_function_end_postfix_offset; }
  void function_end_postfix_offset(size_t n) { m_function_end_postfix_offset = n; }

  token::type_type type() const { return m_token.type(); }
  const char *text() const { return m_token.text(); }

  private:
    token m_token;
    int m_function_arg_count;
    size_t m_function_start_postfix_offset;
    size_t m_function_end_postfix_offset;
  };

public:
  /// Parse the vector of infix expressions and return the postfix.
  template <typename Function_Table>
  static void parse(const rstd::vector<token> &infix,
                    rstd::vector<parsed_token_info> &postfix) {
    size_t infix_offset = 0;
    const token *tok = infix.begin();
    const token *tok_end = infix.end();
    rstd::stack<parsed_token_info> tok_stack;

    for (infix_offset = 0; tok < tok_end; ++tok, ++infix_offset) {
      switch (tok->type()) {
      case token::TYPE_UNKNOWN:
        tok->assert(false, "Token type unknown");
        break;

      case token::TYPE_STRING:
      case token::TYPE_IDENTIFIER_VARIABLE:
      case token::TYPE_INT:
      case token::TYPE_UINT:
      case token::TYPE_DOUBLE:
      case token::TYPE_BOOL_TRUE:
      case token::TYPE_BOOL_FALSE:
      case token::TYPE_UNPARSED_CODE_BLOCK:
        postfix.push_back(parsed_token_info(*tok));
        break;

      case token::TYPE_IDENTIFIER_FUNCTION:
        tok_stack.push(parsed_token_info(*tok, 0, postfix.size()));
        break;

      case token::TYPE_COMMA:
        // To us the comma is only a function argument separator.  In C it can
        // also be an operator.
        while (!tok_stack.empty()
                && (tok_stack.top().type() != token::TYPE_OPEN_PAREN)) {
          postfix.push_back(tok_stack.top());
          tok_stack.pop();
        }

        tok->assert(!tok_stack.empty(), "Mismatched ',' or '('");

        // Increment the number of arguments to the function.
        {
          parsed_token_info open_paren = tok_stack.top();
          tok_stack.pop();
          tok->assert(!tok_stack.empty()
                      && (token::TYPE_IDENTIFIER_FUNCTION == tok_stack.top().type()),
                      "Comma used outside of function call");
          tok_stack.top().incr_function_arg_count();
          tok_stack.push(open_paren);
        }        
        break;

      case token::TYPE_SEMICOLON:
        tok->assert(false, "';' not allowed here");
        break;

      case token::TYPE_OPEN_PAREN:
        tok_stack.push(parsed_token_info(*tok));
        break;
    
      case token::TYPE_CLOSE_PAREN:
        {
          while (!tok_stack.empty()
                  && (tok_stack.top().type() != token::TYPE_OPEN_PAREN)) {
            postfix.push_back(tok_stack.top());
            tok_stack.pop();
          }
  
          tok->assert(!tok_stack.empty(), "Mismatched '('");
  
          // Pop the open parenthesis.
          tok_stack.pop();
  
          // If the thing at the top of the stack is a function then pop it into
          // the output queue.
          if (!tok_stack.empty()
              && (token::TYPE_IDENTIFIER_FUNCTION == tok_stack.top().type())) {
            if (postfix.size() != tok_stack.top().function_start_postfix_offset()) {
              // Only need to account for one argument here, the rest were
              // dealt with in the comma-handling code.
              tok_stack.top().incr_function_arg_count();
            }
            
            tok_stack.top().function_end_postfix_offset(postfix.size());
            postfix.push_back(tok_stack.top());
            tok_stack.pop();        
          }
        }
        break;

      case token::TYPE_OPERATOR:
        // While there is an operator token, at the top of the stack
        // tok is left-associative and its precedence is greater than or equal to that of stack.top,
        // or tok is right-associative and its precedence is greater than that of stack.top...
        while (!tok_stack.empty()
                && (tok_stack.top().tok().first_matching_sym_op_fn_id() != -1)
                && Function_Table::get_info(tok_stack.top().tok().first_matching_sym_op_fn_id()).is_operator()
                && is_greater_precedence<Function_Table>(*tok, tok_stack.top().tok())) {

          // ...pop the stack on to the output queue.
          postfix.push_back(tok_stack.top());
          tok_stack.pop();              
        }

        // Assume that all operator overloads have the same arity.
        tok_stack.push(
          parsed_token_info(
            *tok,
            Function_Table::get_info(tok->first_matching_sym_op_fn_id()).number_arguments()));
        break;
      }
    }

    // Deal with any leftover tokens on the stack.
    while (!tok_stack.empty()) {
      tok->assert((token::TYPE_OPEN_PAREN != tok_stack.top().type())
                  && (token::TYPE_CLOSE_PAREN != tok_stack.top().type()),
                  "Mismatched ( or )");
      postfix.push_back(tok_stack.top());
      tok_stack.pop();      
    }
  }

private:
  template <typename Fn_Table>
  static bool is_greater_precedence(const token &one, const token &two) {
    size_t one_id = one.first_matching_sym_op_fn_id();
    size_t two_id = two.first_matching_sym_op_fn_id();
    typename Fn_Table::fn_info one_info = Fn_Table::get_info(one_id);
    typename Fn_Table::fn_info two_info = Fn_Table::get_info(two_id);
    bool one_is_left_assoc = one_info.is_left_assoc();
    size_t one_precedence = one_info.precedence();
    size_t two_precedence = two_info.precedence();

    return ((one_is_left_assoc && (one_precedence >= two_precedence))
            || (!one_is_left_assoc && (one_precedence > two_precedence)));
  }
};

} // namespaces
}
} 


#endif
