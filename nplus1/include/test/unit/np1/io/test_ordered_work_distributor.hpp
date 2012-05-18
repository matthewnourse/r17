// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_TEST_UNIT_NP1_IO_TEST_WORK_DISTRIBUTOR_HPP
#define NP1_TEST_UNIT_NP1_IO_TEST_WORK_DISTRIBUTOR_HPP


#include "test/unit/np1/io/helper.hpp"


namespace test {
namespace unit {
namespace np1 {
namespace io {


struct distributed_worker_processor_fn {
  reliable_storage_type::id operator()(const reliable_storage_type::id &resource_id,
                                        const ::np1::str::ref &payload) {
    // We can't possibly be asked to operate on the client peer list, so
    // use that as the signal to exit.
    if (resource_id == reliable_storage_type::id::client_peer_list_id()) {
      printf("  worker: received resource id of the client peer list- the signal to exit\n");
      exit(0);
    }

    reliable_storage_type::id new_resource_id = reliable_storage_type::id::generate();
    create_reliable_storage_file(
        new_resource_id,
        read_reliable_storage_file(resource_id) + std::string(payload.ptr(), payload.length()));

    return new_resource_id;
  }
};


struct run_distributed_worker {
  void operator()(const std::string &listen_endpoint) {
    work_distributor_type distributor(NP1_TEST_UNIT_NP1_RELIABLE_STORAGE_LOCAL_ROOT,
                                      NP1_TEST_UNIT_NP1_RELIABLE_STORAGE_REMOTE_ROOT, listen_endpoint, false);
    reliable_storage_type rs(NP1_TEST_UNIT_NP1_RELIABLE_STORAGE_LOCAL_ROOT,
                              NP1_TEST_UNIT_NP1_RELIABLE_STORAGE_REMOTE_ROOT);
    distributed_worker_processor_fn fn;
      
    distributor.process_requests(fn);
  }
};






void test_ordered_work_distributor_send_receive() {
  std::string client_peer_endpoint = write_client_peer_strings_list();  
  std::vector<std::string> worker_peer_strings = write_worker_peer_strings_list();  

  std::vector<pid_t> children = fork_distributed_workers(worker_peer_strings,
                                                          run_distributed_worker());

  // We can only get to here if we are the parent, which acts as the client.
  ordered_work_distributor_type distributor(NP1_TEST_UNIT_NP1_RELIABLE_STORAGE_LOCAL_ROOT,
                                            NP1_TEST_UNIT_NP1_RELIABLE_STORAGE_REMOTE_ROOT, 
                                            client_peer_endpoint);
  
  
  static const size_t NUMBER_WORK_ITEMS = 500;

  // Send out all the requests.
  size_t i;
  for (i = 0; i < NUMBER_WORK_ITEMS; ++i) {
    reliable_storage_type::id req_resource_id(reliable_storage_type::id::generate());
    create_reliable_storage_file(req_resource_id, ::np1::str::to_dec_str(i));
    distributor.send_request(req_resource_id, ::np1::str::ref(" " + ::np1::str::to_dec_str(i)));    
  }

  // Receive all the responses.  They should be returned to us in order.
  reliable_storage_type::id resp_resource_id;
  for (i = 0; (i < NUMBER_WORK_ITEMS) && distributor.receive_response(resp_resource_id); ++i) {
    std::string resp_data = read_reliable_storage_file(resp_resource_id);
    NP1_TEST_ASSERT(resp_data == (::np1::str::to_dec_str(i) + " " + ::np1::str::to_dec_str(i)));
  }

  printf("  number_sinbins: %lu  number_retries: %lu\n",
        (unsigned long)distributor.number_sinbins(),
        (unsigned long)distributor.number_retries());

  kill_distributed_workers(children);

  NP1_TEST_ASSERT(i == NUMBER_WORK_ITEMS);
  NP1_TEST_ASSERT(!distributor.receive_response(resp_resource_id));

}



void test_ordered_work_distributor() {
  NP1_TEST_RUN_TEST(test_ordered_work_distributor_send_receive);
}

} // namespaces
}
}
}

#endif
