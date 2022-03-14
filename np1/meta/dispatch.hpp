// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_META_DISPATCH_HPP
#define NP1_META_DISPATCH_HPP

#include "np1/meta/stream_op_table.hpp"
#include "np1/meta/script.hpp"

namespace np1 {
namespace meta {



class dispatch {
public:
  // Just pass the whole program argument list here.
  static int from_main(int argc, const char *argv[]) {
    const char *real_program_name = argv[0];
  
    NP1_ASSERT(argc >= 2, get_usage(real_program_name));
    NP1_ASSERT(str::is_valid_utf8(argc, argv), "Arguments are not valid UTF-8 strings");
  
    int fake_argc = argc - 1;
    const char **fake_argv = &argv[1];

    rstd::vector<rstd::string> args = str::argv_to_string_vector(fake_argc, fake_argv);

    io::file stdin_f;
    stdin_f.from_stdin();

    io::file stdout_f;
    stdout_f.from_stdout();

    run_once(stdin_f, stdout_f, args);
  
    return 0;
  }


private:
  static rstd::string get_usage(const char *real_program_name) {
    rstd::string result;
    io::string_output_stream sos(result);
    help::markdown::usage(sos);
    return result;
  }
    
  template <typename Input_Stream, typename Output_Stream>
  static void run_once(Input_Stream &input, Output_Stream &output,
                        const rstd::vector<rstd::string> &args) {
    rstd::string op_name = args[0];    

    size_t stream_op_id = stream_op_table::find(op_name.c_str());
    // If the operator name is not found maybe this is the file name of a script or the script itself.
    if ((size_t)-1 == stream_op_id) {
      script::run(input, output, args[0], ((args.size() > 1) ? args[1] : rstd::string()));
    } else {      
      stream_op_table::call(stream_op_id, input, output, args, "[command-line-argument]", 1);
    }
  }
};



} // namespaces
}

#endif
