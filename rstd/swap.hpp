// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_RSTD_SWAP
#define NP1_RSTD_SWAP


namespace rstd {

namespace detail {

//TODO: something faster than this!
template <typename T>
void pod_swap(T &one, T &two) {
  T tmp = one;
  one = two;
  two = tmp;
}

} // namespace detail


//TODO: more POD overloads.
template <typename T>
void swap(T *&one, T *&two) { detail::pod_swap(one, two); }

void swap(int &one, int &two) { detail::pod_swap(one, two); }
void swap(unsigned int &one, unsigned int &two) { detail::pod_swap(one, two); }
void swap(long &one, long &two) { detail::pod_swap(one, two); }
void swap(unsigned long &one, unsigned long &two) { detail::pod_swap(one, two); }
void swap(long long &one, long long &two) { detail::pod_swap(one, two); }
void swap(unsigned long long &one, unsigned long long &two) { detail::pod_swap(one, two); }

template <typename T>
void swap(T &one, T &two) {
  one.swap(two);
}



} // namespaces


#endif
