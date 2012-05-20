// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_META_WORKER_HPP
#define NP1_META_WORKER_HPP

#include "np1/meta/script.hpp"
#include "np1/io/work_distributor.hpp"


namespace np1 {
namespace meta {

/// Receive & execute instructions from other processes.
class worker {
public:
  enum { RELIABLE_STORAGE_TIMEOUT_SECONDS = io::work_distributor::RELIABLE_STORAGE_TIMEOUT_SECONDS };

public:
  static void run(const std::string &reliable_storage_local_root,
                  const std::string &reliable_storage_remote_root,
                  const std::string &listen_endpoint) {
    io::work_distributor distributor(reliable_storage_local_root, reliable_storage_remote_root,
                                      listen_endpoint, false);
    io::reliable_storage rs(reliable_storage_local_root, reliable_storage_remote_root);

    processor_fn fn(rs);
    distributor.process_requests(fn);
  }

private:
  struct processor_fn {
    explicit processor_fn(io::reliable_storage &rs) : m_rs(rs) {}

    io::reliable_storage::id operator()(const io::reliable_storage::id &input_resource_id,
                                          const str::ref &script_text) {
      // Make a new output resource id based on the input plus the script.
      io::reliable_storage::id output_resource_id =
        io::reliable_storage::id::generate_temp(input_resource_id, script_text);
      
      // Open the input and output streams.
      io::reliable_storage::stream rs_input_stream(m_rs);
      NP1_ASSERT(m_rs.open_ro(input_resource_id, RELIABLE_STORAGE_TIMEOUT_SECONDS, rs_input_stream),
                  "Unable to open input stream in reliable storage");

      io::reliable_storage::stream rs_output_stream(m_rs);
      if (m_rs.exists(output_resource_id) || !m_rs.create_wo(output_resource_id, rs_output_stream)) {
        // Assume that we can't create the output stream because it already
        // exists, so this request is a retry of something that is already
        // done.  If we can't create it and it doesn't exist then the client
        // won't be able to open it either and will abort the whole operation
        // anyway, which is what would happen if we crash here.
        return output_resource_id;
      }
  
      // Now run the script.
      //TODO: make a special string input stream so that we don't have to copy
      // the script text.      
      std::string script_text_str(script_text.to_string());
      io::string_input_stream script_input(script_text_str);
      
      log_info("About to execute script.", script_text, input_resource_id, output_resource_id);
      script::run_from_stream(rs_input_stream.file_ref(), rs_output_stream.file_ref(), script_input, true, "[worker]");
      log_info("Finished executing script.", script_text, input_resource_id, output_resource_id);

      // The return value of close is very important because if the close fails
      // then our work has been lost, or not all the input stream could be retrieved.
      NP1_ASSERT(rs_output_stream.close(), "Unable to close reliable storage output stream");
      NP1_ASSERT(rs_input_stream.close(), "Unable to close reliable storage input stream");

      return output_resource_id;
    }

    io::reliable_storage &m_rs;
  };


  static const char *log_id() { return "worker"; }

  static void log_info(const char *description, const str::ref &script_text,
                        const io::reliable_storage::id &input_resource_id,
                        const io::reliable_storage::id &output_resource_id) {
    io::log::info(log_id(), description, " script_text=", script_text.to_string().c_str(),
                  " input_resource_id=", input_resource_id.to_string().c_str(),
                  " output_resource_id=", output_resource_id.to_string().c_str());
  }
};

/// Helper function to avoid circular #includes.
void worker_run(const std::string &reliable_storage_local_root, const std::string &reliable_storage_remote_root,
                const std::string &listen_endpoint) {
  worker::run(reliable_storage_local_root, reliable_storage_remote_root, listen_endpoint);
}



} /// namespaces
}



#endif
