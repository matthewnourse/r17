// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_REL_RECORD_SPLIT_HPP
#define NP1_REL_RECORD_SPLIT_HPP


#include "np1/io/heap_buffer_output_stream.hpp"
#include "np1/process.hpp"

namespace np1 {
namespace rel {


class record_split {
public:
  enum { COMPRESSION_LEVEL = 9 };
  enum { OUTPUT_BUFFER_ALLOCATION_SIZE = 100 * 1024 * 1024 };
  enum { INITIAL_MAX_NUMBER_CHILD_PROCESSES = 2 };

private:
  struct on_child_process_exit {
    void operator()() {}
  };

  typedef process::pool<on_child_process_exit> process_pool_type;

public:
  template <typename Input_Stream, typename Output_Stream>
  void operator()(Input_Stream &input, Output_Stream &output,
                  const rstd::vector<rel::rlang::token> &tokens) {
    // Parse the arguments.
    rstd::vector<rstd::pair<rstd::string, rlang::dt::data_type> > arg_pairs = rlang::compiler::eval_to_strings(tokens);
    NP1_ASSERT(arg_pairs.size() == 2, "Invalid number of arguments to rel.record_split");
    NP1_ASSERT((rlang::dt::TYPE_INT == arg_pairs[0].second) || (rlang::dt::TYPE_UINT == arg_pairs[0].second),
                "First argument to rel.record_split must be an integer");
    NP1_ASSERT(rlang::dt::TYPE_STRING == arg_pairs[1].second, "Second argument to rel.record_split must be a string");
    
    int64_t number_records_per_output_file = str::dec_to_int64(arg_pairs[0].first);
    NP1_ASSERT(number_records_per_output_file > 0,
                "First argument to rel.record_split must be a nonzero positive integer");
    rstd::string file_name_stub = arg_pairs[1].first;

    // Read the headings, we'll put these at the top of every file.
    record headings(input.parse_headings());

    // Do our thing.
    io::heap_buffer_output_stream current_output_stream(OUTPUT_BUFFER_ALLOCATION_SIZE);  
    process_pool_type child_processes(INITIAL_MAX_NUMBER_CHILD_PROCESSES);
    uint64_t file_counter = 0;
    input.parse_records(record_split_callback(headings, number_records_per_output_file, file_name_stub,
                                              current_output_stream, child_processes, file_counter));

    // Compress & write out the last stream.
    if (current_output_stream.size() > 0) {
      child_processes.add(async_compress(current_output_stream, file_counter, file_name_stub),
                          on_child_process_exit());
    }

    child_processes.wait_all();
  }

private:
  static rstd::string make_file_name(const rstd::string &file_name_stub, uint64_t file_counter) {
    return file_name_stub + str::to_hex_str_pad_16(file_counter) + ".gz";
  }

  struct async_compress {
    async_compress(const io::heap_buffer_output_stream &current_output_stream, uint64_t file_counter,
                    const rstd::string &file_name_stub)
      : m_current_output_stream(current_output_stream), m_file_counter(file_counter),
        m_file_name_stub(file_name_stub) {}

    void operator()() {
      // Executed in the child process.
      rstd::string output_file_name(make_file_name(m_file_name_stub, m_file_counter));
      rstd::string temp_output_file_name = output_file_name + ".tmp." + uuid::generate().to_string();
      {
        io::gzfile output_file;
        NP1_ASSERT(output_file.create_wo(temp_output_file_name.c_str(), COMPRESSION_LEVEL),
                    "Unable to create file " + temp_output_file_name);
        output_file.write(m_current_output_stream.ptr(), m_current_output_stream.size());
        output_file.hard_flush();
        output_file.close();
      }

      NP1_ASSERT(io::file::rename(temp_output_file_name.c_str(), output_file_name.c_str()),
                  "Unable to rename file " + temp_output_file_name + " to " + output_file_name);
    }

    const io::heap_buffer_output_stream &m_current_output_stream;
    uint64_t m_file_counter;
    rstd::string m_file_name_stub;
  };


  struct record_split_callback {
    record_split_callback(const record &headings, uint64_t number_records_per_output_file,
                          const rstd::string &file_name_stub, io::heap_buffer_output_stream &current_output_stream,
                          process_pool_type &child_processes, uint64_t &file_counter)
      : m_headings(headings),
        m_number_records_per_output_file(number_records_per_output_file),
        m_file_name_stub(file_name_stub),
        m_current_output_stream(current_output_stream),
        m_child_processes(child_processes),
        m_number_records_in_current_output_stream(0xffffffffffffffffULL),
        m_file_counter(file_counter) {} 
      
    bool operator()(const record_ref &r) {
      if (m_number_records_in_current_output_stream >= m_number_records_per_output_file) {
        if (m_current_output_stream.size() > 0) {
          m_child_processes.add(async_compress(m_current_output_stream, m_file_counter, m_file_name_stub),
                                on_child_process_exit());

          m_current_output_stream.reset();
          ++m_file_counter;
        }

        m_headings.write(m_current_output_stream);
        m_number_records_in_current_output_stream = 0;
      }

      r.write(m_current_output_stream);
      ++m_number_records_in_current_output_stream;
      return true;
    }

    record m_headings;
    uint64_t m_number_records_per_output_file;
    rstd::string m_file_name_stub;
    io::heap_buffer_output_stream &m_current_output_stream;
    process_pool_type &m_child_processes;
    uint64_t m_number_records_in_current_output_stream;
    uint64_t &m_file_counter;    
  };
};


} // namespaces
}


#endif
