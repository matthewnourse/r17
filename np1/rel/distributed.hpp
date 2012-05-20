// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_REL_DISTRIBUTED_HPP
#define NP1_REL_DISTRIBUTED_HPP


#include "np1/io/ordered_work_distributor.hpp"

// Common definitions for distributed relational operators.

#define NP1_REL_DISTRIBUTED_RESOURCE_ID_FIELD_NAME "string:___file_name"

namespace np1 {
namespace rel {
namespace distributed {



// Get a list of fields except the supplied id.
void get_fields_except(const record_ref &r, size_t pariah_id,
                        std::vector<str::ref> &output_fields) {
  size_t i;
  size_t number_fields = r.number_fields();  
  output_fields.clear();
  for (i = 0; i < number_fields; ++i) {
    if (i != pariah_id) {
      output_fields.push_back(r.mandatory_field(i));
    }
  }
}

record get_fields_except(const record_ref &r, size_t pariah_id, uint64_t record_number) {
  std::vector<str::ref> output_fields;
  get_fields_except(r, pariah_id, output_fields);
  return record(output_fields, record_number);
}

record get_fields_except_resource_id(const record_ref &r, uint64_t record_number) {
  return get_fields_except(r, r.mandatory_find_field(NP1_REL_DISTRIBUTED_RESOURCE_ID_FIELD_NAME), record_number);
}



// Check that two records are the same, except for one field which must be
// missing from the first set of headings.
bool are_records_equivalent_except(const record_ref &r1, const record_ref &r2,
                                    size_t except_id) {
  size_t i;
  size_t r1_number_fields = r1.number_fields();
  size_t r2_number_fields = r2.number_fields();

  if (r1_number_fields + 1 != r2_number_fields) {
    return false;
  }
 
  std::vector<str::ref> r2_fields;
  get_fields_except(r2, except_id, r2_fields);  

  for (i = 0; i < r1_number_fields; ++i) {
    if (str::cmp(r1.mandatory_field(i), r2_fields[i]) != 0) {
      return false;
    }
  }
   
  return true;
}



namespace detail {

struct distributor_record_callback {
  distributor_record_callback(
                  io::ordered_work_distributor &distributor,
                  size_t resource_id_field_id,
                  const std::string &command_text,
                  size_t &number_input_records)
    : m_distributor(distributor),
      m_resource_id_field_id(resource_id_field_id),
      m_command_text(command_text),
      m_number_input_records(number_input_records) {}  

  bool operator()(const record_ref &r) const {
    str::ref resource_id_str = r.mandatory_field(m_resource_id_field_id);
    io::reliable_storage::id resource_id(resource_id_str);
    m_distributor.send_request(resource_id, str::ref(m_command_text));
    ++m_number_input_records;
    return true;
  }        

  io::ordered_work_distributor &m_distributor;
  size_t m_resource_id_field_id;
  const std::string &m_command_text;
  size_t &m_number_input_records;
};

} // namespace detail


// Distribute some work.
template <typename Input_Stream, typename Output_Stream>
void distribute(const char *log_id,
                const record &input_headings,
                const record &output_headings,
                const std::string &command_name,
                const std::string &reliable_storage_local_root,
                const std::string &reliable_storage_remote_root,
                const std::string &listen_endpoint,                  
                Input_Stream &input,
                Output_Stream &output,
                const std::vector<rel::rlang::token> &tokens) {
  // Write out the headings so the next distributed operator can do its thing.
  output_headings.write(output);
  output.soft_flush();

  io::log::info(log_id, "Setting up work distribution stuff.");

  io::ordered_work_distributor distributor(reliable_storage_local_root, reliable_storage_remote_root, listen_endpoint);
  std::string command_text;
  io::string_output_stream sos(command_text);
  rlang::io::token_writer::mandatory_write(sos, tokens);

  //TODO: something cleaner than this.
  command_text = command_name + "(" + command_text + ");";

  size_t input_resource_id_field_id = input_headings.mandatory_find_field(
                                        NP1_REL_DISTRIBUTED_RESOURCE_ID_FIELD_NAME);

  io::log::info(log_id, "Distributing work.");

  size_t number_input_records = 0;    

  input.parse_records(
    detail::distributor_record_callback(
      distributor, input_resource_id_field_id, command_text, number_input_records));

  // Tell the next operator in the stream about the completed work items.
  io::reliable_storage::id output_resource_id;
  io::reliable_storage rs(reliable_storage_local_root, reliable_storage_remote_root);
  std::vector<str::ref> output_fields;
  output_fields.resize(output_headings.number_fields());
  size_t output_resource_id_field_id = output_headings.mandatory_find_field(
                                        NP1_REL_DISTRIBUTED_RESOURCE_ID_FIELD_NAME);

  while (distributor.receive_response(output_resource_id)) {
    NP1_ASSERT(number_input_records > 0, "More responses than input records");

    // Write out the output record with the output resource id substituted for the
    // input resource id.
    output_fields[output_resource_id_field_id] = output_resource_id.to_str_ref();
    record_ref::write(output, output_fields);

    // The output volume is fairly low and we want the next operator in the chain
    // to be able to get cracking asap, so we flush the output after every write.
    output.soft_flush();

    --number_input_records;
  }

  NP1_ASSERT(0 == number_input_records, "More input records than responses");

  io::log::info(log_id, "Complete.");
}




} // namespaces
}
}


#endif
