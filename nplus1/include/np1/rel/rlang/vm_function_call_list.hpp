// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_REL_RLANG_VM_FUNCTION_CALL_LIST_HPP
#define NP1_REL_RLANG_VM_FUNCTION_CALL_LIST_HPP


#include "np1/rel/rlang/vm_function_call.hpp"

namespace np1 {
namespace rel {
namespace rlang {

template <size_t N>
class vm_function_call_list {
public:
  vm_function_call_list() : m_size(0) {}

  void push_back(const vm_function_call &fc) {
    NP1_ASSERT(m_size+1 < N, "Maximum number of rlang terms exceeded.  Max: "
                              + str::to_dec_str(N));
    m_function_calls[m_size++] = fc;
  }

  const vm_function_call *begin() const { return &m_function_calls[0]; }
  const vm_function_call *end() const { return &m_function_calls[m_size]; }

  size_t size() const { return m_size; }
  
  vm_function_call &operator[](size_t n) {
    NP1_ASSERT(n < m_size, "Function call offset out of range!");
    return m_function_calls[n];
  }

  const vm_function_call &operator[](size_t n) const {
    NP1_ASSERT(n < m_size, "Function call offset out of range!");
    return m_function_calls[n];
  }

private:  
  vm_function_call m_function_calls[N];
  size_t m_size;
};


} // namespaces
}
}



#endif
