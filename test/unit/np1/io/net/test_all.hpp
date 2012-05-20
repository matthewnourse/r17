// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_TEST_UNIT_NP1_IO_NET_TEST_ALL_HPP
#define NP1_TEST_UNIT_NP1_IO_NET_TEST_ALL_HPP


#include "test/unit/np1/io/net/test_ip_endpoint.hpp"
#include "test/unit/np1/io/net/test_udp_messenger.hpp"

namespace test {
namespace unit {
namespace np1 {
namespace io {
namespace net {


void test_all() {
  test_ip_endpoint();
  test_udp_messenger();
}

} // namespaces
}
}
}
}

#endif
