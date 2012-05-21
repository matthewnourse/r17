// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_IO_NET_IP_ENDPOINT_HPP
#define NP1_IO_NET_IP_ENDPOINT_HPP

#include "np1/hash/fnv1a64.hpp"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


namespace np1 {
namespace io {
namespace net {

//TODO: make the string handling in here safer!
class ip_endpoint {
public:
  enum { MAX_PORT = 65534 };

public:
  ip_endpoint() {
    memset(&m_addr, 0, sizeof(m_addr));    
  }

  /// Construct from a string in the format address:port, crash on error.
  explicit ip_endpoint(const rstd::string &str) { initialize(str.c_str()); }

  /// Construct from a string and a port, crash on error.
  ip_endpoint(const rstd::string &str, int port) {
    initialize(str.c_str(), port);
  }

  /// Construct from an IP number and a port.
  ip_endpoint(uint64_t ip_number, int port) {
    initialize(ip_number, port);
  }

  const struct sockaddr_in *ptr() const { return &m_addr; }
  struct sockaddr_in *ptr() { return &m_addr; }
  size_t size() const { return sizeof(m_addr); }
  
  int port() const { return ntohs(m_addr.sin_port); }
  void port(int port) {
    NP1_ASSERT((port > 0) && (port < MAX_PORT), "Invalid port: " + str::to_dec_str(port));
    m_addr.sin_port = htons(port);
  }

  uint64_t ip_number() const { return m_addr.sin_addr.s_addr; }

  uint64_t hash_add(uint64_t hval) const {
    // Hash the structure as-is so that the hash is the same on machines with
    // different endian-ness.
    hval = hash::fnv1a64::add(&m_addr.sin_addr.s_addr, sizeof(m_addr.sin_addr.s_addr), hval);
    hval = hash::fnv1a64::add(&m_addr.sin_port, sizeof(m_addr.sin_port), hval);
    return hval;
  }

  // Convert to a string, crash on error.
  rstd::string to_string() const {
    char temp[256];
    memset(temp, 0, sizeof(temp));

#ifdef _WIN32
    /* There's no inet_ntop in WinXP & earlier. TODO: move to Windows vista/7 or 
       define own inet_ntop.  The Windows documentation says that the statically
       allocated buffer is allocated per-thread. */
    const char *addr_string = inet_ntoa(m_addr.sin_addr);
    NP1_ASSERT((strlen(addr_string) < sizeof(temp)-1),
                "Unable to convert IP endpoint to string, resulting string is too long");
    strcpy(temp, addr_string);
#else
    NP1_ASSERT(inet_ntop(AF_INET, &m_addr.sin_addr, temp, sizeof(temp)-1),
                "Unable to convert IP endpoint to string");
#endif
    char *p = temp + strlen(temp);
    NP1_ASSERT((p + 7 < temp + sizeof(temp)),
                "Unable to add port to end of IP endpoint string");

    *p++ = ':';
    str::to_dec_str(p, port());
    return temp;
  }

  bool is_valid() const {
    return ((m_addr.sin_addr.s_addr != 0) && (m_addr.sin_port != 0));
  }

  bool operator == (const ip_endpoint &other) const {
    return ((m_addr.sin_addr.s_addr == other.m_addr.sin_addr.s_addr)
            && (m_addr.sin_port == other.m_addr.sin_port));
  }

  bool operator != (const ip_endpoint &other) const {
    return !(operator == (other));  
  }

  bool operator < (const ip_endpoint &other) const {
    if (ip_number() < other.ip_number()) {
      return true;    
    }

    if (ip_number() > other.ip_number()) {
      return false;    
    }
    
    return (port() < other.port());
  }


private:
  void initialize(const char *str) {
    const char *colon_p = strchr(str, ':');
    NP1_ASSERT(colon_p, "Malformed IP endpoint string: " + rstd::string(str));
    int64_t i64 = str::dec_to_int64(colon_p + 1);
    NP1_ASSERT((i64 > 0) && (i64 < MAX_PORT), "Malformed IP endpoint string: " + rstd::string(str));
    *((char *)colon_p) = '\0';
    initialize(str, (int)i64);
    *((char *)colon_p) = ':';
  }

  void initialize(const char *str, int port) {
    memset(&m_addr, 0, sizeof(m_addr));
    unsigned long ip_addr_num = inet_addr(str);
    NP1_ASSERT(INADDR_NONE != ip_addr_num,
                "Unable to convert IP address string '" + rstd::string(str) + "' to IP address");
  
    initialize(ip_addr_num, port);
  }

  void initialize(uint64_t ip_number, int prt) {
    //TODO: ip number validation.

    m_addr.sin_family = AF_INET;
    m_addr.sin_addr.s_addr = ip_number;
    port(prt);
  }

private:
  //TODO: IPv6 :D.
  struct sockaddr_in m_addr;
};


} // namespaces.
}
}



#endif
