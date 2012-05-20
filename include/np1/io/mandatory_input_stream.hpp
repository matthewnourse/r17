// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_IO_MANDATORY_INPUT_STREAM_HPP
#define NP1_IO_MANDATORY_INPUT_STREAM_HPP



#include "np1/simple_types.hpp"


namespace np1 {
namespace io {

/// An input stream where any error results in a process crash.
template <typename Inner_Stream>
class mandatory_input_stream {
public:
  mandatory_input_stream(Inner_Stream &s) : m_stream(s) {}
  ~mandatory_input_stream() {}

  /// Returns 0 on EOF.
  size_t read_some(void *buf, size_t bytes_to_read) {
    size_t bytes_read;
    if (!m_stream.read_some(buf, bytes_to_read, &bytes_read)) {
      NP1_ASSERT(false, "Stream " + m_stream.name() + ": Unable to read from stream");
    }

    return bytes_read;
  }

  size_t read(void *buf, size_t bytes_to_read) {
    size_t bytes_read = 0;
    if (!m_stream.read(buf, bytes_to_read, &bytes_read)) {
      NP1_ASSERT(false, "Stream " + m_stream.name() + ": Unable to read from stream");
    }

    return bytes_read;
  }


  bool close() {
    NP1_ASSERT(m_stream.close(), "Stream " + m_stream.name() + ": close() failed");
    return true;
  }

  /// Assumes that the output stream is also a mandatory stream.
  template <typename Output_Stream>
  void copy(Output_Stream &output) {
    char buffer[256 * 1024];
    size_t number_bytes_read;
    while ((number_bytes_read = read(buffer, sizeof(buffer))) > 0) {
      output.write(buffer, number_bytes_read);
    }
  }

  void copy_overwrite(const std::string &file_name) {
    copy(file_name, true);
  }

  void copy_append(const std::string &file_name) {
    copy(file_name, false);
  }

  const std::string &name() const { return m_stream.name(); }

private:
  /// Disable copy.
  mandatory_input_stream(const mandatory_input_stream &);
  mandatory_input_stream &operator = (const mandatory_input_stream &);

private:
  void copy(const std::string &file_name, bool overwrite) {
    io::file file;    
    const char *name = file_name.c_str();
    bool open_result =
          overwrite ? file.create_or_open_wo_trunc(name)
                    : file.create_or_open_wo_append(name);

    NP1_ASSERT(open_result, "Unable to open output file " + file_name);
    mandatory_output_stream<io::file> mandatory_output(file);
    copy(mandatory_output);
  }


private:
  Inner_Stream &m_stream;
};


} // namespaces
}


#endif
