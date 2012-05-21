// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_REL_RECORD_COUNT_HPP
#define NP1_REL_RECORD_COUNT_HPP

namespace np1 {
namespace rel {


class record_count {
public:
  template <typename Input_Stream, typename Output_Stream>
  void operator()(Input_Stream &input, Output_Stream &output,
                  const rstd::vector<rel::rlang::token> &tokens) {
    NP1_ASSERT(tokens.size() == 0, "rel.record_count accepts no arguments"); 

    // Read & discard the headings.
    input.parse_headings();

    uint64_t number_records = 0;    
    input.parse_records(record_counter_callback(number_records));
    output.write(str::to_dec_str(number_records).c_str());
  }

private:
  // The callback for all records.
  struct record_counter_callback {
    record_counter_callback(uint64_t &number_records) : m_number_records(number_records) {} 
      
    bool operator()(const record_ref &r) const {
      ++m_number_records;
      return true;
    }
    
    uint64_t &m_number_records;
  };
};


} // namespaces
}


#endif
