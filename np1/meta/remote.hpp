// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_META_REMOTE_HPP
#define NP1_META_REMOTE_HPP



namespace np1 {
namespace meta {

// Run some r17 code remotely.  NOTE that currently only file input & output streams are supported.
class remote {
public:
  static void run(io::unbuffered_stream_base &input, io::unbuffered_stream_base &output,
                  const rstd::vector<rel::rlang::token> &tokens) {
    // Split the arguments into a hostname and the rest.
    rstd::vector<rstd::vector<rel::rlang::token> > expressions = rel::rlang::compiler::split_expressions(tokens);
    NP1_ASSERT(expressions.size() >= 2, "meta.remote expects a hostname argument and a script argument");
    rstd::string hostname = rel::rlang::compiler::eval_to_string_only(expressions[0]);

    // Get the rest of the arguments as a string.
    rstd::vector<rstd::vector<rel::rlang::token> >::const_iterator i = expressions.begin();
    ++i; // Skip over the hostname.
    rstd::vector<rstd::vector<rel::rlang::token> >::const_iterator iz = expressions.end();

    rstd::string r17_script;
    io::string_output_stream r17_script_sos(r17_script);
    
    for (; i != iz; ++i) {
      rel::rlang::io::token_writer::mandatory_write(r17_script_sos, *i);      
    }

    r17_script_sos.write(';');

    // Escape the string for use on a bash command line.
    rstd::string escaped_r17_script;
    io::string_output_stream escaped_r17_script_sos(escaped_r17_script);
    str::write_bash_escaped_string(r17_script, escaped_r17_script_sos);

    // Prepare the arguments ready for the exec.
    rstd::vector<rstd::string> exec_args;
    exec_args.push_back("ssh");
    exec_args.push_back(hostname);
    exec_args.push_back("r17");
    exec_args.push_back(escaped_r17_script);

    // Fork then exec so that we can return just like the other operators do and
    // so we can do useful things with stderr.
    int stderr_pipe[2];
    process::mandatory_pipe_create(stderr_pipe);
    pid_t ssh_child_pid = process::mandatory_fork();
    if (0 == ssh_child_pid) {
      // Child.
      close(stderr_pipe[0]);

      // If the host is localhost then just execute the script without the cost
      // of an exec
      if (str::cmp(hostname, "localhost") == 0) {
        io::file output_file;
        dup2(stderr_pipe[1], 2);
        script_run(input, output, r17_script);
        exit(0);
      } else {
        process::mandatory_execvp(exec_args, input.handle(), output.handle(), stderr_pipe[1]);
        NP1_ASSERT(false, "Unreachable code after mandatory_execvp");
      }
    }

    // Parent. Read stderr stream.
    close(stderr_pipe[1]);
    read_and_prefix_stderr(hostname, stderr_pipe[0]);
    process::mandatory_wait_for_child(ssh_child_pid);
  }


  // Read the file descriptor, copying to stderr and prefixing each line with helpful info.
  static void read_and_prefix_stderr(const rstd::string &hostname, int fd) {
    FILE *fp = fdopen(fd, "r");
    NP1_ASSERT(fp, "meta.remote: fdopen failed");
    char line[256 * 1024];
    while (fgets(line, sizeof(line)-1, fp)) {
      fprintf(stderr, "meta.remote(%s): %s", hostname.c_str(), line);
    }
  }
};

} /// namespaces
}



#endif
