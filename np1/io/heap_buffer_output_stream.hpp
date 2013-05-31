// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_IO_HEAP_BUFFER_OUTPUT_STREAM_HPP
#define NP1_IO_HEAP_BUFFER_OUTPUT_STREAM_HPP


#include "np1/io/ext_heap_buffer_output_stream.hpp"

namespace np1 {
namespace io {

/// A buffer that looks like a stream.
class heap_buffer_output_stream
  : public detail::stream_helper<heap_buffer_output_stream> {
private:
  // It makes no sense to buffer this stream so is_unbuffered is not defined.

public:
  // Constructor.  allocation_size is the initial allocation and also the
  // amount by which the buffer will be grown.
  explicit heap_buffer_output_stream(size_t allocation_size)
    : detail::stream_helper<heap_buffer_output_stream>(*this), m_stream(m_heap, allocation_size) {}
  
  /// Destructor.
  ~heap_buffer_output_stream() {}
  
  inline bool soft_flush() { return m_stream.soft_flush(); }
  inline bool hard_flush() { return m_stream.hard_flush(); }


  /// Write a buffer, returns false on error.
  bool write_some(const void *buf, size_t bytes_to_write, size_t *bytes_written_p) {
    return m_stream.write_some(buf, bytes_to_write, bytes_written_p);
  }

  /// Close the stream.
  bool close() { return m_stream.close(); }

  /// Set the buffer pointer back to the beginning.
  void reset() { m_stream.reset(); }

  /// Get a pointer to the buffer.
  unsigned char *ptr() const { return m_stream.ptr(); }

  /// Set the last char in the buffer.
  void set_last_char(char c) { m_stream.set_last_char(c); }

  /// Release ownership of the buffer.  This will render this object unable to write any more.
  void release() { m_stream.release(); }

  /// Get the length of the data in the buffer.
  size_t size() const { return m_stream.size(); }

  const rstd::string &name() const {
    static rstd::string n("[heap buffer output stream]");
    return n;
  }

private:
  /// Disable copy.
  heap_buffer_output_stream(const heap_buffer_output_stream &);
  heap_buffer_output_stream &operator = (const heap_buffer_output_stream &);

private:
  struct c_heap {
    void *alloc(size_t sz) { return ::malloc(sz); }
    void free(void *p) { ::free(p); }   
  };

private:
  c_heap m_heap;
  ext_heap_buffer_output_stream<c_heap> m_stream;
};


} // namespaces
}


#endif
