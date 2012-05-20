// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_COMPRESSED_INT_HPP
#define NP1_COMPRESSED_INT_HPP

namespace np1 {

// 7-bit encoded compressed int functions.  These work well for small integers and are hopeless
// for negative numbers.
class compressed_int {
public:
  enum { MAX_COMPRESSED_INT_SIZE = 10 };

public:
  // Compress into the supplied buffer, which must have at least MAX_COMPRESSED_INT_SIZE free bytes.
  // Returns a pointer to the next byte in the buffer after the compressed int.
  static inline unsigned char *compress(unsigned char *buf, uint64_t i) {
    do {
      *buf = i & 0x7f;
      i >>= 7;
      ++buf;
    } while (i);

    // Set the high bit as this is the last byte in the encoding.
    buf[-1] |= 0x80;
    return buf;
  }

  // Decompress an int from the supplied buffer into the supplied integer.  Returns a pointer to
  // the byte after the compressed int.  Returns 0 if the integer is incomplete.
  static inline const unsigned char *decompress(const unsigned char *buf, const unsigned char *end, uint64_t &result) {
    int shift = 0;
    result = 0;
    do {
      if (buf == end) {
        return 0;
      }

      result |= ((uint64_t)(*buf & 0x7f)) << shift;
      shift += 7;
      ++buf;
    } while (!(buf[-1] & 0x80));

    return buf;
  }

  // Calculate the buffer length required to compress an integer.
  static inline size_t compressed_size(uint64_t ui64) {
    //TODO: something faster than this!
    unsigned char buffer[MAX_COMPRESSED_INT_SIZE];
    return compress(buffer, ui64) - buffer;
  }
};

} // namespaces


#endif


