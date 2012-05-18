// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_IO_MANDATORY_RECORD_INPUT_STREAM_HPP
#define NP1_IO_MANDATORY_RECORD_INPUT_STREAM_HPP

#include <string>
#include "np1/str.hpp"
#include "np1/io/mandatory_input_stream.hpp"

namespace np1  {
namespace io {

/// An input stream for records where all failures crash the process.
template <typename Inner_Stream, typename Record, typename Record_Ref>
class mandatory_record_input_stream {
private:
  // Ensure that the inner stream is unbuffered.
  typedef typename Inner_Stream::is_unbuffered inner_is_unbuffered_type;

public:
  /// Constructor.
  explicit mandatory_record_input_stream(Inner_Stream &s) : m_stream(s) {}
  
  /// Destructor.
  ~mandatory_record_input_stream() {}

  /// Parse record headings from a file of records.  THIS FUNCTION IS SLOW.
  /**
   * Exits the program if the headers could not be parsed.
   */
  Record parse_headings() {
    char c;
    std::vector<unsigned char> headings;
    while (m_stream.read_some(&c, 1) > 0) {
      headings.push_back(c);
      if (Record_Ref::get_record_end(&headings[0], headings.size()) != 0) {
        Record_Ref ref(&headings[0],
                        &headings[0] + headings.size(), 
                        0);
        return Record(ref);
      }
    }
      
    // If we get to here then the headings line is invalid.
    NP1_ASSERT(false, "Stream " + m_stream.name() + ": Invalid headings line.  Buffer (hex, then raw): "
                    + str::get_as_hex_dump(&headings[0], &headings[0] + headings.size()));
    return Record();
  }

  /// Parse records from a CSV file and call the callback for each record.
  /**
   * Returns true if parsing completed normally, false if 
   * a callback asked us to stop.    Exits the program on fatal error. 
   */
  template <typename Record_Callback>
  inline bool parse_records(Record_Callback record_callback) {
    enum { INITIAL_BUFFER_SIZE = 256 * 1024 };
    std::vector<unsigned char> buffer;    
    buffer.resize(INITIAL_BUFFER_SIZE);
    unsigned char *buffer_end = &buffer[0] + buffer.size();
    unsigned char *buffer_read_pos = &buffer[0];
    const unsigned char *start_record = buffer_read_pos;
    ssize_t number_bytes_read;    
    uint64_t record_number = 1;

        
    while ((number_bytes_read =
              m_stream.read_some(buffer_read_pos, 
                                  buffer_end - buffer_read_pos)) > 0) {        
      const unsigned char *buffer_data_end = buffer_read_pos + number_bytes_read;        
      const unsigned char *end_record;
      
      // Find the end of the record to make sure that we have the whole thing in the buffer. 
      while ((end_record = Record_Ref::get_record_end(start_record,
                                                      buffer_data_end - start_record))) {
        // We have the whole record.  Call the callback to deal with the
        // record.
        if (!record_callback(Record_Ref(start_record, end_record, record_number))) {
          return false;
        }
                        
        // Now go around again.            
        start_record = end_record;
        ++record_number;
      }

      // There's probably an incomplete record left in the buffer.  Move it to the start of the buffer.
      ssize_t remainder_length = buffer_data_end - start_record;
      if (remainder_length >= (ssize_t)buffer.size()) {
        size_t start_record_offset = start_record - &buffer[0];
        buffer.resize(buffer.size() + INITIAL_BUFFER_SIZE);
        buffer_end = &buffer[0] + buffer.size();
        start_record = &buffer[0] + start_record_offset;
      }
      
      memmove(&buffer[0], start_record, remainder_length);
      buffer_read_pos = &buffer[0] + remainder_length;
      start_record = &buffer[0];
    }
    
    return true;
  }

  bool close() { return m_stream.close(); }

  /// Assumes that the output stream is also a mandatory stream.
  template <typename Output_Stream>
  void copy(Output_Stream &output) { m_stream.copy(output); }

private:
  /// Disable copy.
  mandatory_record_input_stream(const mandatory_record_input_stream &);
  mandatory_record_input_stream &operator = (const mandatory_record_input_stream &);    

private:
  mandatory_input_stream<Inner_Stream> m_stream; 
};


} // namespaces
}


#endif
