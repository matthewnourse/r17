// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_RSTD_STACK
#define NP1_RSTD_STACK

#include "rstd/list.hpp"

namespace rstd {

template <typename T>
class stack {
public:
  void push(const T &v) { m_stack.push_front(v); }
  void pop() { m_stack.pop_front(); }
  T &top() { return m_stack.front(); }
  const T &top() const { return m_stack.front(); }
  bool empty() const { return m_stack.empty(); }

private:
  list<T> m_stack;
};

} // namespace


#endif
