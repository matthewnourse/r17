// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_REL_JOIN_NATURAL_HPP
#define NP1_REL_JOIN_NATURAL_HPP


#include "np1/io/mandatory_record_input_stream.hpp"
#include "np1/io/gzfile.hpp"
#include "np1/consistent_hash_table.hpp"
#include "np1/rel/detail/join_helper.hpp"


namespace np1 {
namespace rel {


class join_natural {
public:
  template <typename Input_Stream, typename Output_Stream>
  void operator()(Input_Stream &input, Output_Stream &output,
                  const rstd::vector<rel::rlang::token> &tokens) {  
    /* Get the arguments. */
    rstd::string file_name2(rel::rlang::compiler::eval_to_string_only(tokens));
          
    /* Get the list of headers from the first file. */
    record file1_headers(input.parse_headings());                
  
    /* Get the list of headers from the second file. */
    io::gzfile file2;
    if (!file2.open_ro(file_name2.c_str())) {
      NP1_ASSERT(false, "Unable to open input file " + file_name2);
    }

    io::mandatory_record_input_stream<io::gzfile, record, record_ref> file2_stream(file2);
    record file2_headers(file2_stream.parse_headings());              
                
    /* Figure out which headings are common and not common. */
    rstd::vector<rstd::string> common_heading_names;  
    rstd::vector<size_t> file2_non_common_field_numbers;  

    detail::join_helper::find_common_and_non_common_headings(
      file1_headers, file2_headers, common_heading_names, file2_non_common_field_numbers);

    // Now read file2 into memory.
    detail::compare_specs compare_specs2(file2_headers, common_heading_names);
    detail::record_multihashmap<detail::join_helper::empty_type> map2(compare_specs2);
    file2_stream.parse_records(detail::join_helper::insert_record_callback(map2));
    
    // Close file2 'cause it might be using a big buffer and we can use all the RAM we can lay our mitts on.
    file2.close();
        
    // Set up the compare specs for file1.
    detail::compare_specs compare_specs1(file1_headers, common_heading_names);

    // Check that the join participants are all data types we support.
    detail::join_helper::validate_compare_specs(compare_specs1);
    detail::join_helper::validate_compare_specs(compare_specs2);

    rstd::vector<str::ref> file2_non_common_field_refs_storage;
                      
    // Write out the headings.
    detail::join_helper::record_merge_write(
      output, file1_headers.ref(), file2_headers.ref(), file2_non_common_field_numbers,
      file2_non_common_field_refs_storage);
    
    // Now read in file1 and merge as we go.
    input.parse_records(
      natural_merge_record_callback<Output_Stream>(output, map2, compare_specs1, file2_non_common_field_numbers));
  }

private:  
  // The record callback for when we're merging the file1 stream with 
  // file2 (in memory) as part of a natural join.
  template <typename Output>
  struct natural_merge_record_callback {
    natural_merge_record_callback(
        Output &output, detail::record_multihashmap<detail::join_helper::empty_type> &map2,
        const detail::compare_specs &specs1,
        const rstd::vector<size_t> &file2_non_common_field_numbers)
    : m_output(output)
    , m_map2(map2)
    , m_specs1(specs1)
    , m_file2_non_common_field_numbers(file2_non_common_field_numbers) {}
    
    // The record_ref we get here is from file1.
    bool operator()(const record_ref &ref1) {
      bool unused = false;
      return m_map2.for_each(
              detail::join_helper::matching_record_callback<Output>(
                  m_output, m_file2_non_common_field_numbers, m_file2_non_common_field_refs_storage, ref1, unused),
              ref1,
              m_specs1);       
    }
    
    Output &m_output;
    detail::record_multihashmap<detail::join_helper::empty_type> &m_map2;
    detail::compare_specs m_specs1;
    rstd::vector<size_t> m_file2_non_common_field_numbers;
    rstd::vector<str::ref> m_file2_non_common_field_refs_storage;
  };
};

} // namespaces
}


#endif
