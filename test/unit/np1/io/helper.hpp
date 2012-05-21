// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_TEST_UNIT_NP1_IO_HELPER_HPP
#define NP1_TEST_UNIT_NP1_IO_HELPER_HPP


#include "np1/io/reliable_storage.hpp"
#include "np1/io/work_distributor.hpp"
#include "np1/io/ordered_work_distributor.hpp"


namespace test {
namespace unit {
namespace np1 {
namespace io {


rstd::string make_alphabet_test_data_string(size_t size = 10000000) {
  rstd::string test_data;
  size_t i;
  char CHARS[] = "abcdefghijklmnopqrstuvwxyz";
  

  for (i = 0; i < size; ++i) {
    test_data.push_back(CHARS[i % sizeof(CHARS)]);    
  }

  return test_data;
}

} // namespaces
}
}
}


#endif
