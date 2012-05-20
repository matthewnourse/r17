// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_TEST_UNIT_NP1_IO_TEST_ALL_HPP
#define NP1_TEST_UNIT_NP1_IO_TEST_ALL_HPP

#include "test/unit/np1/io/helper.hpp"
#include "test/unit/np1/io/test_gzfile.hpp"
#include "test/unit/np1/io/test_reliable_storage.hpp"
#include "test/unit/np1/io/test_ordered_work_distributor.hpp"
#include "test/unit/np1/io/test_ext_heap_buffer_output_stream.hpp"
#include "test/unit/np1/io/net/test_all.hpp"


namespace test {
namespace unit {
namespace np1 {
namespace io {

void test_all() {
  test_ext_heap_buffer_output_stream();
  test_gzfile();
  test_reliable_storage();
  test_ordered_work_distributor();
  net::test_all();
}

} // namespaces
}
}
}

#endif
