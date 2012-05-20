// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_REL_RLANG_VM_HPP
#define NP1_REL_RLANG_VM_HPP

#include "np1/rel/rlang/vm_function_call_list.hpp"
#include "np1/rel/rlang/vm_stack.hpp"
#include "np1/rel/rlang/simulated_stack.hpp"
#include "np1/rel/rlang/vm_heap.hpp"
#include "np1/rel/rlang/vm_literals.hpp"
#include "np1/rel/rlang/fn/fn.hpp"
#include "np1/rel/rlang/fn/fn_table.hpp"
#include "np1/rel/rlang/fn/op.hpp"
#include "np1/rel/rlang/shunting_yard.hpp"
#include "np1/io/string_output_stream.hpp"
#include "np1/help/markdown.hpp"

namespace np1 {
namespace rel {
namespace rlang {

/// A stack-based virtual machine.
class vm {
public:
  enum { MAX_NUMBER_FUNCTION_CALLS = 1000 };

public:
  vm() : m_return_type(dt::TYPE_INT), m_refers_to_other_record(false) {}

  vm(const vm_literals &literals,
      const vm_function_call_list<MAX_NUMBER_FUNCTION_CALLS> &function_calls,
      dt::data_type return_type, bool refers_to_other_record)
    : m_literals(literals), m_function_calls(function_calls),
      m_return_type(return_type), m_refers_to_other_record(refers_to_other_record) {
    // Add one more function call to the end of the list- this last one tells
    // the VM to stop running.  We do it this way rather than using a loop
    // to reduce the number of jumps.
    m_function_calls.push_back(
        vm_function_call(NP1_REL_RLANG_FN_NUMBER_SUPPORTED_FUNCTIONS, (size_t)-1));
  }


  /// Get the return type of the value that we'll get by running the VM.
  dt::data_type return_type() const { return m_return_type; }

  /// Does this virtual machine actually refer to the "other" record?
  bool refers_to_other_record() const { return m_refers_to_other_record; }


  /// Run the virtual machine against the supplied records.  Returns a reference
  /// to the VM's stack, valid only until the next run.
  vm_stack &run_heap_reset(vm_heap &heap, const record_ref &this_r, const record_ref &other_r) {
    heap.reset();
    return run_no_heap_reset(heap, this_r, other_r);
  }

  vm_stack &run_no_heap_reset(vm_heap &heap, const record_ref &this_r, const record_ref &other_r) {
    m_stack.reset();

#if (NP1_TRACE_ON == 1)
    const vm_function_call *i = m_function_calls.begin();
    const vm_function_call *iz = m_function_calls.end();

    fprintf(stderr, "FUNCTION CALL LIST:\n");
    for (; i < iz; ++i) {
      fprintf(
        stderr,
        "offset=%zu, id=%zu, data=%zu, name='%s'\n",
        i - m_function_calls.begin(),
        i->id(),
        i->data(),
        (i->id() < NP1_REL_RLANG_FN_NUMBER_SUPPORTED_FUNCTIONS) ? fn::fn_table::get_info(i->id()).name() : "[end]");
    }
#endif

    // This will return at the appropriate moment.
    NP1_REL_RLANG_FN_TABLE_CALL_ALL(
        m_function_calls, m_stack, heap, m_literals, this_r, other_r);
  }
  
  bool is_single_call() const {
    return (m_function_calls.size() == 2);  // 1 for the real function call, one for the "stop" function call.
  }

  bool is_push_this_field_only(size_t &field_number) const {
    field_number = 0;    
    if (is_single_call()) {
      const vm_function_call &call = m_function_calls[0];
      if (fn::fn_table::get_info(call.id()).is_push_this()) {
        field_number = call.data();
        return true;
      }            
    }
    
    return false;
  }

  bool is_push_other_field_only(size_t &field_number) const {
    field_number = 0;    
    if (is_single_call() == 1) {
      const vm_function_call &call = m_function_calls[0];
      if (fn::fn_table::get_info(call.id()).is_push_other()) {
        field_number = call.data();
        return true;
      }      
    }
    
    return false;
  }


private:
  vm_stack m_stack;
  vm_literals m_literals;
  vm_function_call_list<MAX_NUMBER_FUNCTION_CALLS> m_function_calls;
  dt::data_type m_return_type;
  bool m_refers_to_other_record;
};


} // namespaces
}
}



#endif
