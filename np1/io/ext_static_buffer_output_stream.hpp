// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_IO_EXT_STATIC_BUFFER_OUTPUT_STREAM_HPP
#define NP1_IO_EXT_STATIC_BUFFER_OUTPUT_STREAM_HPP

#include "np1/io/detail/stream_helper.hpp"

namespace np1 {
namespace io {

/// A statically-allocated buffer that looks like a stream, where the buffer
/// is supplied by the caller ('external').
class ext_static_buffer_output_stream
  : public detail::stream_helper<ext_static_buffer_output_stream> {
private:
  // It makes no sense to buffer this stream so is_unbuffered is not defined.

public:
  /// Constructor.
  ext_static_buffer_output_stream(unsigned char *buffer, size_t size)
  : detail::stream_helper<ext_static_buffer_output_stream>(*this)
  , m_buffer(buffer)
  , m_buffer_pos(&m_buffer[0])
  , m_buffer_end(m_buffer_pos + size) {}
  
  /// Destructor.
  ~ext_static_buffer_output_stream() {}
  

  inline bool soft_flush() { return true; }
  inline bool hard_flush() { return true; }


  /// Write a buffer, returns false on error.
  bool write_some(const void *buf, size_t bytes_to_write, size_t *bytes_written_p) {
    unsigned char *new_pos = m_buffer_pos + bytes_to_write;   
    if (new_pos <= m_buffer_end) {
      memcpy(m_buffer_pos, buf, bytes_to_write);
      m_buffer_pos = new_pos;
      *bytes_written_p = bytes_to_write;
      return true;
    }
    
    *bytes_written_p = 0;
    return false;  
  }

  /// Close the stream.
  bool close() { return true; }

  /// Set the buffer pointer back to the beginning.
  void reset() { m_buffer_pos = m_buffer; }

  /// Get a pointer to the buffer.
  const unsigned char *ptr() const { return m_buffer; }

  /// Set the last char in the buffer.
  void set_last_char(char c) {
    if (m_buffer_end > m_buffer) {
      *(m_buffer_end-1) = c;
    }
  }


  /// Get the length of the data in the buffer.
  size_t size() const { return m_buffer_pos - m_buffer; }

  const rstd::string &name() const {
    static rstd::string n("[static buffer output stream]");
    return n;
  }

private:
  /// Disable copy.
  ext_static_buffer_output_stream(const ext_static_buffer_output_stream &);
  ext_static_buffer_output_stream &operator = (const ext_static_buffer_output_stream &);
      
private:
  unsigned char *m_buffer;
  unsigned char *m_buffer_pos;
  unsigned char *m_buffer_end;  
};


} // namespaces
}


#endif
