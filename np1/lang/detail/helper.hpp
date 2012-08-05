// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_LANG_DETAIL_HELPER_HPP
#define NP1_LANG_DETAIL_HELPER_HPP



namespace np1 {
namespace lang {
namespace detail {
namespace helper {


rstd::string code_to_markdown(const rstd::string code) {
  rstd::string md = "    ";

  const char *p = code.c_str();
  for (; *p; ++p) {
    if ('\n' == *p) {
      md.append("  \n    ");
    } else {
      md.push_back(*p);
    }
  }

  md.push_back('\n');

  return md;
}


} /// namespaces
}
}
}


#endif
