// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_JSON_TOKENIZER_HPP
#define NP1_JSON_TOKENIZER_HPP


#include "np1/simple_types.hpp"
#include "np1/assert.hpp"
#include "np1/str.hpp"
#include <ctype.h>

namespace np1 {
namespace json {
  
class tokenizer {
public:
  template <typename Handler>
  static void raw_tokenize(const str::ref &data, Handler &h) {
    tokenizer obj(data.ptr(), data.ptr() + data.length());
    obj.raw_tokenize(h);
  }

  template <typename Handler>
  static void raw_tokenize(const str::ref &data, const Handler &h) {
    raw_tokenize(data, (Handler &)h);
  }

private:  
  tokenizer(const char *data, const char *end) : m_data(data), m_end(end), m_p(m_data) {
    NP1_ASSERT(m_end >= m_data, "Invalid data passed to JSON tokenizer");
  }

  template <typename Handler>
  void raw_tokenize(Handler &h) {
    while (true) {
      eat_whitespace();
      if (m_p >= m_end) {
        return;
      }

      switch (*m_p) {
        case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9': case '-':
          on_number(h);
          break;

        case '"':
          on_string(h);
          break;

        case '{':
          h.on_open_object(str::ref(m_p, 1));
          m_p++;
          break;

        case ':':
          h.on_object_name_value_delimiter(str::ref(m_p, 1));
          m_p++;
          break;

        case '}':
          h.on_close_object(str::ref(m_p, 1));
          m_p++;
          break;

        case '[':
          h.on_open_array(str::ref(m_p, 1));
          m_p++;
          break;

        case ',':
          h.on_element_delimiter(str::ref(m_p, 1));
          m_p++;
          break;

        case ']':
          h.on_close_array(str::ref(m_p, 1));
          m_p++;
          break;

        case 't': 
          on_true(h);
          break;

        case 'f':
          on_false(h);
          break;

        case 'n':
          on_null(h);
          break;

        default:
          NP1_ASSERT(false, "Invalid character in JSON: " + rstd::string(1, *m_p));
          break;
      }
    }
  }

  void eat_whitespace() {
    while ((m_p < m_end) && isspace(*m_p)) {
      ++m_p;
    }
  }

  template <typename Handler>
  void on_number(Handler &h) {
    const char *start = m_p;
    if ('-' == *m_p) {
      ++m_p;
      NP1_ASSERT((m_p < m_end) && isdigit(*m_p), "JSON number starting with '-' is not followed by a digit: " + debug_fragment());
    }

    const char *int_start = m_p;
    while ((m_p < m_end) && isdigit(*m_p)) {
      ++m_p;
    }
    NP1_ASSERT(m_p > int_start, "JSON number has no integer digits: " + debug_fragment());

    if ((m_p < m_end) && ('.' == *m_p)) {
      ++m_p;
      const char *frac_start = m_p;
      while ((m_p < m_end) && isdigit(*m_p)) {
        ++m_p;
      }
      NP1_ASSERT(m_p > frac_start, "JSON number '.' not followed by digits: " + debug_fragment());
    }

    if ((m_p < m_end) && (('e' == *m_p) || ('E' == *m_p))) {
      ++m_p;
      if ((m_p < m_end) && (('+' == *m_p) || ('-' == *m_p))) {
        ++m_p;
      }

      const char *exp_start = m_p;
      while ((m_p < m_end) && isdigit(*m_p)) {
        ++m_p;
      }
      NP1_ASSERT(m_p > exp_start, "JSON number exponent has no digits: " + debug_fragment());
    }

    if ((m_p < m_end) && !is_stop_char(*m_p)) {
      NP1_ASSERT(false, "Invalid char at end of JSON number: " + rstd::string(1, *m_p) + " number: " + rstd::string(start, m_p+1 - start));
    }

    h.on_number(str::ref(start, m_p - start));
  }

  template <typename Handler>
  void on_string(Handler &h) {
    const char *start = m_p;
    m_p++;
    while (true) {
      const char *end = (const char *)memchr(m_p, '"', m_end - m_p);
      NP1_ASSERT(end, "Unterminated JSON string: " + debug_fragment());
      m_p = end + 1;
      if ('\\' != end[-1]) {
        h.on_string(str::ref(start, m_p - start));
        return;
      }
    }
  }

  void validate_special(const str::ref &expected) {
    size_t len = expected.length();
    size_t remaining = m_end - m_p;
    NP1_ASSERT((remaining >= len) && ((remaining == len) || is_stop_char(m_p[len])) && (str::cmp(expected, str::ref(m_p, len)) == 0),
               "Invalid JSON special value, expected '" + expected.to_string() + "' found: " + debug_fragment());
  }

  template <typename Handler>
  void on_true(Handler &h) {
    str::ref expected("true");
    validate_special(expected);
    h.on_special(expected);
    m_p += expected.length();
  }

  template <typename Handler>
  void on_false(Handler &h) {
    str::ref expected("false");
    validate_special(expected);
    h.on_special(expected);
    m_p += expected.length();
  }

  template <typename Handler>
  void on_null(Handler &h) {
    str::ref expected("null");
    validate_special(expected);
    h.on_special(expected);
    m_p += expected.length();
  }

  rstd::string debug_fragment() {
    const size_t max_len = 30;
    size_t remaining = m_end - m_p;
    return "'" + ((remaining > max_len) ? (rstd::string(m_p, max_len) + "...") : rstd::string(m_p, remaining)) + "'";
  }

  static bool is_stop_char(char c) {
    return ((']' == c) || ('}' == c) || (',' == c) || isspace(c));
  }


private:
  const char *m_data;
  const char *m_end;
  const char *m_p;
};


} // namespaces
}



#endif
