// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_NP1_REL_DETAIL_SORT_MANAGER_HPP
#define NP1_NP1_REL_DETAIL_SORT_MANAGER_HPP


#include "np1/rel/record_ref.hpp"
#include "np1/process.hpp"
#include "np1/fixed_homogenous_heap.hpp"
#include "np1/skip_list.hpp"
#include "np1/io/heap_buffer_output_stream.hpp"
#include "np1/io/mandatory_mapped_record_input_file.hpp"
#include <vector>



namespace np1 {
namespace rel {
namespace detail {



/// Manage memory and I/O for sorting.
template <typename Less_Than, typename Sorter>
class sort_manager {
private:
  struct on_child_process_exit {
    void operator()() {}
  };
  
  typedef process::pool<on_child_process_exit> process_pool_type;

  struct async_sort_chunk {
    async_sort_chunk(Less_Than &less_than, Sorter &sorter, FILE *output_fp)
      : m_less_than(less_than), m_sorter(sorter), m_output_fp(output_fp) {}

    void operator()() {
      // Executed in the child process.
      m_sorter.sort(m_less_than);
      write_sorted(m_sorter, m_output_fp);
    }

    Less_Than &m_less_than;
    Sorter &m_sorter;
    FILE *m_output_fp;
  };


public:
  class sort_state {    
  public:
    explicit sort_state(const Less_Than &less_than)
      : m_less_than(less_than), m_max_chunk_size(environment::sort_chunk_size()),
        m_chunk(m_max_chunk_size), m_chunk_starting_row_number(1),
        m_child_processes(environment::sort_initial_number_threads()) {}
        
    ~sort_state() {
      m_child_processes.wait_all();
      
      std::list<FILE*>::iterator fp_i = m_child_output_fps.begin();
      std::list<FILE*>::iterator fp_iz = m_child_output_fps.end();
      for (; fp_i != fp_iz; ++fp_i) {
        fclose(*fp_i);
      }            
    }

  private:
    /// Disable copy.
    sort_state(const sort_state &);
    sort_state &operator = (const sort_state &);

  public:
    // For sort's use only.
    Less_Than m_less_than;
    size_t m_max_chunk_size;
    io::heap_buffer_output_stream m_chunk;
    uint64_t m_chunk_starting_row_number;
    Sorter m_sorter;
    process_pool_type m_child_processes;
    std::list<FILE*> m_child_output_fps;
    std::list<uint64_t> m_chunk_starting_row_numbers;
  };

public:
  explicit sort_manager(sort_state &state) : m_state(state) {}

  // Called once per input record.  
  bool operator()(const record_ref &r) {
    size_t r_byte_size = r.byte_size();
    NP1_ASSERT(r_byte_size > 0, "Cannot sort empty record!");
    
    // Will the new record fit in our chunk?    
    if (r_byte_size + m_state.m_chunk.size() > m_state.m_max_chunk_size) {
      // It won' fit, make a child process to sort the chunk and keep going asap.
      NP1_ASSERT(r_byte_size < m_state.m_max_chunk_size, "Cannot sort, record is impossibly large");
      FILE *child_output_fp = mandatory_tmpfile();
      m_state.m_child_output_fps.push_back(child_output_fp);
      m_state.m_chunk_starting_row_numbers.push_back(m_state.m_chunk_starting_row_number);
      m_state.m_child_processes.add(async_sort_chunk(m_state.m_less_than, m_state.m_sorter, child_output_fp),
                                    on_child_process_exit());
      m_state.m_chunk.reset();
      m_state.m_chunk_starting_row_number = r.record_number() + 1;
      m_state.m_sorter.clear();
    } 
        
    r.write(m_state.m_chunk);
    unsigned char *r_end = m_state.m_chunk.ptr() + m_state.m_chunk.size();
    unsigned char *r_start =  r_end - r_byte_size;
    m_state.m_sorter.insert(record_ref(r_start, r_end, r.record_number()));
    return true;
  }

  // Complete the sort.
  template <typename Mandatory_Output_Stream>
  void finalize(Mandatory_Output_Stream &output) {        
    // Sort the current chunk.  If there are no child processes then we can just write it out and we're done.
    m_state.m_sorter.sort(m_state.m_less_than);
    if (m_state.m_child_output_fps.empty()) {
      m_state.m_sorter.walk_sorted(record_output_walker<Mandatory_Output_Stream>(output));
      return;
    }
    
    // Write out the sorted chunk so that it's just like all the others.
    FILE *child_output_fp = mandatory_tmpfile();
    m_state.m_child_output_fps.push_back(child_output_fp);
    m_state.m_chunk_starting_row_numbers.push_back(m_state.m_chunk_starting_row_number);
    write_sorted(m_state.m_sorter, child_output_fp);
    
    // Wait for any child processes to finish, then merge from them.
    m_state.m_child_processes.wait_all();
    rewind_fps(m_state.m_child_output_fps);
    mapped_file_manager mfm(m_state.m_child_output_fps, m_state.m_chunk_starting_row_numbers, &m_state.m_less_than);
    record_ref r;
    while (mfm.read_next_record(r)) {
      r.write(output);
    }
  }

private:
  static void write_sorted(Sorter &sorter, FILE *output_fp) {
    typedef io::buffered_output_stream<io::file> buffered_output_type;
    typedef io::mandatory_output_stream<buffered_output_type> mandatory_buffered_output_type;
    io::file output_f;
    output_f.from_handle(output_fp);
    buffered_output_type buffered_output_f(output_f);
    mandatory_buffered_output_type mandatory_buffered_output_f(buffered_output_f);
    sorter.walk_sorted(record_output_walker<mandatory_buffered_output_type>(mandatory_buffered_output_f));
    mandatory_buffered_output_f.hard_flush();
    output_f.release();
  }

  static FILE *mandatory_tmpfile() {
    FILE *fp = tmpfile();
    NP1_ASSERT(fp, "Unable to create temporary file for sorting");
    return fp;
  }
  

  template <typename Mandatory_Output_Stream>
  struct record_output_walker {
    explicit record_output_walker(Mandatory_Output_Stream &o) : m_output(o) {}
    void operator()(const record_ref &r) {
      r.write(m_output);
    }

    Mandatory_Output_Stream &m_output;
  };
  
  void rewind_fps(std::list<FILE*> &fps) {
    std::list<FILE*>::iterator fp_i = fps.begin();
    std::list<FILE*>::iterator fp_iz = fps.end();
    for (; fp_i != fp_iz; ++fp_i) {
      rewind(*fp_i);
    }    
  }

  struct mapped_file_manager {
    typedef io::mandatory_mapped_record_input_file<record_ref> mapped_file_type;
    
    struct current_record_entry {
      current_record_entry() : m_less_than_p(0), m_mapped_file_p(0) {}
      
      current_record_entry(const record_ref &r, Less_Than *ltp, mapped_file_type *mfp)
        : m_r(r), m_less_than_p(ltp), m_mapped_file_p(mfp) {}
        
      bool operator < (const current_record_entry &other) const {
        return (*m_less_than_p)(m_r, other.m_r);        
      }
      
      record_ref m_r;
      Less_Than *m_less_than_p;
      mapped_file_type *m_mapped_file_p;
    };

    typedef skip_list<current_record_entry, np1::detail::skip_list_less_than<current_record_entry>,
                      fixed_homogenous_heap> skip_list_type;

    
    mapped_file_manager(std::list<FILE*> &fps, const std::list<uint64_t> &chunk_starting_row_numbers,
                        Less_Than *less_than_p)
      : m_heap(fps.size(), sizeof(typename skip_list_type::node)),
        m_current_records(m_heap) {
      // We do some hocus-pocus with the starting row numbers.  The chunk starting row numbers are correct
      // but of course the chunks have now been sorted, so even though we set the starting chunk row number to the
      // correct value, each row won't have the same row number as it originally did.  But this is ok because
      // we're just trying to maintain sort stability.  We want to ensure that records from earlier chunks have
      // lower sort values than records from later chunks (assuming that the less_than function looks at row
      // numbers).  If the sort is stable then records in the same chunk will already be correctly ordered so their
      // row numbers will be _relatively_ correct even though they are different from the original values.
      
      NP1_ASSERT(fps.size() == chunk_starting_row_numbers.size(),
                 "INTERNAL ERROR: starting row numbers unknown for some chunks");
      
      std::list<FILE*>::iterator fp_i = fps.begin();
      std::list<FILE*>::iterator fp_iz = fps.end();
      std::list<uint64_t>::const_iterator starting_row_number_i = chunk_starting_row_numbers.begin();      
      
      for (; fp_i != fp_iz; ++fp_i, ++starting_row_number_i) {
        io::file_mapping *mapping = std::detail::mem::alloc_construct<io::file_mapping>(fileno(*fp_i));
        m_mappings.push_back(mapping);
        mapped_file_type *mapped_file =
          std::detail::mem::alloc_construct<mapped_file_type>(*mapping, *starting_row_number_i);
          
        m_mapped_files.push_back(mapped_file);
        mandatory_current_records_insert(
          current_record_entry(mapped_file->mandatory_read_record(), less_than_p, mapped_file));
      }
    }
    
    ~mapped_file_manager() {
      std::list<mapped_file_type *>::iterator mapped_file_i = m_mapped_files.begin();
      std::list<mapped_file_type *>::iterator mapped_file_iz = m_mapped_files.end();
      for (; mapped_file_i != mapped_file_iz; ++mapped_file_i) {
        std::detail::mem::destruct_and_free(*mapped_file_i);        
      }
      
      std::list<io::file_mapping *>::iterator mapping_i = m_mappings.begin();
      std::list<io::file_mapping *>::iterator mapping_iz = m_mappings.end();
      for (; mapping_i != mapping_iz; ++mapping_i) {
        std::detail::mem::destruct_and_free(*mapping_i);
      }
    }
    
    bool read_next_record(record_ref &r) {
      if (m_mapped_files.empty()) {
        return false;
      }

      // Get the next record and remove it from our list.
      typename skip_list_type::iterator next_record_i = m_current_records.begin();
      if (next_record_i == m_current_records.end()) {
        return false;
      }
      
      r = next_record_i->m_r;
      m_current_records.pop_front();      
      
      // Insert the following record from the same file into our list of current records.
      record_ref following_r;
      if (next_record_i->m_mapped_file_p->read_record(following_r)) {
        mandatory_current_records_insert(
          current_record_entry(following_r, next_record_i->m_less_than_p, next_record_i->m_mapped_file_p));
      }
      
      return true;
    }
    
    void mandatory_current_records_insert(const current_record_entry &entry) {      
      NP1_ASSERT(
        m_current_records.insert(entry),
        "Unexpected duplicate record in current record skip list.  New record number="
          + str::to_dec_str(entry.m_r.record_number())
          + "  mapped_file=" + str::to_dec_str((size_t)entry.m_mapped_file_p) 
          + "  data=[" + entry.m_r.to_string()
          + "]  Existing record number="
          +  str::to_dec_str(m_current_records.find(entry)->m_r.record_number())
          + "  mapped_file=" + str::to_dec_str((size_t)(m_current_records.find(entry)->m_mapped_file_p)) 
          + "  data=[" + m_current_records.find(entry)->m_r.to_string() + "]");       
    }
    
    
    
    std::list<mapped_file_type *> m_mapped_files;
    std::list<io::file_mapping *> m_mappings;
    fixed_homogenous_heap m_heap;
    skip_list_type m_current_records;
  };

private:
  sort_state &m_state;
};


} // namespaces
}
}



#endif
