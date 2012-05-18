// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_TEST_UNIT_NP1_IO_NET_TEST_IP_ENDPOINT_HPP
#define NP1_TEST_UNIT_NP1_IO_NET_TEST_IP_ENDPOINT_HPP


#include "np1/io/net/ip_endpoint.hpp"


namespace test {
namespace unit {
namespace np1 {
namespace io {
namespace net {


typedef ::np1::io::net::ip_endpoint ip_endpoint_type;


void test_ip_endpoint_to_and_from_string() {
  ip_endpoint_type ep1("127.0.0.1:80");
  NP1_TEST_ASSERT(ep1.port() == 80);
  NP1_TEST_ASSERT(ep1.to_string() == "127.0.0.1:80");
  
  ip_endpoint_type ep2("192.168.1.1", 8888);
  NP1_TEST_ASSERT(ep2.port() == 8888);
  NP1_TEST_ASSERT(ep2.to_string() == "192.168.1.1:8888");  
}


void test_ip_endpoint_equality() {
  ip_endpoint_type ep1("127.0.0.1:80");
  ip_endpoint_type ep2("192.168.1.1", 8888);

  NP1_TEST_ASSERT(ep1 != ep2);  

  ip_endpoint_type ep3("127.0.0.1:81");
  NP1_TEST_ASSERT(ep1 != ep3);

  ip_endpoint_type ep4("127.0.0.2:80");
  NP1_TEST_ASSERT(ep1 != ep4);

  ip_endpoint_type ep5("127.0.0.1:80");
  NP1_TEST_ASSERT(ep1 == ep5);  
}


void test_ip_endpoint_less_than() {
  ip_endpoint_type ep1("127.0.0.1:80");
  ip_endpoint_type ep2("192.168.1.1", 8888);

  NP1_TEST_ASSERT(ep1 < ep2);  

  ip_endpoint_type ep3("127.0.0.1:81");
  NP1_TEST_ASSERT(ep1 < ep3);

  ip_endpoint_type ep4("127.0.0.2:80");
  NP1_TEST_ASSERT(ep1 < ep4);

  ip_endpoint_type ep5("127.0.0.1:80");
  NP1_TEST_ASSERT(!(ep1 < ep5));  
}


void test_ip_endpoint_hash_add() {
  uint64_t hval = 0;
  ip_endpoint_type ep1("127.0.0.1:80");
  hval = ep1.hash_add(hval);

  // No way of checking what the correct result is.
}

void test_ip_endpoint() {
  NP1_TEST_RUN_TEST(test_ip_endpoint_to_and_from_string);
  NP1_TEST_RUN_TEST(test_ip_endpoint_equality);
  NP1_TEST_RUN_TEST(test_ip_endpoint_less_than);
  NP1_TEST_RUN_TEST(test_ip_endpoint_hash_add);
}

} // namespaces
}
}
}
}

#endif
