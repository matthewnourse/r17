// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_REL_RLANG_VM_FUNCTION_CALL_HPP
#define NP1_REL_RLANG_VM_FUNCTION_CALL_HPP


namespace np1 {
namespace rel {
namespace rlang {

class vm_function_call {
public:
  vm_function_call() : m_id(-1), m_data(-1) {}
  vm_function_call(size_t id, size_t data) : m_id(id), m_data(data) {}

  size_t id() const { return m_id; }
  size_t data() const { return m_data; }
  void data(size_t d) { m_data = d; }

private:  
  size_t m_id;
  size_t m_data;  
};


} // namespaces
}
}



#endif
