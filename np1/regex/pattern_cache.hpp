// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_NP1_REGEX_PATTERN_CACHE_HPP
#define NP1_NP1_REGEX_PATTERN_CACHE_HPP

#include "np1/regex/pattern.hpp"
#include "np1/hash/fnv1a64.hpp"

namespace np1 {
namespace regex {


class pattern_cache {
public:
  enum { CACHE_HASH_TABLE_SIZE = 2048 };

public:
  ~pattern_cache() {}

  static pattern &get(const str::ref &pattern_str, bool case_sensitive) {
    static pattern_cache pc;
    return pc.do_get(pattern_str, case_sensitive);
  }

private:
  // Disable copy and public construction
  pattern_cache() {}
  pattern_cache(const pattern_cache &);
  pattern_cache &operator = (const pattern_cache &);

private:
  pattern &do_get(const str::ref &pattern_str, bool case_sensitive) {
    size_t pattern_str_len = pattern_str.length();
    uint64_t hval = hash::fnv1a64::add(pattern_str.ptr(),
                                        pattern_str_len, hash::fnv1a64::init());
    size_t offset = (size_t)hval & (CACHE_HASH_TABLE_SIZE-1);
    entry *e = &m_entries[offset];
    if ((str::cmp(pattern_str, e->m_pattern_string) != 0) || (case_sensitive != e->m_is_case_sensitive)) {    
      // As this is just a cache, overwrite the existing pattern.
      e->m_pattern.clear();
      bool compile_result = case_sensitive ? e->m_pattern.compile(pattern_str)
                              : e->m_pattern.icompile(pattern_str);
      
      NP1_ASSERT(compile_result,
                  "Unable to compile pattern: " + pattern_str.to_string());
      e->m_pattern_string = pattern_str.to_string();
      e->m_is_case_sensitive = case_sensitive;
    }

    return e->m_pattern;
  }


private:
  struct entry {
    rstd::string m_pattern_string;
    regex::pattern m_pattern;
    bool m_is_case_sensitive;
  };
  
  entry m_entries[CACHE_HASH_TABLE_SIZE];
};

} // namespaces
}


#endif
