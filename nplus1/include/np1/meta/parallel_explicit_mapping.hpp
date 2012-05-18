// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_META_PARALLEL_EXPLICIT_MAPPING_HPP
#define NP1_META_PARALLEL_EXPLICIT_MAPPING_HPP


#define NP1_META_PARALLEL_EXPLICIT_MAPPING_HEADING_FILE_NAME "string:file_name"
#define NP1_META_PARALLEL_EXPLICIT_MAPPING_HEADING_HOST_NAME "string:host_name"


namespace np1 {
namespace meta {

/// Run some r17 code in parallel with an explicit mapping between host and filename.  
template <typename Input_Stream, typename Final_Output_Stream>
class parallel_explicit_mapping {
private:
  // Called in the child process.
  struct child_process_f {
    child_process_f(FILE *child_output_fp, const std::vector<rel::rlang::token> &remote_tokens)
      : m_child_output_fp(child_output_fp), m_remote_tokens(remote_tokens) {}

    void operator()() {
      // The child mustn't read from stdin so create the stdin_file object but don't initialize it.
      io::file stdin_file;

      // We'll write the output to the temporary file.
      io::file stdout_file;
      stdout_file.from_handle(fileno(m_child_output_fp));

      // Go!  remote::run looks after the special 'localhost' hostname.
      remote::run(stdin_file, stdout_file, m_remote_tokens);
    }

    FILE *m_child_output_fp;
    std::vector<rel::rlang::token> m_remote_tokens;    
  };


  // Callled when a child process exits: read the child process's output and send it to the final output.
  struct on_child_process_exit {
    on_child_process_exit(FILE *child_output_fp, Final_Output_Stream &final_output, bool &output_headings_written)
      : m_child_output_fp(child_output_fp), m_final_output(final_output),
        m_output_headings_written(output_headings_written) {}

    void operator()() {
      // Rewind the child output file and set up the stream we'll use to read it.
      io::file child_output_file;
      rewind(m_child_output_fp);
      child_output_file.from_handle(fileno(m_child_output_fp));
      typedef io::mandatory_record_input_stream<io::file, rel::record, rel::record_ref> child_output_stream_type;
      child_output_stream_type child_output_stream(child_output_file);

      // Read the headings, writing them only if they haven't already been written.
      rel::record headings(child_output_stream.parse_headings());
      if (!m_output_headings_written) {
        headings.write(m_final_output);
        m_output_headings_written = true;
      }
      
      // Write out the stream body.
      child_output_stream.copy(m_final_output);

      // Clean up.
      fclose(m_child_output_fp);
    }

    FILE *m_child_output_fp;
    Final_Output_Stream &m_final_output;
    bool &m_output_headings_written;
  };

  typedef process::queued_pool_map<child_process_f, on_child_process_exit> process_pool_map_type; 

public:
  static void run(Input_Stream &input, Final_Output_Stream &output,
                  const std::vector<rel::rlang::token> &tokens) {
    /* Get the headers and the interesting fields out of them. */
    rel::record input_headings(input.parse_headings());
    size_t file_name_field_id =
      input_headings.mandatory_find_heading(NP1_META_PARALLEL_EXPLICIT_MAPPING_HEADING_FILE_NAME);
    size_t host_name_field_id =
      input_headings.mandatory_find_heading(NP1_META_PARALLEL_EXPLICIT_MAPPING_HEADING_HOST_NAME);
    
    // The object that manages all the processes.
    process_pool_map_type process_pool_map;

    bool output_headings_written = false;

    // Translate all the input records into command lines and put them in the pool ready to execute.
    input.parse_records(input_record_callback(file_name_field_id, host_name_field_id, tokens,
                                              process_pool_map, output, output_headings_written));    
    
    // Run all the processes in the pool.
    process_pool_map.wait_all();
  }

private:
  struct input_record_callback {
    input_record_callback(size_t file_name_field_id, size_t host_name_field_id,
                          const std::vector<rel::rlang::token> &tokens, process_pool_map_type &process_pool_map,
                          Final_Output_Stream &final_output, bool &output_headings_written)
      : m_file_name_field_id(file_name_field_id), m_host_name_field_id(host_name_field_id),
        m_tokens(tokens), m_process_pool_map(process_pool_map), m_final_output(final_output),
        m_output_headings_written(output_headings_written) {}

    bool operator()(const rel::record_ref &r) const {
      std::string file_name = r.mandatory_field(m_file_name_field_id).to_string();
      std::string host_name = r.mandatory_field(m_host_name_field_id).to_string();

      // Prefix the tokens with the host name and a command to read the file.
      std::vector<rel::rlang::token> file_read_tokens;
      file_read_tokens.push_back(rel::rlang::token(host_name.c_str(), rel::rlang::token::TYPE_STRING));
      file_read_tokens.push_back(rel::rlang::token(",", rel::rlang::token::TYPE_COMMA));
      file_read_tokens.push_back(rel::rlang::token("io.file.read", rel::rlang::token::TYPE_IDENTIFIER_VARIABLE));
      file_read_tokens.push_back(rel::rlang::token("(", rel::rlang::token::TYPE_OPEN_PAREN));
      file_read_tokens.push_back(rel::rlang::token(file_name.c_str(), rel::rlang::token::TYPE_STRING));
      file_read_tokens.push_back(rel::rlang::token(")", rel::rlang::token::TYPE_CLOSE_PAREN));
      file_read_tokens.push_back(rel::rlang::token("|", rel::rlang::token::TYPE_OPERATOR));

      std::vector<rel::rlang::token> remote_tokens(file_read_tokens);
      remote_tokens.append(m_tokens);
      
      // Create a temporary file for the use of the child.      
      FILE *child_output_fp = tmpfile();
      NP1_ASSERT(child_output_fp, "Unable to create temporary file for child process output");

      // Add into the process pool, to be processed asap.
      m_process_pool_map.add(
        host_name,
        child_process_f(child_output_fp, remote_tokens),
        on_child_process_exit(child_output_fp, m_final_output, m_output_headings_written));

      return true;
    }

    size_t m_file_name_field_id;
    size_t m_host_name_field_id;
    const std::vector<rel::rlang::token> &m_tokens;
    process_pool_map_type &m_process_pool_map;
    Final_Output_Stream &m_final_output;
    bool &m_output_headings_written;
  };
};

} /// namespaces
}



#endif
