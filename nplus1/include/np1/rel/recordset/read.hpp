// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_REL_RECORDSET_READ_HPP
#define NP1_REL_RECORDSET_READ_HPP


namespace np1 {
namespace rel {
namespace recordset {

// Read a recordset file and write all the chunk ids to stdout along with
// the headings from the first chunk.
class read {
public:
  enum { RELIABLE_STORAGE_TIMEOUT_SECONDS = np1::io::work_distributor::RELIABLE_STORAGE_TIMEOUT_SECONDS };


public:
  template <typename Input_Stream, typename Output_Stream>
  void operator()(const std::string &reliable_storage_local_root,
                    const std::string &reliable_storage_remote_root,
                    Input_Stream &input, Output_Stream &output,
                    const std::vector<rel::rlang::token> &tokens) {
    std::vector<std::string> args = rel::rlang::compiler::eval_to_strings_only(tokens);
    std::vector<std::string>::const_iterator i = args.begin();
    std::vector<std::string>::const_iterator iz = args.end();
    for (; i != iz; ++i) {
      do_read(reliable_storage_local_root, reliable_storage_remote_root, input, output, *i);
    }
  }


  template <typename Input_Stream, typename Output_Stream>
  void do_read(const std::string &reliable_storage_local_root,
                const std::string &reliable_storage_remote_root,
                Input_Stream &input, Output_Stream &output,
                const std::string &source_recordset_name) {
    // Open the recordset.
    np1::io::reliable_storage::id source_recordset_resource_id(source_recordset_name);
    np1::io::reliable_storage rs(reliable_storage_local_root, reliable_storage_remote_root);
    np1::io::reliable_storage::stream source_recordset_stream(rs);

    NP1_ASSERT(rs.open_ro(source_recordset_resource_id, RELIABLE_STORAGE_TIMEOUT_SECONDS,
                            source_recordset_stream),
                "Unable to open source recordset stream " + source_recordset_name);
    
    // Read all the lines from the recordset- they are the chunk ids.
    std::vector<std::string> chunk_ids;
    np1::io::text_input_stream<np1::io::reliable_storage::stream> recordset_text_stream(source_recordset_stream);
    //TODO: error handling here.
    recordset_text_stream.read_all(chunk_ids);

    NP1_ASSERT(chunk_ids.size() > 0, "Recordset is empty!");

    // Open the first chunk in the recordset, we need the headings from it.
    np1::io::reliable_storage::id first_chunk_id(chunk_ids[0]);
    np1::io::reliable_storage::stream first_chunk_stream(rs);
    NP1_ASSERT(rs.open_ro(first_chunk_id, RELIABLE_STORAGE_TIMEOUT_SECONDS, first_chunk_stream),
                "Unable to open first chunk stream " + first_chunk_id.to_string());

    // Now we can get the headings and add the one for the resource id.
    np1::io::mandatory_record_input_stream<np1::io::reliable_storage::stream, record, record_ref>
      record_input_stream(first_chunk_stream);
    record headings(record_input_stream.parse_headings());
    std::vector<std::string> heading_fields = headings.fields();
    heading_fields.push_back(NP1_REL_DISTRIBUTED_RESOURCE_ID_FIELD_NAME);

    // Set up an empty set of fields.  These are just placeholders.
    std::vector<std::string> record_fields;
    record_fields.resize(heading_fields.size());

    // Now write out the headings and all the resource ids.
    record_ref::write(output, heading_fields);
    std::vector<std::string>::const_iterator chunk_id_i = chunk_ids.begin();
    std::vector<std::string>::const_iterator chunk_id_iz = chunk_ids.end();
    for (; chunk_id_i < chunk_id_iz; ++chunk_id_i) {
      record_fields[record_fields.size() - 1] = *chunk_id_i;
      record_ref::write(output, record_fields);
    }

    // Close everything.
    NP1_ASSERT(source_recordset_stream.close(),
                "Unable to close source recordset stream");
  }
};


} // namespaces
}
}

#endif
