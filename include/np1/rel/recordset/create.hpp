// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_REL_RECORDSET_CREATE_HPP
#define NP1_REL_RECORDSET_CREATE_HPP


namespace np1 {
namespace rel {
namespace recordset {

// Create a new recordset from a stream of recordset fragment file names OR
// from a stream of data.
class create {
public:
  enum { RELIABLE_STORAGE_TIMEOUT_SECONDS = np1::io::work_distributor::RELIABLE_STORAGE_TIMEOUT_SECONDS };
  enum { DEFAULT_APPROX_MAX_CHUNK_SIZE_BYTES = 100 * 1024 * 1024 };

private:
  typedef np1::io::buffered_output_stream<np1::io::reliable_storage::stream> buffered_reliable_storage_stream_type;
  typedef np1::io::mandatory_output_stream<buffered_reliable_storage_stream_type> mandatory_reliable_storage_stream_type;

public:
  /// Create from a stream of chunk file names.
  template <typename Input_Stream, typename Output_Stream>
  void from_recordset_stream(const std::string &reliable_storage_local_root,
                              const std::string &reliable_storage_remote_root,
                              Input_Stream &input, Output_Stream &output,
                              const std::vector<rel::rlang::token> &tokens) {
    /* Get the headers. */
    record headings(input.parse_headings());

    // Get the resource id field id, this is the only field that we're interested
    // in, the rest are just placeholders.
    size_t resource_id_field_id = headings.mandatory_find_field(
                                      NP1_REL_DISTRIBUTED_RESOURCE_ID_FIELD_NAME);

    // Create the target recordset.
    std::string target_recordset_name(rel::rlang::compiler::eval_to_string_only(tokens));
    np1::io::reliable_storage::id target_recordset_resource_id(target_recordset_name);

    np1::io::reliable_storage rs(reliable_storage_local_root, reliable_storage_remote_root);
    np1::io::reliable_storage::stream target_recordset_stream(rs);
    NP1_ASSERT(rs.create_wo(target_recordset_resource_id, target_recordset_stream),
                "Unable to create recordset " + target_recordset_name);


    // Read all the input recordset chunks ids and write them to the recordset file.
    input.parse_records(
      shallow_copy_chunk_record_callback(
        rs, target_recordset_stream,  resource_id_field_id, headings.ref()));

    NP1_ASSERT(target_recordset_stream.close(),
                "Unable to close target recordset stream");
  }


  /// Create from a stream of data.
  template <typename Input_Stream, typename Output_Stream>
  void from_data_stream(const std::string &reliable_storage_local_root,
                          const std::string &reliable_storage_remote_root,
                          Input_Stream &input, Output_Stream &output,
                          const std::vector<rel::rlang::token> &tokens) {
    /* Get the headers. */
    record headings(input.parse_headings());

    // Interpret the arguments.
    std::vector<std::pair<std::string, rlang::dt::data_type> > args = rel::rlang::compiler::eval_to_strings(tokens);
    NP1_ASSERT((args.size() > 0) && (args.size() <= 2), "Incorrect number of arguments to rel.recordset.create");
    tokens[0].assert(rlang::dt::data_type::TYPE_STRING == args[0].second,
                      "First argument to rel.recordset.create is not a string");
    std::string target_recordset_name = args[0].first;

    uint64_t approx_max_chunk_size = DEFAULT_APPROX_MAX_CHUNK_SIZE_BYTES;
    if (args.size() > 1) {
      tokens[0].assert((rlang::dt::data_type::TYPE_INT == args[1].second)
                        || (rlang::dt::data_type::TYPE_UINT == args[1].second),
                        "Second argument to rel.recordset.create is not an integer");
      approx_max_chunk_size = str::dec_to_int64(args[1].first);
    }

    // Get the stream that will hold the recordset.
    np1::io::reliable_storage::id target_recordset_resource_id(target_recordset_name);

    np1::io::reliable_storage rs(reliable_storage_local_root, reliable_storage_remote_root);
    np1::io::reliable_storage::stream target_recordset_stream(rs);
    NP1_ASSERT(rs.create_wo(target_recordset_resource_id, target_recordset_stream),
                "Unable to create recordset " + target_recordset_name
                  + " in reliable storage '" + reliable_storage_local_root + "'");

    np1::io::reliable_storage::stream current_target_chunk_stream(rs);
    buffered_reliable_storage_stream_type buffered_current_target_chunk_stream(
                                            current_target_chunk_stream);

    mandatory_reliable_storage_stream_type mandatory_current_target_chunk_stream(
                                            buffered_current_target_chunk_stream);

    np1::io::reliable_storage::id current_target_chunk_id;
    uint64_t current_target_chunk_size = 0;

    // Read all the input data and redistribute it into recordset chunks.
    input.parse_records(
        chunk_data_record_callback(
          rs, target_recordset_stream, current_target_chunk_id,
          current_target_chunk_stream, mandatory_current_target_chunk_stream,
          current_target_chunk_size, approx_max_chunk_size, headings.ref()));

    // Close everything.
    mandatory_current_target_chunk_stream.close();

    NP1_ASSERT(target_recordset_stream.close(),
                "Unable to close target recordset stream");
  }


private:
  /// The callback that's called once for each input chunk id.
  struct shallow_copy_chunk_record_callback {
    shallow_copy_chunk_record_callback(
                          np1::io::reliable_storage &rs,
                          np1::io::reliable_storage::stream &target_recordset_stream,
                          size_t resource_id_field_id,
                          const record_ref &recordset_headings)
      : m_rs(rs),
        m_target_recordset_stream(target_recordset_stream),
        m_resource_id_field_id(resource_id_field_id),
        m_recordset_headings(recordset_headings) {}  

    bool operator()(const record_ref &r) { 
      np1::io::reliable_storage::id input_chunk_id(r.mandatory_field(m_resource_id_field_id));
      NP1_ASSERT(m_target_recordset_stream.write(input_chunk_id)
                  && m_target_recordset_stream.write("\n"),
                  "Unable to write to recordset");

      return true;
    }        

    np1::io::reliable_storage &m_rs;
    np1::io::reliable_storage::stream &m_target_recordset_stream;
    size_t m_resource_id_field_id;
    const record_ref &m_recordset_headings;
  };




  // The callback that's called once for each actual record.
  struct chunk_data_record_callback {

    chunk_data_record_callback(np1::io::reliable_storage &rs,
                              np1::io::reliable_storage::stream &target_recordset_stream,
                              np1::io::reliable_storage::id &current_target_chunk_id,
                              np1::io::reliable_storage::stream &current_target_chunk_stream,
                              mandatory_reliable_storage_stream_type &m_mandatory_current_target_chunk_stream,
                              uint64_t &current_target_chunk_size,
                              uint64_t approx_max_chunk_size,
                              const record_ref &headings)
      : m_rs(rs),
        m_target_recordset_stream(target_recordset_stream),
        m_current_target_chunk_id(current_target_chunk_id),
        m_current_target_chunk_stream(current_target_chunk_stream),
        m_mandatory_current_target_chunk_stream(m_mandatory_current_target_chunk_stream),
        m_current_target_chunk_size(current_target_chunk_size),
        m_approx_max_chunk_size(approx_max_chunk_size),
        m_headings(headings) {}  

    bool operator()(const record_ref &r) {
      // Open the current output chunk stream (if not opened) and write its name to the
      // file that lists the chunks in the recordset.
      if (!m_mandatory_current_target_chunk_stream.is_open()) {
        m_current_target_chunk_id = np1::io::reliable_storage::id::generate();

        NP1_ASSERT(m_rs.create_wo(m_current_target_chunk_id, m_current_target_chunk_stream),
                    "Unable to create recordset stream chunk "
                      + m_current_target_chunk_id.to_string());

        NP1_ASSERT(m_target_recordset_stream.write(m_current_target_chunk_id)
                    && m_target_recordset_stream.write("\n"),
                    "Unable to write to recordset");

        m_headings.write(m_mandatory_current_target_chunk_stream);
        m_current_target_chunk_size = 0;
      }

      // Write the record to the output chunk stream, and close the stream
      // if it's big enough.
      size_t record_byte_size = r.byte_size();
      r.write(m_mandatory_current_target_chunk_stream);
      m_current_target_chunk_size += record_byte_size;

      if (m_current_target_chunk_size > m_approx_max_chunk_size) {
        m_mandatory_current_target_chunk_stream.close();
      }

      return true;
    }    
    

    np1::io::reliable_storage &m_rs;
    np1::io::reliable_storage::stream &m_target_recordset_stream;
    np1::io::reliable_storage::id &m_current_target_chunk_id;
    np1::io::reliable_storage::stream &m_current_target_chunk_stream;
    mandatory_reliable_storage_stream_type &m_mandatory_current_target_chunk_stream;
    uint64_t &m_current_target_chunk_size;
    uint64_t m_approx_max_chunk_size;
    const record_ref &m_headings;
  };
};


} // namespaces
}
}

#endif
