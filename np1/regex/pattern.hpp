// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_NP1_REGEX_PATTERN_HPP
#define NP1_NP1_REGEX_PATTERN_HPP


#include <pcre.h>
#include "np1/io/ext_heap_buffer_output_stream.hpp"

namespace np1 {
namespace regex {

/// PCRE regular expression.
class pattern {
private:
  //TODO: PCRE's definition of "ok UTF-8" differs from what is in str.hpp.  We need to reconcile this.
  enum { BASE_PCRE_COMPILE_OPTIONS = PCRE_DOTALL | PCRE_DOLLAR_ENDONLY | PCRE_UTF8 };  
  enum { REPLACEMENT_BUFFER_ALLOCATION_SIZE = 4096 };
  
  typedef enum { NONMATCH_INCLUDE, NONMATCH_EXCLUDE } nonmatch_inclusion_type;

public:
  pattern() : m_pcre(0), m_extra(0), m_capture_count(0) {}
  ~pattern() { clear(); }

  // Compile a pattern, return false on error.  This MUST be called first.
  bool compile(const char *pattern_str) {
    return compile(pattern_str, BASE_PCRE_COMPILE_OPTIONS);
  }

  bool compile(const rstd::string &pattern_str) {
    return compile(pattern_str.c_str());
  }

  bool compile(const str::ref &s) {
    return compile(s.to_string());  
  }

  // Compile a pattern, preparing it for a case-insensitive match.
  bool icompile(const char *pattern_str) {
    return compile(pattern_str, BASE_PCRE_COMPILE_OPTIONS | PCRE_CASELESS);
  }

  bool icompile(const rstd::string &pattern_str) {
    return icompile(pattern_str.c_str());
  }

  bool icompile(const str::ref &s) {
    return icompile(s.to_string());  
  }


  bool match(const char *haystack, size_t haystack_length) {
    return (run(haystack, haystack_length, 0) > 0);
  }

  bool match(const str::ref &haystack) {
    return match(haystack.ptr(), haystack.length());
  }

  bool match(const char *haystack, size_t haystack_length, const char **match_p, size_t *match_length_p) {
    if (run(haystack, haystack_length, 0) <= 0) {
      return false;    
    }

    *match_p = &haystack[m_ovector[0]];
    *match_length_p = m_ovector[1] - m_ovector[0];
    return true;
  }

  template <typename Heap, typename Output>
  bool replace(Heap &h, const char *haystack, size_t haystack_length, size_t haystack_start_offset,
               const char *replacement_spec, size_t replacement_spec_length, Output &output,
               nonmatch_inclusion_type nonmatch_inclusion) {
    int number_matches = run(haystack, haystack_length, haystack_start_offset);
    if (number_matches <= 0) {
      return false;
    }    
    
    if (NONMATCH_INCLUDE == nonmatch_inclusion) {
      output.write(haystack + haystack_start_offset, get_ovector_start_offset(0) - haystack_start_offset);
    }
    
    //TODO: precompile the replacement_spec string.
    const char *replacement_spec_p = replacement_spec;
    const char *replacement_spec_end = replacement_spec_p + replacement_spec_length;
    bool seen_slash = false;
    for (; replacement_spec_p < replacement_spec_end; ++replacement_spec_p) {
      char c = *replacement_spec_p;
      if ('\\' == c) {
        if (!seen_slash) {
          seen_slash = true;
        } else {
          output.write('\\');
          seen_slash = false;
        }
      } else if (seen_slash && isdigit(*replacement_spec_p)) {
        size_t substring_id = c - '0';
        if (substring_id < (size_t)number_matches) {
          NP1_ASSERT(substring_id * 2 + 1 < m_ovector.size(),
                      "Invalid substring id: " + str::to_dec_str(substring_id));
          size_t start_offset = get_ovector_start_offset(substring_id);
          size_t end_offset = get_ovector_end_offset(substring_id);
          output.write(&haystack[start_offset], end_offset - start_offset);
        }        
        seen_slash = false;
      } else {
        output.write(c);
        seen_slash = false;
      }
    }
    
    return true;
  }

  

  template <typename Heap>
  bool replace(Heap &h, const char *haystack, size_t haystack_length, const char *replacement_spec,
                size_t replacement_spec_length, char **result_str_p, size_t *result_str_length_p) {
    io::ext_heap_buffer_output_stream<Heap> buffer_output(h, REPLACEMENT_BUFFER_ALLOCATION_SIZE);
    io::mandatory_output_stream<io::ext_heap_buffer_output_stream<Heap> > output(buffer_output);
  
    bool result = replace(h, haystack, haystack_length, 0, replacement_spec, replacement_spec_length, output,
                          NONMATCH_EXCLUDE);    
    if (result) {
      *result_str_p = (char *)buffer_output.ptr();
      *result_str_length_p = buffer_output.size();
    }
    
    return result;
  }


  template <typename Heap>
  bool replace(Heap &h, const str::ref &haystack, const str::ref &replacement_spec, str::ref &result_str) {
    char *result_c_str;
    size_t result_c_str_length;
    bool result = replace(h, haystack.ptr(), haystack.length(), replacement_spec.ptr(), replacement_spec.length(),
                          &result_c_str, &result_c_str_length);
    if (result) {
      result_str = str::ref(result_c_str, result_c_str_length);      
    }
  
    return result;
  }

  template <typename Heap>
  bool replace_all(Heap &h, const str::ref &haystack, const str::ref &replacement_spec, str::ref &result_str) {
    io::ext_heap_buffer_output_stream<Heap> buffer_output(h, REPLACEMENT_BUFFER_ALLOCATION_SIZE);
    io::mandatory_output_stream<io::ext_heap_buffer_output_stream<Heap> > output(buffer_output);
    size_t haystack_offset = 0;
    bool one_success = false;

    while (replace(h, haystack.ptr(), haystack.length(), haystack_offset, replacement_spec.ptr(),
                   replacement_spec.length(), output, NONMATCH_INCLUDE)) {
      one_success = true;
      haystack_offset = get_ovector_end_offset(0);
    }
    
    if (one_success) {
      output.write(haystack.ptr() + haystack_offset, haystack.length() - haystack_offset);
      result_str = str::ref((char *)buffer_output.ptr(), buffer_output.size());      
    }
  
    return one_success;
  }


  size_t capture_count() const { return m_capture_count; }

  void clear() {
    free(m_pcre);
    m_pcre = 0;
    free(m_extra);
    m_extra = 0;
    m_ovector.clear();
  }

private:
  bool compile(const char *pattern_str, int options) {
    clear();
    const char *errptr = 0;
    int erroffset;
    m_pcre = pcre_compile(pattern_str, options, &errptr, &erroffset, 0);
    if (m_pcre) {
      m_extra = pcre_study(m_pcre, 0, &errptr);
      int back_ref_max;
      int capture_count;
      NP1_ASSERT(pcre_fullinfo(m_pcre, m_extra, PCRE_INFO_BACKREFMAX, &back_ref_max) == 0, "pcre_fullinfo failed!");
      NP1_ASSERT(pcre_fullinfo(m_pcre, m_extra, PCRE_INFO_CAPTURECOUNT, &capture_count) == 0, "pcre_fullinfo failed!");
      // The first third of the ovector is used for holding capture inforation, 2 entries per capture.
      // The last third is used for PCRE's internal machinations, including for backreferences.
      // It should be divisible by 3.  
      size_t ovector_size = back_ref_max + capture_count + 1;
      NP1_ASSERT(ovector_size < NP1_SIZE_T_MAX/3, "Too many back references and/or captures");
      ovector_size *= 3;
      m_ovector.resize(ovector_size);
      m_capture_count = capture_count;
    }
    return !!m_pcre;
  }

  // Returns the number of substrings matched.
  size_t run(const char *haystack, size_t haystack_length, size_t start_offset) {
    // PCRE library only accepts signed int lengths.
    NP1_ASSERT(haystack_length < INT_MAX, "Haystack is too big for regex matching.");
    NP1_ASSERT(start_offset < INT_MAX, "Start offset is too big for regex matching.");
    NP1_ASSERT(m_pcre, "No compiled regex!");

    // PCRE library doesn't like null haystacks even if the length is 0.
    if (!haystack) {
      NP1_ASSERT(0 == haystack_length, "Haystack is null but haystack length is non-null");
      haystack = "";
    }
    
    int haystack_length_int = (int)haystack_length;
    int start_offset_int = (int)start_offset;
    //TODO: add resource use limits.
    int rc = pcre_exec(m_pcre, m_extra, haystack, haystack_length_int, start_offset_int, 0,
                       &m_ovector[0], m_ovector.size());
    if (rc < 0) {
      NP1_ASSERT((rc != PCRE_ERROR_BADUTF8),
                  "Invalid UTF-8 sequence found in haystack!");
      
      NP1_ASSERT((rc == PCRE_ERROR_NOMATCH) || (rc == PCRE_ERROR_PARTIAL),
                  "pcre_exec failed!  rc=" + str::to_dec_str(rc));
      return 0;
    }

    return rc;
  }

  size_t get_ovector_start_offset(size_t substring_id) const {
    return m_ovector[substring_id * 2];
  }
  
  size_t get_ovector_end_offset(size_t substring_id) const {
    return m_ovector[(substring_id * 2) + 1];
  }
  

private:
  /// Disable copy
  pattern(const pattern &);
  pattern &operator = (const pattern &);

private:
  pcre *m_pcre;
  pcre_extra *m_extra;
  rstd::vector<char> m_lower_buffer;
  rstd::vector<int> m_ovector;
  size_t m_capture_count;
};


} // namespaces
} 

#endif

