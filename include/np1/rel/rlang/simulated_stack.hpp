// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_REL_RLANG_SIMULATED_STACK_HPP
#define NP1_REL_RLANG_SIMULATED_STACK_HPP


#include "np1/rel/rlang/vm_stack.hpp"
#include "np1/rel/rlang/dt.hpp"


namespace np1 {
namespace rel {
namespace rlang {


/// The fake stack, used while compiling.
class simulated_stack {
public:
  simulated_stack() : m_size(0), m_byte_size(0) {}

  void push(dt::data_type type) {
    m_byte_size += vm_stack::type_size(type);
    NP1_ASSERT(m_byte_size < vm_stack::MAX_STACK_BYTE_SIZE,
                "Expression is too deeply nested, maximum stack size exceeded");
    m_data_types[m_size++] = type;      
  }

  dt::data_type pop() {
    NP1_ASSERT(m_size > 0, "Attempt to pop from empty simulated stack");
    dt::data_type type = m_data_types[--m_size];
    m_byte_size -= vm_stack::type_size(type);
    return type;
  }

  size_t size() const { return m_size; }

  const dt::data_type &top() const { return at(m_size-1); }
  const dt::data_type &top_minus_1() const { return at(m_size-2); }
  const dt::data_type &top_minus_2() const { return at(m_size-3); }
  

private:
  const dt::data_type &at(size_t offset) const {
    NP1_ASSERT((offset >= 0) && (offset < m_size),
                "Simulated stack operator[] offset out of bounds");
    return m_data_types[offset];
  }


private:
  dt::data_type m_data_types[vm_stack::MAX_STACK_BYTE_SIZE+1];
  size_t m_size;
  size_t m_byte_size;
};



} // namespaces
}
}


#endif
