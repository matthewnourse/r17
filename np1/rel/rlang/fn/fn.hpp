// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_REL_RLANG_FN_FN_HPP
#define NP1_REL_RLANG_FN_FN_HPP

#include "np1/rel/record_ref.hpp"
#include "np1/math.hpp"
#include "np1/uuid.hpp"
#include "np1/time.hpp"
#include "np1/hash/sha256.hpp"
#include "np1/rel/rlang/dt.hpp"
#include "np1/rel/rlang/vm_literals.hpp"
#include "np1/rel/rlang/vm_stack.hpp"
#include "np1/rel/rlang/simulated_stack.hpp"
#include "np1/rel/rlang/vm_heap.hpp"
#include "np1/regex/pattern_cache.hpp"
#include "np1/io/net/curl.hpp"
#include "np1/io/gzfile.hpp"


namespace np1 {
namespace rel {
namespace rlang {
namespace fn {

/// The base for all functions.
struct base {
  static const char *since() { return "1.0"; }
  static const bool is_operator = false;
  static const size_t precedence = -1;
  static const bool is_left_assoc = false;
  static const char *synonym() { return 0; }
};


/// Push literals.
#define NP1_REL_RLANG_FN_NAME_PUSH_LITERAL_STRING "____internal.push_literal_string"
#define NP1_REL_RLANG_FN_NAME_PUSH_LITERAL_INTEGER "____internal.push_literal_integer"
#define NP1_REL_RLANG_FN_NAME_PUSH_LITERAL_UINTEGER "____internal.push_literal_uinteger"
#define NP1_REL_RLANG_FN_NAME_PUSH_LITERAL_DOUBLE "____internal.push_literal_double"
#define NP1_REL_RLANG_FN_NAME_PUSH_LITERAL_BOOLEAN_TRUE "____internal.push_literal_boolean_true"
#define NP1_REL_RLANG_FN_NAME_PUSH_LITERAL_BOOLEAN_FALSE "____internal.push_literal_boolean_false"

struct internal_push_literal_string : public base {
  static const char *name() { return NP1_REL_RLANG_FN_NAME_PUSH_LITERAL_STRING; }
  static const char *description() { return ""; }  
  inline static dt::string call(const vm_literals &literals, size_t literal_number) {
    return literals.get_string(literal_number);
  }
};

struct internal_push_literal_integer : public base {
  static const char *name() { return NP1_REL_RLANG_FN_NAME_PUSH_LITERAL_INTEGER; }
  static const char *description() { return ""; }  
  inline static dt::integer call(const vm_literals &literals, size_t literal_number) {
    return literals.get_integer(literal_number);
  }
};

struct internal_push_literal_uinteger : public base {
  static const char *name() { return NP1_REL_RLANG_FN_NAME_PUSH_LITERAL_UINTEGER; }
  static const char *description() { return ""; }  
  inline static dt::uinteger call(const vm_literals &literals, size_t literal_number) {
    return literals.get_uinteger(literal_number);
  }
};

struct internal_push_literal_double : public base {
  static const char *name() { return NP1_REL_RLANG_FN_NAME_PUSH_LITERAL_DOUBLE; }
  static const char *description() { return ""; }  
  inline static dt::fdouble call(const vm_literals &literals, size_t literal_number) {
    return literals.get_double(literal_number);
  }
};

struct internal_push_literal_boolean_true : public base {
  static const char *name() { return NP1_REL_RLANG_FN_NAME_PUSH_LITERAL_BOOLEAN_TRUE; }
  static const char *synonym() { return "true"; } 
  static const char *description() { return ""; }  
  inline static dt::boolean call(const vm_literals &literals, size_t literal_number) {
    return true;
  }
};

struct internal_push_literal_boolean_false : public base {
  static const char *name() { return NP1_REL_RLANG_FN_NAME_PUSH_LITERAL_BOOLEAN_FALSE; }
  static const char *synonym() { return "false"; } 
  static const char *description() { return ""; }  
  inline static dt::boolean call(const vm_literals &literals, size_t literal_number) {
    return false;
  }
};



/// Push record fields.
#define NP1_REL_RLANG_FN_NAME_PUSH_FIELD_STRING "____internal.push_field_string"
#define NP1_REL_RLANG_FN_NAME_PUSH_FIELD_ISTRING "____internal.push_field_istring"
#define NP1_REL_RLANG_FN_NAME_PUSH_FIELD_INTEGER "____internal.push_field_integer"
#define NP1_REL_RLANG_FN_NAME_PUSH_FIELD_UINTEGER "____internal.push_field_uinteger"
#define NP1_REL_RLANG_FN_NAME_PUSH_FIELD_DOUBLE "____internal.push_field_double"
#define NP1_REL_RLANG_FN_NAME_PUSH_FIELD_BOOLEAN "____internal.push_field_string"
#define NP1_REL_RLANG_FN_NAME_PUSH_FIELD_IPADDRESS "____internal.push_field_string"
#define NP1_REL_RLANG_FN_NAME_PUSH_ROWNUM "____internal.push_rownum"
#define NP1_REL_RLANG_FN_ROWNUM_SPECIAL_VARIABLE_NAME "_rownum"

struct internal_push_field_string : public base {
  static const char *name() { return NP1_REL_RLANG_FN_NAME_PUSH_FIELD_STRING; }
  static const char *description() { return ""; }  
  inline static dt::string call(const record_ref &r, size_t field_number) {
    return r.mandatory_field(field_number);
  }
};

struct internal_push_field_istring : public base {
  static const char *name() { return NP1_REL_RLANG_FN_NAME_PUSH_FIELD_ISTRING; }
  static const char *description() { return ""; }  
  inline static dt::string call(const record_ref &r, size_t field_number) {
    return r.mandatory_field(field_number);
  }
};

struct internal_push_field_integer : public base {
  static const char *name() { return NP1_REL_RLANG_FN_NAME_PUSH_FIELD_INTEGER; }
  static const char *description() { return ""; }  
  inline static dt::integer call(const record_ref &r, size_t field_number) {
    return str::dec_to_int64(r.mandatory_field(field_number));
  }
};

struct internal_push_field_uinteger : public base {
  static const char *name() { return NP1_REL_RLANG_FN_NAME_PUSH_FIELD_UINTEGER; }
  static const char *description() { return ""; }  
  inline static dt::uinteger call(const record_ref &r, size_t field_number) {
    return str::dec_to_int64(r.mandatory_field(field_number));
  }
};

struct internal_push_field_double : public base {
  static const char *name() { return NP1_REL_RLANG_FN_NAME_PUSH_FIELD_DOUBLE; }
  static const char *description() { return ""; }  
  inline static dt::fdouble call(const record_ref &r, size_t field_number) {
    return str::dec_to_double(r.mandatory_field(field_number));
  }
};

struct internal_push_field_boolean : public base {
  static const char *name() { return NP1_REL_RLANG_FN_NAME_PUSH_FIELD_BOOLEAN; }
  static const char *description() { return ""; }  
  inline static dt::boolean call(const record_ref &r, size_t field_number) {
    return str::to_bool(r.mandatory_field(field_number));
  }
};

struct internal_push_field_ipaddress : public base {
  static const char *name() { return NP1_REL_RLANG_FN_NAME_PUSH_FIELD_IPADDRESS; }
  static const char *description() { return ""; }  
  inline static dt::ipaddress call(const record_ref &r, size_t field_number) {
    return r.mandatory_field(field_number);
  }
};


struct internal_push_rownum : public base {
  static const char *name() { return NP1_REL_RLANG_FN_NAME_PUSH_ROWNUM; }
  static const char *description() { return ""; }  
  inline static dt::uinteger call(const record_ref &r, size_t field_number) {
    return r.record_number();
  }
};


#define NP1_REL_RLANG_FN_NAME_IF "____internal.if"
#define NP1_REL_RLANG_FN_NAME_GOTO "____internal.goto"

struct internal_if : public base {
  static const char *name() { return NP1_REL_RLANG_FN_NAME_IF; }
  static const char *description() { return ""; }  
  inline static size_t call(vm_stack &stk, size_t else_offset) {
    dt::boolean cond; stk.pop(cond); return cond ? 1 : else_offset;
  }
};

struct internal_goto : public base {
  static const char *name() { return NP1_REL_RLANG_FN_NAME_GOTO; }
  static const char *description() { return ""; }  
  inline static size_t call(vm_stack &stk, size_t offset) {
    return offset;
  }
};


// Convert to case-sensitive string.
struct to_string : public base {
  static const char *name() { return "to_string"; }
  static const char *description() { return "`to_string(value)` returns a 'sensible' string representation of `value`."; }  
  inline static dt::string call(const dt::string &input) { return input; }
  inline static dt::string call(const dt::istring &input) { return input; }
  inline static dt::string call(vm_heap &heap, const dt::integer &input) {
    return num_to_string_helper(heap, input);  
  }

  inline static dt::string call(vm_heap &heap, const dt::uinteger &input) {
    return num_to_string_helper(heap, input);  
  }

  inline static dt::string call(vm_heap &heap, const dt::fdouble &input) {
    return num_to_string_helper(heap, input);  
  }

  inline static dt::string call(const dt::boolean &input) {
    return str::from_bool(input);
  }

  inline static dt::string call(const dt::ipaddress &input) { return (dt::string)input; }

  template <typename Num>
  inline static dt::string num_to_string_helper(vm_heap &heap, Num input) {
    char num_str[str::MAX_NUM_STR_LENGTH];
    str::to_dec_str(num_str, input);
    size_t length = strlen(num_str);
    char *s = heap.alloc(length);
    memcpy(s, num_str, length);
    return str::ref(s, length);
  }
};


// Convert to case-insensitive string.
struct to_istring : public base {
  static const char *name() { return "to_istring"; }
  static const char *description() { return "`to_istring(value)` returns a 'sensible' case-independent string representation of `value`."; }  
  inline static dt::istring call(const dt::string &input) { return input; }
  inline static dt::istring call(const dt::istring &input) { return input; }
  inline static dt::istring call(vm_heap &heap, const dt::integer &input) { return to_string::call(heap, input); }
  inline static dt::istring call(vm_heap &heap, const dt::uinteger &input) { return to_string::call(heap, input); }
  inline static dt::istring call(vm_heap &heap, const dt::fdouble &input) { return to_string::call(heap, input); }
  inline static dt::istring call(const dt::boolean &input) { return str::from_bool(input); }
  inline static dt::istring call(const dt::ipaddress &input) { return (dt::string)input; }
};

// Convert to upper case.
struct str_to_upper_case : public base {
  static const char *since() { return "1.4.4"; }
  static const char *name() { return "str.to_upper_case"; }
  static const char *description() { return "`str.to_upper_case(str)` returns `str` with all characters converted to upper case."; }
  inline static dt::string call(vm_heap &heap, const dt::string &input) { return to_upper_case(heap, input); }
  inline static dt::istring call(vm_heap &heap, const dt::istring &input) { return to_upper_case(heap, input); }
  
  inline static str::ref to_upper_case(vm_heap &heap, const str::ref &input) {
    const char *source_p = input.ptr();
    const char *source_end = source_p + input.length();
    
    char *target = heap.alloc(input.length() + 1);
    char *target_p = target;
    
    for (; source_p < source_end; ++source_p, ++target_p) {
      *target_p = toupper(*source_p);
    }
    
    *target_p = '\0';
    return str::ref(target, input.length());
  }
};

// Convert to lower case.
struct str_to_lower_case : public base {
  static const char *since() { return "1.4.4"; }
  static const char *name() { return "str.to_lower_case"; }
  static const char *description() { return "`str.to_lower_case(str)` returns `str` with all characters converted to lower case."; }
  inline static dt::string call(vm_heap &heap, const dt::string &input) { return to_lower_case(heap, input); }
  inline static dt::istring call(vm_heap &heap, const dt::istring &input) { return to_lower_case(heap, input); }
  
  inline static str::ref to_lower_case(vm_heap &heap, const str::ref &input) {
    const char *source_p = input.ptr();
    const char *source_end = source_p + input.length();
    
    char *target = heap.alloc(input.length() + 1);
    char *target_p = target;
    
    for (; source_p < source_end; ++source_p, ++target_p) {
      *target_p = tolower(*source_p);
    }
    
    *target_p = '\0';
    return str::ref(target, input.length());
  }
};


/// Regex matching
struct str_regex_match : public base {
  static const char *name() { return "str.regex_match"; }
  static const char *description() { return "`str.regex_match(pattern, haystack)` returns true if `haystack` matches `pattern`.  `pattern` is a Perl-compatible regular expression."; }  
  static const size_t regex_arg_offset = 0;

  inline static dt::boolean call(const dt::string &pattern, const dt::string &haystack) {
    return match(pattern, haystack);
  }

  inline static dt::boolean call(const dt::string &pattern, const dt::istring &haystack) {
    return imatch(pattern, haystack);
  }

  inline static dt::boolean call(const dt::istring &pattern, const dt::string &haystack) {
    return imatch(pattern, haystack);
  }

  inline static dt::boolean call(const dt::istring &pattern, const dt::istring &haystack) {
    return imatch(pattern, haystack);
  }

private:
  inline static bool match(const str::ref &pattern, const str::ref &haystack) {
    return regex::pattern_cache::get(pattern, true).match(haystack);
  }

  inline static bool imatch(const str::ref &pattern, const str::ref &haystack) {
    return regex::pattern_cache::get(pattern, false).match(haystack);
  }

};


struct str_regex_replace : public base {
  static const char *name() { return "str.regex_replace"; }
  //TODO: rewrite this description.
  static const char *description() { return "`str.regex_replace(pattern, haystack, replace_spec)` replaces grouped patterns within `haystack` with replacements in `replace_spec`.  `\\n` (where n is a digit) in replace spec refers to the nth parenthesized subexpression in `pattern`.  If there is no match then `str.regex_replace` returns `haystack`."; }  

  inline static dt::string call(vm_heap &heap, const dt::string &pattern, const dt::string &haystack, const dt::string &replace_spec) {
    return replace(heap, pattern, haystack, replace_spec, true);
  }

  inline static dt::string call(vm_heap &heap, const dt::string &pattern, const dt::string &haystack, const dt::istring &replace_spec) {
    return replace(heap, pattern, haystack, replace_spec, true);
  }

  inline static dt::istring call(vm_heap &heap, const dt::string &pattern, const dt::istring &haystack, const dt::string &replace_spec) {
    return replace(heap, pattern, haystack, replace_spec, false);
  }

  inline static dt::istring call(vm_heap &heap, const dt::string &pattern, const dt::istring &haystack, const dt::istring &replace_spec) {
    return replace(heap, pattern, haystack, replace_spec, false);
  }

private:
  inline static str::ref replace(vm_heap &heap, const str::ref &pattern, const str::ref &haystack,
                                  const str::ref &replace_spec, bool case_sensitive) {
    str::ref result_str;
    bool result = regex::pattern_cache::get(pattern, case_sensitive).replace(heap, haystack, replace_spec, result_str);
    if (result) {
      return result_str;
    }

    return haystack;
  }
};


struct str_regex_replace_empty_on_no_match : public base {
  static const char *name() { return "str.regex_replace_empty_on_no_match"; }
  //TODO: rewrite this description.
  static const char *description() { return "As for `str.regex_replace`, except that it returns the empty string on no match."; }  

  inline static dt::string call(vm_heap &heap, const dt::string &pattern, const dt::string &haystack, const dt::string &replace_spec) {
    return replace(heap, pattern, haystack, replace_spec, true);
  }

  inline static dt::string call(vm_heap &heap, const dt::string &pattern, const dt::string &haystack, const dt::istring &replace_spec) {
    return replace(heap, pattern, haystack, replace_spec, true);
  }

  inline static dt::istring call(vm_heap &heap, const dt::string &pattern, const dt::istring &haystack, const dt::string &replace_spec) {
    return replace(heap, pattern, haystack, replace_spec, false);
  }

  inline static dt::istring call(vm_heap &heap, const dt::string &pattern, const dt::istring &haystack, const dt::istring &replace_spec) {
    return replace(heap, pattern, haystack, replace_spec, false);
  }

private:
  inline static str::ref replace(vm_heap &heap, const str::ref &pattern, const str::ref &haystack,
                                  const str::ref &replace_spec, bool case_sensitive) {
    str::ref result_str;
    bool result = regex::pattern_cache::get(pattern, case_sensitive).replace(heap, haystack, replace_spec, result_str);
    if (result) {
      return result_str;
    }

    return str::ref("", 0);
  }
};


struct str_regex_replace_all : public base {
  static const char *since() { return "1.4.4"; }
  static const char *name() { return "str.regex_replace_all"; }
  static const char *description() { return "`str.regex_replace_all(pattern, haystack, replace_spec)` is the same as `str.regex_replace(pattern, haystack, replace_spec)` except that it continues to search and replace after the first match, and all non-matching substrings are returned as-is."; }  

  inline static dt::string call(vm_heap &heap, const dt::string &pattern, const dt::string &haystack, const dt::string &replace_spec) {
    return replace_all(heap, pattern, haystack, replace_spec, true);
  }

  inline static dt::string call(vm_heap &heap, const dt::string &pattern, const dt::string &haystack, const dt::istring &replace_spec) {
    return replace_all(heap, pattern, haystack, replace_spec, true);
  }

  inline static dt::istring call(vm_heap &heap, const dt::string &pattern, const dt::istring &haystack, const dt::string &replace_spec) {
    return replace_all(heap, pattern, haystack, replace_spec, false);
  }

  inline static dt::istring call(vm_heap &heap, const dt::string &pattern, const dt::istring &haystack, const dt::istring &replace_spec) {
    return replace_all(heap, pattern, haystack, replace_spec, false);
  }

private:
  inline static str::ref replace_all(vm_heap &heap, const str::ref &pattern, const str::ref &haystack,
                                    const str::ref &replace_spec, bool case_sensitive) {
    str::ref result_str;
    bool result = regex::pattern_cache::get(pattern, case_sensitive).replace_all(heap, haystack, replace_spec, result_str);
    if (result) {
      return result_str;
    }

    return haystack;
  }
};



/// Other string matching functions
struct str_starts_with : public base {
  static const char *name() { return "str.starts_with"; }
  static const char *description() { return "`str.starts_with(haystack, needle)` returns true if haystack starts with needle."; }  
  inline static dt::boolean call(const dt::string &haystack, const dt::string &needle) {
    return str::starts_with(haystack, needle);
  }

  inline static dt::boolean call(const dt::istring &haystack, const dt::string &needle) {
    return str::istarts_with(haystack, needle);
  }

  inline static dt::boolean call(const dt::string &haystack, const dt::istring &needle) {
    return str::istarts_with(haystack, needle);
  }

  inline static dt::boolean call(const dt::istring &haystack, const dt::istring &needle) {
    return str::istarts_with(haystack, needle);
  }
};


struct str_ends_with : public base {
  static const char *name() { return "str.ends_with"; }
  static const char *description() { return "str.ends_with(haystack, needle) returns true if haystack ends with needle."; }  
  inline static dt::boolean call(const dt::string &haystack, const dt::string &needle) {
    return str::ends_with(haystack, needle);
  }

  inline static dt::boolean call(const dt::istring &haystack, const dt::string &needle) {
    return str::iends_with(haystack, needle);
  }

  inline static dt::boolean call(const dt::string &haystack, const dt::istring &needle) {
    return str::iends_with(haystack, needle);
  }

  inline static dt::boolean call(const dt::istring &haystack, const dt::istring &needle) {
    return str::iends_with(haystack, needle);
  }
};


struct str_contains : public base {
  static const char *name() { return "str.contains"; }
  static const char *description() { return "str.contains(haystack, needle) returns true if haystack contains needle."; }  
  inline static dt::boolean call(const dt::string &haystack, const dt::string &needle) {
    return str::contains(haystack, needle);
  }

  inline static dt::boolean call(const dt::istring &haystack, const dt::string &needle) {
    return str::icontains(haystack, needle);
  }

  inline static dt::boolean call(const dt::string &haystack, const dt::istring &needle) {
    return str::icontains(haystack, needle);
  }

  inline static dt::boolean call(const dt::istring &haystack, const dt::istring &needle) {
    return str::icontains(haystack, needle);
  }
};


/// UUID generation.
struct str_uuidgen : public base {
  static const char *name() { return "str.uuidgen"; }
  static const char *description() { return "Generate a random UUID using the same random number generator as math.rand64()."; }  
  inline static dt::string call(vm_heap &h) {
    uuid u = uuid::generate();
    size_t sz = u.required_str_size();
    char *p = h.alloc(sz);
    u.to_str(p);
    return dt::string(p);
  }
};

/// SHA-256 hash.
struct str_sha256 : public base {
  static const char *name() { return "str.sha256"; }
  static const char *description() { return "`str.sha256(data)` generates a hex-encoded SHA-256 hash `data`."; }  
  inline static dt::string call(vm_heap &h, const dt::string &data) {
    return hash(h, data.ptr(), data.length());
  }

  inline static dt::string call(vm_heap &h, const dt::istring &data) {
    return hash(h, data.ptr(), data.length());
  }

  inline static dt::string call(vm_heap &h, const dt::integer &data) {
    return hash(h, &data, sizeof(data));
  }

  inline static dt::string call(vm_heap &h, const dt::uinteger &data) {
    return hash(h, &data, sizeof(data));
  }

  inline static dt::string call(vm_heap &h, const dt::boolean &data) {
    unsigned char uc = !!data;
    return hash(h, &uc, 1);
  }
  
  inline static dt::string call(vm_heap &h, const dt::ipaddress &data) {
    return hash(h, data.ptr(), data.length());
  }


  
private:
  inline static dt::string hash(vm_heap &h, const void *data, size_t data_length) {
    unsigned char raw[32];
    hash::sha256::hash((const unsigned char *)data, data_length, raw);
    char *p = h.alloc(64 + 1);
    str::to_hex_str_pad_64(p, raw);
    return dt::string(p, 64);
  }
};




/// Random number generation.
struct math_rand64 : public base {
  static const char *name() { return "math.rand64"; }
  static const char *description() { return "Generate a 64-bit random number.  The seed is data from the OS's random number generator plus the process's PID and the current time in microseconds.  The random state is periodically reset with a new seed.  The random number is generated using a SHA-256 hash of the seed together with the output of the previous SHA-256 hash."; }  
  inline static dt::uinteger call() {
    return math::rand64();
  }
};



/// The current time in microseconds.
struct time_now_epoch_usec : public base {
  static const char *name() { return "time.now_epoch_usec"; }
  static const char *description() { return "The number of microseconds since 1/1/1970 00:00:00 GMT."; }  
  inline static dt::uinteger call() {
    return time::now_epoch_usec();
  }
};


/// Convert microseconds to milliseconds.
struct time_usec_to_msec : public base {
  static const char *name() { return "time.usec_to_msec"; }
  static const char *description() { return "Convert microseconds to milliseconds."; }  
  inline static dt::uinteger call(dt::uinteger usec) { return time::usec_to_msec(usec); }
  inline static dt::integer call(dt::integer usec) { return time::usec_to_msec(usec); }
};

/// Convert milliseconds to microseconds.
struct time_msec_to_usec : public base {
  static const char *name() { return "time.msec_to_usec"; }
  static const char *description() { return "Convert milliseconds to microseconds."; }  
  inline static dt::uinteger call(dt::uinteger usec) { return time::msec_to_usec(usec); }
  inline static dt::integer call(dt::integer usec) { return time::msec_to_usec(usec); }
};


/// Convert microseconds to seconds.
struct time_usec_to_sec : public base {
  static const char *name() { return "time.usec_to_sec"; }
  static const char *description() { return "Convert microseconds to seconds."; }  
  inline static dt::uinteger call(dt::uinteger usec) { return time::usec_to_sec(usec); }
  inline static dt::integer call(dt::integer usec) { return time::usec_to_sec(usec); }
};

/// Convert seocnds to microseconds.
struct time_sec_to_usec : public base {
  static const char *name() { return "time.sec_to_usec"; }
  static const char *description() { return "Convert seconds to microseconds."; }  
  inline static dt::uinteger call(dt::uinteger usec) { return time::sec_to_usec(usec); }
  inline static dt::integer call(dt::integer usec) { return time::sec_to_usec(usec); }
};

/// Date part extraction.
struct time_extract_year : public base {
  static const char *name() { return "time.extract_year"; }
  static const char *description() { return "`time.extract_year(usec_since_epoch)` returns the year part of the supplied date."; }  
  inline static dt::uinteger call(dt::uinteger usec) { return time::extract_year(usec); }
  inline static dt::integer call(dt::integer usec) { return time::extract_year(usec); }
};

struct time_extract_month : public base {
  static const char *name() { return "time.extract_month"; }
  static const char *description() { return "`time.extract_month(usec_since_epoch)` returns the month part of the supplied date."; }  
  inline static dt::uinteger call(dt::uinteger usec) { return time::extract_month(usec); }
  inline static dt::integer call(dt::integer usec) { return time::extract_month(usec); }
};

struct time_extract_day : public base {
  static const char *name() { return "time.extract_day"; }
  static const char *description() { return "`time.extract_day(usec_since_epoch)` returns the day part of the supplied date."; }  
  inline static dt::uinteger call(dt::uinteger usec) { return time::extract_day(usec); }
  inline static dt::integer call(dt::integer usec) { return time::extract_day(usec); }
};

struct time_extract_hour : public base {
  static const char *name() { return "time.extract_hour"; }
  static const char *description() { return "`time.extract_hour(usec_since_epoch)` returns the hour part of the supplied date."; }  
  inline static dt::uinteger call(dt::uinteger usec) { return time::extract_hour(usec); }
  inline static dt::integer call(dt::integer usec) { return time::extract_hour(usec); }
};

struct time_extract_minute : public base {
  static const char *name() { return "time.extract_minute"; }
  static const char *description() { return "`time.extract_minute(usec_since_epoch)` returns the minute part of the supplied date."; }  
  inline static dt::uinteger call(dt::uinteger usec) { return time::extract_minute(usec); }
  inline static dt::integer call(dt::integer usec) { return time::extract_minute(usec); }
};

struct time_extract_second : public base {
  static const char *name() { return "time.extract_second"; }
  static const char *description() { return "`time.extract_second(usec_since_epoch)` returns the second part of the supplied date."; }  
  inline static dt::uinteger call(dt::uinteger usec) { return time::extract_second(usec); }
  inline static dt::integer call(dt::integer usec) { return time::extract_second(usec); }
};

struct time_parse : public base {
  static const char *since() { return "1.3.0"; }
  static const char *name() { return "time.parse"; }
  static const char *description() { return "`time.parse(time_string, format_string)` parses `time_string` according "
                                            "to `format` where `format` is the format string supported by the system's "
                                            "strptime C function.  Note that the time is interpreted as if it is in the "
                                            "local time zone.  On some systems the underlying C library functions ignore "
                                            "time zone specifications completely and in others the time zone behaviour is "
                                            "counter-intuitive.  The time is returned as number of microseconds "
                                            "since 1/1/1970 00:00:00 GMT."; }  
  inline static dt::uinteger call(const dt::string &time_s, const dt::string &format_s) { return parse(time_s, format_s); }
  inline static dt::uinteger call(const dt::istring &time_s, const dt::string &format_s) { return parse(time_s, format_s); }
  inline static dt::uinteger call(const dt::string &time_s, const dt::istring &format_s) { return parse(time_s, format_s); }
  inline static dt::uinteger call(const dt::istring &time_s, const dt::istring &format_s) { return parse(time_s, format_s); }

  static dt::uinteger parse(const str::ref &time_s, const str::ref &format_s) {
    //TODO: do without this null-char-putting.
    char *time_ptr = (char *)time_s.ptr();
    char *end_time_ptr = time_ptr + time_s.length();
    char prev_end_time_char = *end_time_ptr;
    *end_time_ptr = '\0';

    char *format_ptr = (char *)format_s.ptr();
    char *end_format_ptr = format_ptr + format_s.length();
    char prev_end_format_char = *end_format_ptr;
    *end_format_ptr = '\0';

    struct tm tm_buf;
    // If we don't memset then any fields uninitialized by the time string won't be initialized by strptime
    // in release mode.
    memset(&tm_buf, 0, sizeof(tm_buf));

    // We need to set the daylight savings time to "I don't know" otherwise mktime will think that it _is_ in effect.
    tm_buf.tm_isdst = -1;

    char *result = strptime(time_ptr, format_ptr, &tm_buf);
    *end_time_ptr = prev_end_time_char;
    *end_format_ptr = prev_end_format_char;

    NP1_ASSERT(result, "strptime failed on time string: '" + time_s.to_string()
                        + "'  format string: '" + format_s.to_string() + "'");

    return time::sec_to_usec(mktime(&tm_buf));
  }
};

struct time_format : public base {
  static const char *since() { return "1.4.3"; }
  static const char *name() { return "time.format"; }
  static const char *description() { return "`time.format(usec_since_epoch, format_string)` formats `usec_since_epoch` "
                                            "according to `format` where `format` is the format string supported by the system's "
                                            "strftime C function."; }  
  inline static dt::string call(vm_heap &h, const dt::uinteger &time_epoch, const dt::string &format_s) {
    return format(h, time_epoch, format_s);
  }

  inline static dt::string call(vm_heap &h, const dt::uinteger &time_epoch, const dt::istring &format_s) {
    return format(h, time_epoch, format_s);
  }

  static dt::string format(vm_heap &h, const dt::uinteger &time_epoch, const str::ref &format_s) {
    //TODO: do without this null-char-putting.
    char *format_ptr = (char *)format_s.ptr();
    char *end_format_ptr = format_ptr + format_s.length();
    char prev_end_format_char = *end_format_ptr;
    *end_format_ptr = '\0';

    time_t time_sec = time_epoch/1000000;
    struct tm *tm_p = localtime(&time_sec);
    const size_t size_to_alloc = 1024;
    char *formatted_buf = h.alloc(size_to_alloc);
    const size_t max_formatted_length = size_to_alloc - 1;
    size_t bytes_written = strftime(formatted_buf, max_formatted_length, format_ptr, tm_p);

    *end_format_ptr = prev_end_format_char;

    NP1_ASSERT(
      bytes_written != 0,
      "Max formatted time length exceeded.  Max size is " + str::to_dec_str(max_formatted_length) + " bytes.");

    return dt::string(formatted_buf, bytes_written);
  }
};


struct io_net_url_get : public base {
  static const char *name() { return "io.net.url.get"; }
  static const char *description() {
    return "`io.net.url.get(url)` retrieves the resource specified by the supplied URL.  If the URL is an HTTP URL, "
            "`io.net.url.get` uses HTTP GET and returns all HTTP headers along with the resource.  Returns the empty string on "
            "network error.  The resource must be UTF-8.  Any invalid character sequences will be replaced with ?.";
  }

  inline static dt::string call(vm_heap &heap, const dt::string &url) {
    char *result_str = 0;
    size_t result_str_length = 0;
    ::np1::io::net::curl curl;
    if (!curl.get_utf8_safe(heap, url.to_string(), '?', &result_str, &result_str_length)) {
      return dt::string();
    }

    return dt::string(result_str, result_str_length);
  }
};


struct io_file_read : public base {
  static const char *name() { return "io.file.read"; }
  static const char *description() {
    return "`io.file.read(path)` reads the entire file specified by `path`.  Will decompress the file if it's in gzip format. "
            "Returns the empty string on error.  The (possibly decompressed) file must be UTF-8.  "
            "Any invalid character sequences will be replaced with ?.  "
            "To read a file once for the whole input stream, use the io.file.read stream operator.";
  }

  inline static dt::string call(vm_heap &heap, const dt::string &file_name) {
    rstd::string file_name_string(file_name.to_string());
    
    ::np1::io::gzfile f;
    if (!f.open_ro(file_name_string.c_str())) {
      return dt::string();
    }

    // Get the file size so we can provide an order-of-magnitude estimate for the buffer size.
    uint64_t file_size = 0;
    if (!::np1::io::file::get_size(file_name_string.c_str(), file_size)) {
      return dt::string();
    }

    // This cast down from uint64_t to size_t is ok because it's just an allocation estimate.
    ::np1::io::ext_heap_buffer_output_stream<vm_heap> buffer_output_stream(heap, file_size);
    unsigned char buffer[256 * 1024];
    size_t bytes_read = 0;
    bool result;
    while ((result = f.read_some(buffer, sizeof(buffer), &bytes_read)) && (bytes_read > 0)) {
      buffer_output_stream.write(buffer, bytes_read);
    }
    
    if (!result) {
      return dt::string();
    }
    
    char *final_buffer_ptr = (char *)buffer_output_stream.ptr();
    size_t total_size = buffer_output_stream.size();
    str::replace_invalid_utf8_sequences(final_buffer_ptr, total_size, '?');
    return dt::string(final_buffer_ptr, total_size);
  }
};


struct io_file_erase : public base {
  static const char *name() { return "io.file.erase"; }
  static const char *description() {
    return "`io.file.erase(path)` erases the path specified by `path`.  Returns false on error.";
  }

  inline static dt::boolean call(const dt::string &file_name) {
    return ::np1::io::file::erase(file_name.to_string().c_str());
  }
};


struct meta_shell : public base {
  enum { TEMP_FILE_READ_BUFFER_SIZE = 256 * 1024 };
  static const char *name() { return "meta.shell"; }
  static const char *description() {
    return "`meta.shell(command_line)` executes the command line on the local machine and returns the output as a UTF-8 string.  "
            "Any invalid character sequences in the output are replaced with ?.  "
            "To run a command once for the whole input/output stream, use the `meta.shell` stream operator.";
  }

  inline static dt::string call(vm_heap &heap, const dt::string &command) {
    // Set stdout to be a temporary file.
    FILE *tmpfp = tmpfile();
    NP1_ASSERT(tmpfp, "Unable to create temporary file for meta.shell function");
    ::np1::io::file tmpf;
    tmpf.from_handle(fileno(tmpfp));
    int saved_stdout = dup(1);
    dup2(tmpf.handle(), 1);

    // Execute the command.
    NP1_ASSERT(system(command.to_string().c_str()) >= 0, "system() failed for meta.shell");

    // Set stdout back to what it was and read the whole file.
    dup2(saved_stdout, 1);
    close(saved_stdout);
    tmpf.rewind();
    ::np1::io::mandatory_input_stream<np1::io::file> mandatory_tmpf(tmpf);
    ::np1::io::ext_heap_buffer_output_stream<vm_heap> command_output(heap, TEMP_FILE_READ_BUFFER_SIZE);
    mandatory_tmpf.copy(command_output);
    
    // Ensure that the command output is valid UTF-8.
    char *command_output_p = (char *)command_output.ptr();
    size_t command_output_size = command_output.size();
    str::replace_invalid_utf8_sequences(command_output_p, command_output_size, '?');

    return dt::string(command_output_p, command_output_size);
  }
};



}
}
}
}

#endif

