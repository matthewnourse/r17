// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_TEST_PATH_HPP
#define NP1_TEST_PATH_HPP



namespace test {
namespace unit {
namespace np1 {
namespace io {


void test_path_empty() {
  ::np1::io::path path(rstd::string(""));
  NP1_TEST_ASSERT(path.directories().size() == 0);
}

void test_path_empty_component() {
  ::np1::io::path path(rstd::string("fred: : "));
  NP1_TEST_ASSERT(path.directories().size() == 1);
  NP1_TEST_ASSERT(::np1::str::cmp(path.directories()[0], "fred") == 0);
}

void test_path_normal() {
  ::np1::io::path path(rstd::string("fred:jane"));
  NP1_TEST_ASSERT(path.directories().size() == 2);
  NP1_TEST_ASSERT(::np1::str::cmp(path.directories()[0], "fred") == 0);
  NP1_TEST_ASSERT(::np1::str::cmp(path.directories()[1], "jane") == 0);
}

void test_path() {
  NP1_TEST_RUN_TEST(test_path_empty);
  NP1_TEST_RUN_TEST(test_path_empty_component);
  NP1_TEST_RUN_TEST(test_path_normal);
}

} // namespaces
}
}
}

#endif
