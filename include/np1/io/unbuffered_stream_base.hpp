// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_IO_UNBUFFERED_STREAM_BASE_HPP
#define NP1_IO_UNBUFFERED_STREAM_BASE_HPP


#include <string>

namespace np1 {
namespace io {

// The virtual base class for all unbuffered streams.  This should only be used
// where it's impossible to use a concrete class or a template, because virtual
// function calls are expensive.
class unbuffered_stream_base {
public:
  // Wrapping classes can use this to ensure that the thing they are wrapping
  // is unbuffered.
  struct is_unbuffered {};

public:
  /// Reads as much as possible from the file, up to bytes_to_read.
  virtual bool read(void *buf, size_t bytes_to_read, size_t *bytes_read_p) {
    NP1_ASSERT(false, "read not implemented!");
    return false;
  }

  /**
   * Returns false and sets *bytes_read_p=0 on error.  Returns true
   * and sets *bytes_read_p=0 on EOF.
   */
  virtual bool read_some(void *buf, size_t bytes_to_read, size_t *bytes_read_p) {
    NP1_ASSERT(false, "read_some not implemented!");
    return false;
  }

  /// Writes all the buffer or returns false. 
  virtual bool write(const void *buf, size_t bytes_to_write) {
    NP1_ASSERT(false, "write not implemented!");
    return false;
  }

  /// Returns false and sets *bytes_written_p=0 on error. 
  virtual bool write_some(const void *buf, size_t bytes_to_write, size_t *bytes_written_p) {
    NP1_ASSERT(false, "write_some not implemented!");
    return false;
  }

  /// Write a complete character string or return false.
  virtual bool write(const char *str) {
    NP1_ASSERT(false, "write not implemented!");
    return false;
  }

  /// Write a character.  Avoid- it's slow.
  virtual bool write(char c) {
    NP1_ASSERT(false, "write not implemented!");
    return false;
  }

  virtual int handle() {
    NP1_ASSERT(false, "handle() not implemented!");
    return -1;
  }

  virtual bool close() { return true; }  

  virtual const std::string &name() const {
    NP1_ASSERT(false, "name not implemented!");
    static std::string n("[infinite sadness]");
    return n;
  }

protected:
  unbuffered_stream_base() {}

private:
  /// Disable copy.
  unbuffered_stream_base(const unbuffered_stream_base &);
  unbuffered_stream_base &operator = (const unbuffered_stream_base &);

private:
};


} // namespaces
}


#endif
