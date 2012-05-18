// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_IO_UTF16_INPUT_STREAM_HPP
#define NP1_IO_UTF16_INPUT_STREAM_HPP


namespace np1 {
namespace io {


/// A UTF-16 input stream.  The first two bytes must contain the byte order mark.
template <typename Inner_Stream>
class utf16_input_stream {
private:
  // Ensure that the inner stream is unbuffered.
  typedef typename Inner_Stream::is_unbuffered inner_is_unbuffered_type;

public:
  enum { BUFFER_SIZE = 16 * 1024 };

public:
  explicit utf16_input_stream(Inner_Stream &inner_stream)
    : m_stream(inner_stream), m_is_little_endian(is_little_endian(m_stream)) {}
  ~utf16_input_stream() { close(); }

  /// Read two bytes, returns <0 on EOF or error.
  int read_uint16() {
    NP1_PREPROC_STATIC_ASSERT(sizeof(int) > sizeof(uint16_t));
    int first_byte = m_stream.read();
    int second_byte = m_stream.read();

    if ((first_byte < 0) || (second_byte < 0)) {
      return -1;
    }

    if (m_is_little_endian) {
      return (second_byte << 8) | first_byte;    
    }
    
    return (first_byte << 8) | second_byte;    
  }  

  /// Close the file handle, noop if file handle is invalid or a std handle.
  bool close() {
    return m_stream.close();
  }

  const std::string &name() const { return m_stream.name(); }


private:
  /// Disable copy.
  utf16_input_stream(const utf16_input_stream &);
  utf16_input_stream &operator = (const utf16_input_stream &);

private:
  static bool is_little_endian(io::buffered_input_stream<Inner_Stream> &stream) {
    int first_byte = stream.read();
    int second_byte = stream.read();

    if ((0xfe == first_byte) && (0xff == second_byte)) {
      return false;
    }

    if ((0xff == first_byte) && (0xfe == second_byte)) {
      return true;
    }

    NP1_ASSERT(false, "UTF-16 streams must begin with a Byte Order Mark: 0xfffe for little-endian, 0xfeff for big-endian");
    return false; // pacify compiler
  }

private:
  io::buffered_input_stream<Inner_Stream> m_stream;
  bool m_is_little_endian;
};


} // namespaces
}

#endif
