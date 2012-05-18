// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_TEST_UNIT_NP1_IO_TEST_RELIABLE_STORAGE_HPP
#define NP1_TEST_UNIT_NP1_IO_TEST_RELIABLE_STORAGE_HPP


#include "test/unit/np1/io/helper.hpp"

#define NP1_TEST_UNIT_NP1_IO_TEST_RELIABLE_STORAGE_DATA "This is a very exciting test"


namespace test {
namespace unit {
namespace np1 {
namespace io {

void test_reliable_storage_id() {
  reliable_storage_type::id empty_id;
  NP1_TEST_ASSERT(!empty_id.is_valid());

  reliable_storage_type::id client_peer_list_id1 = reliable_storage_type::id::client_peer_list_id();
  reliable_storage_type::id client_peer_list_id2 = reliable_storage_type::id::client_peer_list_id();

  reliable_storage_type::id worker_peer_list_id1 = reliable_storage_type::id::worker_peer_list_id();
  reliable_storage_type::id worker_peer_list_id2 = reliable_storage_type::id::worker_peer_list_id();

  NP1_TEST_ASSERT(client_peer_list_id1 != empty_id);
  NP1_TEST_ASSERT(client_peer_list_id2 == client_peer_list_id1);

  NP1_TEST_ASSERT(worker_peer_list_id1 != empty_id);
  NP1_TEST_ASSERT(worker_peer_list_id2 != client_peer_list_id2);
  NP1_TEST_ASSERT(worker_peer_list_id1 == worker_peer_list_id2);

  reliable_storage_type::id generated_id = reliable_storage_type::id::generate();
  
  NP1_TEST_ASSERT(generated_id != empty_id);
  NP1_TEST_ASSERT(client_peer_list_id1 != generated_id);
  NP1_TEST_ASSERT(worker_peer_list_id1 != generated_id);
  NP1_TEST_ASSERT(generated_id == generated_id);

  reliable_storage_type::id temp_generated_id = reliable_storage_type::id::generate_temp();
  NP1_TEST_ASSERT(temp_generated_id.is_temp());
  NP1_TEST_ASSERT(temp_generated_id == temp_generated_id);
  NP1_TEST_ASSERT(temp_generated_id != generated_id);
  NP1_TEST_ASSERT(client_peer_list_id1 != temp_generated_id);
  NP1_TEST_ASSERT(worker_peer_list_id1 != temp_generated_id);
}


void test_reliable_storage_lifecycle() {
  reliable_storage_type rs(NP1_TEST_UNIT_NP1_RELIABLE_STORAGE_LOCAL_ROOT,
                            NP1_TEST_UNIT_NP1_RELIABLE_STORAGE_REMOTE_ROOT);
  reliable_storage_type::id id = reliable_storage_type::id::generate();
  reliable_storage_type::stream rs_stream(rs);

  NP1_TEST_ASSERT(!rs_stream.is_open());

  // Create & write.
  NP1_TEST_ASSERT(rs.create_wo(id, rs_stream));
  NP1_TEST_ASSERT(rs_stream.is_open());
  NP1_TEST_ASSERT(rs_stream.write(NP1_TEST_UNIT_NP1_IO_TEST_RELIABLE_STORAGE_DATA));
  NP1_TEST_ASSERT(rs_stream.close());
  NP1_TEST_ASSERT(!rs_stream.is_open());

  // Check that a second attempt to create the file actually fails
  // TODO: on S3 this might succeed, should we even test for this?
  NP1_TEST_ASSERT(!rs.create_wo(id, rs_stream));
  NP1_TEST_ASSERT(!rs_stream.is_open());
  

  // Open & read.
  NP1_TEST_ASSERT(rs.open_ro(id, NP1_TEST_UNIT_NP1_RELIABLE_STORAGE_TIMEOUT, rs_stream));
  NP1_TEST_ASSERT(rs_stream.is_open());
  char buffer[8192];
  size_t bytes_read = 0;
  NP1_TEST_ASSERT(rs_stream.read(buffer, sizeof(buffer), &bytes_read));
  NP1_TEST_ASSERT(bytes_read == strlen(NP1_TEST_UNIT_NP1_IO_TEST_RELIABLE_STORAGE_DATA));
  NP1_TEST_ASSERT(memcmp(buffer, NP1_TEST_UNIT_NP1_IO_TEST_RELIABLE_STORAGE_DATA, bytes_read) == 0);
  NP1_TEST_ASSERT(rs_stream.close());
  NP1_TEST_ASSERT(!rs_stream.is_open());

  // Erase.
  NP1_TEST_ASSERT(rs.erase(id));

  // Check that it's really been erased.  This will also test the sleeping
  // code in open_ro.
  NP1_TEST_ASSERT(!rs.open_ro(id, 1, rs_stream));
  NP1_TEST_ASSERT(!rs_stream.is_open());
}


void test_reliable_storage() {
  NP1_TEST_RUN_TEST(test_reliable_storage_id);
  NP1_TEST_RUN_TEST(test_reliable_storage_lifecycle);
}

} // namespaces
}
}
}

#endif
