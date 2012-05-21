// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_IO_STRING_OUTPUT_STREAM_HPP
#define NP1_IO_STRING_OUTPUT_STREAM_HPP


namespace np1 {
namespace io {

/// Code to treat a string as an output stream.
class string_output_stream : public detail::stream_helper<string_output_stream> {
public:
  // Wrapping classes can use this to ensure that the thing they are wrapping
  // is unbuffered.
  struct is_unbuffered {};

public:
  explicit string_output_stream(rstd::string &s)
    : detail::stream_helper<string_output_stream>(*this), m_string(s) {}

  /// Returns false and sets *bytes_written_p=0 on error. 
  bool write_some(const void *buf, size_t bytes_to_write, size_t *bytes_written_p) {
    m_string.append((const char *)buf, bytes_to_write);
    *bytes_written_p = bytes_to_write;
    return true;
  }

  bool close() { return true; }

  const rstd::string &name() const {
    static rstd::string n("[string output]");
    return n;
  }


private:
  /// Disable copy.
  string_output_stream(const string_output_stream &);
  string_output_stream &operator = (const string_output_stream &);

private:
  rstd::string &m_string;  
};


} // namespaces
}

#endif
