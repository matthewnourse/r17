// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_REL_UNIQUE_HPP
#define NP1_REL_UNIQUE_HPP

namespace np1 {
namespace rel {


class unique {
public:
  template <typename Input_Stream, typename Output_Stream>
  void operator()(Input_Stream &input, Output_Stream &output,
                  const std::vector<rel::rlang::token> &tokens) {
    // Read the headings from stdin.
    record headings(input.parse_headings());
    
    // Make the map that will hold the groupings.
    detail::compare_specs specs(headings);
    validate_specs(specs);
    detail::record_multihashmap<empty_type> unique_map(specs);
  
    // Parse the stream and output only the first instance of each record.
    headings.write(output);
    input.parse_records(record_callback<Output_Stream>(output, unique_map));
  }

private:
  //TODO: put the empty type in one place.
  struct empty_type {};

  static void validate_specs(const detail::compare_specs &specs) {
    NP1_ASSERT(
      !specs.has_double(),
      "Unique on floating-point columns is not supported because floating-point equality comparison is unreliable.");   
  }

  
  // The callback for all records.
  template <typename Output>
  struct record_callback {
    record_callback(Output &output, detail::record_multihashmap<empty_type> &m) 
      : m_output(output), m_map(m) {}
      
    bool operator()(const record_ref &r) const {
      if (!m_map.find(r)) {
        m_map.insert(r, empty_type());
        r.write(m_output);
      }
      
      return true;
    }
    
    Output &m_output;
    detail::record_multihashmap<empty_type> &m_map;  
  };
};


} // namespaces
}


#endif
