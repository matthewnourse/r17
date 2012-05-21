// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_IO_STATIC_BUFFER_OUTPUT_STREAM_HPP
#define NP1_IO_STATIC_BUFFER_OUTPUT_STREAM_HPP


#include "np1/io/ext_static_buffer_output_stream.hpp"


namespace np1 {
namespace io {

/// A statically-allocated buffer that looks like a stream.
template <size_t Length>
class static_buffer_output_stream
  : public detail::stream_helper<static_buffer_output_stream<Length> > {
private:
  // It makes no sense to buffer this stream so is_unbuffered is not defined.

public:
  /// Constructor.
  static_buffer_output_stream() 
    : detail::stream_helper<static_buffer_output_stream<Length> >(*this),
      m_stream(m_buffer, Length) {}
  
  /// Destructor.
  ~static_buffer_output_stream() {}
  

  inline bool soft_flush() { return true; }
  inline bool hard_flush() { return true; }

  /// Write a buffer, returns false on error.
  bool write_some(const void *buf, size_t bytes_to_write, size_t *bytes_written_p) {
    return m_stream.write_some(buf, bytes_to_write, bytes_written_p);
  }

  /// Close the stream.
  bool close() { return true; }

  /// Set the buffer pointer back to the beginning.
  void reset() { m_stream.reset(); }

  /// Get a pointer to the buffer.
  const unsigned char *ptr() const { return m_stream.ptr(); }

  /// Set the last char in the buffer.
  void set_last_char(char c) { m_stream.set_last_char(c); }

  /// Get the length of the data in the buffer.
  size_t size() const { return m_stream.size(); }

  const rstd::string &name() const {
    static rstd::string n("[static buffer output]");
    return n;
  }


private:
  /// Disable copy.
  static_buffer_output_stream(const static_buffer_output_stream &);
  static_buffer_output_stream &operator = (const static_buffer_output_stream &);
      
private:
  unsigned char m_buffer[Length];
  ext_static_buffer_output_stream m_stream;
};


} // namespaces
}


#endif
