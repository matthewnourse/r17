// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_RSTD_STRING
#define NP1_RSTD_STRING


#include "rstd/vector.hpp"

namespace rstd {

/// A basic replacement for std::string.
class string {
public:
  static const size_t npos = (size_t)-1;

public:
  string() {}
  string(const char *s) : m_vector(s, s ? strlen(s)+1 : 0) {}
  string(const char *s, size_t len) : m_vector(s, len) { m_vector.push_back('\0'); }
  string(const string &s, size_t len) : m_vector(s.c_str(), len) { m_vector.push_back('\0'); }
  string(size_t n, char c) {
    //TODO: something faster than this.
    size_t i;
    for (i = 0; i < n; ++i) {
      push_back(c);
    }
  }
  string(const string &s) : m_vector(s.m_vector) {}
  ~string() {}

  void push_back(char c) {
    if (m_vector.size() == 0) {
      m_vector.push_back(c);
    } else {
      m_vector[m_vector.size()-1] = c;
    }

    m_vector.push_back('\0');
  }

  void append(const char *s) {
    append(s, strlen(s));
  }

  void append(const char *s, size_t sz) {    
    if (m_vector.size() > 0) {
      m_vector.pop_back();
    }

    m_vector.append(s, sz);
    m_vector.push_back('\0');
  }

  void append(const string &s) {
    append(s.c_str(), s.length());  
  }

  string substr(size_t pos = 0, size_t n = npos) const {
    size_t len = length();
    if ((pos >= len) || (n > len - pos)) {
      return string();
    }

    return string(c_str() + pos, n);
  }

  void clear() { m_vector.clear(); }

  const char *c_str() const { return m_vector.begin(); }
  size_t length() const { return m_vector.size() ? m_vector.size()-1 : 0; }
  bool empty() const { return m_vector.empty() || (m_vector.size() == 1); }

  void swap(string &other) {
    m_vector.swap(other.m_vector);  
  }

  string &operator = (const string &other) {
    string s(other);
    swap(s);
    return *this;
  }

  string operator + (const string &other) const {
    string s(*this);
    s.append(other);
    return s;
  }

  char operator[] (size_t offset) const { return m_vector[offset]; }

private:
  vector<char> m_vector;
};


namespace detail {
  int compare(const char *s1, const char *s2) {
    return strcmp(s1, s2);
  }
} // namespace detail

string operator + (const char *s1, const string &s2) {
  //TODO: something faster than this.
  return string(s1) + s2;
}

bool operator == (const char *s1, const string &s2) {
  return (detail::compare(s1, s2.c_str()) == 0);
}

bool operator == (const string &s1, const string &s2) {
  return (detail::compare(s1.c_str(), s2.c_str()) == 0);
}

bool operator == (const string &s1, const char *s2) {
  return (detail::compare(s1.c_str(), s2) == 0);
}

bool operator < (const string &s1, const string &s2) {
  return (detail::compare(s1.c_str(), s2.c_str()) < 0);
}

} // namespaces 


#endif
