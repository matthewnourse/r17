// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_REL_RECORDSET_IO_MANDATORY_TRANSLATE_RECORD_INPUT_STREAM_HPP
#define NP1_REL_RECORDSET_IO_MANDATORY_TRANSLATE_RECORD_INPUT_STREAM_HPP


namespace np1 {
namespace rel {
namespace recordset {
namespace io {


template <typename Inner_Stream>
class mandatory_translate_record_input_stream {
public:
  enum { RELIABLE_STORAGE_TIMEOUT_SECONDS = ::np1::io::work_distributor::RELIABLE_STORAGE_TIMEOUT_SECONDS };

public:
  mandatory_translate_record_input_stream(Inner_Stream &s, const rstd::string &reliable_storage_local_root,
                                          const rstd::string &reliable_storage_remote_root)
    : m_recordset_stream(s),
      m_rs(reliable_storage_local_root, reliable_storage_remote_root),
      m_resource_id_field_id(-1) {}
  ~mandatory_translate_record_input_stream() {}

  
  /// Parse record headings from the recordset stream.  THIS FUNCTION IS SLOW.
  /**
   * Exits the program if the headers could not be parsed.
   */
  record parse_headings() {
    NP1_ASSERT((m_resource_id_field_id == (size_t)-1), "Attempt to read headings after they have already been read");
    record recordset_stream_headings = m_recordset_stream.parse_headings();
    m_resource_id_field_id = recordset_stream_headings.mandatory_find_field(
                                      NP1_REL_DISTRIBUTED_RESOURCE_ID_FIELD_NAME);

    rstd::vector<str::ref> heading_fields;
    distributed::get_fields_except(recordset_stream_headings.ref(), m_resource_id_field_id, heading_fields);
    record temp(heading_fields, 0);
    m_headings.swap(temp);
    return m_headings;
  }


  /// Parse records from a CSV file and call the callback for each record.
  /**
   * Returns true if parsing completed normally, false if 
   * a callback asked us to stop.    Exits the program on fatal error. 
   */
  template <typename Record_Callback>
  inline bool parse_records(Record_Callback record_callback) {
    NP1_ASSERT((m_resource_id_field_id != (size_t)-1), "Attempt to read records before headings have been read");
    return m_recordset_stream.parse_records(
            chunk_record_callback<Record_Callback>(m_rs, m_headings.ref(), m_resource_id_field_id, record_callback));
  }

  /// Assumes that the output stream is also a mandatory stream.  Buffering is recommended.
  template <typename Output_Stream>
  void copy(Output_Stream &output) {
    parse_headings().write(output);
    parse_records(copy_callback<Output_Stream>(output));
  }



private:
  /// The callback that's called once for each input chunk id.
  template <typename Record_Callback>
  struct chunk_record_callback {
    chunk_record_callback(::np1::io::reliable_storage &rs,
                          const record_ref &headings,
                          size_t resource_id_field_id,
                          Record_Callback &callback)
      : m_rs(rs),
        m_headings(headings),
        m_resource_id_field_id(resource_id_field_id),
        m_callback(callback) {}  


    bool operator()(const record_ref &r) { 
      // Open the stream for the input chunk and check that its headings
      // match what we expect.
      ::np1::io::reliable_storage::id input_chunk_id(r.mandatory_field(m_resource_id_field_id));
      ::np1::io::reliable_storage::stream input_chunk_stream(m_rs);
      NP1_ASSERT(m_rs.open_ro(input_chunk_id, RELIABLE_STORAGE_TIMEOUT_SECONDS,
                              input_chunk_stream),
                  "Unable to open input chunk stream " + input_chunk_id.to_string());

      ::np1::io::mandatory_record_input_stream<np1::io::reliable_storage::stream, record, record_ref>
        input_chunk_record_stream(input_chunk_stream);

      record input_chunk_headings(input_chunk_record_stream.parse_headings());

      NP1_ASSERT(
        input_chunk_headings.ref().is_equal(m_headings),
        "Headings in chunk are not equivalent to recordset stream headings.  Chunk headings: "
          + input_chunk_headings.to_string() + "  Expected headings: " + m_headings.to_string());        

      // Read the whole chunk and copy it to the output.
      bool result = input_chunk_record_stream.parse_records(m_callback);

      NP1_ASSERT(input_chunk_record_stream.close(),
                  "Unable to close input chunk record stream "
                  + input_chunk_id.to_string());

      return result;
    }        


    ::np1::io::reliable_storage &m_rs;
    const record_ref &m_headings;
    size_t m_resource_id_field_id;
    Record_Callback m_callback;
  };


  template <typename Output_Stream>
  struct copy_callback {
    explicit copy_callback(Output_Stream &output) : m_output(output) {}
    bool operator()(const record_ref &r) { 
      r.write(m_output);
      return true;
    }

    Output_Stream &m_output;
  };
  

private:
  /// Disable copy.
  mandatory_translate_record_input_stream(const mandatory_translate_record_input_stream &);
  mandatory_translate_record_input_stream &operator = (const mandatory_translate_record_input_stream &);

private:
  Inner_Stream &m_recordset_stream;
  ::np1::io::reliable_storage m_rs;
  record m_headings;
  size_t m_resource_id_field_id;
};


} // namespaces
}
}
}

#endif
