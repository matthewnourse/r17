// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_TEST_UNIT_NP1_IO_NET_TEST_UDP_MESSENGER_HPP
#define NP1_TEST_UNIT_NP1_IO_NET_TEST_UDP_MESSENGER_HPP


#include "np1/io/net/udp_messenger.hpp"


namespace test {
namespace unit {
namespace np1 {
namespace io {
namespace net {


typedef ::np1::io::net::udp_messenger udp_messenger_type;



void run_worker_udp_messenger(const std::vector<std::string> &client_peer_strings,
                              const std::vector<std::string> &worker_peer_strings,
                              const std::string &endpoint_info) {
  udp_messenger_type messenger(client_peer_strings, worker_peer_strings,
                                endpoint_info, false);

  udp_messenger_type::message request_message;
  while (true) {
    if (messenger.receive(request_message)) {
      NP1_TEST_ASSERT(request_message.has_valid_peer());

      // DON'T assume that message payload is null-terminated in production code.
      std::string request_payload = (const char *)request_message.payload_ptr();
      if (request_payload == "exit") {
        return;
      }

      udp_messenger_type::message response_message(
        request_message, udp_messenger_type::message::ack_marker());

      std::string response_payload = "x" + request_payload;
      response_message.append(response_payload.c_str(), response_payload.length() + 1);
      messenger.send(response_message);
    }
  }
}



void test_udp_messenger_send_receive() {
  std::vector<std::string> client_peer_strings;
  std::vector<std::string> worker_peer_strings;

  client_peer_strings.push_back("127.0.0.1:22222");  // The requestor.

  worker_peer_strings.push_back("127.0.0.1:22223");  // Workers
  worker_peer_strings.push_back("127.0.0.1:22224");  
  worker_peer_strings.push_back("127.0.0.1:22225");  
  worker_peer_strings.push_back("127.0.0.1:22226");  
  worker_peer_strings.push_back("127.0.0.1:22227");  
  worker_peer_strings.push_back("127.0.0.1:22228");  
  worker_peer_strings.push_back("127.0.0.1:22229"); 

  static const size_t NUMBER_WORKERS = 7;
  static const size_t NUMBER_MESSAGES = 1000;

  size_t i;
  for (i = 0; i < NUMBER_WORKERS; ++i) {
    pid_t pid = ::np1::process::mandatory_fork();
    switch (pid) {
    case 0:
      // Child.
      {
        run_worker_udp_messenger(client_peer_strings, worker_peer_strings,
                                  worker_peer_strings[i]);
        exit(0);
      }
      break;

    default:
      // Parent- just go around again.
      break;
    }        
  }

  // We can only get to here if we are the parent.
  udp_messenger_type messenger(client_peer_strings, worker_peer_strings,
                                client_peer_strings[0], false);
  
  // Send all the messages and receive any responses we get while sending.
  // We'll just pretend there is one resource per message because in a real query
  // that's (very) approximately what will happen anyway.
  ::np1::skip_list<std::string> worker_responses;
  size_t unique_responses_received = 0;

  udp_messenger_type::message incoming;

  for (i = 0; i < NUMBER_MESSAGES; ++i) {
    std::string outgoing_payload = ::np1::str::to_dec_str(i);
    std::string resource_id = outgoing_payload;
    ::np1::str::ref resource_id_ref = ::np1::str::ref(resource_id);
    udp_messenger_type::message outgoing(resource_id_ref);
    outgoing.append(outgoing_payload.c_str(), outgoing_payload.length() + 1);
    
    if (messenger.receive(incoming, 0)) {
      // DON'T assume that the message is null-terminated in production code!
      if (worker_responses.insert(std::string((const char *)incoming.payload_ptr()))) {
        ++unique_responses_received;
      }      
    }

    messenger.send(outgoing);
  }

  // Receive all the leftover responses.
  while (unique_responses_received < NUMBER_MESSAGES) {
    if (messenger.receive(incoming)) {
      // DON'T assume that the message is null-terminated in production code!
      if (worker_responses.insert(std::string((const char *)incoming.payload_ptr()))) {
        ++unique_responses_received;
      }
    }
  }

  printf("  number_sinbins: %lu  number_retries: %lu\n",
          (unsigned long)messenger.number_sinbins(),
          (unsigned long)messenger.number_retries());

  printf("  Telling workers to shut down\n");  

  // Tell all the workers to shut down.
  for (i = 0; i < NUMBER_WORKERS; ++i) {
    std::string outgoing_payload = "exit";
    ::np1::io::net::ip_endpoint worker_ep(worker_peer_strings[i]);    
    udp_messenger_type::message outgoing(worker_ep);
    outgoing.append(outgoing_payload.c_str(), outgoing_payload.length() + 1);
    messenger.send(outgoing);
  }

  printf("  Waiting for workers to shut down\n");  


  // Wait for all the workers to shut down.
  int status;
  do {
    udp_messenger_type::message ignored;
    messenger.receive(ignored);
  } while (::wait(&status) != -1);
  
  // Now check that all the response messages are present and accounted for.
  for (i = 0; i < NUMBER_MESSAGES; ++i) {
    NP1_TEST_ASSERT(
      worker_responses.find("x" + ::np1::str::to_dec_str(i)) != worker_responses.end());
  }  
}



void test_udp_messenger() {
  NP1_TEST_RUN_TEST(test_udp_messenger_send_receive);
}

} // namespaces
}
}
}
}

#endif
