// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_META_SHELL_HPP
#define NP1_META_SHELL_HPP



namespace np1 {
namespace meta {

// Execute a command in the system shell once, piping the input to the command's stdin and the command's
// stdout to the output.
class shell {
public:
  static void run(io::unbuffered_stream_base &input, io::unbuffered_stream_base &output,
                  const rstd::vector<rel::rlang::token> &tokens) {
    rstd::string command = rel::rlang::compiler::eval_to_string_only(tokens);
    run(input, output, command);
  }

  static void run(io::unbuffered_stream_base &input, io::unbuffered_stream_base &output,
                  const rstd::string &command) {
  
    // Set stdin to be the supplied input stream.
    int saved_stdin = dup(0);
    dup2(input.handle(), 0);

    // Set stdout to be the supplied output stream.
    int saved_stdout = dup(1);
    dup2(output.handle(), 1);

    // Execute the command.
    NP1_ASSERT(system(command.c_str()) >= 0, "system() failed for meta.shell stream operator.");

    // Set stdin & stdout back to what they were.
    dup2(saved_stdin, 0);
    dup2(saved_stdout, 1);
    close(saved_stdin);
    close(saved_stdout);   
  }
};

} /// namespaces
}



#endif
