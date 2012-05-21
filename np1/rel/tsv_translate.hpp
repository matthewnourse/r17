// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_REL_TSV_TRANSLATE_HPP
#define NP1_REL_TSV_TRANSLATE_HPP

#include "np1/rel/rlang/rlang.hpp"
#include "np1/io/text_input_stream.hpp"

namespace np1 {
namespace rel {


class tsv_translate {
public:
  template <typename Input_Stream, typename Output_Stream>
  void from_tsv(Input_Stream &input, Output_Stream &output,
                  const rstd::vector<rlang::token> &args) {
    // If there are any arguments then treat them as heading names.
    rstd::vector<rstd::string> heading_names =
      (args.size() > 0) ? rlang::compiler::eval_to_strings_only(args) : rstd::vector<rstd::string>();
    if (heading_names.size() > 0) {
      write_headings(heading_names, output);
    }

    io::text_input_stream<Input_Stream>::read_all_line_by_line(
        input, from_tsv_line_callback<Output_Stream>(output, heading_names.size()));
  }


  template <typename Input_Stream, typename Output_Stream>
  void to_tsv(Input_Stream &input, Output_Stream &output,
              const rstd::vector<rlang::token> &args) {
    NP1_ASSERT(args.size() == 0, "rel.to_tsv expects no arguments.");
    input.parse_records(to_tsv_record_callback<Output_Stream>(output));
  }


private:
  template <typename T, typename Output>
  static void write_headings(const rstd::vector<T> &headings, Output &output) {
    rstd::vector<rstd::string> safe_header_fields;
    typename rstd::vector<T>::const_iterator heading_i = headings.begin();
    typename rstd::vector<T>::const_iterator heading_iz = headings.end();
    for (; heading_i != heading_iz; ++heading_i) {
      safe_header_fields.push_back(detail::helper::convert_to_valid_header_name(to_str_ref(*heading_i)));
    }

    record_ref::write_headings(output, safe_header_fields, rlang::dt::TYPE_STRING);    
  }
  
  static str::ref to_str_ref(const rstd::string &s) { return str::ref(s); }
  static str::ref to_str_ref(const str::ref &s) { return s; }
  
  template <typename Output>
  struct from_tsv_line_callback {
    explicit from_tsv_line_callback(Output &o, size_t number_known_headings)
      : m_output(o), m_number_fields(number_known_headings), m_headings_written(number_known_headings > 0) {}

    bool operator()(const str::ref &line, uint64_t line_number) {
      NP1_ASSERT(str::is_valid_utf8(line), "Line " + str::to_dec_str(line_number) + " is not a valid UTF-8 string");

      m_fields.clear();
      m_parsed_field_data.clear();

      const char *field = line.ptr();
      const char *line_end = field + line.length();
      const char *field_end;

      while ((field_end = (const char *)memchr(field, '\t', line_end - field))) {
        m_fields.push_back(get_field(field, field_end, line_number));
        field = field_end + 1;
      }

      // Add in the last field in the line.
      m_fields.push_back(get_field(field, line_end, line_number));

      // Sanity check and write out the record.
      if (m_number_fields > 0) {
        NP1_ASSERT(m_fields.size() == m_number_fields,
                    "Unexpected number of fields at line number " + str::to_dec_str(line_number) + ".  Expected: "
                    + str::to_dec_str(m_number_fields) + "  Found: " + str::to_dec_str(m_fields.size()));
      } else {
        m_number_fields = m_fields.size();
      }

      if (m_headings_written) {
        record_ref::write(m_output, m_fields);
      } else {
        write_headings(m_fields, m_output);
        m_headings_written = true;
      }

      return true;
    }

    inline str::ref get_field(const char *field, const char *field_end, uint64_t line_number) {
      // Check for a slash...if no slash then the field is good to go as-is.
      size_t field_length = field_end - field;
      if (!memchr(field, '\\', field_length)) {
        return str::ref(field, field_length);
      }

      rstd::string parsed_field;
      bool seen_slash = false;
    
      const char *p = field;
      for (; p < field_end; ++p) {
        char c = *p;
        switch (c) {
        case '\\':
          if (seen_slash) {
            parsed_field.push_back('\\');
            seen_slash = false;
          } else {
            seen_slash = true;
          }
          break;
  
        case 'n':
          if (seen_slash) {
            parsed_field.push_back('\n');
            seen_slash = false;
          } else {
            parsed_field.push_back('n');
          }
          break;
  
        case 'r':
          if (seen_slash) {
            parsed_field.push_back('\r');
            seen_slash = false;
          } else {
            parsed_field.push_back('r');
          }
          break;
  
        case 't':
          if (seen_slash) {
            parsed_field.push_back('\t');
            seen_slash = false;
          } else {
            parsed_field.push_back('t');
          }
          break;
  
        default:
          // If we've seen a slash then it's not an escape we accept, just put the slash and the character in there
          // as-is.
          if (seen_slash) {
            parsed_field.push_back('\\');
          }  
          parsed_field.push_back(c);
          break;
        }
      }

      if (seen_slash) {
        parsed_field.push_back('\\');
      }

      m_parsed_field_data.push_back(parsed_field);
      return str::ref(m_parsed_field_data.back());
    }


    Output &m_output;
    size_t m_number_fields;
    bool m_headings_written;
    rstd::vector<str::ref> m_fields;
    rstd::list<rstd::string> m_parsed_field_data;   
  };




  template <typename Output>
  struct to_tsv_record_callback {
    explicit to_tsv_record_callback(Output &o) : m_output(o), m_number_fields(-1) {}  

    bool operator()(const record_ref &r) {
      size_t number_fields = r.number_fields();
      if (m_number_fields >= 0) {
        NP1_ASSERT((size_t)m_number_fields == number_fields,
                    "Unexpected number of fields at record number " + str::to_dec_str(r.record_number())
                    + ".  Expected: " + str::to_dec_str(m_number_fields) + "   Actual: " + str::to_dec_str(number_fields));        
      } else {
        m_number_fields = number_fields;
      }

      size_t field_counter = 0;
      while (true) {
        write(r.field(field_counter));
        ++field_counter;
        if (field_counter >= number_fields) {
          m_output.write('\n');
          return true;
        }

        m_output.write('\t');
      }
    }        

    void write(const str::ref &field) {
      // Fast path: if there are no characters that need escaping just write out the field as-is.
      const char *p = field.ptr();
      const char *end = p + field.length();
      for (; (p < end) && (*p > '\r'); ++p) {        
      }
      
      if (p >= end) {
        m_output.write(field.ptr(), field.length());
        return;
      }
      
      // Dammit, we have to do more work.
      p = field.ptr();
            
      for (; p < end; ++p) {
        switch (*p) {
        case '\n':
          m_output.write("\\n");
          break;

        case '\r':
          m_output.write("\\r");
          break;

        case '\t':
          m_output.write("\\t");
          break;

        //TODO: more escapes?  Error on NUL ?

        default:
          m_output.write(*p);
          break;
        }
      }
    }
  

    Output &m_output;
    int m_number_fields;
  };



};


} // namespaces
}

#endif

