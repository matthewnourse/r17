// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_IO_MANDATORY_MAPPED_RECORD_INPUT_FILE_HPP
#define NP1_IO_MANDATORY_MAPPED_RECORD_INPUT_FILE_HPP

#include "rstd/string.hpp"
#include "np1/io/file_mapping.hpp"
#include "np1/assert.hpp"

namespace np1  {
namespace io {

/// A memory mapped input file of records where all failures crash the process.
template <typename Record_Ref>
class mandatory_mapped_record_input_file {
public:
  /// Constructor.
  explicit mandatory_mapped_record_input_file(io::file_mapping &fm, uint64_t starting_record_number = 0)
    : m_mapping(fm), m_ptr((const unsigned char *)m_mapping.ptr()), m_end(m_ptr + m_mapping.size()),
      m_record_number(starting_record_number) {}
  
  /// Destructor.
  ~mandatory_mapped_record_input_file() {}

  /// Read one record from the mapping.  Crashes on error, returns false on EOF.
  bool read_record(Record_Ref &r) {
    const unsigned char *end_record = Record_Ref::get_record_end(m_ptr, m_end - m_ptr);
    if (!end_record) {
      NP1_ASSERT(m_ptr == m_end, "Incomplete record at end of mapped record file");
      return false;
    }
      
    r = Record_Ref(m_ptr, end_record, m_record_number++);
    m_ptr = end_record;
    return true;
  }
  
  Record_Ref mandatory_read_record() {
    Record_Ref r;
    NP1_ASSERT(read_record(r), "Unexpected end-of-file in mapped record file");
    return r;
  }
  
private:
  /// Disable copy.
  mandatory_mapped_record_input_file(const mandatory_mapped_record_input_file &);
  mandatory_mapped_record_input_file &operator = (const mandatory_mapped_record_input_file &);    
  
private:
  io::file_mapping &m_mapping;
  const unsigned char *m_ptr;
  const unsigned char *m_end;
  uint64_t m_record_number;
};


} // namespaces
}


#endif
