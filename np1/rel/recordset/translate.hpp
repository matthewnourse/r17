// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_REL_RECORDSET_TRANSLATE_HPP
#define NP1_REL_RECORDSET_TRANSLATE_HPP


#include "np1/rel/recordset/io/mandatory_translate_record_input_stream.hpp"

namespace np1 {
namespace rel {
namespace recordset {


class translate {
public:
  /// Read a stream of recordset file names and write out a stream of data.
  template <typename Input_Stream, typename Output_Stream>
  void from_recordset_stream_to_data_stream(
        const rstd::string &reliable_storage_local_root, const rstd::string &reliable_storage_remote_root,
        Input_Stream &input, Output_Stream &output, const rstd::vector<rel::rlang::token> &tokens) {
    io::mandatory_translate_record_input_stream<Input_Stream> translator_stream(input, reliable_storage_local_root,
                                                                                reliable_storage_remote_root);
    translator_stream.parse_headings().write(output);
    translator_stream.parse_records(record_callback<Output_Stream>(output));
  }

private:
  template <typename Output_Stream>
  struct record_callback {
    explicit record_callback(Output_Stream &output) : m_output(output) {}

    bool operator()(const record_ref &r) { 
      r.write(m_output);
      return true;
    }        

    Output_Stream &m_output;
  };
};


} // namespaces
}
}

#endif
