// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_IO_ORDERED_WORK_DISTRIBUTOR_HPP
#define NP1_IO_ORDERED_WORK_DISTRIBUTOR_HPP


#include "np1/io/work_distributor.hpp"


namespace np1 {
namespace io {


// A higher-level interface to work_distributor for the benefit of clients.
// Orders responses so that they appear in the same order as their matching
// requests, and other niceties.  Crashes on error.  A single instance of
// this class should only be used for a single set of related work items.
class ordered_work_distributor {
public:
  enum { NO_PROGRESS_TIMEOUT_MS = 5 * 60 * 1000 }; // 5 minutes.

  typedef net::udp_messenger::message_id_type request_id_type;

public:
  ordered_work_distributor(const rstd::string &reliable_storage_local_root,
                            const rstd::string &reliable_storage_remote_root,
                            const rstd::string &listen_endpoint)
    : m_distributor(reliable_storage_local_root, reliable_storage_remote_root, listen_endpoint, true) {
  }

  // Send a work item request.
  void send_request(const reliable_storage::id &resource_id,
                      const str::ref &request_payload) {
    // Check for any responses and save them.
    work_distributor::work_item_response resp;
    while (receive_response(resp, 0)) {
      m_available_responses.insert(resp);
      log_info("send_request: received & stored response.", resp.request_id());
    }
    
    // Now send the request we were asked to send.
    work_distributor::request_id_type request_id =
      m_distributor.send_request(resource_id, request_payload);
    m_outstanding_requests.push_back(request_id);
    log_info("send_request: sent request.", request_id);
  }

  // Receive a work item response.  Crashes if there is no progress within
  // a reasonable time.  Returns false if all responses have been received.
  // This method should ONLY be called after all requests have been sent.
  bool receive_response(reliable_storage::id &resource_id) {
    while (!m_outstanding_requests.empty()) {
      // If we've already received a response for the first-most request...
      available_responses_type::iterator response_i
        = m_available_responses.find(m_outstanding_requests.front());
      if (response_i != m_available_responses.end()) {
        log_info("receive_response: found available response.", response_i->request_id());

        //...use that response as what should be returned to the user.
        NP1_ASSERT(!response_i->is_error(), "Dequeued response is error response!");
        resource_id = response_i->resource_id();
        m_available_responses.erase(m_outstanding_requests.front());
        m_outstanding_requests.pop_front();
        return true;
      }

      // Receive a response for real.
      work_distributor::work_item_response resp;
      mandatory_receive_response(resp);

      // If the response is for the topmost outstanding request, no need to
      // enqueue it, just return it.
      if (m_outstanding_requests.front() == resp.request_id()) {
        log_info("receive_response: received response that happens to match topmost outstanding request.",
                  resp.request_id());

        m_outstanding_requests.pop_front();
        resource_id = resp.resource_id();
        return true;
      }

      m_available_responses.insert(resp);
    }

    log_info("receive_response: no more outstanding requests.");

    return false;
  }

  uint64_t number_sinbins() const { return m_distributor.number_sinbins(); }
  uint64_t number_retries() const { return m_distributor.number_retries(); }


private:
    /// Disable copy.
  ordered_work_distributor(const ordered_work_distributor &);
  ordered_work_distributor &operator = (const ordered_work_distributor &);

private:
  bool receive_response(work_distributor::work_item_response &resp, uint32_t timeout_ms) {
    if (m_distributor.receive_response(resp, timeout_ms)) {    
      NP1_ASSERT(!resp.is_error(),
                  "Error response received from worker: " + rstd::string(resp.error_message()));
      return true;
    }

    return false;
  }

  void mandatory_receive_response(work_distributor::work_item_response &resp) {
    if (!receive_response(resp, NO_PROGRESS_TIMEOUT_MS)) {
      const char *description = "No progress made within the 'no progress' timeout";
      //TODO: this should be log_error.
      log_info(description);
      NP1_ASSERT(false, description);
    }
  }


  static const char *log_id() { return "ordered_work_distributor"; }

  void log_info(const char *description, request_id_type request_id) {
    log::info(log_id(), description, " request_id=", request_id);
  }

  void log_info(const char *description) {
    log::info(log_id(), description);
  }

  struct available_responses_less_than {
    bool operator()(const work_distributor::work_item_response &r1,
                    const work_distributor::work_item_response &r2) const {
      return r1.message().message_id() < r2.message().message_id();
    }

    bool operator()(const work_distributor::work_item_response &r1,
                    request_id_type id) const {
      return r1.message().message_id() < id;
    }

    bool operator()(request_id_type id, const work_distributor::work_item_response &r2) const {
      return id < r2.message().message_id();
    }
  };

private:
  work_distributor m_distributor;
  rstd::list<work_distributor::request_id_type> m_outstanding_requests;
  typedef skip_list<work_distributor::work_item_response,
                    available_responses_less_than> available_responses_type;
  available_responses_type m_available_responses;
};




} // namespaces
}



#endif
