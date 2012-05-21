// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_UUID_HPP
#define NP1_UUID_HPP

#include "np1/math.hpp"


// A random UUID that conforms to RFC 4122.
namespace np1 {

class uuid {
public:
  static uuid generate() {
    uuid u;

    // Get the random data.
    u.m_fields.m_time_low = math::rand64();
    u.m_fields.m_time_mid = math::rand64();
    u.m_fields.m_time_hi_and_version = math::rand64();
    u.m_fields.m_clock_seq_hi_and_reserved = math::rand64();
    u.m_fields.m_clock_seq_low = math::rand64();
    u.m_fields.m_node = math::rand64();

    // The node field is meant to be only 6 bytes long.
    u.m_fields.m_node &= 0xffffffffffffULL;

    // RFC 4122 says that this is how we tell the world that this is a random
    // UUID.
    u.m_fields.m_clock_seq_hi_and_reserved &= 0x7f;
    u.m_fields.m_time_hi_and_version &= 0x4fff;

    return u;
  }

  size_t required_str_size() const {
    return 38; // Include trailing null.
  }

  /// The supplied string must be long enough to hold the UUID and a trailing
  /// null.
  void to_str(char *str) const {
    char *p = str;

    str::to_hex_str_pad_8(p, m_fields.m_time_low);
    p += 8;
    *p++ = '-';
    str::to_hex_str_pad_4(p, m_fields.m_time_mid);
    p += 4;
    *p++ = '-';
    str::to_hex_str_pad_4(p, m_fields.m_time_hi_and_version);
    p += 4;
    *p++ = '-';
    str::to_hex_str_pad_2(p, m_fields.m_clock_seq_hi_and_reserved);
    p += 2;
    str::to_hex_str_pad_2(p, m_fields.m_clock_seq_low);
    p += 2;
    *p++ = '-';
    str::to_hex_str_pad_12(p, m_fields.m_node);
  }

  rstd::string to_string() const {
    char s[39];
    to_str(s);
    return s;
  }

private:
  uuid() {}

private:
  struct fields {
    uint32_t m_time_low;
    uint16_t m_time_mid;
    uint16_t m_time_hi_and_version;
    uint8_t m_clock_seq_hi_and_reserved;
    uint8_t m_clock_seq_low;
    uint64_t m_node;
  };

  fields m_fields;
};


} // namespaces


#endif
