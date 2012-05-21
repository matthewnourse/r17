// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_REL_RLANG_IO_TOKEN_WRITER_HPP
#define NP1_REL_RLANG_IO_TOKEN_WRITER_HPP


#include "np1/rel/rlang/token.hpp"

namespace np1 {
namespace rel {
namespace rlang {
namespace io {


class token_writer {
public:
  // Write out the list of tokens to the output stream, crashes on error.
  template <typename Mandatory_Output_Stream>
  static void mandatory_write(Mandatory_Output_Stream &os,
                              const rstd::vector<token> &tokens) {
    rstd::vector<token>::const_iterator token_i = tokens.begin();
    rstd::vector<token>::const_iterator token_iz = tokens.end();

    size_t prev_line_number;

    if (token_i < token_iz) {
      prev_line_number = token_i->line_number();
    }

    for (; token_i < token_iz; ++token_i) {
      // Keep the line numbers consistent.
      while (prev_line_number < token_i->line_number()) {
        os.write('\n');
        ++prev_line_number;
      }

      switch (token_i->type()) {
      case token::TYPE_UNKNOWN:
        NP1_ASSERT(false, "Unable to write out TYPE_UNKNOWN token");
        break;

      case token::TYPE_STRING:
        write_string(os, token_i->text());
        break;

      case token::TYPE_UINT:
        os.write(token_i->text());
        os.write("U");
        break;
  
      case token::TYPE_IDENTIFIER_VARIABLE:
      case token::TYPE_IDENTIFIER_FUNCTION:
      case token::TYPE_INT:
      case token::TYPE_DOUBLE:
      case token::TYPE_BOOL_TRUE:
      case token::TYPE_BOOL_FALSE:
      case token::TYPE_OPEN_PAREN:
      case token::TYPE_CLOSE_PAREN:
      case token::TYPE_COMMA:
      case token::TYPE_SEMICOLON:
      case token::TYPE_OPERATOR:
        os.write(token_i->text());
        break;
      }

      if (token_i + 1 < token_iz) {
        os.write(' ');
      }
    }
  }

private:
  // Write out a string, quoting as we go.
  template <typename Mandatory_Output_Stream>
  static void write_string(Mandatory_Output_Stream &os, const char *str) {
    os.write('"');
    for (; *str; ++str) {
      if ('"' == *str) {
        os.write('\\');
      }

      os.write(*str);
    }

    os.write('"');
  }

};



} // namespaces
}
}
}



#endif
