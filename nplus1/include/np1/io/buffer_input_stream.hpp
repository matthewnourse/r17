// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_IO_BUFFER_INPUT_STREAM_HPP
#define NP1_IO_BUFFER_INPUT_STREAM_HPP


namespace np1 {
namespace io {

/// Code to treat a buffer as an input stream.
class buffer_input_stream : public detail::stream_helper<buffer_input_stream> {
public:
  // Wrapping classes can use this to ensure that the thing they are wrapping
  // is unbuffered.
  struct is_unbuffered {};

public:
  explicit buffer_input_stream(const unsigned char *p, size_t sz)
    : detail::stream_helper<buffer_input_stream>(*this), m_buffer(p), m_ptr(p), m_size(sz), m_remaining_size(sz) {}

  /**
   * Returns false and sets *bytes_read_p=0 on error.  Returns true
   * and sets *bytes_read_p=0 on EOF.
   */
  bool read_some(void *buf, size_t bytes_to_read, size_t *bytes_read_p) {
    if (bytes_to_read > m_remaining_size) {
      bytes_to_read = m_remaining_size;  
    }

    memcpy(buf, m_ptr, bytes_to_read);
    m_ptr += bytes_to_read;
    m_remaining_size -= bytes_to_read;    
    *bytes_read_p = bytes_to_read;
    return true;
  }

  bool close() { return true; }

  bool rewind() {
    m_ptr = m_buffer;
    m_remaining_size = m_size;
    return true;
  }

  const std::string &name() const {
    static std::string n("[buffer input]");
    return n;
  }


private:
  /// Disable copy.
  buffer_input_stream(const buffer_input_stream &);
  buffer_input_stream &operator = (const buffer_input_stream &);

private:
  const unsigned char *m_buffer;
  const unsigned char *m_ptr;
  size_t m_size;
  size_t m_remaining_size;
};


} // namespaces
}

#endif
