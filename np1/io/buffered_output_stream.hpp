// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_IO_BUFFERED_OUTPUT_STREAM_HPP
#define NP1_IO_BUFFERED_OUTPUT_STREAM_HPP


#include "np1/io/static_buffer_output_stream.hpp"

namespace np1 {
namespace io {

/// A simple buffering mechansim for a stream.
template <typename Inner_Stream>
class buffered_output_stream {
private:
  // Ensure that the inner stream is unbuffered.
  typedef typename Inner_Stream::is_unbuffered inner_is_unbuffered_type;

public:
  enum { BUFFER_SIZE = 16 * 1024 };

public:
  /// Constructor.
  explicit buffered_output_stream(Inner_Stream &inner) : m_stream(inner) {}
  
  /// Destructor.
  ~buffered_output_stream() { soft_flush(); }
  

  /// Flush the buffer, returns false on error.
  inline bool soft_flush() {
    if (!write_now(m_buffer.ptr(), m_buffer.size())) {
      return false;
    }

    m_buffer.reset();
    return true;
  }

  inline bool hard_flush() {
    return (soft_flush() && m_stream.hard_flush());
  }


  /// Write a buffer, returns false on error.
  inline bool write(const void *buf, size_t length) {
    /* If the string will fit in the buffer, just write it there.
     * If the string won't fit in the buffer then flush the buffer and 
     * write the string straight out to stdout as well just in case 
     * the buffer is too small for the string. */
    if (m_buffer.write(buf, length)) {
      return true;
    }
  
    if (!soft_flush()) {
      return false;
    }
    
    return write_now(buf, length);      
  }


  /// Write a character to the stream.  Use sparingly, it's inefficient.
  inline bool write(char c) { return write(&c, 1); }

  /// Write a C string to the stream.
  inline bool write(const char *s) { return write(s, strlen(s)); }

  /// Close the stream.
  bool close() {
    soft_flush();
    return m_stream.close();
  }

  bool is_open() const { return m_stream.is_open(); }

  const rstd::string &name() const { return m_stream.name(); }


private:
  /// Disable copy.
  buffered_output_stream(const buffered_output_stream &);
  buffered_output_stream &operator = (const buffered_output_stream &);

private:
  // Write a string straight out to the inner stream, bypassing the buffer.
  inline bool write_now(const void *buf, size_t length) {
    return m_stream.write(buf, length);
  }
      
private:
  Inner_Stream &m_stream;
  static_buffer_output_stream<BUFFER_SIZE> m_buffer;
  
};


} // namespaces
}


#endif
