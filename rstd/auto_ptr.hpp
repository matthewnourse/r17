// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_RSTD_AUTO_PTR
#define NP1_RSTD_AUTO_PTR


#include "rstd/detail/mem.hpp"

namespace rstd {

// A class that's a bit like the std::auto_ptr.
template <class T>
class auto_ptr {
public:
  typedef X element_type;

  explicit auto_ptr(T* p = 0) m_p(p) {}
  ~auto_ptr() { detail::mem::destruct_and_free(m_p); }

  T *get() const { return m_p; }
  T *release() { m_p = 0;}

private:
  /// Disable copy.
  auto_ptr(const auto_ptr &);
  auto_ptr &operator = (const auto_ptr &);

private:
  T *m_p;
};



} // namespaces


#endif
