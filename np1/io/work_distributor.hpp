// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_IO_WORK_DISTRIBUTOR_HPP
#define NP1_IO_WORK_DISTRIBUTOR_HPP


#include "np1/io/net/udp_messenger.hpp"
#include "np1/io/reliable_storage.hpp"
#include "rstd/string.hpp"


namespace np1 {
namespace io {


// Distribute work items to workers.  Workers also call this class to receive
// work item requests.
class work_distributor {
public:
  enum { RELIABLE_STORAGE_TIMEOUT_SECONDS = 30 };
  enum { PEER_LIST_TIME_CHECK_TIMEOUT_USEC = 60 * 1000 * 1000 }; // 1 minute.
  
  typedef net::udp_messenger::message_id_type request_id_type;

  class work_item_request {
  public:
    work_item_request() {}

    // For the use of work_distributor.
    work_item_request(const reliable_storage::id &resource_id,
                      const str::ref &request_payload)
      : m_message(str::ref(resource_id.ptr(), reliable_storage::id::LENGTH)) {
      m_message.append(resource_id.ptr(), reliable_storage::id::LENGTH+1);
      m_message.append(request_payload.ptr(), request_payload.length());
    }

    // For the use of work_distributor.
    net::udp_messenger::message &message() { return m_message; }
    const net::udp_messenger::message &message() const { return m_message; }

    //TODO: this needs to be more rigorous.
    bool is_valid() const {
      const unsigned char *p = m_message.payload_ptr();
      return ((m_message.payload_size() > reliable_storage::id::LENGTH)
              && ('\0' == p[reliable_storage::id::LENGTH])
              && (memchr(p, '\0', reliable_storage::id::LENGTH) == 0)
              && (memchr(&p[reliable_storage::id::LENGTH+1], '\0',
                          m_message.payload_size() - reliable_storage::id::LENGTH - 1) == 0));
    }

    str::ref payload() const {
      const char *p = (const char *)m_message.payload_ptr();
      return str::ref(&p[reliable_storage::id::LENGTH+1],
                      m_message.payload_size() - reliable_storage::id::LENGTH - 1);
    }

    reliable_storage::id resource_id() const {
      return reliable_storage::id((const char *)m_message.payload_ptr());
    }

  private:
    net::udp_messenger::message m_message;
  };


  class work_item_response {
  public:
    work_item_response() {}

    typedef enum {
      FLAG_ERROR = 1
    } flags_type_type;

    // For the use of work_distributor.
    work_item_response(const work_item_request &req,
                        const reliable_storage::id &resource_id)
      : m_message(req.message(), net::udp_messenger::message::ack_marker()) {
      char flags = 0;
      m_message.append(&flags, 1);
      m_message.append(resource_id.ptr(), reliable_storage::id::LENGTH);
      m_message.append("\0", 1);
    }

    work_item_response(const work_item_request &req,
                        const char *error_description)
      : m_message(req.message(), net::udp_messenger::message::ack_marker()) {
      size_t error_description_length = strlen(error_description);
      char flags = FLAG_ERROR;
      m_message.append(&flags, 1);
      if (m_message.remaining_available_payload_size() < error_description_length + 1) {
       // This check should always return true. 
       if (m_message.remaining_available_payload_size() > 0) {
          m_message.append(error_description, m_message.remaining_available_payload_size() - 1);
        }
      } else {
        m_message.append(error_description, error_description_length);
      }

      m_message.append("\0", 1);
    }

    // For the use of work_distributor.
    net::udp_messenger::message &message() { return m_message; }
    const net::udp_messenger::message &message() const { return m_message; }

    //TODO: this needs to be more rigorous.
    bool is_valid() const {
      const unsigned char *p = m_message.payload_ptr();
      size_t remaining_payload_size = m_message.payload_size();      
      char flag = *p;
      ++p;
      --remaining_payload_size;
      
      const unsigned char *final_null_p =
        (const unsigned char *)memchr(p, '\0', remaining_payload_size);

      if (final_null_p+1 != m_message.payload_ptr() + m_message.payload_size()) {
        return false;
      }
  
      if (FLAG_ERROR == flag) {
        // No more checks to do.
        return true;
      }

      return (final_null_p - p == reliable_storage::id::LENGTH);
    }
    
    bool is_error() const {
      return (FLAG_ERROR == *m_message.payload_ptr());
    }

    // Only valid if is_error() == true.
    const char *error_message() const {
      return (const char *)m_message.payload_ptr() + 1;    
    }

    // Only valid if is_error() == false;
    reliable_storage::id resource_id() const {
      return reliable_storage::id((const char *)m_message.payload_ptr() + 1);
    }

    request_id_type request_id() const { return m_message.message_id(); }
  
  private:
    net::udp_messenger::message m_message;  
  };

public:
  // Construct and start listening on the endpoint.
  work_distributor(const rstd::string &reliable_storage_local_root,
                    const rstd::string &reliable_storage_remote_root,
                    const rstd::string &listen_endpoint,
                    bool ok_to_search_for_available_endpoint) 
    : m_storage(reliable_storage_local_root, reliable_storage_remote_root),
      m_messenger(read_client_peer_list(m_storage), read_worker_peer_list(m_storage), listen_endpoint,
                  ok_to_search_for_available_endpoint),
      m_last_checked_peer_list_time(time::now_epoch_usec()),
      m_last_worker_peer_list_mtime(0),
      m_last_client_peer_list_mtime(0) {}

  /****************************************************************************
   * Methods for clients.
   ***************************************************************************/ 
  // Request a work item.  Crashes on fatal error.  Returns a unique request id
  // that the caller can use to associate responses with requests.
  request_id_type send_request(const reliable_storage::id &resource_id,
                               const str::ref &request_payload) {
    update_peer_list_if_necessary();
    work_item_request req(resource_id, request_payload);
    NP1_ASSERT(req.is_valid(), "Outgoing work_item_request is invalid!");
    m_messenger.send(req.message());
    return req.message().message_id();
  }

  // Receive a response for a work item.  Returns false if there is no available
  // response.  Callers can associate a response with a request using
  // resp.request_id().
  bool receive_response(work_item_response &resp,
                        uint32_t read_timeout_ms = net::udp_messenger::SOCKET_READ_TIMEOUT_MILLISECONDS) {
    update_peer_list_if_necessary();

    bool result;

    //TODO: messages that fail vailidation should be returned as error responses.
    while ((result = m_messenger.receive(resp.message(), read_timeout_ms))
            && !resp.is_valid()) {
      log_info("receive_response: incoming response failed validation.");
    }

    return result;
  }

  size_t number_messages_awaiting_ack() const {
    return m_messenger.number_messages_awaiting_ack();
  }

  size_t number_worker_peers() const {
    return m_messenger.number_worker_peers();
  }
  


  /****************************************************************************
   * Methods for workers.
   ***************************************************************************/
  // Process all incoming requests, calling fn for each one.
  template <typename F>
  void process_requests(F &fn) {
    do {
      update_peer_list_if_necessary();

      bool received;
      work_item_request req;

      // Send an error response for any message that fails validation.
      while ((received = m_messenger.receive(req.message())) && !req.is_valid()) {
        log_info("receive_request: incoming message failed validation.");
        work_item_response error_resp(req, "Failed validation");
        m_messenger.send(error_resp.message());
      }

      if (received) {
        responder_pre_crash_handler pch(this, req);
        global_info::pre_crash_handler_push(&pch);
        reliable_storage::id resource_id = fn(req.resource_id(), req.payload());
        work_item_response resp(req, resource_id);
        global_info::pre_crash_handler_pop();

        // Not much point doing this inside the crash-handler-protected area...
        // if it fails then there's no chance that the pre-crash-handler will
        // be able to respond either.
        m_messenger.send(resp.message());
      }
    } while (true);    //TODO: only run for a certain number of iterations, then
                       // stop so that everything can be reset.
  }

  uint64_t number_sinbins() const { return m_messenger.number_sinbins(); }
  uint64_t number_retries() const { return m_messenger.number_retries(); }
  

private:
  /// Disable copy.
  work_distributor(const work_distributor &);
  work_distributor &operator = (const work_distributor &);

private:
  // Our pre-crash handler that attempts to send an error message to the client.
  struct responder_pre_crash_handler : public global_info::pre_crash_handler {
    responder_pre_crash_handler(work_distributor *owner,
                                const work_item_request &req)
      : m_owner(owner), m_request(req) {}

    virtual void call(const char *crash_msg) {
      work_item_response resp(m_request, crash_msg);
      m_owner->m_messenger.send(resp.message());
    }

    work_distributor *m_owner;
    work_item_request m_request;
  };


  // Read the client peer list from reliable storage.
  static rstd::vector<rstd::string> read_client_peer_list(reliable_storage &rs) {
    return read_lines(reliable_storage::id::client_peer_list_id(), rs);
  }

  // Read the worker peer list from reliable storage.
  static rstd::vector<rstd::string> read_worker_peer_list(reliable_storage &rs) {
    return read_lines(reliable_storage::id::worker_peer_list_id(), rs);
  }

  // Read a file of lines from reliable storage.
  static rstd::vector<rstd::string> read_lines(const reliable_storage::id &i,
                                              reliable_storage &rs) {
    reliable_storage::stream rs_stream(rs);
    NP1_ASSERT(rs.open_ro(i, RELIABLE_STORAGE_TIMEOUT_SECONDS, rs_stream),
                "Unable to open config file in reliable_storage");
    rstd::vector<rstd::string> lines;
    text_input_stream<reliable_storage::stream> input(rs_stream);
    //TODO: check for errors.
    input.read_all(lines);
    return lines;
  }

  // Update the messenger's list of peers if either of the files have changed.
  void update_peer_list_if_necessary() {
    uint64_t now = time::now_epoch_usec();
    if (now > m_last_checked_peer_list_time + PEER_LIST_TIME_CHECK_TIMEOUT_USEC) {
      uint64_t worker_peer_list_mtime = 0;
      uint64_t client_peer_list_mtime = 0;
  
      if (!m_storage.get_mtime_usec(reliable_storage::id::worker_peer_list_id(),
                                    worker_peer_list_mtime)
          || !m_storage.get_mtime_usec(reliable_storage::id::client_peer_list_id(),
                                        client_peer_list_mtime)) {
        //TODO: should this be fatal, or just log an error and continue?
        NP1_ASSERT(false, "Unable to get mtime for peer list");
      }

      m_last_checked_peer_list_time = now;

      if ((worker_peer_list_mtime != m_last_worker_peer_list_mtime)
          || (client_peer_list_mtime != m_last_client_peer_list_mtime)) {
        log_info("Peer list(s) have changed, updating...");
        m_messenger.set_peer_list(read_client_peer_list(m_storage), read_worker_peer_list(m_storage));
        m_last_worker_peer_list_mtime = worker_peer_list_mtime;
        m_last_client_peer_list_mtime = client_peer_list_mtime;
        log_info("Updated peer lists.");
      } else {
        log_info("Checked peer list mtimes, no need to update now.");      
      }
    }    
  }

  void log_info(const char *description) {
    log::info(log_id(), description);
  }

  static const char *log_id() { return "work_distributor"; }
  
private:
  reliable_storage m_storage;
  net::udp_messenger m_messenger;
  uint64_t m_last_checked_peer_list_time;
  uint64_t m_last_worker_peer_list_mtime;
  uint64_t m_last_client_peer_list_mtime;
};




} // namespaces
}



#endif
