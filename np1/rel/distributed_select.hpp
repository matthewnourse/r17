// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_REL_DISTRIBUTED_SELECT_HPP
#define NP1_REL_DISTRIBUTED_SELECT_HPP


#include "np1/rel/rlang/rlang.hpp"
#include "np1/io/ordered_work_distributor.hpp"
#include "np1/io/log.hpp"


namespace np1 {
namespace rel {



class distributed_select {
public:
  template <typename Input_Stream, typename Output_Stream>
  void operator()(const rstd::string &reliable_storage_local_root,
                  const rstd::string &reliable_storage_remote_root,
                  const rstd::string &listen_endpoint,                  
                  Input_Stream &input, Output_Stream &output,
                  const rstd::vector<rel::rlang::token> &tokens) {
    log_info("Reading headers and compiling expression against headers.");

    /* Get the headers. */
    record input_headings(input.parse_headings());

    // Compile to check that the headings & tokens are likely to work
    // when we distribute.  Also we need to write out the correct set of headings.
    rstd::vector<rlang::compiler::vm_info> vm_infos;
    rlang::vm_heap heap;
    rlang::compiler::compile_select(tokens, input_headings.ref(), vm_infos);

    log_info("Getting resource id field id and setting up output headers.");

    // Get the resource id field id, this is the only field that we're interested
    // in, the rest are just placeholders to help catch errors early.
    size_t resource_id_field_id = input_headings.mandatory_find_field(
                                      NP1_REL_DISTRIBUTED_RESOURCE_ID_FIELD_NAME);

    // Get the recordset stream headings so the next distributed operator can do its thing.
    rstd::vector<rstd::string> output_heading_strings = select::make_output_headings(vm_infos);
    output_heading_strings.push_back(input_headings.mandatory_field(resource_id_field_id).to_string());
    record output_headings(output_heading_strings, 0);

    log_info("Distributing.");

    // Do the distribution.
    distributed::distribute(log_id(), input_headings, output_headings, "rel.select", reliable_storage_local_root,
                            reliable_storage_remote_root, listen_endpoint, input, output, tokens);  
  }

private:
  static const char *log_id() { return "distributed_select"; }

  void log_info(const char *description) {
    io::log::info(log_id(), description);
  }
};


} // namespaces
}


#endif
