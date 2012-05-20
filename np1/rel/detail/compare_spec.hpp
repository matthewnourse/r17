// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_REL_DETAIL_COMPARE_SPEC_HPP
#define NP1_REL_DETAIL_COMPARE_SPEC_HPP


#include "np1/rel/detail/helper.hpp"

namespace np1 {
namespace rel {
namespace detail {
  
class compare_spec {
public:
  typedef int (*compare_function_t)(const char *f1, size_t f1_length, const char *f2, size_t f2_length);
  typedef uint64_t (*hash_function_t)(const char *f, size_t f_length,  uint64_t hval);

public:
  compare_spec() : m_compare_function(0), m_hash_function(0), m_field_number(-1) {}

  template <typename Record>
  compare_spec(const Record &headings, const char *heading_name) {
    m_field_number = headings.mandatory_find_heading(heading_name);

    str::ref typed_heading_name = headings.mandatory_field(m_field_number);
    if (str::starts_with(typed_heading_name, rlang::dt::to_string(rlang::dt::TYPE_STRING))) {
      m_compare_function = helper::string_compare;
      m_hash_function = helper::string_hash_add;               
    } else if (str::starts_with(typed_heading_name, rlang::dt::to_string(rlang::dt::TYPE_ISTRING))) {
      m_compare_function = helper::istring_compare;
      m_hash_function = helper::istring_hash_add;
    } else if (str::starts_with(typed_heading_name, rlang::dt::to_string(rlang::dt::TYPE_INT))) {
      m_compare_function = helper::int_compare;
      m_hash_function = helper::int_hash_add;
    } else if (str::starts_with(typed_heading_name, rlang::dt::to_string(rlang::dt::TYPE_UINT))) {
      m_compare_function = helper::uint_compare;
      m_hash_function = helper::uint_hash_add;
    } else if (str::starts_with(typed_heading_name, rlang::dt::to_string(rlang::dt::TYPE_DOUBLE))) {
      m_compare_function = helper::double_compare;
      m_hash_function = helper::double_hash_add;
    } else if (str::starts_with(typed_heading_name, rlang::dt::to_string(rlang::dt::TYPE_INT))) {
      m_compare_function = helper::bool_compare;
      m_hash_function = helper::bool_hash_add;
    } else if (str::starts_with(typed_heading_name, rlang::dt::to_string(rlang::dt::TYPE_IPADDRESS))) {
      m_compare_function = helper::ipaddress_or_ipnumber_compare;
      m_hash_function = helper::ipnumber_or_ipaddress_hash_add;
    } else {
      NP1_ASSERT(false, "Unrecognised type at start of heading name: " + typed_heading_name.to_string());
    }  
  }

  ~compare_spec() {}
  
  compare_function_t compare_function() const { return m_compare_function; }
  hash_function_t hash_function() const { return m_hash_function; }
  size_t field_number() const { return m_field_number; }
  bool is_double() const { return (m_compare_function == helper::double_compare); }  
  
  //TODO: this shouldn't go here, but where should it go?
  static std::string untyped_heading(const char *heading, size_t len) {
    const char *heading_end = heading + len;
    const char *colon = (const char *)memchr(heading, ':', len);
    NP1_ASSERT(colon && (colon+1 < heading_end),
                "Heading is not a typed heading: " + std::string(heading, len));

    return std::string(colon+1, heading_end - colon - 1);
  }
  
private:
  compare_function_t m_compare_function;
  hash_function_t m_hash_function;
  size_t m_field_number;
};

  
} // namespaces
}
}




#endif
