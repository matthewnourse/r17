// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_LANG_R_HPP
#define NP1_LANG_R_HPP


#include "np1/io/named_temp_file.hpp"
#include "np1/lang/detail/helper.hpp"



namespace np1 {
namespace lang {

// Execute an inline R script.
class r {
public:
  static void run(io::unbuffered_stream_base &input, io::unbuffered_stream_base &output,
                  const rstd::vector<rel::rlang::token> &tokens) {
    NP1_ASSERT((tokens.size() == 1) && (tokens[0].type() == rel::rlang::token::TYPE_UNPARSED_CODE_BLOCK),
               "lang.R's argument is an R code block enclosed in "
                  NP1_TOKEN_UNPARSED_CODE_BLOCK_DELIMITER "s.");

    rstd::string user_r_code = tokens[0].text();

    // Read the headers so we can generate our supporting code.
    io::mandatory_record_input_stream<io::unbuffered_stream_base, rel::record, rel::record_ref>
        record_input_stream(input);

    rel::record headers = record_input_stream.parse_headings();

    rstd::string combined_r_code = r_helper_code() + r_input_table_code(headers.ref()) + user_r_code;

    io::named_temp_file temp_file;

    // Write out the R code to the temp file.
    io::mandatory_output_stream<io::file> temp_stream(temp_file.real_file());
    temp_stream.write(combined_r_code.c_str());
    temp_stream.close();

    // Run the R code and the stream operators that will translate to and from TSV.
    // rel.to_tsv doesn't care if there are headers or not.  We need to fork and to some post-processing translation
    // because R returns booleans as TRUE or FALSE and there's no way to change this within R.    
    rstd::string r17_script =
      "rel.to_tsv() | meta.shell('Rscript --slave " + rstd::string(temp_file.file_name()) + "') | rel.from_tsv();";

    int translation_pipe[2];
    process::mandatory_pipe_create(translation_pipe);
    pid_t child_pid = process::mandatory_fork();
    if (0 == child_pid) {
      // Child.
      close(translation_pipe[0]);
      io::file translation_file;
      translation_file.from_handle(translation_pipe[1]);
      meta::script_run(input, translation_file, r17_script);
      return;
    }

    // Parent.
    close(translation_pipe[1]);
    io::file translation_f;
    translation_f.from_handle(translation_pipe[0]);
    io::mandatory_record_input_stream<io::file, rel::record, rel::record_ref> translation_record_stream(translation_f);
    rel::record headings = translation_record_stream.parse_headings();
    headings.write(output);
    rstd::vector<size_t> bool_column_ids = find_bool_columns(headings);
    if (bool_column_ids.empty()) {
      translation_record_stream.copy(output);
    } else {
      typedef io::buffered_output_stream<io::unbuffered_stream_base> buffered_output_type;
      buffered_output_type buffered_output(output);
      typedef io::mandatory_output_stream<buffered_output_type> mandatory_output_type;
      mandatory_output_type mandatory_output(buffered_output);
      record_translator_callback<mandatory_output_type> callback(
                                                          bool_column_ids, headings.number_fields(), mandatory_output);
      translation_record_stream.parse_records(callback);
    }    
  }

  static rstd::string r_helper_code_markdown() {
    return detail::helper::code_to_markdown(r_helper_code());
  }

private:
  static const char *r_helper_code() {
    return
      "r17WriteTable <- function(colNames, t) {\n"
      "    write.table(sep=\"\\t\", quote=FALSE, row.names=FALSE, col.names=colNames, t)\n"
      "}\n";
  }
      

  static rstd::string r_input_table_code(const rel::record_ref &headings) {
    rstd::string code =
      "r17InputTable <- read.table(header=FALSE, file=\"stdin\", sep=\"\\t\", quote=\"\", col.names=c(";

    // Column names.
    size_t heading_i;
    for (heading_i = 0; heading_i < headings.number_fields(); ++heading_i) {
      str::ref heading = headings.mandatory_field(heading_i);
      code.push_back('"');
      code.append(rel::detail::helper::get_heading_without_type_tag(heading).to_string());
      code.push_back('"');
      if (heading_i + 1 < headings.number_fields()) {
        code.push_back(',');
      }
    }

    code.append("), colClasses=c(");

    // Column types.
    for (heading_i = 0; heading_i < headings.number_fields(); ++heading_i) {
      str::ref heading = headings.mandatory_field(heading_i);
      code.push_back('"');
      code.append(r_type_from_r17_type(rel::detail::helper::mandatory_get_heading_type_tag(heading)));
      code.push_back('"');
      if (heading_i + 1 < headings.number_fields()) {
        code.push_back(',');
      }
    }

    code.append("))\n");

    return code;
  }

  static const char *r_type_from_r17_type(const str::ref &type_tag) {
    switch (rel::rlang::dt::mandatory_from_string(type_tag)) {
      case rel::rlang::dt::TYPE_STRING:
      case rel::rlang::dt::TYPE_ISTRING:
      case rel::rlang::dt::TYPE_IPADDRESS:
        return "character";

      case rel::rlang::dt::TYPE_INT: 
      case rel::rlang::dt::TYPE_UINT:
        return "integer";
        break;

      case rel::rlang::dt::TYPE_DOUBLE:
        return "numeric";

      case rel::rlang::dt::TYPE_BOOL: 
        return "logical";
    }

    NP1_ASSERT(false, "Unreachable");
    return NULL;
  }

  static rstd::vector<size_t> find_bool_columns(const rel::record &headings) {
    rstd::vector<size_t> bool_column_ids;
    size_t heading_i;

    for (heading_i = 0; heading_i < headings.number_fields(); ++heading_i) {
      str::ref heading = headings.mandatory_field(heading_i);
      str::ref heading_type_tag = rel::detail::helper::mandatory_get_heading_type_tag(heading);
      if (rel::rlang::dt::mandatory_from_string(heading_type_tag) == rel::rlang::dt::TYPE_BOOL) {
        bool_column_ids.push_back(heading_i);        
      }
    }

    return bool_column_ids;
  }

  template <typename Output_Stream>
  struct record_translator_callback {
    record_translator_callback(const rstd::vector<size_t> &bool_column_ids, size_t number_fields, Output_Stream &output)
      : m_bool_column_ids(bool_column_ids), m_number_fields(number_fields), m_output(output) {
      m_output_fields.resize(m_number_fields);
    }

    bool operator()(const rel::record_ref &r) {
      static const str::ref true_string_ref = str::from_bool(true);
      static const str::ref false_string_ref = str::from_bool(false);

      size_t input_field_id;
      rstd::vector<str::ref>::iterator output_field_iter = m_output_fields.begin();
      for (input_field_id = 0; input_field_id < m_number_fields; ++input_field_id, ++output_field_iter) {
        str::ref input_field = r.mandatory_field(input_field_id);
        if (is_in(m_bool_column_ids, input_field_id)) {
          if ((input_field.length() > 0) && ('T' == *input_field.ptr())) {
            *output_field_iter = true_string_ref;
          } else {
            *output_field_iter = false_string_ref;
          }
        } else {
          *output_field_iter = input_field;
        }
      }

      rel::record_ref::write(m_output, m_output_fields);
      return true;
    }

    static bool is_in(const rstd::vector<size_t> &vec, const size_t field_id) {
      rstd::vector<size_t>::const_iterator ii = vec.begin();
      rstd::vector<size_t>::const_iterator iz = vec.end();
      for (; ii != iz; ++ii) {
        if (field_id == *ii) {
          return true;
        }
      }

      return false;
    }

    rstd::vector<size_t> m_bool_column_ids;
    size_t m_number_fields;
    rstd::vector<str::ref> m_output_fields;
    Output_Stream &m_output;    
  };
};

} /// namespaces
}



#endif
