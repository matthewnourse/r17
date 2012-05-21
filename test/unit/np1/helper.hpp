// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_TEST_UNIT_NP1_HELPER_HPP
#define NP1_TEST_UNIT_NP1_HELPER_HPP

#include "np1/io/reliable_storage.hpp"


#define NP1_TEST_UNIT_NP1_RELIABLE_STORAGE_LOCAL_ROOT "/tmp/np1_test_reliable_storage_local_root"
#define NP1_TEST_UNIT_NP1_RELIABLE_STORAGE_REMOTE_ROOT "http://localhost/np1_test_reliable_storage_remote_root"
#define NP1_TEST_UNIT_NP1_RELIABLE_STORAGE_TIMEOUT ((unsigned int)1)

namespace test {
namespace unit {
namespace np1 {

typedef ::np1::io::reliable_storage reliable_storage_type;
typedef ::np1::io::work_distributor work_distributor_type;
typedef ::np1::io::ordered_work_distributor ordered_work_distributor_type;


void create_reliable_storage_file(const reliable_storage_type::id &id,
                                    const rstd::string &data) {
  reliable_storage_type rs(NP1_TEST_UNIT_NP1_RELIABLE_STORAGE_LOCAL_ROOT,
                            NP1_TEST_UNIT_NP1_RELIABLE_STORAGE_REMOTE_ROOT);
  reliable_storage_type::stream rs_stream(rs);
  NP1_TEST_ASSERT(rs.create_wo(id, rs_stream));
  NP1_TEST_ASSERT(rs_stream.write(data.c_str(), data.length()));
  NP1_TEST_ASSERT(rs_stream.close());
}


void create_reliable_storage_file(const reliable_storage_type::id &id,
                                  const rstd::vector<rstd::string> &data) {
  rstd::string file_data;
  
  size_t i;
  for (i = 0; i < data.size(); ++i) {
    file_data.append(data[i]);
    file_data.append("\n");
  }

  create_reliable_storage_file(id, file_data);
}


rstd::string read_reliable_storage_file(const reliable_storage_type::id &id) {
  reliable_storage_type rs(NP1_TEST_UNIT_NP1_RELIABLE_STORAGE_LOCAL_ROOT,
                            NP1_TEST_UNIT_NP1_RELIABLE_STORAGE_REMOTE_ROOT);
  rstd::vector<char> data;
  data.resize(1024 * 1024);
  reliable_storage_type::stream rs_stream(rs);
  NP1_TEST_ASSERT(rs.open_ro(id, NP1_TEST_UNIT_NP1_RELIABLE_STORAGE_TIMEOUT, rs_stream));
  size_t bytes_read = 0;
  NP1_TEST_ASSERT(rs_stream.read(&data[0], data.size(), &bytes_read));
  NP1_TEST_ASSERT(rs_stream.close());

  return rstd::string(&data[0], bytes_read);
}


rstd::string write_client_peer_strings_list() {
  rstd::vector<rstd::string> client_peers;

  client_peers.push_back("127.0.0.1:22222");
  client_peers.push_back("127.0.0.1:22223");
  client_peers.push_back("127.0.0.1:22224");
  client_peers.push_back("127.0.0.1:22225");
  client_peers.push_back("127.0.0.1:22226");
  client_peers.push_back("127.0.0.1:22227");
  client_peers.push_back("127.0.0.1:22228");
  client_peers.push_back("127.0.0.1:22229");
  client_peers.push_back("127.0.0.1:22230");
  client_peers.push_back("127.0.0.1:22231");
  client_peers.push_back("127.0.0.1:22232");
  client_peers.push_back("127.0.0.1:22233");
  client_peers.push_back("127.0.0.1:22234");
  client_peers.push_back("127.0.0.1:22235");
  client_peers.push_back("127.0.0.1:22236");
  client_peers.push_back("127.0.0.1:22237");
  client_peers.push_back("127.0.0.1:22238");
  client_peers.push_back("127.0.0.1:22239");
  client_peers.push_back("127.0.0.1:22240");

  reliable_storage_type rs(NP1_TEST_UNIT_NP1_RELIABLE_STORAGE_LOCAL_ROOT,
                            NP1_TEST_UNIT_NP1_RELIABLE_STORAGE_REMOTE_ROOT);
  rs.erase(reliable_storage_type::id::client_peer_list_id());

  create_reliable_storage_file(
    reliable_storage_type::id::client_peer_list_id(), client_peers);

  return client_peers[0];
}



rstd::vector<rstd::string> write_worker_peer_strings_list() {

  rstd::vector<rstd::string> worker_peer_strings;

  worker_peer_strings.push_back("127.0.0.1:32223");  // Workers
  worker_peer_strings.push_back("127.0.0.1:32224");  
  worker_peer_strings.push_back("127.0.0.1:3225");  
  worker_peer_strings.push_back("127.0.0.1:32226");  
  worker_peer_strings.push_back("127.0.0.1:32227");  
  worker_peer_strings.push_back("127.0.0.1:32228");  
  worker_peer_strings.push_back("127.0.0.1:32229");  
  worker_peer_strings.push_back("127.0.0.1:32230");  
  worker_peer_strings.push_back("127.0.0.1:32231");  
  worker_peer_strings.push_back("127.0.0.1:32232");  
  worker_peer_strings.push_back("127.0.0.1:32233");  
  worker_peer_strings.push_back("127.0.0.1:32234");  
  worker_peer_strings.push_back("127.0.0.1:32235");  
  worker_peer_strings.push_back("127.0.0.1:32236");  
  worker_peer_strings.push_back("127.0.0.1:32237");  
  worker_peer_strings.push_back("127.0.0.1:32238");  
  worker_peer_strings.push_back("127.0.0.1:32239");  
  worker_peer_strings.push_back("127.0.0.1:32240");  
  worker_peer_strings.push_back("127.0.0.1:32241");  
  worker_peer_strings.push_back("127.0.0.1:32242");  
  worker_peer_strings.push_back("127.0.0.1:32243");  
  worker_peer_strings.push_back("127.0.0.1:32244");  
  worker_peer_strings.push_back("127.0.0.1:32245");  
  worker_peer_strings.push_back("127.0.0.1:32246");  
  worker_peer_strings.push_back("127.0.0.1:32247");  
  worker_peer_strings.push_back("127.0.0.1:32248");  
  worker_peer_strings.push_back("127.0.0.1:32249");  

  reliable_storage_type rs(NP1_TEST_UNIT_NP1_RELIABLE_STORAGE_LOCAL_ROOT,
                            NP1_TEST_UNIT_NP1_RELIABLE_STORAGE_REMOTE_ROOT);
  rs.erase(reliable_storage_type::id::worker_peer_list_id());

  create_reliable_storage_file(
    reliable_storage_type::id::worker_peer_list_id(), worker_peer_strings);

  return worker_peer_strings;
}


template <typename Worker_F>
rstd::vector<pid_t> fork_distributed_workers(const rstd::vector<rstd::string> &worker_peer_strings,
                                            Worker_F worker_f) {
  rstd::vector<pid_t> children;

  size_t i;
  for (i = 0; i < worker_peer_strings.size(); ++i) {
    pid_t pid = ::np1::process::mandatory_fork();
    switch (pid) {
    case 0:
      // Child.
      {
        worker_f(worker_peer_strings[i]);
        exit(0);
      }
      break;

    default:
      // Parent- just go around again.
      children.push_back(pid);
      break;
    }        
  }

  return children;
}


void kill_distributed_workers(const rstd::vector<pid_t> &worker_pids) {
  printf("  Waiting for children to exit\n");
  // Kill all the child processes.
  rstd::vector<pid_t>::const_iterator child_i = worker_pids.begin();
  rstd::vector<pid_t>::const_iterator child_iz = worker_pids.end();
  for (; child_i != child_iz; ++child_i) {
    kill(*child_i, SIGTERM);
    int status;
    waitpid(*child_i, &status, 0);
  }
}


void make_test_data_record_string(rstd::string &test_data, size_t number_records) {
  test_data.clear();

  test_data.append("string:mul1_str\tint:mul1_int\tstring:mul7_str\tint:mul7_int\n");

  size_t counter;
  for (counter = 0; counter < number_records; ++counter) {
    char num_str[32];
    test_data.append("name_");
    ::np1::str::to_dec_str(num_str, counter);
    test_data.append(num_str);
    test_data.append("\t");
    test_data.append(::np1::str::to_dec_str(counter));
    test_data.append("\t");
    
    ::np1::str::to_dec_str(num_str, counter % 7);
    test_data.append("name_");
    test_data.append(num_str);
    test_data.append("\t");
    test_data.append(::np1::str::to_dec_str(counter));
    test_data.append("\n");
  }

  printf("  test data length is %zuK\n", test_data.length()/1024);
}


void make_large_test_data_record_string(rstd::string &test_data) {
  make_test_data_record_string(test_data, 1000000);  
}

void make_very_large_test_data_record_string(rstd::string &test_data) {
  make_test_data_record_string(test_data, 10000000);  
}


} // namespaces
}
}

#endif
