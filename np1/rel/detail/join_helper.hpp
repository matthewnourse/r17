// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_REL_DETAIL_JOIN_HELPER_HPP
#define NP1_REL_DETAIL_JOIN_HELPER_HPP




namespace np1 {
namespace rel {
namespace detail {
  
/// Some join-specific helper functions. 
namespace join_helper
{

typedef struct empty_struct {} empty_type;  



// Figure out the common and non-common headings between two heading records.
void find_common_and_non_common_headings(const record &file1_headers, const record &file2_headers,
                                          rstd::vector<rstd::string> &common_heading_names,
                                          rstd::vector<size_t> &file2_non_common_field_numbers) {
  rstd::vector<size_t> file1_common_field_numbers;
  rstd::vector<size_t> file2_common_field_numbers;

  // Find the common headings.
  size_t number_file1_columns = file1_headers.number_fields(); 
  size_t number_file2_columns = file2_headers.number_fields();
  size_t i1;
  size_t i2;

  for (i1 = 0; (i1 < number_file1_columns); ++i1) {
    str::ref heading1 = file1_headers.field(i1);      
    for (i2 = 0; (i2 < number_file2_columns); ++i2) {
      str::ref heading2 = file2_headers.field(i2);
        
      if ((heading1.length() == heading2.length())
          && (memcmp(heading1.ptr(), heading2.ptr(), heading1.length()) == 0)) {
        common_heading_names.push_back(heading1.to_string());            
        file1_common_field_numbers.push_back(i1);
        file2_common_field_numbers.push_back(i2);          
      }
    }  
  }
  
  size_t i;
  size_t field_number;
  
  /* Figure out which fields from file2 are not common. */
  for (field_number = 0; field_number < number_file2_columns; ++field_number){
    bool found = false;
    for (i = 0; !found && (i < file2_common_field_numbers.size()); ++i) {
      if (file2_common_field_numbers[i] == field_number) {
        found = true;
      }
    }
    
    if (!found) {
      file2_non_common_field_numbers.push_back(field_number);        
    }
  }
}



/* Write out the supplied records as a single record, writing out only one
 * copy of the common field. */
template <typename Output>
void record_merge_write(
              Output &output,
              const record_ref &r1, 
              const record_ref &r2, 
              const rstd::vector<size_t> &r2_non_common_field_numbers,
              rstd::vector<str::ref> &r2_non_common_field_refs_storage) {
  /* If the first record is empty, then we can just write out the
   * second record. */
  if (r1.is_empty()) {
    r2.write(output);
    return;
  }

  if (r2_non_common_field_numbers.size() > 0) {
    // Get the non-matching columns from the second record.    
    r2_non_common_field_refs_storage.resize(r2_non_common_field_numbers.size());

    rstd::vector<size_t>::const_iterator n_i = r2_non_common_field_numbers.begin();
    rstd::vector<size_t>::const_iterator n_iz = r2_non_common_field_numbers.end();
  
    rstd::vector<str::ref>::iterator r_i = r2_non_common_field_refs_storage.begin();

    for (; n_i != n_iz; ++n_i, ++r_i) {
      *r_i = r2.field(*n_i);
    }

    record_ref::write(output, r1, r2_non_common_field_refs_storage);
  } else {
    r1.write(output);      
  }
}


// Called for each record that matches during a merge.
template <typename Output>
struct matching_record_callback {
  matching_record_callback(
    Output &output, 
    const rstd::vector<size_t> &file2_non_common_field_numbers,
    rstd::vector<str::ref> &file2_non_common_field_refs_storage,
    const record_ref &ref1,
    bool &called)
  : m_output(output)
  , m_file2_non_common_field_numbers(file2_non_common_field_numbers)
  , m_file2_non_common_field_refs_storage(file2_non_common_field_refs_storage)
  , m_ref1(ref1)
  , m_called(called) {
    m_called = false;
  }
  
  bool operator()(const record &r2, const empty_type &v) {
    record_merge_write(m_output, m_ref1, r2.ref(),
                        m_file2_non_common_field_numbers,
                        m_file2_non_common_field_refs_storage);
    m_called = true;
    return true;
  }
  
  Output &m_output;
  const rstd::vector<size_t> &m_file2_non_common_field_numbers;
  rstd::vector<str::ref> &m_file2_non_common_field_refs_storage;
  const record_ref &m_ref1;
  bool &m_called;
};


// The record callback for when we're reading in the file that will be inserted
// into memory.
struct insert_record_callback {
  explicit insert_record_callback(detail::record_multihashmap<empty_type> &m) : m_map(m) {}
   
  bool operator()(const record_ref &r) const {
    m_map.insert(r, empty_type());    
    return true;
  }
  
  detail::record_multihashmap<empty_type> &m_map;
};


// We don't support joins on double fields because floating point equality comparison is not reliable.
void validate_compare_specs(const compare_specs &specs) {
  NP1_ASSERT(
    !specs.has_double(),
    "Joins on floating-point columns are not supported because floating-point equality comparison is unreliable.");   
}

} // namespaces
}
}
}


#endif
