// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_REL_STR_SPLIT_HPP
#define NP1_REL_STR_SPLIT_HPP


#include "np1/rel/rlang/rlang.hpp"
#include "np1/io/log.hpp"


#define NP1_REL_STR_SPLIT_COUNTER_HEADING_NAME "uint:_counter"


namespace np1 {
namespace rel {



class str_split {
public:
  template <typename Input_Stream, typename Output_Stream>
  void operator()(Input_Stream &input, Output_Stream &output,
                  const rstd::vector<rel::rlang::token> &tokens) {
    /* Get the headers. */
    record headings(input.parse_headings());

    // Parse the arguments and figure out if the regular expression is case sensitive.  
    size_t target_heading_id;
    regex::pattern split_regex_pattern;
    parse_arguments(headings, tokens, target_heading_id, split_regex_pattern);

    // Write out the headings.
    record_ref::write(output, headings.ref(), NP1_REL_STR_SPLIT_COUNTER_HEADING_NAME);        
    
    // Now do the real work.
    input.parse_records(
      record_callback<Output_Stream>(output, target_heading_id, split_regex_pattern));
  }


  static void parse_arguments(const record &headings, const rstd::vector<rel::rlang::token> &tokens,
                               size_t &target_heading_id, regex::pattern &split_regex_pattern) {
    rstd::vector<rstd::vector<rlang::token> > arg_expressions = rlang::compiler::split_expressions(tokens);
    NP1_ASSERT(arg_expressions.size() == 2, "Incorrect number of arguments to rel.str_split");
    rstd::vector<rstd::string> heading_names;
    rlang::compiler::compile_heading_name_list(arg_expressions[0], headings.ref(), heading_names);
    NP1_ASSERT(heading_names.size() == 1, "Incorrect number of headings supplied to rel.str_split");
    target_heading_id = headings.mandatory_find_heading(heading_names[0]);
    rstd::string split_regex_str = rlang::compiler::eval_to_string_only(arg_expressions[1]);

    // Figure out the type of the heading and check that it's ok.
    rlang::dt::data_type target_heading_type =
      rlang::dt::mandatory_from_string(
                  detail::helper::mandatory_get_heading_type_tag(headings.mandatory_field(target_heading_id)));

    NP1_ASSERT((rlang::dt::TYPE_STRING == target_heading_type) || (rlang::dt::TYPE_ISTRING == target_heading_type),
                "Invalid heading type argument for rel.str_split");

    bool is_case_sensitive = (rlang::dt::TYPE_STRING == target_heading_type);

    // Compile the regular expression.
    NP1_ASSERT(
      is_case_sensitive ? split_regex_pattern.compile(split_regex_str) : split_regex_pattern.icompile(split_regex_str),
      "Invalid regex pattern argument to rel.str_split: " + split_regex_str);    
  }


  static rstd::vector<rstd::string> make_output_headings(const rstd::vector<str::ref> &input_headings) {
    rstd::vector<rstd::string> result;
    rstd::vector<str::ref>::const_iterator i = input_headings.begin();
    rstd::vector<str::ref>::const_iterator iz = input_headings.end();
    
    for (; i != iz; ++i) {
      result.push_back(i->to_string());
    }

    result.push_back(NP1_REL_STR_SPLIT_COUNTER_HEADING_NAME);
    return result;
  }

private:
  template <typename Output_Stream>
  struct record_callback {
    explicit record_callback(Output_Stream &output,
                              size_t target_field_id,
                              regex::pattern &split_regex_pattern)
    : m_output(output), m_target_field_id(target_field_id), m_split_regex_pattern(split_regex_pattern) {}
    
    bool operator()(const record_ref &r) {
      str::ref target_field = r.mandatory_field(m_target_field_id);
      const char *match;
      size_t match_length;
      const char *current = target_field.ptr();
      const char *end = current + target_field.length();
      size_t counter = 0;

      for (counter = 0; do_match(current, end, &match, &match_length); ++counter, current = match + match_length) {
        write_output_record(r, str::ref(current, match - current), counter);
      }

      write_output_record(r, str::ref(current, end - current), counter);    
      return true;
    }

    bool do_match(const char *current, const char *end, const char **match_p, size_t *match_length_p) {
      return m_split_regex_pattern.match(current, end - current, match_p, match_length_p);
    }

    void write_output_record(const record_ref &input_record, const str::ref &target_field, size_t counter) {
      char counter_str[str::MAX_NUM_STR_LENGTH + 1];
      str::to_dec_str(counter_str, counter);

      size_t i;
      size_t number_fields = input_record.number_fields();
      for (i = 0; i < number_fields; ++i) {
        if (i != m_target_field_id) {
          m_output_fields.push_back(input_record.mandatory_field(i));
        } else {
          m_output_fields.push_back(target_field);
        }
      }
        
      record_ref::write(m_output, m_output_fields, counter_str);
      m_output_fields.clear();
    }
    
    Output_Stream &m_output;
    size_t m_target_field_id;
    regex::pattern &m_split_regex_pattern;
    rstd::vector<str::ref> m_output_fields;
  };
};


} // namespaces
}

#endif

