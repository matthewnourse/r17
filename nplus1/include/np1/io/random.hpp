// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_IO_RANDOM_HPP
#define NP1_IO_RANDOM_HPP


#include "np1/io/file.hpp"
#include "np1/io/detail/stream_helper.hpp"


namespace np1 {
namespace io {

/// A random number generator that uses a pretty good source of randomness.
class random : public detail::stream_helper<random> {
public:
  // Wrapping classes can use this to ensure that the thing they are wrapping
  // is unbuffered.
  struct is_unbuffered {};

public:
  random() : detail::stream_helper<random>(*this) {
#ifndef _WIN32
    if (!m_file.open_ro("/dev/urandom")) {
      // This should only happen on Unixes without /dev/urandom (FreeBSD only?).
      NP1_ASSERT(m_file.open_ro("/dev/random"), "Unable to open source of randomness");
    }
#endif
}

  ~random() {}

  /**
   * Returns false and sets *bytes_read_p=0 on error.  Returns true
   * and sets *bytes_read_p=0 on EOF.
   */
  bool read_some(void *buf, size_t bytes_to_read, size_t *bytes_read_p) {
#ifdef _WIN32
    unsigned int val;
    NP1_ASSERT(rand_s(&val) == 0, "rand_s failed!");
    if (bytes_to_read < sizeof(val)) {
      memcpy(buf, val, bytes_to_read);
      *bytes_read_p = bytes_to_read;
    } else {
      memcpy(buf, val, sizeof(val));
      *bytes_read_p = sizeof(val);
    }
    
    return true;
#else   
    return m_file.read_some(buf, bytes_to_read, bytes_read_p) ;
#endif
  }

  const std::string &name() const {
    static std::string n("[random]");
    return n;
  }


private:
  /// Disable copy.
  random(const random &);
  random &operator = (const random &);

private:
#ifndef _WIN32
  file m_file;
#endif
};


} // namespaces
}


#endif
