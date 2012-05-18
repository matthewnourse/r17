// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_IO_RELIABLE_STORAGE_HPP
#define NP1_IO_RELIABLE_STORAGE_HPP


#include "np1/io/log.hpp"
#include "np1/io/gzfile.hpp"
#include "np1/hash/sha256.hpp"

namespace np1 {
namespace io {


// An abstraction for "reliable storage" accessed via HTTP GET and PUT.  The
// storage may be "eventually consistent".
class reliable_storage {
public:
  // An identifier for an object in the reliable storage.
  class id {
  public:
    enum { LENGTH = 64 };

  public:
    id() { m_data[0] = '\0'; }

    explicit id(const char *s) {
      initialize(s, strlen(s));
    }

    explicit id(const str::ref &s) {
      initialize(s.ptr(), s.length());
    }

    explicit id(const std::string &s) {
      initialize(s.c_str(), s.length());
    }

    bool is_valid() const { return (strlen(m_data) == LENGTH); }

    static id generate() {
      id i;
      math::rand256_hex_encoded(i.m_data);
      return i;
    }

    static id generate_temp() {
      id i = generate();
      i.make_temp();
      return i;
    }

    static id generate_temp(const io::reliable_storage::id &input_resource_id,
                            const str::ref &script_text) {
      id i;
      hash::sha256 hash_ctx;
      hash_ctx.update((const unsigned char *)input_resource_id.ptr(), LENGTH);
      hash_ctx.update((const unsigned char *)script_text.ptr(), script_text.length());
      unsigned char digest[32];
      hash_ctx.final(digest);
      str::to_hex_str_pad_64(i.m_data, digest);
      i.make_temp();
      return i;
    }

    bool is_temp() const {
      return ('t' == m_data[0]);
    }

    const char *ptr() const { return m_data; }

    str::ref to_str_ref() const { return str::ref(m_data, (m_data[0] ? LENGTH : 0)); }
    std::string to_string() const { return to_str_ref().to_string(); }

    bool operator == (const id &other) const {
      return (memcmp(m_data, other.m_data, LENGTH) == 0);
    }

    bool operator != (const id &other) const {
      return !(operator == (other));
    }


    // Well-known ids.
    static const id &client_peer_list_id() {
      static id i("0000000000000000000000000000000000000000000000000000000000000000");
      return i;
    }
  
    static const id &worker_peer_list_id() {
      static id i("0000000000000000000000000000000000000000000000000000000000000001");
      return i;
    }

  private:
    void initialize(const char *s, size_t len) {
      NP1_ASSERT(len == LENGTH, "Invalid reliable_storage::id: " + std::string(s));
      memcpy(m_data, s, len);
      m_data[LENGTH] = '\0';
    }

    void make_temp() {
      // This is safe because 't' is outside the 0-9, a-f range of the hex-encoded id.
      m_data[0] = 't'; 
    }

  private:
    char m_data[LENGTH + 1];
  };



  // A stream over an object in the reliable storage.  When the handle
  // to a stream is closed, changes are synced with the reliable store.
  // It's VERY important to check the result of 'close'.
  class stream : public detail::stream_helper<stream> {
  public:
    // Wrapping classes can use this to ensure that the thing they are wrapping
    // is unbuffered.
    struct is_unbuffered {};

  public:
    explicit stream(reliable_storage &owner)
      : detail::stream_helper<stream>(*this), m_owner(owner), m_for_writing(false) {}
    
    // For the use of reliable_storage only.
    bool open_ro(const char *local_path, const id &i) {
      bool success = m_file.open_ro(local_path);
      if (!success)

      if (success) {
        lock();
        m_id = i;
      }

      return success;
    }
  
    bool create_wo(const char *local_path, const id &i) {
      bool success = m_file.create_wo(local_path);
      if (success) {
        lock();
        m_id = i;
        m_for_writing = true;
      }

      return success;
    }

    bool read_some(void *buf, size_t bytes_to_read, size_t *bytes_read_p) {
      return m_file.read_some(buf, bytes_to_read, bytes_read_p);    
    }

    bool write_some(const void *buf, size_t bytes_to_write, size_t *bytes_written_p) {
      return m_file.write_some(buf, bytes_to_write, bytes_written_p);
    }

    bool write(const id &i) {
      NP1_ASSERT(i.is_valid(), "Attempt to write invalid reliable_storage id");
      str::ref s = i.to_str_ref();
      return detail::stream_helper<stream>::write(s.ptr(), s.length());
    }

    bool write(const char *s) { return detail::stream_helper<stream>::write(s); }
    bool write(const void *s, size_t n) { return detail::stream_helper<stream>::write(s, n); }


    gzfile &file_ref() { return m_file; }

    bool is_open() const { return m_file.is_open(); }

    bool close() {
      uint64_t start_time = time::now_epoch_usec();
      m_owner.m_stats.m_number_close++;
  
      bool result = m_file.close();
      if (result && m_for_writing) {        
        result = m_owner.http_put(m_id);
      }
      
      m_for_writing = false;
      uint64_t elapsed_time = time::now_epoch_usec() - start_time;
      if (!result) {
        m_owner.m_stats.m_number_close_failures++;
        m_owner.log_final("close failed.", m_file.name(), elapsed_time);
      } else {
        m_owner.log_final("close success.", m_file.name(), elapsed_time);
      }
  
      m_owner.m_stats.m_time_spent_in_close += elapsed_time;
      return result;
    }

    const std::string &name() const { return m_file.name(); }

  private:
    /// Disable copy.
    stream(const stream &);
    stream &operator = (const stream &);
    
  private:
    void lock() {
      NP1_ASSERT(m_file.lock_exclusive(), "Unable to lock reliable storage file " + m_file.name());
    }    

  private:
    reliable_storage &m_owner;
    gzfile m_file;
    id m_id;
    bool m_for_writing;
  };



  enum { SLEEP_INTERVAL_USEC = 1000000 };

public:
  reliable_storage(const std::string &local_root, const std::string &remote_root)
    : m_local_root(local_root), m_remote_root(remote_root) {}

  // Keep trying to open the object identified by 'id' until timeout_seconds
  // have elapsed.
  bool open_ro(const id &i, uint32_t timeout_seconds, stream &s) {
    uint64_t start_time = time::now_epoch_usec();
    m_stats.m_number_open_ro++;
    std::string local_path = make_local_path(i, in_between_dirs_noop());
    if (!s.open_ro(local_path.c_str(), i)) {
      // File not in local copy of the reliable storage, go get it from server.
      if (!http_get_with_retry(i, timeout_seconds)) {
        uint64_t elapsed_time = time::now_epoch_usec() - start_time;
        m_stats.m_time_spent_in_open_ro += elapsed_time;
        m_stats.m_number_open_ro_timeouts++;
        log_final("open_ro timed out.", make_remote_path(i), elapsed_time);
        return false;
      }    

      if (!s.open_ro(local_path.c_str(), i)) {
        uint64_t elapsed_time = time::now_epoch_usec() - start_time;
        m_stats.m_time_spent_in_open_ro += elapsed_time;
        log_final("Can't open after HTTP GET: ", local_path, elapsed_time);
        return false;
      }
    }

    uint64_t elapsed_time = time::now_epoch_usec() - start_time;
    m_stats.m_time_spent_in_open_ro += elapsed_time;
    log_final("open_ro success.", local_path, elapsed_time);
    return true;
  }


  // Create an object identified by 'id'.  
  bool create_wo(const id &i, stream &s) {
    uint64_t start_time = time::now_epoch_usec();
    m_stats.m_number_create_wo++;
    std::string path = make_local_path(i, in_between_dirs_mkdir());

    bool result = s.create_wo(path.c_str(), i);
    
    uint64_t elapsed_time = time::now_epoch_usec() - start_time;
    if (!result) {
      m_stats.m_number_create_wo_failures++;
      log_final("create_wo failed.", path, elapsed_time);
    } else {
      log_final("create_wo success.", path, elapsed_time);
    }

    m_stats.m_time_spent_in_create_wo += elapsed_time;
    return result;
  }

  // Delete an object identified by 'id'.
  bool erase(const id &i) {
    uint64_t start_time = time::now_epoch_usec();
    m_stats.m_number_erase++;
    std::string path = make_local_path(i, in_between_dirs_noop());

    bool result = http_delete(i);
    if (result) {
      result = file::erase(path.c_str());
    }
    
    uint64_t elapsed_time = time::now_epoch_usec() - start_time;
    if (!result) {
      m_stats.m_number_erase_failures++;
      log_final("erase failed.", path, elapsed_time);
    } else {
      log_final("erase success.", path, elapsed_time);
    }

    m_stats.m_time_spent_in_erase += elapsed_time;
    return result;
  }

  // Get the mtime of the LOCAL copy of the file only.
  bool get_mtime_usec(const id &i, uint64_t &mtime) {
    uint64_t start_time = time::now_epoch_usec();
    m_stats.m_number_mtime++;
    std::string path = make_local_path(i, in_between_dirs_noop());

    bool result = file::get_mtime_usec(path.c_str(), mtime);
    
    uint64_t elapsed_time = time::now_epoch_usec() - start_time;
    if (!result) {
      m_stats.m_number_mtime_failures++;
      log_final("mtime failed.", path, elapsed_time);
    } else {
      log_final("mtime success.", path, elapsed_time);
    }

    m_stats.m_time_spent_in_mtime += elapsed_time;
    return result;
  }

  // Returns true if a file exists either locally or remotely.
  bool exists(const id &i) {
    uint64_t start_time = time::now_epoch_usec();
    m_stats.m_number_exists++;
    std::string path = make_local_path(i, in_between_dirs_noop());
    bool result = false;
    uint64_t elapsed_time = 0;

    if (!(result = file::exists(path.c_str()))) {
      result = http_exists(i);
    }

    elapsed_time = time::now_epoch_usec() - start_time;
    m_stats.m_time_spent_in_exists += elapsed_time;
    return result;
  }


private:
  /// Disable copy.
  reliable_storage(const reliable_storage &);
  reliable_storage &operator = (const reliable_storage &);

private:
  static const char *log_id() {
    return "reliable_storage";
  }

  void log_final(const char *description, const std::string &path, uint64_t elapsed_time) const {
    log::info(log_id(), description, " path=", path.c_str(), " elapsed_time=", elapsed_time);
  }

  template <typename... Arguments>
  static void log_progress(const Arguments& ...arguments) {    
    log::info(log_id(), arguments...);
  }


  template <typename F>
  std::string make_local_path(const id &i, const F &in_between_dirs_f) const {
    return make_path(m_local_root, i, in_between_dirs_f);
  }

  std::string make_local_path(const id &i) const {
    return make_path(m_local_root, i, in_between_dirs_noop());
  }

  std::string make_remote_path(const id &i) {
    return make_path(m_remote_root, i, in_between_dirs_noop());
  }

  template <typename F>
  std::string make_remote_path(const id &i, const F &in_between_dirs_f) const {
    return make_path(m_remote_root, i, in_between_dirs_f);
  }


  template <typename F>
  static std::string make_path(const std::string &root, const id &i,
                                const F &in_between_dirs_f) {
    NP1_ASSERT(i.is_valid(),
                "Attempt to make a reliable storage path from an invalid id!  id: '"
                + i.to_string() + "'");

    std::string path(root);
    const char *p = i.ptr();
    int count;
    for (count = 0; count < 3; ++count, p += 2) {
      path.append("/");
      path.append(p, 2);
      in_between_dirs_f(path);
    }

    path.append("/");
    path.append(p);      
    return path;  
  }


  
  /// HTTP GET a file into a local path.
  bool http_get(const id &i) {
    std::string remote_path(make_remote_path(i));
    std::string local_path(make_local_path(i, in_between_dirs_mkdir()));
    std::string temp_local_path = local_path + "_" + str::to_dec_str(time::now_epoch_usec())
        + "_" + str::to_dec_str(math::rand64()) + ".tmp";

    log_progress("HTTP GETting '", remote_path.c_str(), "' into '", temp_local_path.c_str(),
                  "' then renaming to '", local_path.c_str(), "'");

    file temp_f;
    if (!temp_f.create_wo(temp_local_path.c_str())) {
      log_progress("Unable to create file to receive HTTP GET: ", temp_local_path.c_str());
      return false;
    }

    temp_f.lock_exclusive();

    long http_status;
    bool result = m_curl.get_no_headers(remote_path, temp_f, http_status);
    if (!result) {
      log_progress("HTTP GET failed for: ", remote_path.c_str(), "  HTTP status: ", http_status);
    } else {
      result = temp_f.hard_flush();
      if (!result) {
        log_progress("Hard flush failed for HTTP GET temp file: ", temp_local_path.c_str());
      }
    }

    temp_f.hard_flush();
    temp_f.close();
    if (!result) {
      if (!file::erase(temp_local_path.c_str())) {
        log_progress("Erase failed for HTTP GET temp file: ", temp_local_path.c_str());        
      }

      return result;
    }

    result = file::rename(temp_local_path.c_str(), local_path.c_str());
    if (!result) {
      log_progress("Rename failed for HTTP GET temp file '", temp_local_path.c_str(),
                    "'   Target name: '", local_path.c_str(), "'");
    } else {
      log_progress("Successfully got '", remote_path.c_str(), "' into '", local_path.c_str(), "'");    
    }

    return result;
  }


  // HTTP GET, keep retrying to allow it time to get to the server.
  bool http_get_with_retry(const id &i, uint32_t timeout_seconds) {
    uint64_t give_up_time = time::now_epoch_usec() + time::sec_to_usec(timeout_seconds);
    bool result;
    
    while (!(result = http_get(i)) && (time::now_epoch_usec() < give_up_time)) {
      sleep();
    }

    if (!result) {
      log_progress("HTTP GET failed within timeout period of ", timeout_seconds, " seconds");
    }

    return result;
  }


  // HTTP PUT a file.
  bool http_put(const id &i) {
    std::string remote_path(make_remote_path(i, in_between_dirs_mkcol(m_curl)));
    std::string local_path(make_local_path(i));

    log_progress("HTTP PUTting '", local_path.c_str(), "' into '", remote_path.c_str(), "'");

    file local_f;
    if (!local_f.open_ro(local_path.c_str())) {
      log_progress("Unable to open local path '", local_path.c_str(), "' for reading");
      return false;
    }

    long http_status;
    bool result = m_curl.put_no_headers(remote_path, local_f, http_status);
    log_progress("HTTP PUT '", local_path.c_str(), "' into '", remote_path.c_str(), "' ",
                  (result ? "succeeded" : "failed"), "  HTTP status: ", http_status);
    return result;
  }


  // HTTP DELETE a file.
  bool http_delete(const id &i) {
    std::string remote_path(make_remote_path(i));
    log_progress("HTTP DELETEing '", remote_path.c_str(), "'");
    long http_status;
    bool result = m_curl.delete_no_headers(remote_path, http_status);
    log_progress("HTTP DELETE '", remote_path.c_str(), "' ", (result ? "succeeded" : "failed"),
                  "  HTTP status: ", http_status);
    return result;
  }

  // Check to see if a remote file exists.
  bool http_exists(const id &i) {
    std::string remote_path(make_remote_path(i));
    log_progress("HTTP HEADing '", remote_path.c_str(), "'");
    long http_status;
    bool result = m_curl.exists(remote_path, http_status);
    log_progress("HTTP exists '", remote_path.c_str(), "' ", (result ? "yes" : "no"),
                  "  HTTP status: ", http_status);
    return result;
  }

  void sleep() {
    uint64_t start_time = time::now_epoch_usec();
    usleep(SLEEP_INTERVAL_USEC);
    uint64_t end_time = time::now_epoch_usec();
    m_stats.m_time_spent_sleeping += end_time - start_time;
    m_stats.m_number_sleeps++;
  }

  struct in_between_dirs_noop {
    void operator()(const std::string &path) const {}  
  };

  struct in_between_dirs_mkdir {
    void operator()(const std::string &path) const {
      // Don't bother checking the result. It will fail if the directory already
      // exists but that's ok for us.
      file::mkdir(path.c_str());
    }  
  };

  struct in_between_dirs_mkcol {
    explicit in_between_dirs_mkcol(net::curl &c) : m_curl(c) {}

    void operator()(const std::string &path) const {
      // Don't bother checking the result. It will fail if the directory already
      // exists but that's ok for us.
      long http_status;
      m_curl.mkcol_no_headers(path, http_status);
    }

    net::curl &m_curl;
  };


  struct stats {
    stats() : m_number_sleeps(0), m_time_spent_sleeping(0),
              m_number_open_ro(0), m_time_spent_in_open_ro(0),
              m_number_open_ro_timeouts(0),
              m_number_create_wo(0), m_time_spent_in_create_wo(0),
              m_number_create_wo_failures(0),
              m_number_close(0), m_time_spent_in_close(0),
              m_number_close_failures(0),
              m_number_erase(0), m_time_spent_in_erase(0),
              m_number_erase_failures(0),
              m_number_mtime(0), m_time_spent_in_mtime(0),
              m_number_mtime_failures(0),
              m_number_exists(0), m_time_spent_in_exists(0) {}
    uint64_t m_number_sleeps;
    uint64_t m_time_spent_sleeping;
    uint64_t m_number_open_ro;
    uint64_t m_time_spent_in_open_ro;
    uint64_t m_number_open_ro_timeouts;
    uint64_t m_number_create_wo;
    uint64_t m_time_spent_in_create_wo;
    uint64_t m_number_create_wo_failures;
    uint64_t m_number_close;
    uint64_t m_time_spent_in_close;
    uint64_t m_number_close_failures;
    uint64_t m_number_erase;
    uint64_t m_time_spent_in_erase;
    uint64_t m_number_erase_failures;
    uint64_t m_number_mtime;
    uint64_t m_time_spent_in_mtime;
    uint64_t m_number_mtime_failures;
    uint64_t m_number_exists;
    uint64_t m_time_spent_in_exists;
  };

private:
  std::string m_local_root;
  std::string m_remote_root;
  stats m_stats;
  net::curl m_curl;
};

} // namespaces
}




#endif
