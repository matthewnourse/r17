// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_IO_NET_UDP_SOCKET_HPP
#define NP1_IO_NET_UDP_SOCKET_HPP


#include "np1/io/net/ip_endpoint.hpp"


namespace np1 {
namespace io {
namespace net {

class udp_socket {
private:
#ifdef _WIN32
  typedef SOCKET handle_type;
#else
  typedef int handle_type;
#endif

#ifdef _WIN32  
  typedef int socket_sockaddr_length_type; 
#else
  typedef socklen_t socket_sockaddr_length_type; 
#endif


public:
  udp_socket() : m_handle(invalid_handle_value()) {}
  ~udp_socket() { close(); }
  

  /// Create a UDP socket, returns false on error.
  bool create() {
    close();

#ifdef _WIN32
    m_handle = socket(AF_INET, SOCK_DGRAM, IPPROTO_TCP);
#else
    m_handle = socket(PF_INET, SOCK_DGRAM, 0);
#endif

    return (invalid_handle_value() != m_handle);
  }

  /// Bind an address to the UDP socket, returns false on error.
  bool bind(const ip_endpoint &endpoint) {
    return (::bind(m_handle, (struct sockaddr *)endpoint.ptr(), endpoint.size()) == 0);
  }

  /// Read a packet into the buffer, returns false if the read failed.
  bool read(void *buf, size_t len, size_t *bytes_read, ip_endpoint &endpoint) {
    socket_sockaddr_length_type remote_addr_len = endpoint.size();
  
    ssize_t num_bytes_received = recvfrom(m_handle, buf, len, 0,
                                          (struct sockaddr *)endpoint.ptr(),
                                          &remote_addr_len);
    if (num_bytes_received < 0) {
      return false;  
    }
  
    if (remote_addr_len != endpoint.size()) {
      //TODO: what to do here?
      return false;
    }
  
    *bytes_read = num_bytes_received;
    return true;
  }

  // Wait for there to be some data in the socket ready for reading.
  bool wait_for_readability(uint32_t timeout_ms) {  
    struct timeval tv;
    tv.tv_sec = timeout_ms/1000;
    tv.tv_usec = (timeout_ms % 1000) * 1000;
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(m_handle, &fds);
    int result = select(m_handle+1, &fds, NULL, NULL, &tv);
    return (result == 1);
  }



  /// Write a packet, returns false if the write failed.
  bool write(const ip_endpoint &endpoint, const void *buf, size_t len) {
    if (sendto(m_handle, buf, len, 0, (const struct sockaddr *)endpoint.ptr(),
                endpoint.size()) != (ssize_t)len) {
      return false;  
    }
  
    return true;
  }


  /* Set the send and receive timeouts on a socket.
     NOTE that this probably does not work on Solaris or HP-UX. 
     NOTE that on Windows you can't wait more than 24 days.
     NOTE that 0 means forever!
     NOTE that setting a short timeout and then calling read() will probably
          wait longer than you expect (on Linux at least).  The better solution
     is to call wait_for_readability() before calling read(). */
  bool set_timeouts(uint32_t send_timeout_ms, uint32_t receive_timeout_ms) {  
#ifdef _WIN32
    uint64_t ms = send_timeout_ms;
    if (ms > INT_MAX) {
      ms = INT_MAX;
    }
    int tv = (int)ms;
    if (setsockopt(m_handle, SOL_SOCKET, SO_SNDTIMEO, (const char *)&tv, sizeof(tv)) != 0) {
      return false;
    }
  
    ms = receive_timeout_ms;
    if (ms > INT_MAX) {
      ms = INT_MAX;
    }
    tv = (int)ms;
    if (setsockopt(m_handle, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof(tv)) != 0) {
      return false;
    }
  
#else  
    struct timeval tv;
    tv.tv_sec = send_timeout_ms/1000;
    tv.tv_usec = (send_timeout_ms % 1000) * 1000;
    if (setsockopt(m_handle, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv)) != 0) {
      return false;
    }
  
    tv.tv_sec = receive_timeout_ms/1000;
    tv.tv_usec = (receive_timeout_ms % 1000) * 1000;
    if (setsockopt(m_handle, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) != 0) {
      return false;
    }
#endif
  
    return true;
  }


  void close() {
    if (m_handle != invalid_handle_value()) {
#ifdef _WIN32
      shutdown(m_handle, SD_BOTH);
      closesocket(m_handle);
#else
      shutdown(m_handle, SHUT_RDWR);
      ::close(m_handle);
#endif
    }

    m_handle = invalid_handle_value();
  }

private:
  /// Disable copy.
  udp_socket(const udp_socket &);
  udp_socket &operator = (const udp_socket &);

private:
  static inline handle_type invalid_handle_value() {
#ifdef _WIN32
    return INVALID_SOCKET;
#else
    return -1;
#endif
}

private:
  handle_type m_handle;
};


} // namespaces.
}
}



#endif
