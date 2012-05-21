// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_IO_STRING_INPUT_STREAM_HPP
#define NP1_IO_STRING_INPUT_STREAM_HPP


#include "np1/io/buffer_input_stream.hpp"


namespace np1 {
namespace io {

/// Code to treat a string as an input stream.
class string_input_stream : public detail::stream_helper<string_input_stream> {
public:
  // Wrapping classes can use this to ensure that the thing they are wrapping
  // is unbuffered.
  struct is_unbuffered {};

public:
  explicit string_input_stream(const rstd::string &s)
    : detail::stream_helper<string_input_stream>(*this), m_stream((const unsigned char *)s.c_str(), s.length()) {}

  /**
   * Returns false and sets *bytes_read_p=0 on error.  Returns true
   * and sets *bytes_read_p=0 on EOF.
   */
  bool read_some(void *buf, size_t bytes_to_read, size_t *bytes_read_p) {
    return m_stream.read_some(buf, bytes_to_read, bytes_read_p);
  }

  bool close() { return m_stream.close(); }

  bool rewind() { return m_stream.rewind(); }

  const rstd::string &name() const {
    static rstd::string n("[string input]");
    return n;
  }


private:
  /// Disable copy.
  string_input_stream(const string_input_stream &);
  string_input_stream &operator = (const string_input_stream &);

private:
  buffer_input_stream m_stream;  
};


} // namespaces
}

#endif
