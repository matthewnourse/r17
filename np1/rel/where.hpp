// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_REL_WHERE_HPP
#define NP1_REL_WHERE_HPP


#include "np1/rel/rlang/rlang.hpp"


namespace np1 {
namespace rel {



class where {
public:
  template <typename Input_Stream, typename Output_Stream>
  void operator()(Input_Stream &input, Output_Stream &output,
                  const std::vector<rel::rlang::token> &tokens) {
    /* Get the headers. */
    record headings(input.parse_headings());

    /* Use the arguments to construct the VM. */
    rlang::vm_heap heap;
    record empty_headings;
    rlang::vm vm = rlang::compiler::compile_single_expression(
                      tokens, headings.ref(), empty_headings.ref());

    // Check that the expression is actually a boolean expression.
    NP1_ASSERT(vm.return_type() == rlang::dt::TYPE_BOOL,
                "Expression is not a boolean expression");
  
    // Write out the headings and then do the work.
    headings.write(output);

    //TODO: a fast path for simple comparisons.  Remember that integers can't
    // be compared with memcmp because of leading zeroes.
    input.parse_records(record_callback<Output_Stream>(vm, output, heap));
  }

private:
  template <typename Output>
  struct record_callback {
    record_callback(rlang::vm &vm, Output &o, rlang::vm_heap &h)
      : m_vm(vm), m_output(o), m_heap(h) {}  

    bool operator()(const record_ref &r) const {
      rlang::vm_stack &stack = m_vm.run_heap_reset(m_heap, r, m_empty.ref());
      bool result;
      stack.pop(result);
      if (result) {
        r.write(m_output);
      }

      return true;
    }        

    rlang::vm &m_vm;
    Output &m_output;
    rlang::vm_heap &m_heap;
    record m_empty;
  };
};


} // namespaces
}


#endif
