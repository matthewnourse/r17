// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_REL_DISTRIBUTED_GROUP_HPP
#define NP1_REL_DISTRIBUTED_GROUP_HPP


#include "np1/rel/rlang/rlang.hpp"
#include "np1/io/ordered_work_distributor.hpp"
#include "np1/io/log.hpp"
#include "np1/io/heap_buffer_output_stream.hpp"
#include "np1/rel/recordset/io/mandatory_translate_record_input_stream.hpp"
#include "np1/process.hpp"


namespace np1 {
namespace rel {


class distributed_group {
private:
  enum { RECORDSET_STREAM_ALLOCATION_SIZE = 1024 * 1024 };

public:
  template <typename Input_Stream, typename Output_Stream>
  void operator()(const rstd::string &reliable_storage_local_root,
                  const rstd::string &reliable_storage_remote_root,
                  const rstd::string &listen_endpoint,                  
                  Input_Stream &input, Output_Stream &output,
                  const rstd::vector<rel::rlang::token> &original_tokens) {
    log_info("Reading headers and doing basic argument checks.");

    const char *original_aggregator;
    const char *aggregator_heading_name;

    group::parse_arguments(original_tokens, &original_aggregator, &aggregator_heading_name);

    log_info("Parsed arguments.");

    const char *for_distribution_aggregator = original_aggregator;
    rstd::vector<rel::rlang::token> for_distribution_tokens = original_tokens;
 
    // Average can't be accurately computed by workers, we need them to calculate something
    // we can calculate the average from.
    if (str::cmp(original_aggregator, NP1_REL_GROUP_AGGREGATOR_AVG) == 0) {
      //TODO: something cleaner than this!
      strcpy(for_distribution_tokens[0].text(), NP1_REL_GROUP_AGGREGATOR_SUM_COUNT);
      for_distribution_aggregator = NP1_REL_GROUP_AGGREGATOR_SUM_COUNT;        
    }

    record recordset_input_headings(input.parse_headings());
    if (aggregator_heading_name) {
      recordset_input_headings.mandatory_find_heading(aggregator_heading_name);
    }

    str::ref aggregator_type_tag =
      aggregator_heading_name
        ? group::mandatory_get_aggregator_heading_type_tag(recordset_input_headings, aggregator_heading_name)
        : str::ref();
        
    record recordset_output_headings(
              group::get_output_headings(for_distribution_aggregator, aggregator_heading_name, aggregator_type_tag,
                                         recordset_input_headings));
    
    // Fork the worker process and make the parent wait for results.
    int translated_stream_pipe[2];
    process::mandatory_pipe_create(translated_stream_pipe);
    pid_t child_pid = process::mandatory_fork();
    if (0 == child_pid) {
      // Child: distribute & translate.
      ::close(translated_stream_pipe[0]);

      // Fork the process that will do the distribution.
      int distribution_results_pipe[2];
      process::mandatory_pipe_create(distribution_results_pipe);
      child_pid = process::mandatory_fork();
      if (0 == child_pid) {
        // Child: distribute.
        ::close(distribution_results_pipe[0]);
        io::file distribution_results_file;
        distribution_results_file.from_handle(distribution_results_pipe[1]);
        io::mandatory_output_stream<io::file> mandatory_recordset_output_stream(distribution_results_file);
    
        // Do the distribution.
        distributed::distribute(log_id(), recordset_input_headings, recordset_output_headings, "rel.group",
                                reliable_storage_local_root, reliable_storage_remote_root, listen_endpoint, input,
                                mandatory_recordset_output_stream, for_distribution_tokens);

        distribution_results_file.close();
        exit(0);
      }

      // Parent: translate results.
      ::close(distribution_results_pipe[1]);
      translate(distribution_results_pipe[0], translated_stream_pipe[1], reliable_storage_local_root,
                reliable_storage_remote_root);
      process::mandatory_wait_for_child(child_pid);
      exit(0);
    }

    // Outermost parent: postprocess results.
    ::close(translated_stream_pipe[1]);
    postprocess_results(translated_stream_pipe[0], original_aggregator, aggregator_heading_name,
                        recordset_input_headings, recordset_output_headings, output);
    process::mandatory_wait_for_child(child_pid);
  }


private:
  void translate(int from_fd, int to_fd, const rstd::string &reliable_storage_local_root,
                  const rstd::string &reliable_storage_remote_root) {
    // Set up the "from" stream.
    io::file distribution_results_file;
    distribution_results_file.from_handle(from_fd);
    typedef io::mandatory_record_input_stream<io::file, record, record_ref> recordset_input_stream_type;
    recordset_input_stream_type recordset_input_stream(distribution_results_file);
    recordset::io::mandatory_translate_record_input_stream<recordset_input_stream_type>
      translator_stream(recordset_input_stream, reliable_storage_local_root, reliable_storage_remote_root);
    
    // Set up the "to" stream.
    io::file translated_file;
    translated_file.from_handle(to_fd);
    io::buffered_output_stream<io::file> buffered_translated_file(translated_file);
    io::mandatory_output_stream<io::buffered_output_stream<io::file> >
      mandatory_translated_file(buffered_translated_file);
    
    // Copy the whole stream.
    translator_stream.copy(mandatory_translated_file); 
  }


  template <typename Output_Stream>
  void postprocess_results(int translated_stream_fd, const char *original_aggregator,
                            const char *aggregator_heading_name, const record &recordset_input_headings,
                            const record &recordset_output_headings, Output_Stream &output) {
    log_info("Postprocessing results.");
    io::file translated_stream_file;
    translated_stream_file.from_handle(translated_stream_fd);
    io::mandatory_record_input_stream<io::file, record, record_ref> translated_stream(translated_stream_file);

    // The grouping functions expect that the input stream headings have already been read.
    translated_stream.parse_headings();

    // The final output of this operator is a normal data stream.
    record normal_output_headings(distributed::get_fields_except_resource_id(recordset_output_headings.ref(), 0));

    if (str::cmp(original_aggregator, NP1_REL_GROUP_AGGREGATOR_COUNT) == 0) {
      // We need to sum the 'count' field.
      group::group_sum(normal_output_headings, normal_output_headings, NP1_REL_GROUP_OUTPUT_HEADING_COUNT,
                        translated_stream, output);
    } else if (str::cmp(original_aggregator, NP1_REL_GROUP_AGGREGATOR_AVG) == 0) {
      // The output of distribution is a "sum" and a "count".
      str::ref aggregator_type_tag =
        aggregator_heading_name
          ? group::mandatory_get_aggregator_heading_type_tag(normal_output_headings, NP1_REL_GROUP_OUTPUT_HEADING_SUM)
          : str::ref();
          
      record new_normal_output_headings(
                group::get_output_headings(
                        original_aggregator, aggregator_heading_name, aggregator_type_tag,
                        distributed::get_fields_except_resource_id(recordset_input_headings.ref(), 0)));
      group::group_sum_count_to_avg(normal_output_headings, new_normal_output_headings, translated_stream, output);
    } else if (str::cmp(original_aggregator, NP1_REL_GROUP_AGGREGATOR_SUM) == 0) {
      // Just sum the sums.
      group::group_sum(normal_output_headings, normal_output_headings, NP1_REL_GROUP_OUTPUT_HEADING_SUM,
                        translated_stream, output);
    } else if (str::cmp(original_aggregator, NP1_REL_GROUP_AGGREGATOR_MIN) == 0) {                   
      // Just get the min of the min.
      group::group_min(normal_output_headings, normal_output_headings, aggregator_heading_name,
                        translated_stream, output);
    } else if (str::cmp(original_aggregator, NP1_REL_GROUP_AGGREGATOR_MAX) == 0) {                   
      // Just get the max of the max.
      group::group_max(normal_output_headings, normal_output_headings, aggregator_heading_name,
                        translated_stream, output);
    } else {
      NP1_ASSERT(false, "Aggregator unsupported for distribution: " + rstd::string(original_aggregator));
    }
  }

  
private:
  static const char *log_id() { return "distributed_group"; }

  void log_info(const char *description) {
    io::log::info(log_id(), description);
  }
};


} // namespaces
}


#endif
