// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_STR_HPP
#define NP1_STR_HPP

#include <string>
#include <ctype.h>
#include "lib/liberal/int64/int64.hpp"

// String utilities.

namespace np1 {
namespace str {

/**
 * A constant string reference, perhaps to a substring.
 */
class ref {
public:
  ref() : m_ptr(0), m_length(0) {}
  explicit ref(const char *p) : m_ptr(p), m_length(strlen(p)) {}
  explicit ref(const std::string &s) : m_ptr(s.c_str()), m_length(s.length()) {}
  ref(const char *p, size_t n) : m_ptr(p), m_length(n) {}
  inline const char *ptr() const { return m_ptr; }
  inline size_t length() const { return m_length; }
  inline bool is_null() const { return !m_ptr; }
  std::string to_string() const { return std::string(m_ptr, m_length); }

  // For the use of vm_stack and similar.
  inline void set_ptr(const char *new_ptr) { m_ptr = new_ptr; }
  inline void set_length(size_t len) { m_length = len; }

  void swap(str::ref &other) {
    std::swap(m_ptr, other.m_ptr);
    std::swap(m_length, other.m_length);
  }
private:
  const char *m_ptr;
  size_t m_length;
};


enum { MAX_NUM_STR_LENGTH = 64 };


/* Don't be tempted to use strtoll, it sets things to LONG_MAX on overflow and
   does other useless crap. */
int64_t partial_dec_to_int64(const char *s, const char *s_end,
                              char **end_of_number_p) {
  while ((s < s_end) && isspace(*s)) {
    ++s;
  }

  if (s == s_end) {
    *end_of_number_p = (char *)s;
    return 0;
  }
  
  bool is_negative = ('-' == *s);
  int64_t end_multiplier = 1;
  if (is_negative) {
    ++s;
    end_multiplier = -1;
  }

  int64_t i = 0;

  for (; (s < s_end) && isdigit(*s); ++s) {
    i = (i * 10) + (*s - '0');
  }

  i *= end_multiplier;
  *end_of_number_p = (char *)s;
  return i;
}


int64_t partial_dec_to_int64(const char *s, size_t s_length,
                              size_t &number_length) {
  char *endptr;
  int64_t num = partial_dec_to_int64(s, s + s_length, &endptr);
  number_length = endptr - s;
  return num;
}


int64_t partial_hex_to_int64(const char *s, const char *s_end,
                              char **end_of_number_p) {
  while ((s < s_end) && isspace(*s)) {
    ++s;
  }

  if (s == s_end) {
    *end_of_number_p = (char *)s;
    return 0;
  }
  
  int64_t i = 0;

  for (; (s < s_end) && isxdigit(*s); ++s) {
    i = (i * 16) + (isdigit(*s) ? (*s - '0') : (tolower(*s) - 'a' + 10));
  }

  *end_of_number_p = (char *)s;
  return i;
}


int64_t dec_to_int64(const char *s, size_t length) {
  char *end_of_number_p;
  const char *s_end = s + length;
  int64_t result = partial_dec_to_int64(s, s_end, &end_of_number_p);
  NP1_ASSERT(s_end == end_of_number_p || isspace(*end_of_number_p),
              "String is not a decimal number: '" + std::string(s, length) + "'");  

  return result;
}



int64_t dec_to_int64(const str::ref &s) {
  return dec_to_int64(s.ptr(), s.length());
}


int64_t dec_to_int64(const char *s) {
  return dec_to_int64(s, strlen(s));
}

int64_t dec_to_int64(const std::string &s) {
  return dec_to_int64(s.c_str(), s.length());
}


int64_t hex_to_int64(const char *s) {
  char *end_of_number_p;
  const char *s_end = s + strlen(s);
  int64_t result = partial_hex_to_int64(s, s_end, &end_of_number_p);
  NP1_ASSERT(s_end == end_of_number_p || isspace(*end_of_number_p),
              "String is not a hexadecimal number");  

  return result;
}

// Ideally we would do our own translation here to avoid the weirdnesses in strtod but it's too hard for now.
double partial_dec_to_double(const char *s, const char *s_end, char **end_of_number_p) {
  char old_char = *s_end;
  *((char *)s_end) = '\0';
  const double d = strtod(s, end_of_number_p);
  *((char *)s_end) = old_char;
  return d;
}

double dec_to_double(const char *s, size_t length) {
  char *end_of_number_p;
  const char *s_end = s + length;
  double result = partial_dec_to_double(s, s_end, &end_of_number_p);
  NP1_ASSERT(s_end == end_of_number_p || isspace(*end_of_number_p),
              "String is not a decimal floating-point number: '" + std::string(s, length) + "'");  

  return result;
}

double dec_to_double(const str::ref &s) {
  return dec_to_double(s.ptr(), s.length());
}



void to_dec_str(char *num_str, uint64_t i) {
  char reversed[MAX_NUM_STR_LENGTH];
  char *reversed_ptr = reversed;
  do {
    *reversed_ptr++ = '0' + lib::int64::modulus(i, 10);
    i = lib::int64::divide(i, 10);
  } while (i != 0);
  
  while (--reversed_ptr >= reversed) {
    *num_str++ = *reversed_ptr;
  }
  
  *num_str = '\0';
}

void to_dec_str(char *num_str, int64_t i) {
  if (i < 0) {
    *num_str++ = '-';
    i = -i;
  }
  
  to_dec_str(num_str, (uint64_t)i);
}

void to_dec_str(char *num_str, int32_t i) {
  to_dec_str(num_str, (int64_t)i);
}

void to_dec_str(char *num_str, uint32_t ui) {
  to_dec_str(num_str, (uint64_t)ui);
}

//TODO: surely we can do something cleaner than this.  The trouble is that on other platforms, size_t is the same as uint32_t.
#ifdef __APPLE__
void to_dec_str(char *num_str, size_t sz) {
  to_dec_str(num_str, (uint64_t)sz);
}

void to_dec_str(char *num_str, ssize_t sz) {
  to_dec_str(num_str, (uint64_t)sz);
}
#endif



template <typename T>
std::string to_dec_str(T i) {
  char num_str[MAX_NUM_STR_LENGTH];
  to_dec_str(num_str, i);
  return num_str;
}

//TODO: we could do this faster ourselves.
void to_hex_str(char *num_str, int64_t i) {
#ifdef _WIN32
  sprintf(num_str, "%I64x", i);
#else
  sprintf(num_str, "%llx", (long long)i);
#endif
}


namespace detail {
  inline char *hex_encode_2(char *s, int64_t i) {
    static const char HEX_CHARS[] = "0123456789abcdef";
    *s++ = HEX_CHARS[(i >> 4) & 0xf];
    *s++ = HEX_CHARS[i & 0xf];
    return s;
  }

  inline char *hex_encode_4(char *s, int64_t i) {
    return hex_encode_2(hex_encode_2(s, i >> 8), i);
  }

  inline char *hex_encode_8(char *s, int64_t i) {
    return hex_encode_4(hex_encode_4(s, i >> 16), i);
  }

  inline char *hex_encode_12(char *s, int64_t i) {
    return hex_encode_8(hex_encode_4(s, i >> 32), i);
  }

  inline char *hex_encode_16(char *s, int64_t i) {
    return hex_encode_8(hex_encode_8(s, i >> 32), i);
  }  
} // namespace detail


void to_hex_str_pad_2(char *num_str, int64_t i) {
  *(detail::hex_encode_2(num_str, i)) = '\0';
}

void to_hex_str_pad_4(char *num_str, int64_t i) {
  *(detail::hex_encode_4(num_str, i)) = '\0';
}

void to_hex_str_pad_8(char *num_str, int64_t i) {
  *(detail::hex_encode_8(num_str, i)) = '\0';
}

void to_hex_str_pad_12(char *num_str, int64_t i) {
  *(detail::hex_encode_12(num_str, i)) = '\0';
}

void to_hex_str_pad_16(char *num_str, int64_t i) {
  *(detail::hex_encode_16(num_str, i)) = '\0';
}

std::string to_hex_str_pad_16(int64_t i) {
  char num_str[17];
  to_hex_str_pad_16(num_str, i);
  return num_str;
}

void to_hex_str_pad_64(char *num_str, const unsigned char *bytes) {
  const unsigned char *p = bytes;
  const unsigned char *end = p + 32;

  for (; p < end; ++p) {
    num_str = detail::hex_encode_2(num_str, *p);
  }

  *num_str = '\0';
}


std::string to_hex_str(int64_t i) {
  char num_str[MAX_NUM_STR_LENGTH];
  to_hex_str(num_str, i);
  return num_str;
}


void to_dec_str(char *num_str, double d) {
  sprintf(num_str, "%f", d);
}

std::string to_dec_str(double d) {
  char num_str[MAX_NUM_STR_LENGTH];
  to_dec_str(num_str, d);
  return num_str;
}

bool partial_to_bool(const char *s, const char *s_end, char **end_of_bool_p) {
  *end_of_bool_p = (char *)s;

  while ((s < s_end) && isspace(*s)) {
    ++s;
  }
  
  if (s >= s_end) {
    return false;
  }

  if ('t' == *s) {
    if (s_end - s < 4) {
      return false;
    }

    if (('r' == *++s) && ('u' == *++s) && ('e' == *++s) && ((++s == s_end) || isspace(*s))) {
      *end_of_bool_p = (char *)s;
      return true;
    }
  } else if ('f' == *s) {
    if (s_end - s < 5) {
      return false;
    }

    if (('a' == *++s) && ('l' == *++s) && ('s' == *++s) && ('e' == *++s)
          && ((++s == s_end) || isspace(*s))) {
      *end_of_bool_p = (char *)s;
      return false;
    }
  } 

  return false;
}



bool to_bool(const char *s, size_t length) {
  char *end_of_bool_p;
  const char *s_end = s + length;
  bool result = partial_to_bool(s, s_end, &end_of_bool_p);
  NP1_ASSERT((end_of_bool_p != s)
                && ((end_of_bool_p == s_end) || isspace(*end_of_bool_p)),
              "String is not a bool");
  return result;
}


bool to_bool(const str::ref &s) {
  return to_bool(s.ptr(), s.length());
}

str::ref from_bool(bool b) { return b ? str::ref("true", 4) : str::ref("false", 5); }

int icmp(const char *s1, const char *s2) {
#ifdef _WIN32
  return _stricmp(s1, s2);
#else
  return strcasecmp(s1, s2);
#endif
}

int icmp(const char *s1, const char *s2, size_t n) {
#ifdef _WIN32
  return _strincmp(s1, s2, n);
#else
  return strncasecmp(s1, s2, n);
#endif
}

int icmp(const char *s1, size_t n1, const char *s2, size_t n2) {
  int result;  
  if (n1 < n2) {
    result = icmp(s1, s2, n1);
    if (0 == result) {
      return -1;
    }

    return result;
  }

  if (n1 > n2) {
    result = icmp(s1, s2, n2);
    if (0 == result) {
      return 1;
    }

    return result;
  }

  return icmp(s1, s2, n1);  
}


int icmp(const ref &r1, const ref &r2) {
  return icmp(r1.ptr(), r1.length(), r2.ptr(), r2.length());
}

int icmp(const ref &r1, const char *s2) {
  return icmp(r1.ptr(), r1.length(), s2, strlen(s2));
}

int icmp(const char *s1, const str::ref &s2) {
  return icmp(s1, strlen(s1), s2.ptr(), s2.length());
}


int icmp(const std::string &s1, const char *s2) {
  return icmp(str::ref(s1), s2);
}

int icmp(const char *s1, const std::string &s2) {
  return icmp(s1, str::ref(s2));
}

int icmp(const std::string &s1, const std::string &s2) {
  return icmp(str::ref(s1), str::ref(s2));
}


int cmp(const char *s1, const char *s2, size_t n) {
  return strncmp(s1, s2, n);
}

int cmp(const char *s1, size_t n1, const char *s2, size_t n2) {
  int result;  
  if (n1 < n2) {
    result = cmp(s1, s2, n1);
    if (0 == result) {
      return -1;
    }

    return result;
  }

  if (n1 > n2) {
    result = cmp(s1, s2, n2);
    if (0 == result) {
      return 1;
    }

    return result;
  }

  return cmp(s1, s2, n1);  
}


int cmp(const char *s1, const char *s2) {
  return strcmp(s1, s2);
}

int cmp(const str::ref &r1, const str::ref &r2) {
  return cmp(r1.ptr(), r1.length(), r2.ptr(), r2.length());
}

int cmp(const str::ref &r1, const char *s) {
  return cmp(r1.ptr(), r1.length(), s, strlen(s));
}

int cmp(const char *s1, const str::ref &s2) {
  return cmp(s1, strlen(s1), s2.ptr(), s2.length());
}

int cmp(const std::string &s1, const char *s2) {
  return cmp(str::ref(s1), s2);
}

int cmp(const char *s1, const std::string &s2) {
  return cmp(s1, str::ref(s2));
}

int cmp(const str::ref &s1, const std::string &s2) {
  return cmp(s1, str::ref(s2));
}



/// Does a string start with another string?
inline bool starts_with(const char *haystack, size_t haystack_length,
                          const char *needle, size_t needle_length) {
  if (needle_length > haystack_length) {
    return false;
  }
  
  return (memcmp(haystack, needle, needle_length) == 0);
}

inline bool starts_with(const char *haystack, size_t haystack_length,
                          const char *needle) {
  return starts_with(haystack, haystack_length, needle, strlen(needle));
}

inline bool starts_with(const str::ref &haystack, const str::ref &needle) {
  return starts_with(haystack.ptr(), haystack.length(), needle.ptr(), needle.length());
}


inline bool starts_with(const str::ref &haystack, const char *needle) {
  return starts_with(haystack.ptr(), haystack.length(), needle, strlen(needle));
}


inline bool starts_with(const char *haystack, const str::ref &needle) {
  return starts_with(haystack, strlen(haystack), needle.ptr(), needle.length());
}

inline bool starts_with(const std::string &haystack, const char *needle) {
  return starts_with(str::ref(haystack.c_str()), str::ref(needle, strlen(needle)));
}



/// Case-insensitive starts_with.
inline bool istarts_with(const char *haystack, size_t haystack_length,
                          const char *needle, size_t needle_length) {  
  if (needle_length > haystack_length) {
    return false;
  }
  
  return (icmp(haystack, needle, needle_length) == 0);
}

inline bool istarts_with(const str::ref &r1, const str::ref &r2) {
  return istarts_with(r1.ptr(), r1.length(), r2.ptr(), r2.length());
}


inline bool istarts_with(const std::string &haystack, const char *needle) {
  return istarts_with(str::ref(haystack), str::ref(needle, strlen(needle)));
}



/// Does a string end with another (possibly null-terminated) string?
inline bool ends_with(const char *haystack, size_t haystack_length,
                      const char *needle, size_t needle_length) {
  if (needle_length > haystack_length) {
    return false;
  }
  
  return (memcmp(haystack + haystack_length - needle_length, needle, 
                  needle_length) == 0);
}

inline bool ends_with(const str::ref &r1, const str::ref &r2) {
  return ends_with(r1.ptr(), r1.length(), r2.ptr(), r2.length());
}

inline bool ends_with(const std::string &s1, const std::string &s2) {
  return ends_with(s1.c_str(), s1.length(), s2.c_str(), s2.length());
}


/// Case-insensitive ends_with.
inline bool iends_with(const char *haystack, size_t haystack_length,
             const char *needle, size_t needle_length) {
  if (needle_length > haystack_length) {
    return false;
  }
    
  return (icmp(haystack + haystack_length - needle_length, 
                needle, needle_length) == 0);
}

inline bool iends_with(const str::ref &r1, const str::ref &r2) {
  return iends_with(r1.ptr(), r1.length(), r2.ptr(), r2.length());
}


/// Does a string contain another string?
inline bool contains(const char *haystack, size_t haystack_length,
                      const char *needle, size_t needle_length) {
  while (needle_length <= haystack_length) {
    if (starts_with(haystack, haystack_length, needle, needle_length)) {
      return true;
    }
    
    ++haystack;
    --haystack_length;
  }
  
  return false;
}

inline bool contains(const str::ref &r1, const str::ref &r2) {
  return contains(r1.ptr(), r1.length(), r2.ptr(), r2.length());
}

/// Case-insensitive contains.
inline bool icontains(const char *haystack, size_t haystack_length,
                      const char *needle, size_t needle_length) {
  while (needle_length <= haystack_length) {
    if (istarts_with(haystack, haystack_length, needle, needle_length)) {
      return true;
    }
    
    ++haystack;
    --haystack_length;
  }
  
  return false;
}

inline bool icontains(const str::ref &r1, const str::ref &r2) {
  return icontains(r1.ptr(), r1.length(), r2.ptr(), r2.length());
}



std::vector<std::string> argv_to_string_vector(int argc, const char **argv) {
  std::vector<std::string> result;
  int i;
  for (i = 0; i < argc; ++i) {
    result.push_back(std::string(argv[i]));
  }

  return result;
}


std::string implode(const std::vector<std::string> &a, const std::string &in_between) {
  std::string result;

  std::vector<std::string>::const_iterator i = a.begin();
  std::vector<std::string>::const_iterator iz = a.end();

  if (i == iz) {
    return result;
  }

  while (true) {
    result.append(*i);  
    ++i;
    if (i >= iz) {
      return result;
    }

    result.append(in_between);
  }
  
  NP1_ASSERT(false, "Unreachable");
}




template <typename Text_Input_Stream, typename Text_Output_Stream>
bool read_quoted_string(Text_Input_Stream &input, Text_Output_Stream &output) {
  bool seen_slash = false;
  int c = 0;

  int quote_char = input.read();
  if ((quote_char != '\'') && (quote_char != '\"')) {
    return false;
  }

  while ((c = input.read()) > 0) {
    if ('\\' == c) {
      if (!seen_slash) {
        seen_slash = true;
      } else {
        // We've just seen a '\\'.
        if (!output.write((char)c)) {
          return false;
        }
        seen_slash = false;
      }
    } else if ((quote_char == c) && !seen_slash) {
      return true;
    } else {
      // If we saw a slash and the next thing is not a known escape char
      // then just write out the slash and the next thing as-is, rather than
      // just eating the slash or reporting 'invalid escape'.
      if (seen_slash) {
        switch (c) {
        case 'n':
          c = '\n';
          break;

        case 'r':
          c = '\r';
          break;

        case 't':
          c = '\t';
          break;

        //TODO: more escapes?

        default:
          if (quote_char != c) {
            output.write('\\');
          }
        }
      }

      seen_slash = false;
      if (!output.write((char)c)) {
        return false;
      }
    }
  }

  return false;
}




template <typename Text_Output_Stream>
void write_bash_escaped_string(const std::string &s, Text_Output_Stream &output) {
  const char *p = s.c_str();
  const char *end = p + s.length();

  for (; p < end; ++p) {
    if (((*p >= 2) && (*p <= 26))
        || ('\\' == *p)
        || ('#' == *p)
        || ('?' == *p)
        || ('`' == *p)
        || ('(' == *p)
        || (')' == *p)
        || ('*' == *p)
        || ('>' == *p)
        || ('<' == *p)
        || ('~' == *p)
        || ('|' == *p)
        || (';' == *p)
        || (' ' == *p)
        || ('"' == *p)
        || ('!' == *p)
        || ('$' == *p)
        || ('&' == *p)
        || ('\'' == *p)) {
      output.write('\\');
      output.write(*p);
    } else {
      output.write(*p);
    }
  }
}


template <typename Text_Output_Stream>
void write_hex_dump(const unsigned char *start, const unsigned char *end, Text_Output_Stream &output) {
  const unsigned char *p;
  // Write out the hex representation.
  for (p = start; p < end; ++p) {
    char num_str[4];
    num_str[2] = ' ';
    num_str[3] = '\0';
    detail::hex_encode_2(num_str, *p);
    output.write(num_str);
  }
  
  output.write('\n');
  
  // Write out a 'safe' literal representation.
  for (p = start; p < end; ++p) {
    if ((*p >= 32) && (*p <= 126)) {
      output.write((char)*p);
    } else {
      output.write('.');
    }
  }
}

namespace detail {
  struct basic_string_output_stream {
    void write(const char *s) { m_s.append(s); }
    void write(char c) { m_s.push_back(c); }
    std::string m_s;
  };  
} // namespace detail

std::string get_as_hex_dump(const unsigned char *start, const unsigned char *end) {
  detail::basic_string_output_stream bsos;
  write_hex_dump(start, end, bsos);
  return bsos.m_s;
}

std::string get_as_hex_dump(const char *start, const char *end) {
  return get_as_hex_dump((const unsigned char *)start, (const unsigned char *)end);
}


/**************************************************************************
 * The unicode-handling code below is a MODIFIED copy of some functions from
 * http://www.unicode.org/Public/PROGRAMS/CVTUTF/ConvertUTF.c
 * 
 */

/*
 * Copyright 2001-2004 Unicode, Inc.
 * 
 * Disclaimer
 * 
 * This source code is provided as is by Unicode, Inc. No claims are
 * made as to fitness for any particular purpose. No warranties of any
 * kind are expressed or implied. The recipient agrees to determine
 * applicability of information provided. If this file has been
 * purchased on magnetic or optical media from Unicode, Inc., the
 * sole remedy for any claim will be exchange of defective media
 * within 90 days of receipt.
 * 
 * Limitations on Rights to Redistribute This Code
 * 
 * Unicode, Inc. hereby grants the right to freely use the information
 * supplied in this file in the creation of products supporting the
 * Unicode Standard, and to make copies of this file in any form
 * for internal or external distribution as long as this notice
 * remains attached.
 */



/*
 * Utility routine to tell whether a sequence of bytes is legal UTF-8.
 * This must be called with the length pre-determined by the first byte.
 * The length can be set by:
 *  length = trailing_utf8_bytes[*source]+1;
 * and the sequence is illegal right away if there aren't that many bytes
 * available.
 * If presented with a length > 4, this returns false.  The Unicode
 * definition of UTF-8 goes up to 4-byte sequences.
 */
static bool is_legal_utf8_char_sequence(const unsigned char *source, int length) {
    unsigned char a;
    const unsigned char *srcptr = source+length;
    switch (length) {
    default: return false;
  /* Everything else falls through when "true"... */
    case 4: if ((a = (*--srcptr)) < 0x80 || a > 0xBF) return false;
    case 3: if ((a = (*--srcptr)) < 0x80 || a > 0xBF) return false;
    case 2: if ((a = (*--srcptr)) > 0xBF) return false;

  switch (*source) {
      /* no fall-through in this inner switch */
      case 0xE0: if (a < 0xA0) return false; break;
      case 0xED: if (a > 0x9F) return false; break;
      case 0xF0: if (a < 0x90) return false; break;
      case 0xF4: if (a > 0x8F) return false; break;
      default:   if (a < 0x80) return false;
  }

    case 1: if (*source >= 0x80 && *source < 0xC2) return false;
    }
    if (*source > 0xF4) return false;
    return true;
}


// Given a first byte, calculate the number of trailing bytes.
unsigned short number_utf8_trailing_bytes(unsigned char first_byte) {
  /*
   * Index into the table below with the first byte of a UTF-8 sequence to
   * get the number of trailing bytes that are supposed to follow it.
   * Note that *legal* UTF-8 values can't have 4 or 5-bytes. The table is
   * left as-is for anyone who may want to do such conversion, which was
   * allowed in earlier algorithms.
   */
  static const char trailing_utf8_bytes[256] = {
      0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
      0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
      0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
      0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
      0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
      0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
      1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
      2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, 3,3,3,3,3,3,3,3,4,4,4,4,5,5,5,5
  };
  
  return trailing_utf8_bytes[first_byte];
}


// Returns true if the string is a 7-bit ASCII string.
bool is_7bit_ascii(const unsigned char *s, size_t n) {
  const unsigned char *p = s;
  const unsigned char *end = s + n;
  
  for (; p < end; ++p) {
    if (!(*p) || (*p > 0x7f)) {
      return false;
    }
  }
  
  return true;
}


// Check that the string is a valid UTF-8 string.
bool is_valid_utf8(const char *s, size_t n) {
  const unsigned char *p = (const unsigned char *)s;

  if (is_7bit_ascii(p, n)) {
    return true;
  }

  const unsigned char *end = p + n;
  unsigned short char_len = 0;  

  for (; p < end; p += char_len) {
    char_len = number_utf8_trailing_bytes(*p) + 1;
    if (!(*p) || (char_len + p > end) || !is_legal_utf8_char_sequence(p, char_len)) {
      return false;
    }
  }

  return true;
}

bool is_valid_utf8(const ref &r) {
  return is_valid_utf8(r.ptr(), r.length());
}

bool is_valid_utf8(const char *s) {
  return is_valid_utf8(s, strlen(s));
}

bool is_valid_utf8(const std::string &s) {
  return is_valid_utf8(s.c_str(), s.length());
}

bool is_valid_utf8(int argc, const char **argv) {
  int i;
  for (i = 0; i < argc; ++i) {
    if (!is_valid_utf8(argv[i])) {
      return false;
    }
  }

  return true;
}

// Replace any invalid UTF-8 character sequences with a replacement character.
// the length of the string is not affected.
void replace_invalid_utf8_sequences(char *s, size_t n, char replacement_char) {
  unsigned char *p = (unsigned char *)s;
  if (is_7bit_ascii(p, n)) {
    return;
  }
  
  unsigned char *end = p + n;
  unsigned short char_len = 0;  

  for (; p < end; p += char_len) {
    char_len = number_utf8_trailing_bytes(*p) + 1;
    if (!(*p) || (char_len + p > end) || !is_legal_utf8_char_sequence(p, char_len)) {
      unsigned char *seq_end = p + char_len;
      if (seq_end > end) {
        seq_end = end;      
      }
      unsigned char *seq_p = p;
      for (; seq_p < seq_end; ++seq_p) {
        *seq_p = replacement_char;
      }
    }
  }
}


template <typename Input_Stream, typename Output_Stream>
bool convert_utf16_to_utf8(Input_Stream &input, Output_Stream &output) {    
  static const int half_shift = 10; /* used for shifting by 10 bits */
  
  static const uint32_t half_base = 0x0010000UL;

  static const uint32_t UNI_SUR_HIGH_START = 0xD800;
  static const uint32_t UNI_SUR_HIGH_END = 0xDBFF;
  static const uint32_t UNI_SUR_LOW_START = 0xDC00;
  static const uint32_t UNI_SUR_LOW_END = 0xDFFF;
  static const uint32_t UNI_REPLACEMENT_CHAR = 0x0000FFFD;

  static const uint8_t first_byte_mark[7] = { 0x00, 0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC };


  int u16;
  while ((u16 = input.read_uint16()) >= 0) {
    uint32_t ch = u16;
    unsigned short bytes_to_write = 0;
    const uint32_t byte_mask = 0xBF;
    const uint32_t byte_mark = 0x80; 
    /* If we have a surrogate pair, convert to UTF32 first. */
    if (ch >= UNI_SUR_HIGH_START && ch <= UNI_SUR_HIGH_END) {
      /* Try to get the 16 bits following the high surrogate. */
      int u16_2 = input.read_uint16();
      if (u16_2 < 0) {
        // Invalid stream.
        return false;
      }

      uint32_t ch2 = u16_2;
      /* If it's a low surrogate, convert to UTF32. */
      if (ch2 >= UNI_SUR_LOW_START && ch2 <= UNI_SUR_LOW_END) {
          ch = ((ch - UNI_SUR_HIGH_START) << half_shift) + (ch2 - UNI_SUR_LOW_START) + half_base;
      } else { /* it's an unpaired high surrogate */
          return false;
      }
    } else {
      /* UTF-16 surrogate values are illegal in UTF-32 */
      if (ch >= UNI_SUR_LOW_START && ch <= UNI_SUR_LOW_END) {
        return false;
      }
    }

    /* Figure out how many bytes the result will require */
    if (ch < (uint32_t)0x80) {
      bytes_to_write = 1;
    } else if (ch < (uint32_t)0x800) {
      bytes_to_write = 2;
    } else if (ch < (uint32_t)0x10000) {
      bytes_to_write = 3;
    } else if (ch < (uint32_t)0x110000) {
      bytes_to_write = 4;
    } else {
      bytes_to_write = 3;
      ch = UNI_REPLACEMENT_CHAR;
    }

    char target_buffer[5];
    char *target = target_buffer + bytes_to_write;
    switch (bytes_to_write) { /* note: everything falls through. */
        case 4: *--target = (uint8_t)((ch | byte_mark) & byte_mask); ch >>= 6;
        case 3: *--target = (uint8_t)((ch | byte_mark) & byte_mask); ch >>= 6;
        case 2: *--target = (uint8_t)((ch | byte_mark) & byte_mask); ch >>= 6;
        case 1: *--target =  (uint8_t)(ch | first_byte_mark[bytes_to_write]);
    }
    
    output.write(target_buffer, bytes_to_write);
  }

  return true;
}



} // namespaces
}


#endif
