// Copyright 2021 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_RSTD_MOVE
#define NP1_RSTD_MOVE


namespace rstd {

namespace detail {

//TODO: something faster than this!
template <typename T>
void pod_move(T &dest, T &src) {
  dest = src;
}

} // namespace detail


//TODO: more POD overloads.
template <typename T>
void move(T *&one, T *&two) { detail::pod_move(one, two); }

void move(int &one, int &two) { detail::pod_move(one, two); }
void move(unsigned int &one, unsigned int &two) { detail::pod_move(one, two); }
void move(long &one, long &two) { detail::pod_move(one, two); }
void move(unsigned long &one, unsigned long &two) { detail::pod_move(one, two); }
void move(long long &one, long long &two) { detail::pod_move(one, two); }
void move(unsigned long long &one, unsigned long long &two) { detail::pod_move(one, two); }

template <typename T>
void move(T &dest, T &src) {
  dest.move_from(src);
}



} // namespaces


#endif
