// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_IO_DETAIL_STREAM_HELPER_HPP
#define NP1_IO_DETAIL_STREAM_HELPER_HPP


#include "np1/io/unbuffered_stream_base.hpp"

namespace np1 {
namespace io {
namespace detail {

/// Some niceties wrapped around a lower-level object.
/**
 * These methods are here rather than in stream_base to avoid the virtual
 * function call overhead where possible.
 */
template <typename Inner_Stream>
class stream_helper : public unbuffered_stream_base {
public:
  explicit stream_helper(Inner_Stream &inner_stream) : m_stream(inner_stream) {}
  ~stream_helper() {}

  /// Reads as much as possible from the file, up to bytes_to_read.
  bool read(void *buf, size_t bytes_to_read, size_t *bytes_read_p) {
    unsigned char *buf_start = (unsigned char *)buf;
    unsigned char *buf_p = buf_start;
    unsigned char *buf_end = buf_p + bytes_to_read;
    size_t bytes_read = 1;

    while ((buf_p < buf_end) && (bytes_read > 0)) {
      if (!m_stream.read_some(buf_p, buf_end - buf_p, &bytes_read)) {
        return false;
      }

      buf_p += bytes_read;
    }

    *bytes_read_p = buf_p - buf_start;
    return true;
  }

  /// Writes all the buffer or returns false. 
  bool write(const void *buf, size_t bytes_to_write) {
    unsigned char *p = (unsigned char *)buf;
    unsigned char *end = p + bytes_to_write;
  
    while (p < end) {
      size_t bytes_written;
      if (!m_stream.write_some(p, end - p, &bytes_written)) {
        return false;
      }
  
      p += bytes_written;
    }
  
    return true;
  }

  /// Write a complete character string or return false.
  bool write(const char *str) {
    return write(str, strlen(str));
  }

  /// Write a character.  Avoid- it's slow.
  bool write(char c) {
    return write(&c, 1);  
  }


private:
  /// Disable copy.
  stream_helper(const stream_helper &);
  stream_helper &operator = (const stream_helper &);

private:
  Inner_Stream &m_stream;
};


} // namespaces
}
}


#endif
