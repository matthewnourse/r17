// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_TEXT_UTF16_TO_UTF8_HPP
#define NP1_TEXT_UTF16_TO_UTF8_HPP

#include "np1/rel/rlang/rlang.hpp"
#include "np1/io/utf16_input_stream.hpp"

namespace np1 {
namespace text {


class utf16_to_utf8 {
public:
  template <typename Input_Stream, typename Output_Stream>
  void operator()(Input_Stream &input, Output_Stream &output, const rstd::vector<rel::rlang::token> &args) {
    // Sort out the arguments.
    NP1_ASSERT(args.size() == 0, "text.utf16_to_utf8 expects no arguments.");

    io::utf16_input_stream<Input_Stream> utf16_input(input);

    NP1_ASSERT(str::convert_utf16_to_utf8(utf16_input, output), "Invalid UTF-16 stream");
  }
};


} // namespaces
}

#endif

