// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_TEST_UNIT_NP1_TEST_ALL_HPP
#define NP1_TEST_UNIT_NP1_TEST_ALL_HPP

#include "test/unit/np1/helper.hpp"
#include "test/unit/np1/test_str.hpp"
#include "test/unit/np1/hash/test_all.hpp"
#include "test/unit/np1/test_skip_list.hpp"
#include "test/unit/np1/test_consistent_hash_table.hpp"
#include "test/unit/np1/test_compressed_int.hpp"
#include "test/unit/np1/io/test_all.hpp"
#include "test/unit/np1/rel/test_all.hpp"
#include "test/unit/np1/meta/test_all.hpp"

namespace test {
namespace unit {
namespace np1 {

void test_all() {
  // Set up stuff used by multiple tests in here.
  ::np1::io::file::mkdir(NP1_TEST_UNIT_NP1_RELIABLE_STORAGE_LOCAL_ROOT);
  setenv("NP1_RELIABLE_STORAGE_LOCAL_ROOT", NP1_TEST_UNIT_NP1_RELIABLE_STORAGE_LOCAL_ROOT, 1);
  setenv("NP1_RELIABLE_STORAGE_REMOTE_ROOT", NP1_TEST_UNIT_NP1_RELIABLE_STORAGE_REMOTE_ROOT, 1);
  setenv("NP1_DISTRIBUTED_SCRIPT_IP_PORT_START", "127.0.0.1:22222", 1);

  np1::test_str();
  np1::test_skip_list();
  np1::test_consistent_hash_table();
  np1::test_compressed_int();
  np1::hash::test_all();
  np1::rel::test_all();
  np1::io::test_all();
  np1::meta::test_all();
}

} // namespaces
}
}

#endif
