// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_IO_NET_UDP_MESSENGER_HPP
#define NP1_IO_NET_UDP_MESSENGER_HPP


#include "np1/io/net/udp_socket.hpp"
#include "np1/consistent_hash_table.hpp"
#include "np1/io/log.hpp"

// A worker peer is allowed to have a proportionally higher number of queued messages than the expected average.
// But when it exceeds this level then its workload will be given to other peers.
//TODO: these defines are out here rather than being 'static const' inside the class because g++ 4.6 doesn't like
// them in there without a 'constexpr' and g++ 4.4 doesn't like constexpr.  Move these back in when there's no need
// to support g++ 4.4 any more.
#define NP1_IO_NET_UDP_MESSENGER_ALLOWED_WORKER_Q_SIZE_PROPORTION_BEFORE_REDISTRIBUTION ((double)1.2)
#define NP1_IO_NET_UDP_MESSENGER_MIN_Q_SIZE_TO_BE_CONSIDERED_OVERCOMMITTED ((double)5)


namespace np1 {
namespace io {
namespace net {


class udp_messenger {
public:
  typedef uint32_t checksum_type;
  typedef uint64_t message_id_type;
  typedef uint8_t flags_type;
  enum { CONSISTENT_HASH_TABLE_SERVER_DUPLICATES = 50 };
  enum { MAX_MESSAGE_SIZE_INCLUDING_OVERHEAD = 512 };
  enum { MESSAGE_OVERHEAD_SIZE = sizeof(checksum_type) + sizeof(message_id_type) + sizeof(flags_type) };
  enum { MAX_MESSAGE_SIZE = MAX_MESSAGE_SIZE_INCLUDING_OVERHEAD - MESSAGE_OVERHEAD_SIZE };
  enum { MAX_RESOURCE_ID_SIZE = 64 };
  enum { SOCKET_WRITE_TIMEOUT_MILLISECONDS = 1000 };
  enum { SOCKET_READ_TIMEOUT_MILLISECONDS = 1000 };
  static const uint64_t WAIT_FOR_ACK_TIMEOUT_USEC = 2 * 1000 * 1000;
  static const uint64_t MAX_NUMBER_PEER_RETRIES_BEFORE_SINBIN = 10;
  static const uint64_t PEER_SINBIN_TIMEOUT_USEC = WAIT_FOR_ACK_TIMEOUT_USEC * 10;

  class message {
  public:
    typedef enum {
      FLAG_ACK = 1
    } flags_type_type;

  public:
    message() {
      clear();
    }

    // Construct a message that could be sent to the "most appropriate" peer
    // based on the resource id.
    explicit message(const str::ref &resource_id)
        : m_payload_size(0), m_sent_time(0) {
      size_t resource_id_length = resource_id.length();      
      NP1_ASSERT(resource_id_length <= MAX_RESOURCE_ID_SIZE,
                  "Resource id length exceeds MAX_RESOURCE_ID_SIZE");
      memcpy(m_resource_id, resource_id.ptr(), resource_id_length);
      m_resource_id[resource_id_length] = '\0';
      memset(m_body, 0, sizeof(m_body));  
    }

    // Construct an ack message.
    struct ack_marker {};
    message(const message &original, const ack_marker &)
        : m_peer(original.peer_endpoint()), m_payload_size(0), m_sent_time(0) {
      m_resource_id[0] = '\0';      
      memset(m_body, 0, sizeof(m_body));
      message_id(original.message_id());
      flags(FLAG_ACK);  
    }

    // Construct a message that's destined for a particular peer.
    explicit message(const ip_endpoint &ep) : m_peer(ep), m_payload_size(0), m_sent_time(0) {
      m_resource_id[0] = '\0';      
      memset(m_body, 0, sizeof(m_body));
    }

    // Reset the message.
    void clear() {
      memset(m_body, 0, sizeof(m_body));  
      m_resource_id[0] = '\0';
      m_peer = ip_endpoint();
      m_payload_size = 0;
      m_sent_time = 0;
    }


    // Append some data into the message.
    void append(const void *data, size_t length) {
      NP1_ASSERT(MAX_MESSAGE_SIZE - m_payload_size >= length,
                  "Message length exceeds MAX_MESSAGE_SIZE");
      memcpy(payload_ptr() + m_payload_size, data, length);
      m_payload_size += length;
    }

    bool has_valid_resource_id() const { return (m_resource_id[0] != '\0'); }
    bool has_valid_peer() const { return m_peer.is_valid(); }
    const char *resource_id() const { return m_resource_id; }
    const ip_endpoint &peer_endpoint() const { return m_peer; }
    const unsigned char *payload_ptr() const {
      return ((message *)this)->payload_ptr();
    }

    unsigned char *payload_ptr() {
      return flags_ptr() + sizeof(flags_type);    
    }

    size_t payload_size() const { return m_payload_size; }

    size_t remaining_available_payload_size() const {
      return MAX_MESSAGE_SIZE - m_payload_size;
    }

    // For the use of the udp_messenger class.
    const void *body_ptr() const { return m_body; }
    void *body_ptr() { return m_body; }
    size_t body_size() const { return m_payload_size + MESSAGE_OVERHEAD_SIZE; }
    void body_size(size_t sz) {
      NP1_ASSERT(sz >= MESSAGE_OVERHEAD_SIZE, "Message body is impossibly small");
      m_payload_size = sz - MESSAGE_OVERHEAD_SIZE;
    }

    void peer_endpoint(const ip_endpoint &p) { m_peer = p; }  


    message_id_type message_id() const {
      message_id_type id;
      memcpy(&id, message_id_ptr(), sizeof(message_id_type));
      return id;
    }

    void message_id(message_id_type id) {
      memcpy(message_id_ptr(), &id, sizeof(message_id_type));
    }

    void set_checksum() {
      checksum_type c = generate_checksum();
      memcpy(checksum_ptr(), &c, sizeof(checksum_type));
    }

    checksum_type checksum() const {
      checksum_type c;
      memcpy(&c, checksum_ptr(), sizeof(checksum_type));
      return c;
    }

    bool check_checksum() const {
      return (generate_checksum() == checksum());
    }

    bool is_ack() const { return flags() & FLAG_ACK; }

    uint64_t sent_time() const { return m_sent_time; }
    void sent_time(uint64_t st) { m_sent_time = st; }


  private:
    checksum_type generate_checksum() const {
      uint64_t hval = hash::fnv1a64::init();
      hval = hash::fnv1a64::add(message_id_ptr(), m_payload_size, hval);
      checksum_type checksum = (hval & 0xffffffff);
      return checksum;
    }
  

    unsigned char *checksum_ptr() {
      return m_body;
    }

    const unsigned char *checksum_ptr() const {
      return ((message *)this)->checksum_ptr();
    }


    unsigned char *message_id_ptr() {
      return checksum_ptr() + sizeof(checksum_type);    
    }

    const unsigned char *message_id_ptr() const {
      return ((message *)this)->message_id_ptr();
    }


    unsigned char flags() const { return *flags_ptr(); }
    void flags(unsigned char f) { *flags_ptr() = f; }

    unsigned char *flags_ptr() {
      return message_id_ptr() + sizeof(message_id_type);
    }

    const unsigned char *flags_ptr() const {
      return ((message *)this)->flags_ptr();
    }

  private:    
    unsigned char m_body[MAX_MESSAGE_SIZE_INCLUDING_OVERHEAD];
    char m_resource_id[MAX_RESOURCE_ID_SIZE + 1];
    ip_endpoint m_peer;
    size_t m_payload_size;
    uint64_t m_sent_time;
  };


private:
  struct peer {
    peer() : m_number_retries_since_last_received(0), m_next_sinbin_exit_time(0), m_is_worker(false),
              m_number_unacked_messages(0) {}

    peer(const ip_endpoint &ep, bool is_worker)
      : m_endpoint(ep), m_number_retries_since_last_received(0), m_next_sinbin_exit_time(0), m_is_worker(is_worker),
        m_number_unacked_messages(0) {}

    ip_endpoint m_endpoint;
    uint64_t m_number_retries_since_last_received;    
    uint64_t m_next_sinbin_exit_time;               // 0 if the peer is considered active.
    bool m_is_worker;

    //NOTE that this is currently just an ESTIMATE of the number of unacked messages.  The true number should
    // be no higher than this.
    uint64_t m_number_unacked_messages;
  };

  struct peer_less_than {
    bool operator()(const peer &p1, const peer &p2) const {
      return p1.m_endpoint < p2.m_endpoint;
    }

    bool operator()(const ip_endpoint &ep, const peer &p2) const {
      return ep < p2.m_endpoint;
    }

    bool operator()(const peer &p1, const ip_endpoint &ep) const {
      return p1.m_endpoint < ep;
    }
  };

  typedef skip_list<peer, peer_less_than> peer_list_type;
  
public:
  /// Crashes on error.
  explicit udp_messenger(const rstd::vector<rstd::string> &client_peer_strings,
                          const rstd::vector<rstd::string> &worker_peer_strings,
                          const rstd::string &start_endpoint_info,
                          bool ok_to_search_for_available_endpoint)
    : m_number_worker_peers(0)
    , m_worker_peer_table(CONSISTENT_HASH_TABLE_SERVER_DUPLICATES)
    , m_socket_endpoint(start_endpoint_info)    
    , m_number_sinbins(0)
    , m_number_retries(0) {
    // Set up the socket.
    NP1_ASSERT(m_socket.create(), "Unable to create UDP socket");
          
    bool successful_bind = false;
    do {
      NP1_ASSERT((is_peer_in(worker_peer_strings, m_socket_endpoint)
                  || is_peer_in(client_peer_strings, m_socket_endpoint)),
                  "Unable to find endpoint " + m_socket_endpoint.to_string()
                  + " in peer list, so it's pointless listening on that endpoint.");

      log_info("Attempting to bind.", m_socket_endpoint);
      successful_bind = m_socket.bind(m_socket_endpoint);
      if (!successful_bind && ok_to_search_for_available_endpoint) {
        log_info("Failed to bind.", m_socket_endpoint);
        m_socket_endpoint.port(m_socket_endpoint.port() + 1);
        log_info("Attempting to bind to new endpoint.", m_socket_endpoint);
      }
    } while (!successful_bind && ok_to_search_for_available_endpoint);

    NP1_ASSERT(successful_bind,
                "Unable to bind UDP socket to endpoint " + m_socket_endpoint.to_string());

    log_info("Bound UDP socket.", m_socket_endpoint);

    m_socket.set_timeouts(SOCKET_WRITE_TIMEOUT_MILLISECONDS,
                          SOCKET_READ_TIMEOUT_MILLISECONDS);

    global_info::listening_endpoint(m_socket_endpoint.to_string().c_str());

    // Set up the list of peers.
    set_peer_list(client_peer_strings, worker_peer_strings);
  }

  ~udp_messenger() {
    global_info::listening_endpoint("");
  }



  /// Update this object's understanding of the world around it.
  void set_peer_list(const rstd::vector<rstd::string> &client_peer_strings,
                      const rstd::vector<rstd::string> &worker_peer_strings) {
    m_worker_peer_table.clear();
    m_peers.clear();
    m_number_worker_peers = 0;

    log::info(log_id(), "Setting peer list. client_peer_strings.size()=",
                client_peer_strings.size(), " worker_peer_strings.size()=",
                worker_peer_strings.size());
       
    // Insert all the worker peers into our peer list.
    // Don't insert our own endpoint :).
    rstd::vector<rstd::string>::const_iterator si = worker_peer_strings.begin();
    rstd::vector<rstd::string>::const_iterator siz = worker_peer_strings.end();
    for (; si < siz; ++si) {
      ip_endpoint ep(*si);
      if (ep != m_socket_endpoint) {
        m_peers.insert(peer(ep, true));
        ++m_number_worker_peers;
      }
    }

    // Insert all the client peers into our peer list.
    si = client_peer_strings.begin();
    siz = client_peer_strings.end();
    for (; si < siz; ++si) {
      ip_endpoint ep(*si);
      if (ep != m_socket_endpoint) {
        peer p(ep, false);
        m_peers.insert(p);
        log_info("Inserted client peer to peers list.", p);
      }
    }    

    // Insert only the worker peers into our consistent hash table.
    // Client peers aren't responsible for any resources so shouldn't go into
    // the consistent hash table.
    peer_list_type::iterator pi = m_peers.begin();
    peer_list_type::iterator piz = m_peers.end();
    for (; pi != piz; ++pi) {
      if (pi->m_is_worker) {
        bool result = m_worker_peer_table.insert(pi);
        log::info(log_id(), "Consistent hash table insert result for peer '",
                    pi->m_endpoint.to_string().c_str(), "' is ", result);
      } else {
        log_info("Peer is a client peer, did _not_ insert into consistent hash table.", *pi);
      }
    }
  }



  // Send a message. Crashes on fatal error.
  void send(message &outgoing, bool is_resend = false) {
    NP1_ASSERT(outgoing.has_valid_resource_id() || outgoing.has_valid_peer(),
                "Outgoing message must have either valid resource id or valid peer");

    // Set up the message for sending, then send.
    // ACKs have the message id of the original message.
    if (!outgoing.is_ack() && !is_resend) {
      outgoing.message_id(generate_next_message_id());
    }

    outgoing.set_checksum();
    uint64_t now = time::now_epoch_usec();

    // Get a reference to the target peer, if available.
    peer_list_type::iterator peer_iter = m_peers.end();
    if (outgoing.has_valid_peer()) {
      peer_iter = m_peers.find(outgoing.peer_endpoint());
      NP1_ASSERT(peer_iter != m_peers.end(),
                  "Message has valid peer but this peer is not found in our peer list!");

      // Remove peer from sin bin if necessary.
      if ((peer_iter->m_next_sinbin_exit_time != 0) && (now > peer_iter->m_next_sinbin_exit_time)) {
        log_info("Peer has done time in sinbin, will remove now.", *peer_iter);
        peer_iter->m_next_sinbin_exit_time = 0;
      }
    }

    // If this message is a retry and we've retried the peer a lot since we last heard from it,
    // we might need to sinbin the peer.
    if (is_resend) {
      NP1_ASSERT(peer_iter != m_peers.end(),
                  "Message has been sent but peer is invalid!");
      if ((0 == peer_iter->m_next_sinbin_exit_time)
          && (peer_iter->m_number_retries_since_last_received > MAX_NUMBER_PEER_RETRIES_BEFORE_SINBIN)) {
        peer_iter->m_next_sinbin_exit_time = now + PEER_SINBIN_TIMEOUT_USEC;
        log_info("Sinbinned peer.", *peer_iter);
        ++m_number_sinbins;
      } else {
        peer_iter->m_number_retries_since_last_received++;
      }
    }

    // If there's no resource id then this message must go to the target peer
    // and only the target peer.
    if (!outgoing.has_valid_resource_id()) {
      NP1_ASSERT(peer_iter != m_peers.end(),
                  "Message has no valid resource id or valid peer!");

      if (peer_iter->m_next_sinbin_exit_time > now) {
        log_info("Peer is sinbinned but message is only for that peer so we have to send it there anyway.",
                  *peer_iter, outgoing);
      }

      log_info("Sending direct message to peer.", outgoing);
      NP1_ASSERT(do_send(outgoing, now, *peer_iter, is_resend), "Unable to send direct message to peer.");
      return;
    }

    // If we get to here then the message can be sent to any one of several
    // peers.  We just need to find one that's not sinbinned.
    log_info("Looking for peer suitable to receive message.", outgoing);

    consistent_hash_table_type::iterator hashed_peer_i =
      m_worker_peer_table.lower_bound(outgoing.resource_id());

    consistent_hash_table_type::iterator hashed_peer_iz = m_worker_peer_table.end();

    for (; hashed_peer_i != hashed_peer_iz; ++hashed_peer_i) {
      peer_list_type::iterator peer_i = *hashed_peer_i;

      if (peer_i->m_next_sinbin_exit_time > now) {
        log_info("Peer is sinbinned so we can't send to it.", *peer_i);
      } else if (!is_peer_overcommitted(*peer_i)) {
        outgoing.peer_endpoint(peer_i->m_endpoint);
        log_info("Sending message to peer.", *peer_i, outgoing);
        NP1_ASSERT(do_send(outgoing, now, *peer_i, is_resend), "Unable to send message to peer.");
        return;
      }
    }

    log_info("No available peers to receive message ", outgoing);
    NP1_ASSERT(false, "No available peers to receive message.");
  }


  // Read just one message.  Returns true if there was something to receive.
  // May also retry messages that haven't been responded to yet.
  /// receive() is the only mechanism for retrying, so you need to keep calling
  /// it until you're really finished.  You should also call receive with
  /// a 0 timeout as often as possible to try to avoid losing messages. 
  bool receive(message &received,
                uint32_t read_timeout_ms = SOCKET_READ_TIMEOUT_MILLISECONDS) {
    ip_endpoint from;
    size_t bytes_read;

    // Keep going until something definitely good or something definitely bad
    // happens.
    while (true) {
      received.clear();
  
      // We don't deal with duplicate non-ack messages here, so actions
      // resulting from messages must be be idempotent.  It's
      // near-impossible to ensure that messages are only delivered once in the presence
      // of machine/network failure, even if you use TCP.
  
      // Wait for some data, retrying old messages if need be.
      if (!busy_wait_for_readability(read_timeout_ms)) {
        return false;
      }
  
      // Someone loves us! 
      if (!m_socket.read(received.body_ptr(), MAX_MESSAGE_SIZE_INCLUDING_OVERHEAD,
                          &bytes_read, from)) {
        //TODO: should this be a more serious error or assertion failure?
        log::info(log_id(), "Socket is apparently readable but I couldn't read from it."); 
        return false;
      }

      bool successful_receive = false;  
      if (bytes_read < MESSAGE_OVERHEAD_SIZE) {
        log_info("Received undersize message from peer. Ignoring.", from);
      } else {
        received.body_size(bytes_read);
        NP1_ASSERT(from.is_valid(), "Received 'from' endpoint is invalid!");
        received.peer_endpoint(from);
        peer_list_type::iterator peer_iter;  
  
        if ((peer_iter = m_peers.find(from)) == m_peers.end()) {
          log_info("Received message from unknown peer. Ignoring.", received);
        } else if (!received.check_checksum()) {
          log_info("Received message, checksum check failed.", received);          
        } else if (received.is_ack()) {

          // Even if the message is not awaiting ack, we should deduct this ack
          // from the peer's score.
          if (peer_iter->m_number_unacked_messages > 0) {
            peer_iter->m_number_unacked_messages--;
          }

          if (!m_messages_awaiting_ack.erase(received.message_id())) {
            log_info("Received ack, but original message is not awaiting ack.", received);
          } else {
            log_info("Received ack.  Original message has been removed from the 'awaiting ack' list.",
                      received);
            successful_receive = true;
          }
        } else {
          log_info("Received non-ack message.", received);
          successful_receive = true;
        }
  
        if (peer_iter != m_peers.end()) {
          peer_iter->m_number_retries_since_last_received = 0;
          if (peer_iter->m_next_sinbin_exit_time > 0) {
            log_info("Removing peer from sinbin.", received);          
          }
          peer_iter->m_next_sinbin_exit_time = 0;
        }
      }

      if (successful_receive) {
        return true;    
      }
    }

    NP1_ASSERT(false, "Unreachable");
    return false; 
  }

  size_t number_messages_awaiting_ack() const {
    return m_messages_awaiting_ack.size();
  }

  size_t number_worker_peers() const {
    return m_worker_peer_table.size();
  }

  uint64_t number_sinbins() const { return m_number_sinbins; }
  uint64_t number_retries() const { return m_number_retries; }

private:
  /// Disable copy.
  udp_messenger(const udp_messenger &);
  udp_messenger &operator = (const udp_messenger &);

private:
  static const char *log_id() {
    return "udp_messenger";
  }

  uint64_t generate_next_message_id() {
    uint64_t id = 0;
    // Straight serial numbers have problems when you restart the process
    // unless you store them somewhere.  Time-based serial numbers have problems
    // if the computer clock is moved backwards.  So we take the most
    // often-changing part of the time and append some randomness.
    id = (time::now_epoch_usec() & 0xffffffff) << 32;
    id |= math::rand64() & 0xffffffff;
    return id;
  }

  // Retry just one message if necessary.  Return true if a message was retried, false otherwise.
  bool retry_one() {
    skip_list<message, message_less_than>::const_iterator waiting_i = m_messages_awaiting_ack.begin();
    skip_list<message, message_less_than>::const_iterator waiting_iz = m_messages_awaiting_ack.end();

    // Messages are sorted by message id and the front part of the message id
    // is the low 32-bits of the time in microseconds. So they should be
    // sorted from oldest to newest, modulo those messages that are so old
    // that the 32-bit microsecond time has rolled over.
    //TODO: check a max-number-retries for each message.
    uint64_t now = time::now_epoch_usec();
    if ((waiting_i != waiting_iz)
      && (now > waiting_i->sent_time() + WAIT_FOR_ACK_TIMEOUT_USEC)) {
      message m = *waiting_i;
      m_messages_awaiting_ack.erase(*waiting_i);
      log_info("Message has not yet received an ack.  Resending.", m);
      send(m, true);
      ++m_number_retries;
      return true;
    }
    
    return false;
  }


  // Just send, don't go looking for any peers.
  bool do_send(message &m, uint64_t now, peer &p, bool is_resend) {
    NP1_ASSERT(m.has_valid_peer(), "do_send(m, now) where m has no valid peer!");

    if (m_socket.write(m.peer_endpoint(), m.body_ptr(), m.body_size())) {
      m.sent_time(now);

      if (!m.is_ack()) {
        NP1_ASSERT(m_messages_awaiting_ack.insert(m),
                    "Sent message that was already in the 'awaiting ack' list!");
        log_info("Message is not an ack, so have stored it in the 'awaiting ack' list.", m);

        if (!is_resend) {
          p.m_number_unacked_messages++;
        }
      }
      
      return true;
    }

    return false;
  }


  // Wait for socket readability and periodically retry old messages as we do it.
  bool busy_wait_for_readability(uint32_t read_timeout_ms) {
    uint32_t individual_read_timeout_ms = read_timeout_ms;
    if (individual_read_timeout_ms > SOCKET_READ_TIMEOUT_MILLISECONDS) {
      individual_read_timeout_ms = SOCKET_READ_TIMEOUT_MILLISECONDS;
    }
    
    uint64_t start_usec = time::now_epoch_usec();  
    bool result = false;    

    // Retry all messages that need retrying, until the socket becomes readable.
    while (retry_one() && !(result = m_socket.wait_for_readability(0))) {
    }

    if (result) {
      // Socket is readable, no need to wait any more.
      return result;
    }

    // Now we can wait at a more relaxed pace.
    // Note that just setting the socket timeout may cause a longer wait than
    // what the caller wants, and also we need to wake up to retry messages.
    while (!(result = m_socket.wait_for_readability(individual_read_timeout_ms))
              && (time::usec_to_msec(time::now_epoch_usec() - start_usec) < read_timeout_ms)) {
      retry_one();
    } 

    return result;
  }


  bool is_peer_overcommitted(const peer &p) const {
    double peer_q_size = (double)p.m_number_unacked_messages;
    double expected_q_size = ((double)m_messages_awaiting_ack.size())/((double)m_number_worker_peers);
    if ((peer_q_size > NP1_IO_NET_UDP_MESSENGER_MIN_Q_SIZE_TO_BE_CONSIDERED_OVERCOMMITTED)
        && (expected_q_size
              * NP1_IO_NET_UDP_MESSENGER_ALLOWED_WORKER_Q_SIZE_PROPORTION_BEFORE_REDISTRIBUTION < peer_q_size)) {
      log_info("Peer is overcommitted.", p, peer_q_size, expected_q_size);
      return true;
    }

    return false;
  }


  // Logging helpers.
#define NP1_IO_NET_UDP_MESSENGER_MESSAGE_LOG_ARGS(m__) \
"  Message: id=", m__.message_id(), " peer=", m__.peer_endpoint().to_string().c_str(), " resource_id=", m__.resource_id(), \
" is_ack=", m__.is_ack(), " checksum=", m__.checksum(), " sent_time=", m__.sent_time()

#define NP1_IO_NET_UDP_MESSENGER_PEER_LOG_ARGS(p__) \
"  Peer: endpoint=", p__.m_endpoint.to_string().c_str(), " number_retries_since_last_received=", p__.m_number_retries_since_last_received, " next_sinbin_exit_time=", p__.m_next_sinbin_exit_time, " number_unacked_messages=", p__.m_number_unacked_messages

  void log_info(const char *description, const message &m) const {
    log::info(log_id(), description, NP1_IO_NET_UDP_MESSENGER_MESSAGE_LOG_ARGS(m));
  }

  void log_info(const char *description, const peer &p) const {
    log::info(log_id(), description, NP1_IO_NET_UDP_MESSENGER_PEER_LOG_ARGS(p));
  }

  void log_info(const char *description, const peer &p, double peer_q_size, double expected_q_size) const {
    log::info(log_id(), description, NP1_IO_NET_UDP_MESSENGER_PEER_LOG_ARGS(p), " peer_q_size=", peer_q_size,
              " expected_q_size=", expected_q_size);
  }

  void log_info(const char *description, const peer &p, const message &m) const {
    log::info(log_id(), description, NP1_IO_NET_UDP_MESSENGER_PEER_LOG_ARGS(p),
                NP1_IO_NET_UDP_MESSENGER_MESSAGE_LOG_ARGS(m));
  }


  void log_info(const char *description, const ip_endpoint &ep) const {
    log::info(log_id(), description, "Endpoint: ", ep.to_string().c_str());
  }


  bool is_peer_in(const rstd::vector<rstd::string> &lst, const ip_endpoint &ep) {
    rstd::string ep_str = ep.to_string();
    rstd::vector<rstd::string>::const_iterator i = lst.begin();
    rstd::vector<rstd::string>::const_iterator iz = lst.end();
    for (; i < iz; ++i) {
      if (*i == ep_str) {
        return true;
      }
    }
  
    return false;
  }

  struct hash_function {
    uint64_t operator()(peer_list_type::iterator pi, uint64_t consistent_hash_internal) {
      uint64_t hval = hash::fnv1a64::init();
      hval = pi->m_endpoint.hash_add(hval);
      hval = hash::fnv1a64::add(&consistent_hash_internal, sizeof(consistent_hash_internal), hval);
      return hval;
    }

    uint64_t operator()(const char *resource_id, uint64_t consistent_hash_internal) {
      uint64_t hval = ::np1::hash::fnv1a64::init();
      hval = hash::fnv1a64::add(resource_id, strlen(resource_id), hval);
      hval = hash::fnv1a64::add(&consistent_hash_internal, sizeof(consistent_hash_internal), hval);
      return hval;    
    }
  };

  struct message_less_than {
    bool operator()(const message &m1, const message &m2) {
      return m1.message_id() < m2.message_id();
    }

    bool operator()(uint64_t id, const message &m2) {
      return id < m2.message_id();
    }

    bool operator()(const message &m1, uint64_t id) {
      return m1.message_id() < id;
    }
  };


private:
  // The actual list of peers.
  peer_list_type m_peers;

  // The number of worker peers.
  size_t m_number_worker_peers;

  // A consistent hash table to help us assign ownership of resources to
  // peers.
  typedef consistent_hash_table<peer_list_type::iterator, hash_function>
            consistent_hash_table_type;
  consistent_hash_table_type m_worker_peer_table;

  // Messages that have been sent, need an ack and haven't got one yet.
  //TODO: rather than copying whole messages between the queue and this object,
  // better to pass references around.
  skip_list<message, message_less_than> m_messages_awaiting_ack;

  // The socket and the endpoint that it's bound to.
  udp_socket m_socket;
  ip_endpoint m_socket_endpoint;

  // Some stats.
  uint64_t m_number_sinbins;
  uint64_t m_number_retries;
};


} // namespaces.
}
}



#endif
