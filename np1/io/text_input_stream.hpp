// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_IO_TEXT_INPUT_STREAM_HPP
#define NP1_IO_TEXT_INPUT_STREAM_HPP


#include "np1/io/buffered_input_stream.hpp"

namespace np1 {
namespace io {


/// Text input with line counting.
template <typename Unbuffered_Stream>
class text_input_stream {
private:
  // Ensure that the inner stream is unbuffered.
  typedef typename Unbuffered_Stream::is_unbuffered inner_is_unbuffered_type;

public:
  explicit text_input_stream(Unbuffered_Stream &inner)
    : m_stream(inner), m_line_number(1), m_ungotc(-1) {}
  ~text_input_stream() { close(); }

  /// Read one byte, returns <0 on EOF or error.
  int read() {
    if (m_ungotc >= 0) {
      int ungot = m_ungotc;
      m_ungotc = -1;
      return ungot;  
    }

    int c = m_stream.read();
    if ('\n' == c) {
      ++m_line_number;
    }

    return c;
  }

  // Put one byte back on the stream.  Fail if a byte is already ungot.
  void unget(int c) {
    NP1_ASSERT(m_ungotc < 0, "Unable to push a byte back on to the stream");
    m_ungotc = c;
  }

  // Read the whole file and return it as a vector of lines.
  //TODO: detect errors!
  void read_all(rstd::vector<rstd::string> &lines) {
    read_all(lines, '\n');
  }
  
  // Read the whole file and return it as a vector, using the supplied delimiter.
  void read_all(rstd::vector<rstd::string> &values, const char delim) {
    rstd::string value;
    values.clear();
    int c;
    // Stop on NUL as well as on EOF & error.
    while ((c = read()) > 0) {
      if (delim == c) {
        values.push_back(value);
        value.clear();
      } else {
        value.push_back((char)c);
      }
    }

    if (value.length() > 0) {
      values.push_back(value);
    }
  }
  

  // Read the whole file, calling a callback for each line.  This is MUCH faster
  // than read_all().  Returns false if the callback returns false.
  // The callback must have this prototype:
  // bool f(const str::ref &line, uint64_t line_number);
  template <typename Callback>
  static bool read_all_line_by_line(Unbuffered_Stream &input, Callback line_callback) {
    enum { BUFFER_SIZE = 256 * 1024 };
    unsigned char buffer[BUFFER_SIZE + 1];            
    unsigned char *buffer_end = buffer + BUFFER_SIZE;
    unsigned char *buffer_read_pos = buffer;
    const unsigned char *start_line = buffer_read_pos;
    ssize_t number_bytes_read;
    uint64_t line_number = 1;
    mandatory_input_stream<Unbuffered_Stream> mandatory_input(input);        
    const unsigned char *buffer_data_end = 0;
  
    while ((number_bytes_read =
              mandatory_input.read_some(buffer_read_pos, 
                                        buffer_end - buffer_read_pos)) > 0) {        
      buffer_data_end = buffer_read_pos + number_bytes_read;        
      const unsigned char *end_line;
      
      // Find the end of the line to make sure that we have the whole thing in the buffer. 
      while ((end_line = (const unsigned char *)memchr(start_line, '\n', buffer_data_end - start_line))) {
        // We have the whole line.  Call the callback to deal with the line.
        if (!line_callback(str::ref((const char *)start_line, end_line - start_line), line_number)) {
          return false;
        }
                        
        // Now go around again.            
        start_line = end_line + 1;
        ++line_number;
      }

      // There's probably an incomplete line left in the buffer.  Move it to the start of the buffer.
      ssize_t remainder_length = buffer_data_end - start_line; 
      NP1_ASSERT(remainder_length < BUFFER_SIZE, "Stream " + input.name()
                   + ": line is too long.  Remainder length: " + str::to_dec_str(remainder_length));
      memmove(buffer, start_line, remainder_length);
      buffer_read_pos = buffer + remainder_length;
      start_line = buffer;
    }

    return true;
  }


  //TODO: other functions for reading.

  /// Close the file handle, noop if file handle is invalid or a std handle.
  bool close() {
    return m_stream.close();
  }

  /// Get the current line number.
  size_t line_number() const { return m_line_number; }

  const rstd::string &name() const {
    static rstd::string n("[text input]");
    return n;
  }

private:
  /// Disable copy.
  text_input_stream(const text_input_stream &);
  text_input_stream &operator = (const text_input_stream &);

private:
  io::buffered_input_stream<Unbuffered_Stream> m_stream;
  size_t m_line_number;
  int m_ungotc;
};


} // namespaces
}

#endif
