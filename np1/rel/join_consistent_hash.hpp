// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_REL_JOIN_CONSISTENT_HASH_HPP
#define NP1_REL_JOIN_CONSISTENT_HASH_HPP


#include "np1/io/mandatory_record_input_stream.hpp"
#include "np1/io/gzfile.hpp"
#include "np1/consistent_hash_table.hpp"
#include "np1/rel/detail/join_helper.hpp"


namespace np1 {
namespace rel {


class join_consistent_hash {
public:
  template <typename Input_Stream, typename Output_Stream>
  void operator()(Input_Stream &input, Output_Stream &output,
                  const std::vector<rel::rlang::token> &tokens) {  
    /* Get the arguments. */
    std::string file_name2(rel::rlang::compiler::eval_to_string_only(tokens));
          
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
    std::vector<std::string> common_heading_names;  
    std::vector<size_t> file2_non_common_field_numbers;  

    detail::join_helper::find_common_and_non_common_headings(
      file1_headers, file2_headers, common_heading_names, file2_non_common_field_numbers);

    // Now read file2 into memory.
    consistent_hash_table<record, consistent_hash_function> consistent_hash_table2(1);
    file2_stream.parse_records(consistent_hash_table_insert_record_callback(consistent_hash_table2));
    
    // Close file2 'cause it might be using a big buffer and we can use all the RAM we can lay our mitts on.
    file2.close();
        
    // Hash join based on consistent hash.
    NP1_ASSERT(common_heading_names.size() == 0, "join.consistent_hash does not support common heading names");

    std::vector<str::ref> file2_non_common_field_refs_storage;

    // Write out the headings.
    detail::join_helper::record_merge_write(
      output, file1_headers.ref(), file2_headers.ref(), file2_non_common_field_numbers,
      file2_non_common_field_refs_storage);

    // Now read in file1 and merge as we go.
    input.parse_records(
      consistent_hash_record_callback<Output_Stream>(
        output, consistent_hash_table2, file2_non_common_field_numbers)); 
  }

private:
  // Function for the consistent hash table.
  struct consistent_hash_function {
    uint64_t operator()(const record &r, uint64_t consistent_hash_internal) {
      uint64_t hval = hash::fnv1a64::init();
      size_t number_fields = r.number_fields();
      size_t field_id;
      for (field_id = 0; field_id < number_fields; ++field_id) {
        str::ref field = r.mandatory_field(field_id);
        hval = hash::fnv1a64::add(field.ptr(), field.length(), hval);
      }
      hval = hash::fnv1a64::add(&consistent_hash_internal, sizeof(consistent_hash_internal), hval);
      return hval;
    }
  };

  template <typename Output>
  struct consistent_hash_record_callback {
    consistent_hash_record_callback(
        Output &output, consistent_hash_table<record, consistent_hash_function> &cht2,
        const std::vector<size_t> &file2_non_common_field_numbers)
        : m_output(output), m_cht2(cht2), m_file2_non_common_field_numbers(file2_non_common_field_numbers) {}
    
    // The record_ref we get here is from file1.
    bool operator()(const record_ref &ref1) {
      //TODO: constructing a record object is very wasteful here.
      consistent_hash_table<record, consistent_hash_function>::iterator iter2 = m_cht2.lower_bound(record(ref1));
      NP1_ASSERT(iter2 != m_cht2.end(), "rel.join.consistent_hash expects a non-empty 'other' file.");
      
      detail::join_helper::record_merge_write(
        m_output, ref1, iter2->ref(), m_file2_non_common_field_numbers, m_file2_non_common_field_refs_storage);

      return true;
    }
    
    Output &m_output;
    consistent_hash_table<record, consistent_hash_function> &m_cht2;
    std::vector<size_t> m_file2_non_common_field_numbers;
    std::vector<str::ref> m_file2_non_common_field_refs_storage;
  };

  // The record callback for inserting into the consistent hash table.
  struct consistent_hash_table_insert_record_callback {
    explicit consistent_hash_table_insert_record_callback(
              consistent_hash_table<record, consistent_hash_function> &cht) : m_cht(cht) {}
     
    bool operator()(const record_ref &r) const {
      m_cht.insert_allow_duplicates(record(r));    
      return true;
    }
    
    consistent_hash_table<record, consistent_hash_function> &m_cht;
  };
};

} // namespaces
}


#endif
