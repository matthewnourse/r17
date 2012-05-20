// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_IO_BUFFERED_INPUT_STREAM_HPP
#define NP1_IO_BUFFERED_INPUT_STREAM_HPP


namespace np1 {
namespace io {


/// Input buffering wrapped around a lower-level object.
template <typename Inner_Stream>
class buffered_input_stream {
private:
  // Ensure that the inner stream is unbuffered.
  typedef typename Inner_Stream::is_unbuffered inner_is_unbuffered_type;

public:
  enum { BUFFER_SIZE = 16 * 1024 };

public:
  explicit buffered_input_stream(Inner_Stream &inner_stream)
    : m_buffer_p(m_buffer), m_buffer_end(m_buffer_p), m_stream(inner_stream) {}
  ~buffered_input_stream() { close(); }

  /// Read one byte, returns <0 on EOF or error.
  int read() {
    if (m_buffer_p >= m_buffer_end) {
      size_t bytes_read;
      if ((!m_stream.read(m_buffer, sizeof(m_buffer), &bytes_read))
          || (0 == bytes_read)) {
        return -1;  
      }

      m_buffer_p = m_buffer;      
      m_buffer_end = m_buffer_p + bytes_read; 
    }

    return *m_buffer_p++;
  }

  //TODO: other functions for reading.

  /// Close the file handle, noop if file handle is invalid or a std handle.
  bool close() {
    return m_stream.close();
  }

  const std::string &name() const { return m_stream.name(); }


private:
  /// Disable copy.
  buffered_input_stream(const buffered_input_stream &);
  buffered_input_stream &operator = (const buffered_input_stream &);

private:
  unsigned char m_buffer[BUFFER_SIZE];
  unsigned char *m_buffer_p;
  unsigned char *m_buffer_end;
  Inner_Stream &m_stream;
};


} // namespaces
}

#endif
