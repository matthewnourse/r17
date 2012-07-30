// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_REL_RLANG_IO_TOKEN_INPUT_STREAM_HPP
#define NP1_REL_RLANG_IO_TOKEN_INPUT_STREAM_HPP


#include "np1/rel/rlang/token.hpp"
#include "np1/io/text_input_stream.hpp"
#include "np1/io/string_output_stream.hpp"

namespace np1 {
namespace rel {
namespace rlang {
namespace io {

/// Read an input stream one token at a time.
template <typename Inner_Stream, typename Fn_Table>
class token_input_stream {
public:
  explicit token_input_stream(Inner_Stream &input) : m_stream(input) {}

  /// Read the next token from the stream.  Returns false on EOF or error.
  bool read(token &tok) {
    static const token empty;
    tok = empty;

    int c;
    // Eat whitespace and comments, stop on EOF & error.
    while (((c = m_stream.read()) >= 0) && (('#' == c) || isspace(c))) {
      if ('#' == c) {
        while (((c = m_stream.read()) >= 0) && (c != '\n')) {
        }
      }
    }

    if (c <= 0) {
      return false;
    }

    tok.line_number(m_stream.line_number());

    // A string is something in either single or double quotes.
    m_stream.unget(c);
    if (('\"' == c) || ('\'' == c)) {      
      read_quoted_string_token(tok, c);
    } else if (is_initial_identifier_char(c)) {
      read_identifier_token(tok);
    } else if (isdigit(c)) {
      read_number_token(tok, false);
    } else {
      read_special_char_token(tok);
    }
 
    return true;
  }


private:
  // Add line number information to the error message.
  rstd::string error_message(const rstd::string &message) {
    return error_message(m_stream.line_number(), message);
  }

  static rstd::string error_message(size_t line_number, const rstd::string &message) {
    return "line " + str::to_dec_str(line_number) + ": " + message;
  }

  static rstd::string error_message(size_t line_number, const rstd::string &message,
                                    const rstd::string &tok) {
    return "line " + str::to_dec_str(line_number) + ": " + message
            + "  token: " + tok;
  }


  // Read a string in double or single quotes.
  void read_quoted_string_token(token &tok, char quote_char) {
    np1::io::string_output_stream temp_stream(tok.writeable_text());
    NP1_ASSERT(str::read_quoted_string(m_stream, temp_stream), error_message("Incomplete string"));
    tok.type(token::TYPE_STRING);
  }

  // Read an identifier.
  void read_identifier_token(token &tok) {
    np1::io::string_output_stream tok_stream(tok.writeable_text());
    int c = 0;

    tok.type(token::TYPE_IDENTIFIER_VARIABLE);

    while (((c = m_stream.read()) > 0) && is_identifier_char(c)) {
      tok_stream.write((char)c);
    }

    m_stream.unget(c);

    // If this is actually a synonym for an operator or it's a boolean then
    // change its type.
    size_t function_id = Fn_Table::find_first(str::ref(tok.text()));
    if ((function_id != (size_t)-1) && Fn_Table::get_info(function_id).is_operator()) {
      tok.type(token::TYPE_OPERATOR);
      tok.first_matching_sym_op_fn_id(function_id);
    } else if (str::cmp(tok.text(), "true") == 0) {
      tok.type(token::TYPE_BOOL_TRUE);    
    } else if (str::cmp(tok.text(), "false") == 0) {
      tok.type(token::TYPE_BOOL_FALSE);    
    }
  }



  // Read a token that starts with a special character.
  // Special-char tokens will be read greedily using the supplied function
  // objects to decide when to stop.
  void read_special_char_token(token &tok) {
    np1::io::string_output_stream tok_stream(tok.writeable_text());

    tok.type(token::TYPE_UNKNOWN);

    // One-char symbols.    
    char first_char = m_stream.read();
    tok_stream.write(first_char);
    switch (first_char) {
    case '(':
      tok.type(token::TYPE_OPEN_PAREN);      
      break;

    case ')':
      tok.type(token::TYPE_CLOSE_PAREN);
      break;

    case ',':
      tok.type(token::TYPE_COMMA);
      break;

    case ';':
      tok.type(token::TYPE_SEMICOLON);
      break;

    default:
      {
        int c = 0;
      
        // Keep reading until it's not a valid operator any more.
        while (Fn_Table::is_valid_partial_match(str::ref(tok.text()))
                && ((c = m_stream.read()) >= 0)) {
          tok_stream.write((char)c);
        }
  
        if (c >= 0) {        
          m_stream.unget(tok.text()[tok.text_length() - 1]);
          tok.text_remove_last();
        }
  
        size_t first_matching_fn_id = Fn_Table::find_first(str::ref(tok.text()));
        tok.assert(first_matching_fn_id != (size_t)-1, "Unknown special-char token");
        tok.first_matching_sym_op_fn_id(first_matching_fn_id);
        tok.type(token::TYPE_OPERATOR);
      }
      break;
    }    
  }


  // Read a number.
  void read_number_token(token &tok, bool is_negative) {
    np1::io::string_output_stream tok_stream(tok.writeable_text());
    int c = 0;
    bool is_fp = false;
    bool is_valid_number = true;
    bool seen_e = false;

    tok.type(token::TYPE_INT);

    if (is_negative) {
      tok_stream.write('-');
    }

     while (is_valid_number && ((c = m_stream.read()) > 0)) {      
      if (!is_fp) {
        if ('.' == c) {
          is_fp = true;
        } else {
          is_valid_number = isdigit(c);
        }
      } else {
        if (!isdigit(c)) {
          if ('E' == toupper(c)) {
            if (!seen_e) {
              seen_e = true;
            } else {
              is_valid_number = false;
            }
          } else if (('+' == c) || ('-' == c)) {
            seen_e = false;          
          } else {
            is_valid_number = false;
          }
        }
      }
      
      if (is_valid_number) {
        tok_stream.write((char)c);
      }
    }
  
    if (is_fp) {
      // Check that this is a valid-looking floating-point number.
      char *endptr = NULL;
      const char *token_end = tok.text() + tok.text_length();
      str::partial_dec_to_double(tok.text(), token_end, &endptr);
      NP1_ASSERT((endptr == token_end),
                 error_message("Invalid floating-point number.  Token: " + rstd::string(tok.text())));
      tok.type(token::TYPE_DOUBLE);      
    }
  
    if ('U' == c) {
      NP1_ASSERT(!is_fp, "Unsigned floating-point numbers are not supported");
      tok.type(token::TYPE_UINT);
      c = ' ';
    }

    NP1_ASSERT(
      !isalpha(c),  // Even floating point numbers can't end with an alpha char.
      error_message("Invalid number: '" + rstd::string(1, (char)c)
                    + "'.  Token: " + rstd::string(tok.text())));    
    m_stream.unget(c);
  }

  char mandatory_read() {
    int c = m_stream.read();    
    NP1_ASSERT(c >= 0, error_message("Unterminated operator"));
    return c;
  }

  bool is_identifier_char(char c) {
    // Colons are allowed so that variables can have type tags.  TODO: this seems
    // unclean, and also we should be checking for only one colon.
    return isalnum(c) || ('_' == c) || ('.' == c) || (':' == c);
  }
  
  bool is_initial_identifier_char(char c) {
    return isalpha(c) || ('_' == c);  
  }

private:
  np1::io::text_input_stream<Inner_Stream> m_stream;  
};



} // namespaces
}
}
}

#endif
