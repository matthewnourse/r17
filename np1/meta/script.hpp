// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_META_SCRIPT_HPP
#define NP1_META_SCRIPT_HPP

#include "np1/meta/stream_op_table.hpp"
#include "np1/process.hpp"
#include "np1/io/path.hpp"

namespace np1 {
namespace meta {

/// Read and execute a complete script.
class script {
public:
  typedef enum {
    JOINER_PIPE = '|',
    JOINER_SEMICOLON = ';'
  } joiner_type;

private:
  struct stream_op_call {  
    // -1 if it's not a call to a builtin.
    size_t builtin_stream_op_id;    
    rel::rlang::token stream_op_token;    
    rstd::vector<rel::rlang::token> arguments;
    bool output_is_recordset_stream;
    stream_op_table_io_type_type output_type;
    stream_op_table_io_type_type input_type;
    size_t script_line_number;
  };
  
  class pipeline {
  public:
    void push_back(const stream_op_call &call, const rel::rlang::token &tok) {
      // Check that the new call's input is compatible with the existing pipeline's output.
      // If the existing pipeline is empty then we just have to trust that the user knows what they are
      // doing- maybe they are piping data in from an external source.      
      if (!m_calls.empty()) {
        stream_op_table_io_type_type pline_output_type = m_calls.back().output_type;
        const char *pline_last_name = m_calls.back().stream_op_token.text();

        stream_op_table_io_type_type next_stream_op_input_type = call.input_type;
        const char *next_stream_op_name = call.stream_op_token.text();
        bool valid = (pline_output_type == next_stream_op_input_type)
                      || ((pline_output_type == STREAM_OP_TABLE_IO_TYPE_ANY)
                            && (next_stream_op_input_type != STREAM_OP_TABLE_IO_TYPE_NONE))
                      || ((pline_output_type != STREAM_OP_TABLE_IO_TYPE_NONE)
                            && (next_stream_op_input_type == STREAM_OP_TABLE_IO_TYPE_ANY));
        if (!valid) {
          tok.assert(false,
                      ("Incompatible stream I/O types.  "  + rstd::string(pline_last_name) + "'s output ("
                        + rstd::string(stream_op_table_io_type_to_text(pline_output_type)) + ") is not compatible with "
                        + next_stream_op_name + "'s input ("
                        + rstd::string(stream_op_table_io_type_to_text(next_stream_op_input_type)) + ")").c_str());
        }
      }

      // If we get to here then we're in the clear.
      m_calls.push_back(call);
    }


    void clear() { m_calls.clear(); }
#ifdef _WIN32
    void run(io::unbuffered_stream_base &input, io::unbuffered_stream_base &output) {
      NP1_ASSERT(false, "Pipelines not implemented on Windows yet.");      
    }
#else
    void run(io::unbuffered_stream_base &input, io::unbuffered_stream_base &output, bool is_worker,
             const rstd::string &script_file_name) {
      NP1_ASSERT(m_calls.size() > 0, "Attempt to run an empty pipeline!");

      // If there is only one thing in the pipeline and that thing is a builtin then there is no need
      // to fork, just call the operator directly.
      if ((m_calls.size() == 1) && ((size_t)-1 != m_calls[0].builtin_stream_op_id)) {
        stream_op_table::call(
          m_calls[0].builtin_stream_op_id, input, output, m_calls[0].arguments, false, is_worker, script_file_name,
          m_calls[0].script_line_number);        
        return;
      } 

      rstd::vector<stream_op_call>::const_iterator last_i = m_calls.end() - 1;
      rstd::vector<stream_op_call>::const_iterator i = last_i;
      rstd::vector<stream_op_call>::const_iterator first_i = m_calls.begin();

      int prev_in_pipeline_stdout = -1;      
      size_t child_counter = 0;
      rstd::vector<pid_t> child_pids;

      for (; i >= first_i; --i, ++child_counter) {
        int pipe_fds[2];
        process::mandatory_pipe_create(pipe_fds);

        // Remember we're walking backwards through the pipeline.
        int child_stdout = (i == last_i) ? -1 : prev_in_pipeline_stdout;
        int child_stdin = (i == first_i) ? -1 : pipe_fds[0];

        // Save this even if we've hit the first op in the pipeline so that
        // we can clean it up when we've finished starting all the processes.
        prev_in_pipeline_stdout = pipe_fds[1];

        pid_t pid = process::mandatory_fork();
        if (0 == pid) {
          // Child.
          global_info::stream_op_details_reset();
          global_info::listening_endpoint("");

          if (prev_in_pipeline_stdout != -1) {
            close(prev_in_pipeline_stdout);
          }
          
          io::unbuffered_stream_base *stdin_stream = &input;
          io::unbuffered_stream_base *stdout_stream = &output;
          io::file stdin_f;
          io::file stdout_f;

          if (child_stdin != -1) {
            stdin_f.from_handle(child_stdin);
            stdin_stream = &stdin_f;
          }

          if (child_stdout != -1) {
            stdout_f.from_handle(child_stdout);
            stdout_stream = &stdout_f;
          }
          
          if ((size_t)-1 == i->builtin_stream_op_id) {
            script::run(
              *stdin_stream,
              *stdout_stream,
              compound_op_find(script_file_name, i->script_line_number, i->stream_op_token),
              false);
          } else {
            stream_op_table::call(
                i->builtin_stream_op_id, *stdin_stream, *stdout_stream, i->arguments, 
                (i > first_i) ? (i-1)->output_is_recordset_stream : false,
                is_worker, script_file_name, i->script_line_number);
          }
          
          exit(0);
        } else {
          // Parent.
          if (child_stdin != -1) {
            close(child_stdin);
          }
  
          if (child_stdout != -1) {
            close(child_stdout);
          }
  
          child_pids.push_back(pid);
        }        
      }

      if (prev_in_pipeline_stdout != -1) {
        close(prev_in_pipeline_stdout);
      }

      // Wait for all the child processes that we started.
      rstd::vector<pid_t>::const_iterator child_i = child_pids.begin();
      rstd::vector<pid_t>::const_iterator child_iz = child_pids.end();
      for (; child_i != child_iz; ++child_i) {
        process::mandatory_wait_for_child(*child_i);
      }
    }
#endif


    bool get_output_is_recordset_stream() {
      rstd::vector<stream_op_call>::const_iterator i = m_calls.begin();
      rstd::vector<stream_op_call>::const_iterator iz = m_calls.end();
      bool prev_output_is_recordset_stream = false;
      rstd::string prev_op_name;
  
      for (; i < iz; ++i) {
        prev_output_is_recordset_stream =
          (i->builtin_stream_op_id != (size_t)-1)
            && stream_op_table_outputs_recordset_stream(i->builtin_stream_op_id, i->arguments,
                                                        prev_output_is_recordset_stream);
        prev_op_name = i->stream_op_token.text_str();
      }
  
      return prev_output_is_recordset_stream;
    }

  private:
    rstd::vector<stream_op_call> m_calls;
  };


public:
  static void run(io::unbuffered_stream_base &input, io::unbuffered_stream_base &output,
                  const rstd::vector<rstd::string> &args, bool is_worker) {
    NP1_ASSERT(args.size() == 2, "Usage: " + args[0] + " file_name_or_inline_script");
    run(input, output, args[1], is_worker);
  }

  static void run(io::unbuffered_stream_base &input, io::unbuffered_stream_base &output,
                  const rstd::string &file_name_or_inline_script, bool is_worker) {
    NP1_ASSERT(str::is_valid_utf8(file_name_or_inline_script),
                "File name or script '" + file_name_or_inline_script + "' is not valid UTF-8");

    io::file script_file;
    if (!script_file.open_ro(file_name_or_inline_script.c_str())) {
      io::string_input_stream script_stream(file_name_or_inline_script);
      run_from_stream(input, output, script_stream, is_worker, "[inline]");
    } else {
      run_from_stream(input, output, script_file, is_worker, file_name_or_inline_script);
    }
  }

  template <typename Script_Input_Stream>
  static void run_from_stream(io::unbuffered_stream_base &input,
                              io::unbuffered_stream_base &output,
                              Script_Input_Stream &script_file,
                              bool is_worker,
                              const rstd::string &script_file_name) {
    //TODO: should the script file be checked for valid UTF-8?
    rstd::vector<pipeline> pipelines;
    compile(script_file, pipelines, is_worker);

    rstd::vector<pipeline>::iterator pipeline_i = pipelines.begin();
    rstd::vector<pipeline>::iterator pipeline_iz = pipelines.end();
    
    for (; pipeline_i < pipeline_iz; ++pipeline_i) {
      pipeline_i->run(input, output, is_worker, script_file_name);
    }
  }


private:
  // Do a basic compile step, crashes on error.
  template <typename Script_Input_Stream>
  static void compile(Script_Input_Stream &script_file, rstd::vector<pipeline> &pipelines,
                      bool is_worker) {
    rstd::vector<rel::rlang::token> tokens;

    // Use the rlang compiler to get a list of tokens, still in prefix format.
    // We only use symbols that the rlang compiler knows about, even if they
    // have different semantics.
    rel::rlang::compiler::compile_single_expression_to_prefix(script_file, tokens);
    
    // Now break up the list of tokens into pipelines.
    pipeline pline;
    rstd::vector<rel::rlang::token>::const_iterator tok_i = tokens.begin();
    rstd::vector<rel::rlang::token>::const_iterator tok_end = tokens.end();

    while (tok_i < tok_end) {
      // Get the stream operator.
      tok_i->assert(tok_i->type() == rel::rlang::token::TYPE_IDENTIFIER_FUNCTION,
                    "Expected identifier but found non-identifier token");
      rstd::vector<rel::rlang::token>::const_iterator stream_op_tok_i = tok_i;

      size_t builtin_stream_op_id = stream_op_table_find(stream_op_tok_i->text(), is_worker);
      rel::rlang::token stream_op_token(*stream_op_tok_i);
      
      // Check that the next thing is an open parenthesis.
      tok_i->assert(tok_i + 1 < tok_end, "Unexpected end of file, expected '('");
      ++tok_i;
      tok_i->assert(tok_i->type() == rel::rlang::token::TYPE_OPEN_PAREN,
                    "Expected '(' after stream operator");
      ++tok_i;

      // Get the arguments to the stream operator call.
      size_t paren_stack_depth = 1;
      rstd::vector<rel::rlang::token> arguments;
      for (; (tok_i < tok_end) && paren_stack_depth > 0; ++tok_i) {
        if (tok_i->type() == rel::rlang::token::TYPE_CLOSE_PAREN) {
          --paren_stack_depth;
        } else if (tok_i->type() == rel::rlang::token::TYPE_OPEN_PAREN) {
          ++paren_stack_depth;
        }

        if (paren_stack_depth > 0) {
          arguments.push_back(*tok_i);
        }
      }

      if (tok_i >= tok_end) {
        (tok_i-1)->assert(false, "Unexpected end of file, mismatched parentheses or missing ';'");
      }

      // The pipeline might output a recordset stream.  If the operator doesn't
      // accept recordset streams then we need to insert an operator that
      // can convert the stream.
      bool prev_output_is_recordset_stream = pline.get_output_is_recordset_stream();
      bool accepts_recordset_stream =
        (builtin_stream_op_id != (size_t)-1) && stream_op_table_accepts_recordset_stream(builtin_stream_op_id, tokens);
      if (!accepts_recordset_stream && prev_output_is_recordset_stream) {
        append_recordset_stream_to_data_stream_translator(pline, is_worker, *tok_i);
        prev_output_is_recordset_stream = false;
      }


      // Add the stream operator call to the pipeline.
      bool output_is_recordset_stream =
        (builtin_stream_op_id != (size_t)-1)
          && stream_op_table_outputs_recordset_stream(builtin_stream_op_id, arguments, prev_output_is_recordset_stream);
      
      stream_op_call so_call = {
        builtin_stream_op_id,
        stream_op_token,
        arguments,
        output_is_recordset_stream,
        (builtin_stream_op_id != (size_t)-1)
          ? stream_op_table_output_type(builtin_stream_op_id)
          : STREAM_OP_TABLE_IO_TYPE_ANY,
        (builtin_stream_op_id != (size_t)-1)
          ? stream_op_table_input_type(builtin_stream_op_id)
          : STREAM_OP_TABLE_IO_TYPE_ANY,
        stream_op_tok_i->line_number()
      };
      
      pline.push_back(so_call, *tok_i);

      // Figure out if this is the end of the pipeline or if there is more to
      // come.
      if (tok_i->type() == rel::rlang::token::TYPE_SEMICOLON) {
        // End of pipeline.  If the final output is a recordset stream then
        // we need to translate it.
        if (pline.get_output_is_recordset_stream()) {
          append_recordset_stream_to_data_stream_translator(pline, is_worker, *tok_i);
        }

        // Now we can add the pipeline to the list of pipelines.  
        pipelines.push_back(pline);
        pline.clear();        
      } else {
        // There should be another stream operator call for this pipeline.
        tok_i->assert((tok_i->type() == rel::rlang::token::TYPE_OPERATOR)
                        && (str::cmp(tok_i->text(), "|") == 0),
                      "Unexpected token after stream operator call, expected ';' or '|'");
      }

      ++tok_i;
    }
  }

  static void append_recordset_stream_to_data_stream_translator(pipeline &pline, bool is_worker,
                                                                const rel::rlang::token &tok) {
    rstd::vector<rel::rlang::token> empty_arguments;
    size_t builtin_stream_op_id = stream_op_table_find(NP1_REL_RECORDSET_TRANSLATE_NAME, is_worker);
    stream_op_call so_call = {
      builtin_stream_op_id,
      rel::rlang::token(NP1_REL_RECORDSET_TRANSLATE_NAME, rel::rlang::token::TYPE_IDENTIFIER_FUNCTION),
      empty_arguments,
      false,
      stream_op_table_output_type(builtin_stream_op_id),
      stream_op_table_input_type(builtin_stream_op_id),
      tok.line_number() };
    pline.push_back(so_call, tok);
  }
  
  static rstd::string compound_op_find(const rstd::string &calling_script_file_name,
                                       const size_t calling_script_line_number,
                                       const rel::rlang::token &compound_op_token) {
    io::path path(environment::r17_path());
    
    path.push_front(io::file::parse_directory(calling_script_file_name));
    const rstd::string compound_op_file_name = path.find(compound_op_token.text_str());
    if (compound_op_file_name.length() > 0) {
      return compound_op_file_name;
    }
    
    // Not found.
    rstd::string message("Stream operator is not a builtin and was not found on this path: " + path.to_string());
    compound_op_token.assert(false, message.c_str());
    return rstd::string("UNREACHABLE");
  }
};

/// Helper function to avoid circular #includes.
void script_run(io::unbuffered_stream_base &input, io::unbuffered_stream_base &output,
                const rstd::string &file_name, bool is_worker) {
  script::run(input, output, file_name, is_worker);
}




} /// namespaces
}



#endif
