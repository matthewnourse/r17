// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_REL_DISTRIBUTED_WHERE_HPP
#define NP1_REL_DISTRIBUTED_WHERE_HPP


#include "np1/rel/rlang/rlang.hpp"
#include "np1/io/ordered_work_distributor.hpp"
#include "np1/io/log.hpp"


namespace np1 {
namespace rel {



class distributed_where {
public:
  template <typename Input_Stream, typename Output_Stream>
  void operator()(const std::string &reliable_storage_local_root,
                  const std::string &reliable_storage_remote_root,
                  const std::string &listen_endpoint,                  
                  Input_Stream &input, Output_Stream &output,
                  const std::vector<rel::rlang::token> &tokens) {
    log_info("Reading headers and compiling expression against headers.");

    /* Get the headers. */
    record headings(input.parse_headings());

    // Compile, just to check that the headings & tokens are likely to work
    // when we distribute.
    record empty_headings;
    rlang::vm vm = rlang::compiler::compile_single_expression(
                      tokens, headings.ref(), empty_headings.ref());

    // Check that the expression is actually a boolean expression.
    NP1_ASSERT(vm.return_type() == rlang::dt::TYPE_BOOL,
                "Expression is not a boolean expression");

    // Do the distribution.
    distributed::distribute(log_id(), headings, headings, "rel.where", reliable_storage_local_root,
                            reliable_storage_remote_root, listen_endpoint, input, output, tokens);
  }

private:
  static const char *log_id() { return "distributed_where"; }

  void log_info(const char *description) {
    io::log::info(log_id(), description);
  }
};


} // namespaces
}


#endif
