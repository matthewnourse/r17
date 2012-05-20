// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_IO_EXT_HEAP_BUFFER_OUTPUT_STREAM_HPP
#define NP1_IO_EXT_HEAP_BUFFER_OUTPUT_STREAM_HPP


namespace np1 {
namespace io {

/// A buffer that looks like a stream, where the heap
/// is supplied by the caller ('external').
template <typename Heap>
class ext_heap_buffer_output_stream
  : public detail::stream_helper<ext_heap_buffer_output_stream<Heap> > {
private:
  // It makes no sense to buffer this stream so is_unbuffered is not defined.

public:
  // Constructor.  allocation_size is the initial allocation and also the
  // amount by which the buffer will be grown.
  ext_heap_buffer_output_stream(Heap &heap, size_t allocation_size)
  : detail::stream_helper<ext_heap_buffer_output_stream<Heap> >(*this)
  , m_heap(heap)
  , m_allocation_size(allocation_size)
  , m_buffer((unsigned char *)m_heap.alloc(m_allocation_size))
  , m_buffer_pos(m_buffer)
  , m_buffer_end(m_buffer_pos + m_allocation_size) {}
  
  /// Destructor.
  ~ext_heap_buffer_output_stream() {
    m_heap.free((char *)m_buffer);
  }
  

  inline bool soft_flush() { return true; }
  inline bool hard_flush() { return true; }


  /// Write a buffer, returns false on error.
  bool write_some(const void *buf, size_t bytes_to_write, size_t *bytes_written_p) {
    NP1_ASSERT(m_buffer_pos <= m_buffer_end, "existing buffer_pos is past buffer_end!");

    unsigned char *new_pos = m_buffer_pos + bytes_to_write;   
    if (new_pos > m_buffer_end) {
      size_t current_size = m_buffer_end - m_buffer;
      size_t new_size = current_size + m_allocation_size;
      if (new_pos > m_buffer + new_size) {
        new_size = current_size + bytes_to_write;
      }

      size_t current_used_size = m_buffer_pos - m_buffer;
      NP1_ASSERT(new_size >= current_used_size, "new buffer is not big enough to hold even the existing data!");
      NP1_ASSERT(new_size >= current_used_size + bytes_to_write,
                  "new buffer is not big enough to hold the existing data plus the new data!");
      unsigned char *new_buffer = (unsigned char *)m_heap.alloc(new_size);
      if (new_buffer > m_buffer) {
        NP1_ASSERT(m_buffer + current_used_size <= new_buffer, "new buffer is not allocated at the correct address!");
      } else {
        NP1_ASSERT(new_buffer + current_used_size <= m_buffer, "new buffer is not big enough or not allocated at the correct address!");
      }

      memcpy(new_buffer, m_buffer, current_used_size);
      m_buffer_pos = new_buffer + current_used_size;
      m_buffer_end = new_buffer + new_size;
      m_heap.free((char *)m_buffer);
      m_buffer = new_buffer;
      new_pos = m_buffer_pos + bytes_to_write;
    }

    NP1_ASSERT(new_pos <= m_buffer_end, "new_pos is past buffer_end!");
    NP1_ASSERT(m_buffer_pos < m_buffer_end, "buffer_pos is past or equal to buffer_end after possible resize");
    memcpy(m_buffer_pos, buf, bytes_to_write);
    m_buffer_pos = new_pos;
    *bytes_written_p = bytes_to_write;
    return true;
  }

  /// Close the stream.
  bool close() { return true; }

  /// Set the buffer pointer back to the beginning.
  void reset() { m_buffer_pos = m_buffer; }

  /// Get a pointer to the buffer.
  unsigned char *ptr() const { return m_buffer; }  

  /// Set the last char in the buffer.
  void set_last_char(char c) {
    if (m_buffer_end > m_buffer) {
      *(m_buffer_end-1) = c;
    }
  }

  /// Release ownership of the buffer.  This will render this object unable to write any more.
  void release() {
    m_buffer = m_buffer_pos = m_buffer_end = 0;
  }

  /// Get the length of the data in the buffer.
  size_t size() const { return m_buffer_pos - m_buffer; }

  const std::string &name() const {
    static std::string n("[ext heap buffer output stream]");
    return n;
  }

private:
  /// Disable copy.
  ext_heap_buffer_output_stream(const ext_heap_buffer_output_stream &);
  ext_heap_buffer_output_stream &operator = (const ext_heap_buffer_output_stream &);
      
private:
  Heap &m_heap;
  size_t m_allocation_size;
  unsigned char *m_buffer;
  unsigned char *m_buffer_pos;
  unsigned char *m_buffer_end;  
};


} // namespaces
}


#endif
