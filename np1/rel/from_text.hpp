// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_REL_FROM_TEXT_HPP
#define NP1_REL_FROM_TEXT_HPP

#include "np1/rel/rlang/rlang.hpp"

namespace np1 {
namespace rel {


class from_text {
public:
  typedef enum {
    ERROR_ON_NON_MATCHING,
    IGNORE_NON_MATCHING
  } on_non_matching_action_type;

private:
  static const char REPLACEMENT_STRING_DELIMITER_CHAR = '\n';

public:
  template <typename Input_Stream, typename Output_Stream>
  void operator()(Input_Stream &input, Output_Stream &output, const rstd::vector<rlang::token> &args,
                  on_non_matching_action_type on_non_matching_action) {
    // Sort out the arguments.
    rstd::vector<rstd::string> compiled_args = rlang::compiler::eval_to_strings_only(args);
    NP1_ASSERT(compiled_args.size() > 1,
                "rel.from_text's arguments are a regular expression then at least one header name.");

    rstd::string regular_expression = compiled_args[0];
    compiled_args.pop_front();

    // compiled_args now contains a list of heading names.  If the headings don't have a type then set the
    // type to string.
    rstd::vector<rstd::string>::iterator ii = compiled_args.begin();
    rstd::vector<rstd::string>::iterator iz = compiled_args.end();
    for (ii = compiled_args.begin(); ii != iz; ++ii) {
      str::ref type_tag = detail::helper::get_heading_type_tag(str::ref(*ii));
      if (!type_tag.is_null()) {
        rlang::dt::mandatory_from_string(type_tag);
      } else {
        *ii = detail::helper::make_typed_heading_name(rlang::dt::to_string(rlang::dt::TYPE_STRING), *ii);
      }
    }

    // Compile the regular expression.
    regex::pattern &compiled_pattern = regex::pattern_cache::get(str::ref(regular_expression), true);

    NP1_ASSERT(compiled_pattern.capture_count() == compiled_args.size(),
                "Number of regex captures (" + str::to_dec_str(compiled_pattern.capture_count())
                  + ") doesn't match the number of headings (" +
                  str::to_dec_str(compiled_args.size()) + ")");

    NP1_ASSERT(compiled_pattern.capture_count() <= 9, "Too many regex captures, currently-supported max is 9.");

    // Make the replacement string.
    rstd::string replacement_string;
    size_t n;
    for (ii = compiled_args.begin(), n = 1; ii != iz; ++ii, ++n) {
      replacement_string.push_back('\\');
      replacement_string.append(str::to_dec_str(n));
      replacement_string.push_back(REPLACEMENT_STRING_DELIMITER_CHAR);
    }    

    // Write out the headings.
    record_ref::write(output, compiled_args);

    // Now write out all the lines.
    rlang::vm_heap heap;
    io::text_input_stream<Input_Stream>::read_all_line_by_line(
            input, 
            line_callback<Output_Stream>(compiled_pattern, replacement_string, heap, output, on_non_matching_action));
  }

private:
  template <typename Output>
  struct line_callback {
    line_callback(regex::pattern &compiled_pattern, const rstd::string &replacement_string, rlang::vm_heap &heap,
                  Output &output, on_non_matching_action_type on_non_matching_action)
      : m_compiled_pattern(compiled_pattern),
        m_replacement_string(replacement_string),
        m_replacement_string_length(replacement_string.length()),
        m_heap(heap),
        m_output(output),
        m_on_non_matching_action(on_non_matching_action) {}

    bool operator()(const str::ref &line, uint64_t line_number) {
      char *result_str;
      size_t result_length;
      if (!m_compiled_pattern.replace(m_heap, line.ptr(), line.length(), m_replacement_string.c_str(),
                                        m_replacement_string_length, &result_str, &result_length)) {
        NP1_ASSERT(IGNORE_NON_MATCHING == m_on_non_matching_action,
                    "Line " + str::to_dec_str(line_number) + " failed match: " + line.to_string());
        return true;        
      }

      const char *p = result_str;
      const char *end = p + result_length;
      const char *end_field;
      m_fields.clear();
      for (; (p < end); p = end_field + 1) {
        end_field = (const char *)memchr(p, REPLACEMENT_STRING_DELIMITER_CHAR, end - p);
        NP1_ASSERT(end_field, "Replacement string delimiter char not found!");
        m_fields.push_back(str::ref(p, end_field - p));
      }

      record_ref::write(m_output, m_fields);
      m_heap.reset();
      return true;
    }

    regex::pattern &m_compiled_pattern;
    rstd::string m_replacement_string;
    size_t m_replacement_string_length;
    rlang::vm_heap &m_heap;    
    Output &m_output;
    rstd::vector<str::ref> m_fields;
    on_non_matching_action_type m_on_non_matching_action;
  };
};


} // namespaces
}

#endif

