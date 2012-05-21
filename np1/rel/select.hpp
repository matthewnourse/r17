// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_REL_SELECT_HPP
#define NP1_REL_SELECT_HPP


#include "np1/rel/rlang/rlang.hpp"


namespace np1 {
namespace rel {



class select {
public:
  template <typename Input_Stream, typename Output_Stream>
  void operator()(Input_Stream &input, Output_Stream &output,
                  const rstd::vector<rel::rlang::token> &tokens) {
    /* Get the headers. */
    record headings(input.parse_headings());
    
    /* Use the arguments to construct the VMs. */
    rstd::vector<rlang::compiler::vm_info> vm_infos;
    rlang::vm_heap heap;
    rlang::compiler::compile_select(tokens, headings.ref(), vm_infos);

    // Write out the headings.
    rstd::vector<rstd::string> output_headings = make_output_headings(vm_infos);
    record_ref::write(output, output_headings);        
    
    // Many select columns are just a copy from the current or previous record, so provide a fast path for them.
    rstd::vector<vm_fastpath_summary> fastpath_summaries = create_vm_fastpath_summaries(vm_infos);
    
    // Now do the real work.
    if (rlang::compiler::any_references_to_other_record(tokens)) {
      pin_prev_record_output_stream<Output_Stream> pinning_os(output, output_headings);
      input.parse_records(
        record_callback<pin_prev_record_output_stream<Output_Stream> >(
            vm_infos, fastpath_summaries, pinning_os, heap));
    } else {
      passthrough_output_stream<Output_Stream> passthrough_os(output);
      input.parse_records(
        record_callback<passthrough_output_stream<Output_Stream> >(
            vm_infos, fastpath_summaries, passthrough_os, heap));
    }
  }

  // Get references to the output headings.
  static rstd::vector<rstd::string> make_output_headings(
                                  const rstd::vector<rlang::compiler::vm_info> &vm_infos) {
    rstd::vector<rstd::string> output_headings;
    rstd::vector<rlang::compiler::vm_info>::const_iterator vm_info_i = vm_infos.begin();
    rstd::vector<rlang::compiler::vm_info>::const_iterator vm_info_iz = vm_infos.end();

    for (; vm_info_i < vm_info_iz; ++vm_info_i) {
      output_headings.push_back(str::ref(vm_info_i->get_typed_heading_name()).to_string());
    }

    return output_headings;
  }

private:
  /// Storage space for holding numbers represented as strings.
  class num_str_buffer {
  public:
    enum { MAX_BUFFER_LENGTH = 16 * 1024 };

  public:
    num_str_buffer() : m_buffer_pos(0) {}

    template <typename T>
    str::ref append(T i) {
      check_capacity();
      char *p = &m_buffer[m_buffer_pos];
      str::to_dec_str(p, i);
      size_t length = strlen(p);
      m_buffer_pos += length;
      return str::ref(p, length);      
    }

    void clear() {
      m_buffer_pos = 0;    
    }

  private:
    inline void check_capacity() {
      NP1_ASSERT(m_buffer_pos + str::MAX_NUM_STR_LENGTH < MAX_BUFFER_LENGTH,
                  "Maximum number string buffer length exceeded");   
    }

  private:
    char m_buffer[MAX_BUFFER_LENGTH];
    size_t m_buffer_pos;
  };

  
  struct vm_fastpath_summary {
    typedef enum {NONE = 0, PUSH_THIS = 1, PUSH_OTHER = 2} fastpath_type;
    
    vm_fastpath_summary() : m_type(NONE), m_field_number(-1) {}
    
    explicit vm_fastpath_summary(const rlang::compiler::vm_info &vm_info) {
      m_type = NONE;
      m_field_number = -1;

      const rlang::vm &vm = vm_info.get_vm();
      if (vm.is_single_call()) {
        if (vm.is_push_this_field_only(m_field_number)) {
          m_type = PUSH_THIS;
        } else if (vm.is_push_other_field_only(m_field_number)) {
          m_type = PUSH_OTHER;
        }
      }  
    }

    bool is_fastpath() const { return !!m_type; }
    fastpath_type type() const { return m_type; }
    size_t field_number() const { return m_field_number; }

    fastpath_type m_type;
    size_t m_field_number;
  };


  static rstd::vector<vm_fastpath_summary> create_vm_fastpath_summaries(
                                            const rstd::vector<rlang::compiler::vm_info> &vm_infos) {
    rstd::vector<vm_fastpath_summary> summaries;
    summaries.resize(vm_infos.size());
    rstd::vector<rlang::compiler::vm_info>::const_iterator vm_info_i = vm_infos.begin();
    rstd::vector<rlang::compiler::vm_info>::const_iterator vm_info_iz = vm_infos.end();
    rstd::vector<vm_fastpath_summary>::iterator summary_i = summaries.begin();
    for (; vm_info_i != vm_info_iz; ++vm_info_i, ++summary_i) {
      *summary_i = vm_fastpath_summary(*vm_info_i);
    }
    
    return summaries;
  }
    


  template <typename Prev_Handling_Output>
  struct record_callback {
    explicit record_callback(rstd::vector<rlang::compiler::vm_info> &vm_infos,
                              const rstd::vector<vm_fastpath_summary> &fastpath_summaries,
                              Prev_Handling_Output &output,
                              rlang::vm_heap &heap)
    : m_vm_infos(vm_infos), m_fastpath_summaries(fastpath_summaries), m_output(output), m_heap(heap) {
      m_field_refs.resize(m_vm_infos.size());
    }
    
    bool operator()(const record_ref &r) {
      rstd::vector<rlang::compiler::vm_info>::iterator vm_info_i = m_vm_infos.begin();
      rstd::vector<rlang::compiler::vm_info>::iterator vm_info_iz = m_vm_infos.end();
      rstd::vector<vm_fastpath_summary>::const_iterator fastpath_summary_i = m_fastpath_summaries.begin();
      rstd::vector<str::ref>::iterator fr_i = m_field_refs.begin();
      
      m_num_str_buffer.clear();

      uint64_t prev_record_number = r.record_number() - 1;
      for (; (vm_info_i < vm_info_iz); ++vm_info_i, ++fastpath_summary_i, ++fr_i) {
        if (fastpath_summary_i->is_fastpath()) {          
          size_t field_number = fastpath_summary_i->field_number();
          if (fastpath_summary_i->type() == vm_fastpath_summary::PUSH_THIS) {
            *fr_i = r.mandatory_field(field_number);
          } else {
            *fr_i = m_output.get_prev(prev_record_number).mandatory_field(field_number);
          }
        } else {
          rlang::vm &vm = vm_info_i->get_vm();
          
          // Not a fast path, just do the normal thing.
          rlang::vm_stack &stack = vm.run_no_heap_reset(
                                    m_heap, r, m_output.get_prev(prev_record_number));
          switch (vm.return_type()) {
          case rlang::dt::TYPE_STRING:
          case rlang::dt::TYPE_ISTRING:
          case rlang::dt::TYPE_IPADDRESS:
            {
              str::ref s;
              stack.pop(s);
              *fr_i = s;
            }
            break;
  
          case rlang::dt::TYPE_INT:
            {
              int64_t i;
              stack.pop(i);            
              *fr_i = m_num_str_buffer.append(i);
            }
            break;
    
          case rlang::dt::TYPE_UINT:
            {
              uint64_t ui;
              stack.pop(ui);            
              *fr_i = m_num_str_buffer.append(ui);
            }
            break;

          case rlang::dt::TYPE_DOUBLE:
            {
              double d;
              stack.pop(d);
              *fr_i = m_num_str_buffer.append(d);
            }
            break;
  
          case rlang::dt::TYPE_BOOL:
            {
              bool b;
              stack.pop(b);
              *fr_i = str::from_bool(b);
            }
            break;
          }
        }
      }

      record_ref::write(m_output, m_field_refs);
      m_output.flush_and_reset();
      m_heap.reset();
      return true;
    }
    
    rstd::vector<rlang::compiler::vm_info> &m_vm_infos;
    const rstd::vector<vm_fastpath_summary> &m_fastpath_summaries;
    rstd::vector<str::ref> m_field_refs;
    num_str_buffer m_num_str_buffer;
    Prev_Handling_Output &m_output;
    rlang::vm_heap &m_heap;
    record m_empty;
  };


  /// This output stream wrapper stores the "previous" record.
  template <typename Output_Stream>
  struct pin_prev_record_output_stream {
    enum { MAX_PREV_RECORD_LENGTH = 32 * 1024 };

    pin_prev_record_output_stream(Output_Stream &output,
                                  const rstd::vector<rstd::string> &output_headings)
      : m_output(output), m_curr(&m_buffer1), m_prev(&m_buffer2) {
      // Use the headings to construct a sensible "prev" record for the
      // very first record.
      //TODO: document this behaviour somewhere!
      rstd::vector<str::ref> fake_prev_fields;
      fake_prev_fields.resize(output_headings.size());

      size_t i = 0;
      size_t number_fields = output_headings.size();
      for (; i < number_fields; ++i) {
        str::ref heading = str::ref(output_headings[i]);
        fake_prev_fields[i] =
          rlang::dt::empty_value(rlang::dt::mandatory_from_string(
                                  detail::helper::mandatory_get_heading_type_tag(heading))); 
      }

      record_ref::write(*m_prev, fake_prev_fields);
    }

    inline void write(const void *buf, size_t length) {
      NP1_ASSERT(m_curr->write(buf, length),
                  "Record too long to be stored in prev buffer");
    }

    inline void write(char c) { write(&c, 1); }
    inline void write(const char *s) { write(s, strlen(s)); }

    record_ref get_prev(size_t record_number) const {
      return record_ref(m_prev->ptr(), m_prev->ptr() + m_prev->size(), record_number);
    }

    void flush_and_reset() {
      m_output.write(m_curr->ptr(), m_curr->size());      
      m_prev->reset();
      rstd::swap(m_prev, m_curr);
    }

    Output_Stream &m_output;
    io::static_buffer_output_stream<MAX_PREV_RECORD_LENGTH> m_buffer1;
    io::static_buffer_output_stream<MAX_PREV_RECORD_LENGTH> m_buffer2;

    io::static_buffer_output_stream<MAX_PREV_RECORD_LENGTH> *m_curr;
    io::static_buffer_output_stream<MAX_PREV_RECORD_LENGTH> *m_prev;
  };


  /// This output stream wrapper always returns an empty "previous" record.
  template <typename Output_Stream>
  struct passthrough_output_stream {
    explicit passthrough_output_stream(Output_Stream &output) : m_output(output) {}

    inline void write(const void *buf, size_t length) {
      m_output.write(buf, length);
    }

    inline void write(char c) { write(&c, 1); }
    inline void write(const char *s) { write(s, strlen(s)); }

    record_ref get_prev(size_t record_number) const {
      return record_ref(0, 0, record_number);
    }

    void flush_and_reset() {}

    Output_Stream &m_output;  
  };
};


} // namespaces
}

#endif

