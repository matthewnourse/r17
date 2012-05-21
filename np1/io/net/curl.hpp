// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_IO_NET_CURL_HPP
#define NP1_IO_NET_CURL_HPP


#include "np1/str.hpp"
#include "np1/io/ext_heap_buffer_output_stream.hpp"
#include <curl/curl.h>


namespace np1 {
namespace io {
namespace net {

// A thin wrapper around libcurl.
class curl {
private:
  enum { EXPECTED_MAX_CONTENT_SIZE = 128 * 1024 };

public:
  curl() : m_handle(curl_easy_init()) { NP1_ASSERT(m_handle, "Unable to create cURL handle."); }
  ~curl() { curl_easy_cleanup(m_handle); }

  template <typename Output>
  bool get(const rstd::string &url, Output &output) {
    curl_easy_setopt(m_handle, CURLOPT_URL, url.c_str()); 
    curl_easy_setopt(m_handle, CURLOPT_NOPROGRESS, 1L);
    curl_easy_setopt(m_handle, CURLOPT_NOSIGNAL, 1L);
    curl_easy_setopt(m_handle, CURLOPT_WRITEFUNCTION, write_data_container<Output>::write_data);
    curl_easy_setopt(m_handle, CURLOPT_WRITEDATA, &output);
    curl_easy_setopt(m_handle, CURLOPT_WRITEHEADER, write_data_container<Output>::write_data);
    curl_easy_setopt(m_handle, CURLOPT_HEADERDATA, &output);
    CURLcode result = curl_easy_perform(m_handle);
    curl_easy_reset(m_handle);

    return (0 == result);
  }

  template <typename Heap>
  bool get_utf8_safe(Heap &h, const rstd::string &url, char replacement_char, char **result_str,
                      size_t *result_str_length_p) {
    ext_heap_buffer_output_stream<Heap> output(h, EXPECTED_MAX_CONTENT_SIZE);
    if (!get(url, output)) {
      return false;
    }

    str::replace_invalid_utf8_sequences((char *)output.ptr(), output.size(), replacement_char);

    *result_str = (char *)output.ptr();
    *result_str_length_p = output.size();
    output.release();
    return true;
  }

  // Returns false for HTTP status other than 200.
  template <typename Output>
  bool get_no_headers(const rstd::string &url, Output &output, long &http_status) {
    curl_easy_setopt(m_handle, CURLOPT_URL, url.c_str()); 
    curl_easy_setopt(m_handle, CURLOPT_NOPROGRESS, 1L);
    curl_easy_setopt(m_handle, CURLOPT_NOSIGNAL, 1L);
    curl_easy_setopt(m_handle, CURLOPT_WRITEFUNCTION, write_data_container<Output>::write_data);
    curl_easy_setopt(m_handle, CURLOPT_WRITEDATA, &output);
    curl_easy_setopt(m_handle, CURLOPT_WRITEHEADER, noop_write_data);
    curl_easy_setopt(m_handle, CURLOPT_HEADERDATA, 0);
    CURLcode result = curl_easy_perform(m_handle);
    http_status = 501;
    if (0 == result) {
      if (curl_easy_getinfo(m_handle, CURLINFO_RESPONSE_CODE, &http_status) != CURLE_OK) {
        http_status = 500;
      }
    }

    curl_easy_reset(m_handle);

    return ((0 == result) && (200 == http_status));
  }
  

  template <typename Input>
  bool put_no_headers(const rstd::string &url, Input &input, long &http_status) {
    curl_easy_setopt(m_handle, CURLOPT_UPLOAD, 1L);
    
    single_entry_curl_slist header_list("Transfer-Encoding: chunked");
    curl_easy_setopt(m_handle, CURLOPT_HTTPHEADER, header_list.m_list);

    curl_easy_setopt(m_handle, CURLOPT_URL, url.c_str()); 
    curl_easy_setopt(m_handle, CURLOPT_NOPROGRESS, 1L);
    curl_easy_setopt(m_handle, CURLOPT_NOSIGNAL, 1L);
    curl_easy_setopt(m_handle, CURLOPT_READFUNCTION, read_data_container<Input>::read_data);
    curl_easy_setopt(m_handle, CURLOPT_READDATA, &input);
    curl_easy_setopt(m_handle, CURLOPT_WRITEFUNCTION, noop_write_data);
    curl_easy_setopt(m_handle, CURLOPT_WRITEDATA, 0);
    curl_easy_setopt(m_handle, CURLOPT_WRITEHEADER, noop_write_data);
    curl_easy_setopt(m_handle, CURLOPT_HEADERDATA, 0);
    CURLcode result = curl_easy_perform(m_handle);
    http_status = 501;
    if (0 == result) {
      if (curl_easy_getinfo(m_handle, CURLINFO_RESPONSE_CODE, &http_status) != CURLE_OK) {
        http_status = 500;
      }
    }

    curl_easy_reset(m_handle);

    return ((0 == result) && (http_status >= 200) && (http_status < 300));    
  }


  bool delete_no_headers(const rstd::string &url, long &http_status) {
    curl_easy_setopt(m_handle, CURLOPT_URL, url.c_str()); 
    curl_easy_setopt(m_handle, CURLOPT_CUSTOMREQUEST, "DELETE"); 
    curl_easy_setopt(m_handle, CURLOPT_NOPROGRESS, 1L);
    curl_easy_setopt(m_handle, CURLOPT_NOSIGNAL, 1L);
    curl_easy_setopt(m_handle, CURLOPT_WRITEFUNCTION, noop_write_data);
    curl_easy_setopt(m_handle, CURLOPT_WRITEDATA, NULL);
    curl_easy_setopt(m_handle, CURLOPT_WRITEHEADER, noop_write_data);
    curl_easy_setopt(m_handle, CURLOPT_HEADERDATA, NULL);
    CURLcode result = curl_easy_perform(m_handle);
    http_status = 501;
    if (0 == result) {
      if (curl_easy_getinfo(m_handle, CURLINFO_RESPONSE_CODE, &http_status) != CURLE_OK) {
        http_status = 500;
      }
    }

    curl_easy_reset(m_handle);

    return ((0 == result) && (((http_status >= 200) && (http_status < 300)) || (404 == http_status)));
  }


  bool mkcol_no_headers(const rstd::string &url, long &http_status) {
    curl_easy_setopt(m_handle, CURLOPT_URL, url.c_str()); 
    curl_easy_setopt(m_handle, CURLOPT_CUSTOMREQUEST, "MKCOL"); 
    curl_easy_setopt(m_handle, CURLOPT_NOPROGRESS, 1L);
    curl_easy_setopt(m_handle, CURLOPT_NOSIGNAL, 1L);
    curl_easy_setopt(m_handle, CURLOPT_WRITEFUNCTION, noop_write_data);
    curl_easy_setopt(m_handle, CURLOPT_WRITEDATA, NULL);
    curl_easy_setopt(m_handle, CURLOPT_WRITEHEADER, noop_write_data);
    curl_easy_setopt(m_handle, CURLOPT_HEADERDATA, NULL);
    CURLcode result = curl_easy_perform(m_handle);
    http_status = 501;
    if (0 == result) {
      if (curl_easy_getinfo(m_handle, CURLINFO_RESPONSE_CODE, &http_status) != CURLE_OK) {
        http_status = 500;
      }
    }

    curl_easy_reset(m_handle);

    return ((0 == result) && (http_status >= 200) && (http_status < 300));
  }

  bool exists(const rstd::string &url, long &http_status) {
    curl_easy_setopt(m_handle, CURLOPT_URL, url.c_str()); 
    curl_easy_setopt(m_handle, CURLOPT_NOBODY, 1L); 
    curl_easy_setopt(m_handle, CURLOPT_NOPROGRESS, 1L);
    curl_easy_setopt(m_handle, CURLOPT_NOSIGNAL, 1L);
    curl_easy_setopt(m_handle, CURLOPT_WRITEFUNCTION, noop_write_data);
    curl_easy_setopt(m_handle, CURLOPT_WRITEDATA, NULL);
    curl_easy_setopt(m_handle, CURLOPT_WRITEHEADER, noop_write_data);
    curl_easy_setopt(m_handle, CURLOPT_HEADERDATA, NULL);
    CURLcode result = curl_easy_perform(m_handle);
    http_status = 501;
    if (0 == result) {
      if (curl_easy_getinfo(m_handle, CURLINFO_RESPONSE_CODE, &http_status) != CURLE_OK) {
        http_status = 500;
      }
    }

    curl_easy_reset(m_handle);

    return ((0 == result) && (http_status >= 200) && (http_status < 300));
  }


private:
  template <typename Output>
  struct write_data_container {
    static size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream) {
      Output *output_p = (Output *)stream;
      size_t bytes_to_write = size * nmemb;
      if (!output_p->write(ptr, bytes_to_write)) {
        return 0;    
      }
      
      return bytes_to_write;
    }
  };

  static size_t noop_write_data(void *ptr, size_t size, size_t nmemb, void *stream) {
    return size * nmemb;
  }

  template <typename Input>
  struct read_data_container {
    static size_t read_data(void *ptr, size_t size, size_t nmemb, void *stream) {
      Input *input_p = (Input *)stream;
      size_t bytes_to_read = size * nmemb;
      size_t bytes_read;
      if (!input_p->read(ptr, bytes_to_read, &bytes_read)) {
        return CURL_READFUNC_ABORT;    
      }
      
      return bytes_read;
    }
  };

  struct single_entry_curl_slist {
    explicit single_entry_curl_slist(const char *s) : m_list(curl_slist_append(0, s)) {}
    ~single_entry_curl_slist() { if (m_list) { curl_slist_free_all(m_list); } }
    curl_slist *m_list;
  };

private:
  /// Disable copy.
  curl(const curl &);
  curl &operator = (const curl &);

private:
  CURL *m_handle;
};

} // namespaces
}
}



#endif
