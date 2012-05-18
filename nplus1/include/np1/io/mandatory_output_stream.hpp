// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_IO_MANDATORY_OUTPUT_STREAM_HPP
#define NP1_IO_MANDATORY_OUTPUT_STREAM_HPP

#include "np1/io/file.hpp"
#include "np1/assert.hpp"
#include "np1/io/buffered_output_stream.hpp"

namespace np1 {
namespace io {

/// A buffered stream where any error crashes the process.
template <typename Inner_Stream>
class mandatory_output_stream {
public:
  /// Constructor.
  explicit mandatory_output_stream(Inner_Stream &s) : m_stream(s) {}
  
  /// Destructor.
  ~mandatory_output_stream() {}
  

  inline void soft_flush() {
    NP1_ASSERT(m_stream.soft_flush(), "Stream " + m_stream.name() + ": soft flush failed");
  }

  inline void hard_flush() {
    NP1_ASSERT(m_stream.hard_flush(), "Stream " + m_stream.name() + ": hard flush failed");
  }

  inline void write(const void *buf, size_t length) {
    NP1_ASSERT(m_stream.write(buf, length), "Stream " + m_stream.name() + ": write buf failed");
  }

  inline void write(char c) {
    NP1_ASSERT(m_stream.write(c), "Stream " + m_stream.name() + ": write char failed");
  }

  inline void write(const char *s) {
    NP1_ASSERT(m_stream.write(s), "Stream " + m_stream.name() + ": write char failed");
  }

  bool close() {
    NP1_ASSERT(m_stream.close(), "Stream " + m_stream.name() + ": close() failed");
    return true;
  }
 
  bool is_open() const { return m_stream.is_open(); }

private:
  /// Disable copy.
  mandatory_output_stream(const mandatory_output_stream &);
  mandatory_output_stream &operator = (const mandatory_output_stream &);
    
  
private:
  Inner_Stream &m_stream;
};


} // namespaces
}


#endif
