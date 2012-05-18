// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_REL_DETAIL_HELPER_HPP
#define NP1_REL_DETAIL_HELPER_HPP


#include "np1/hash/fnv1a64.hpp"
#include "np1/rel/rlang/dt.hpp"

namespace np1 {
namespace rel {
namespace detail {
namespace helper {


uint64_t hash_add(uint8_t b, uint64_t hval) {
  return hash::fnv1a64::add(b, hval);
}


uint64_t hash_add(const char *p, size_t len, uint64_t hval) {
  return hash::fnv1a64::add(p, len, hval);
}
  
uint64_t hash_init(size_t somedata = 0x12345) {
  return hash::fnv1a64::init(somedata);
}
 




//TODO: remove this and use the str:: functions instead.
int string_compare(const char *f1, size_t f1_length, const char *f2, size_t f2_length) {
  ssize_t compare_length = (f1_length > f2_length) ? f2_length : f1_length;
  int compare_result = memcmp(f1, f2, compare_length);
  if (0 == compare_result) {
    if (f1_length > f2_length) {
      return 1;
    } else if (f1_length < f2_length) {
      return -1;
    }
    
    return 0;
  }
  
  return compare_result;
}


//TODO: remove this and use the str:: functions instead.
int istring_compare(const char *f1, size_t f1_length, const char *f2, size_t f2_length) {
  ssize_t compare_length = (f1_length > f2_length) ? f2_length : f1_length;
  int compare_result = strncasecmp(f1, f2, compare_length);
  if (0 == compare_result)  {
    if (f1_length > f2_length) {
      return 1;
    } else if (f1_length < f2_length) {
      return -1;
    }
    
    return 0;
  }
  
  return compare_result;
}


uint64_t string_hash_add(const char *str, size_t length, uint64_t hval) {
  return hash_add(str, length, hval);
}


uint64_t istring_hash_add(const char *str, size_t length, uint64_t hval) {
  const char *end = str + length;
  for (; str < end; ++str) {
    hval = hash_add(tolower(*str), hval);
  }
   
  return hval;  
}


inline const char *strip_leading_spaces_and_zeroes(const char *str, size_t length, size_t &new_length) {
  new_length = length;
  while ((new_length > 0) && ((' ' == *str) || ('0' == *str))) {
    ++str;
    --new_length;
  }
  
  return str;
}


inline const char *strip_leading_spaces(const char *str, size_t length, size_t &new_length) {
  new_length = length;
  while ((new_length > 0) && ((' ' == *str))) {
    ++str;
    --new_length;
  }
  
  return str;  
}


// Concatenate two strings, allocating the result from the supplied heap object.
template <typename Heap>
inline str::ref string_concat(Heap &heap, const str::ref &one, const str::ref &two) {
  size_t new_length = one.length() + two.length();
  char *new_s = heap.alloc(new_length);
  memcpy(new_s, one.ptr(), one.length());
  memcpy(new_s + one.length(), two.ptr(), two.length());
  return str::ref(new_s, new_length);
}


int uint_compare(const char *f1, size_t f1_length, const char *f2, size_t f2_length) {
  f1 = strip_leading_spaces_and_zeroes(f1, f1_length, f1_length);
  f2 = strip_leading_spaces_and_zeroes(f2, f2_length, f2_length);
  
  // Try to avoid the expense of converting to integer.
  if (f1_length > f2_length) {
    return 1;
  }
  
  if (f1_length < f2_length) {
    return -1;
  }
  
  // They are the same length.
  while (f1_length > 0) {
    if (*f1 > *f2) {
      return 1;
    }
    
    if (*f1 < *f2) {
      return -1;
    }
    
    ++f1;
    ++f2;
    --f1_length;
  } 
  
  return 0;
}


uint64_t uint_hash_add(const char *field, size_t field_length, uint64_t hval) {
  field = strip_leading_spaces_and_zeroes(field, field_length, field_length);
  return hash_add(field, field_length, hval);
}


int int_compare(const char *f1, size_t f1_length, const char *f2, size_t f2_length) {
  f1 = strip_leading_spaces_and_zeroes(f1, f1_length, f1_length);
  f2 = strip_leading_spaces_and_zeroes(f2, f2_length, f2_length);
  
  // Avoid converting to integer if possible.
  if (*f1 == '-') {
    if (*f2 == '-') {
      ++f1;
      ++f2;
      --f1_length;
      --f2_length;
      return -(uint_compare(f1, f1_length, f2, f2_length));
    }
    
    return -1;    
  }
  
  if (*f2 == '-') {
    return 1;
  }
  
  return uint_compare(f1, f1_length, f2, f2_length);
}



uint64_t int_hash_add(const char *field, size_t field_length, uint64_t hval) {
  field = strip_leading_spaces_and_zeroes(field, field_length, field_length);
  return hash_add(field, field_length, hval);
}


int double_compare(const char *f1, size_t f1_length,
                   const char *f2, size_t f2_length) {
  const double d1 = str::dec_to_double(f1, f1_length);
  const double d2 = str::dec_to_double(f2, f2_length);
  if (d1 > d2) {
    return 1;
  }
  
  if (d1 < d2) {
    return -1;
  }
    
  return 0;
}


uint64_t double_hash_add(const char *field, size_t field_length, uint64_t hval) {
  field = strip_leading_spaces_and_zeroes(field, field_length, field_length);
  return hash_add(field, field_length, hval);
}



// Convert a boolean field to a number.  True is 1, false is 0.
int bool_to_int(const char *f, size_t length) {
  if (length > 0) {    
    if ('1' == *f) {
      return 1; 
    }
    
    if ('0' == *f) {
      return 0;
    }
    
    char error_string[128];
    sprintf(error_string, "Invalid boolean value: '%c' (0x%x)", *f, (int)*f);
    NP1_ASSERT(false, error_string);    
  }
  
  return 0;   
}



// Compare two boolean fields.  true is 1, false is 0.
int bool_compare(const char *f1, size_t f1_length, const char *f2, size_t f2_length) {
  f1 = strip_leading_spaces(f1, f1_length, f1_length);
  f2 = strip_leading_spaces(f2, f2_length, f2_length);
  int f1_val = bool_to_int(f1, f1_length);
  int f2_val = bool_to_int(f2, f2_length);
  return (f1_val - f2_val);  
}


uint64_t bool_hash_add(const char *field, size_t field_length, uint64_t hval) {
  field = strip_leading_spaces(field, field_length, field_length); 
  return hash_add(field, field_length, hval);
}


const char *ipnumber_to_number(uint64_t &num, const char *ipnumber, 
                                const char *max_end_of_number) {
  char *end_p;
  num = str::partial_dec_to_int64(ipnumber, max_end_of_number, &end_p);
  
  return end_p;
}

// Convert an IP address component to a number and return a pointer to
// the end of the component.
const char *ipaddress_component_to_number(unsigned int &num, const char *start, const char *end_of_string,
                                          const char *ipaddress, size_t length) {
  if (start >= end_of_string) {
    NP1_ASSERT(false, "Invalid IP address, start >= end_of_string");
  }
  
  uint64_t ui64 = 0;
  const char *end_p = ipnumber_to_number(ui64, start, end_of_string);
    
  if ((ui64 <= 255) && (end_p != start) 
    && ((*end_p == '.') || (end_p == end_of_string))) {
    num = (unsigned int)ui64;
    return end_p;
  }
  
  NP1_ASSERT(false, "Invalid IP address: '" + str::ref(ipaddress, length).to_string() + "'");    
  return 0;
}


// Convert an IP address to a number.
unsigned int ipaddress_to_number(const char *ipaddress, size_t length) {   
  unsigned int component0 = 0;
  unsigned int component1 = 0;
  unsigned int component2 = 0;
  unsigned int component3 = 0;
  const char *end_of_ipaddress = ipaddress + length;
  
  const char *end_component;
  end_component = 
    ipaddress_component_to_number(component0, ipaddress, end_of_ipaddress, ipaddress, length);
    
  end_component =
    ipaddress_component_to_number(component1, end_component + 1, end_of_ipaddress, ipaddress, length);

  end_component =
    ipaddress_component_to_number(component2, end_component + 1, end_of_ipaddress, ipaddress, length);

  ipaddress_component_to_number(component3, end_component + 1, end_of_ipaddress, ipaddress, length);

  unsigned int ipnumber = component0 * 16777216;
  ipnumber += component1 * 65536;
  ipnumber += component2 * 256;
  ipnumber += component3;   
  
  return ipnumber;
}


// Compare to unsigned integers.
int compare_c_uints(unsigned int ui1, unsigned int ui2) {
  if (ui1 < ui2) {
    return -1;
  }
  
  if (ui1 > ui2) {
    return 1;
  }
  
  return 0;
}


// Compare two fields that may be IP addresses or IP numbers.
int ipaddress_or_ipnumber_compare(const char *f1, size_t f1_length,
                                    const char *f2, size_t f2_length) {
  f1 = strip_leading_spaces_and_zeroes(f1, f1_length, f1_length);
  f2 = strip_leading_spaces_and_zeroes(f2, f2_length, f2_length);

  bool f1_is_ip_number = !memchr(f1, '.', f1_length);
  bool f2_is_ip_number = !memchr(f2, '.', f2_length);

  if (f1_is_ip_number) {
    if (f2_is_ip_number) {
      return uint_compare(f1, f1_length, f2, f2_length);
    }
    
    uint64_t f1_number;
    const char *end_p = ipnumber_to_number(f1_number, f1, f1 + f1_length);
    if (end_p != f1 + f1_length) {
      NP1_ASSERT(false, "Invalid IP number: " + std::string(f1, f1_length));
    }
    
    unsigned int f2_number = ipaddress_to_number(f2, f2_length);
    
    //TODO: this cast is probably unsafe.    
    return compare_c_uints((unsigned int)f1_number, f2_number); 
  }
  
  if (f2_is_ip_number) {
    uint64_t f2_number;
    const char *end_p = ipnumber_to_number(f2_number, f2, f2 + f2_length);
    if (end_p != f2 + f2_length) {
      NP1_ASSERT(false, "Invalid IP number: " + std::string(f2, f2_length));
    }
    
    unsigned int f1_number = ipaddress_to_number(f1, f1_length);
    
    //TODO: this cast is probably unsafe.    
    return compare_c_uints((unsigned int)f1_number, f2_number);
  }
  
  return compare_c_uints(ipaddress_to_number(f1, f1_length),
                          ipaddress_to_number(f2, f2_length));    
} 


int ipaddress_or_ipnumber_compare(const str::ref &f1, const str::ref &f2) {
  return ipaddress_or_ipnumber_compare(f1.ptr(), f1.length(), f2.ptr(), f2.length());
}


// Hash an ipnumber or ipaddress field.
uint64_t ipnumber_or_ipaddress_hash_add(const char *field, size_t field_length, 
                                        uint64_t hval) {
  field = strip_leading_spaces_and_zeroes(field, field_length, field_length);
  return hash_add(field, field_length, hval);
}


// Figure out if a heading refers to "this" record or the "other" record.
typedef enum {
  RECORD_IDENTIFIER_THIS,
  RECORD_IDENTIFIER_OTHER
} record_identifier_type;

record_identifier_type get_heading_record_identifier(
                              const str::ref &heading_name,
                              str::ref &heading_name_without_record_identifier) {
    static const char *THIS_STRING = "this";
    static const size_t THIS_STRING_LENGTH = strlen(THIS_STRING); 
    static const char *OTHER_STRING = "other";
    static const size_t OTHER_STRING_LENGTH = strlen(OTHER_STRING);
    static const char *PREV_STRING = "prev";
    static const size_t PREV_STRING_LENGTH = strlen(PREV_STRING);


    // By default, the token text refers to "this" record.
    const char *heading_name_p = heading_name.ptr();
    const char *dot = strchr(heading_name_p, '.');
    record_identifier_type type = RECORD_IDENTIFIER_THIS;
    
    if (!dot) {
      type = RECORD_IDENTIFIER_THIS;
    } else if (str::cmp(heading_name_p, dot - heading_name_p, THIS_STRING, THIS_STRING_LENGTH) == 0) {
      type = RECORD_IDENTIFIER_THIS;
      heading_name_p += THIS_STRING_LENGTH + 1;
    } else if (str::cmp(heading_name_p, dot - heading_name_p, OTHER_STRING, OTHER_STRING_LENGTH) == 0) {
      type = RECORD_IDENTIFIER_OTHER;
      heading_name_p += OTHER_STRING_LENGTH + 1;
    } else if (str::cmp(heading_name_p, dot - heading_name_p, PREV_STRING, PREV_STRING_LENGTH) == 0) {
      type = RECORD_IDENTIFIER_OTHER;
      heading_name_p += PREV_STRING_LENGTH + 1;
    } else {
      NP1_ASSERT(false, "Invalid record identifier prefix in "
                        + std::string(heading_name.ptr()) + "  Valid values are:\n"
                        + std::string(THIS_STRING) + "\n"
                        + std::string(OTHER_STRING) + "\n"
                        + std::string(PREV_STRING) + "\n");
    }

    heading_name_without_record_identifier =
      str::ref(heading_name_p, strlen(heading_name_p));

    return type;
}


// Get a heading name without the type tag, if it has a type tag.
str::ref get_heading_without_type_tag(const str::ref &heading_name) {
  const char *colon = (const char *)memchr(heading_name.ptr(), ':',
                                            heading_name.length());
  if (!colon) {
    return heading_name;
  }

  if (heading_name.ptr() + heading_name.length() - 1 == colon) {
    return heading_name;
  }

  return str::ref(colon + 1, heading_name.length() - (colon - heading_name.ptr()) - 1);
}


str::ref get_heading_without_type_tag(const std::string &heading_name) {
  return get_heading_without_type_tag(str::ref(heading_name));
}



// Get the type tag out of the heading name.  Returns a null str::ref on
// failure.
str::ref get_heading_type_tag(const str::ref &heading_name) {
  const char *colon = (const char *)memchr(heading_name.ptr(), ':',
                                            heading_name.length());
  if (!colon) {
    return str::ref();
  }

  if (heading_name.ptr() + heading_name.length() - 1 == colon) {
    return str::ref();
  }

  if (colon == heading_name.ptr()) {
    return str::ref();  
  }
  
  return str::ref(heading_name.ptr(), colon - heading_name.ptr());
}

str::ref get_heading_type_tag(const char *heading_name) {
  return get_heading_type_tag(str::ref(heading_name));
}

str::ref get_heading_type_tag(const std::string &heading_name) {
  return get_heading_type_tag(str::ref(heading_name));
}


// Get the type tag out of the heading name, crash the process on error.
str::ref mandatory_get_heading_type_tag(const str::ref &heading_name) {
  str::ref type_tag = get_heading_type_tag(heading_name);
  NP1_ASSERT(!type_tag.is_null(),
              "Type tag not found in heading: " + heading_name.to_string());
  return type_tag;
}
  

str::ref mandatory_get_heading_type_tag(const std::string &heading_name) {
  return mandatory_get_heading_type_tag(str::ref(heading_name));
}



std::string make_typed_heading_name(const std::string &type_name, const std::string &name) {
  return type_name + ":" + name;
}


std::string convert_to_valid_header_name(const str::ref &s) {
  std::string result;
  const char *p = s.ptr();
  const char *end = p + s.length();
  for (; p < end; ++p) {
    if (isalnum(*p)) {
      result.push_back(*p);
    } else if (':' == *p) {
      if (rlang::dt::is_valid_tag(str::ref(s.ptr(), p - s.ptr()))) {
        result.push_back(*p);
      } else {
        result.push_back('_');
      }
    } else {
      result.push_back('_');
    }
  }

  return result;
}



} // namespaces
}
}
}


#endif
