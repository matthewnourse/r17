// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_TEXT_STRIP_CR_HPP
#define NP1_TEXT_STRIP_CR_HPP

#include "np1/rel/rlang/rlang.hpp"

namespace np1 {
namespace text {


class strip_cr {
public:
  template <typename Input_Stream, typename Output_Stream>
  void operator()(Input_Stream &input, Output_Stream &output, const std::vector<rel::rlang::token> &args) {
    // Sort out the arguments.
    NP1_ASSERT(args.size() == 0, "text.strip_cr expects no arguments");

    // Do the work.
    char buffer[256 * 1024];
    size_t bytes_read = 0;
    bool read_result;
    while ((read_result = input.read_some(buffer, sizeof(buffer), &bytes_read)) && (bytes_read > 0)) {
      const char *p = buffer;
      const char *end = p + bytes_read;
      const char *cr_p;
      while ((p < end) && (cr_p = (const char *)memchr(p, '\r', end - p))) {
        output.write(p, cr_p - p);
        p = cr_p + 1;
      }

      output.write(p, end - p);
    }
  }
};


} // namespaces
}

#endif

