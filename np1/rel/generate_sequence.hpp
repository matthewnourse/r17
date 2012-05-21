// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_REL_GENERATE_SEQUENCE_HPP
#define NP1_REL_GENERATE_SEQUENCE_HPP


#include "np1/rel/rlang/rlang.hpp"


#define NP1_REL_GENERATE_SEQUENCE_OUTPUT_HEADING_NAME "int:_seq" 

namespace np1 {
namespace rel {



class generate_sequence {
public:
  template <typename Input_Stream, typename Output_Stream>
  void operator()(Input_Stream &input, Output_Stream &output,
                  const rstd::vector<rel::rlang::token> &tokens) {
    rstd::vector<rstd::pair<rstd::string, rlang::dt::data_type> > arguments = rlang::compiler::eval_to_strings(tokens);

    NP1_ASSERT((arguments.size() == 2)
                  && (rlang::dt::TYPE_INT == arguments[0].second)
                  && (rlang::dt::TYPE_INT == arguments[1].second),
                "rel.generate_sequence(start, end) expects 2 integer arguments.");
    int64_t start_sequence = str::dec_to_int64(arguments[0].first);
    int64_t end_sequence = str::dec_to_int64(arguments[1].first);
    int64_t i;

    record_ref::write(output, NP1_REL_GENERATE_SEQUENCE_OUTPUT_HEADING_NAME);
    for (i = start_sequence; i < end_sequence; ++i) {
      char num_str[str::MAX_NUM_STR_LENGTH];
      str::to_dec_str(num_str, i);
      record_ref::write(output, num_str);
    }
  }
};


} // namespaces
}

#endif

