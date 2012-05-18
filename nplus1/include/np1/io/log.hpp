// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_IO_LOG_HPP
#define NP1_IO_LOG_HPP


#include "np1/global_info.hpp"
#include "np1/io/static_buffer_output_stream.hpp"
#include <syslog.h>


namespace np1 {
namespace io {


class log {
public:
  typedef enum {
    SEVERITY_INFO,
    SEVERITY_WARNING,
    SEVERITY_ERROR
  } severity_type;

  enum { MAX_LOG_ENTRY_LENGTH = 8192 };

public:
  template <typename... Arguments>
  static void info(const char *log_id, const Arguments& ...arguments) {    
    do_log(SEVERITY_INFO, log_id, arguments...);
  }

private:
  template <typename... Arguments>
  static void do_log(severity_type severity, const char *log_id,
                      const Arguments& ...arguments) {
    write_to_buffer(severity, log_id, arguments...);    
    write_to_log((const char *)buffer_output_stream().ptr());    
  }


  static void write_to_log(const char *completed_string) {
    static bool is_log_open = false;
    if (!is_log_open) {
      openlog("r17", LOG_PID, LOG_USER);
      is_log_open = true;
    }

    // None of our messages are system-critical.
    syslog(LOG_INFO, "%s", completed_string);
  }


  template <typename... Arguments>
  static void write_to_buffer(severity_type severity, const char *log_id,
                              const Arguments& ...arguments) {
    buffer_output_stream().reset();
    write_info_if_available(global_info::stream_op_name());
    write_info_if_available(global_info::listening_endpoint());
    write(log_id);
    write(": ");

    switch (severity) {
    case SEVERITY_INFO:
      buffer_output_stream().write("info: ");
      break;

    case SEVERITY_WARNING:
      buffer_output_stream().write("warning: ");
      break;

    case SEVERITY_ERROR:
      buffer_output_stream().write("error: ");
      break;
    }

    write(arguments...);
    if (!buffer_output_stream().write('\0')) {
      buffer_output_stream().set_last_char('\0');
    }
  }
  

  template <typename T, typename... Arguments>
  static void write(const T &t, const Arguments&... arguments) {
    write_atom(t);
    write(arguments...);
  }

  static void write_atom(char c) {
    buffer_output_stream().write(c);
  }

  static void write_atom(bool b) {
    write_atom(b ? "true" : "false");
  }

  static void write_atom(const char *s) {
    buffer_output_stream().write(s);
  }

  // We need to do our own integer conversion so that the NP1_ASSERT stuff
  // (which is used by the string handling) can call this.
  static void write_atom(unsigned long long ui) {
    char num_str[64];
#ifdef _WIN32
    sprintf(num_str, "%I64u", ui);
#else
    sprintf(num_str, "%llu", (unsigned long long)ui);
#endif

    write(num_str);
  }

  static void write_atom(long long i) {
    char num_str[64];
#ifdef _WIN32
    sprintf(num_str, "%I64d", i);
#else
    sprintf(num_str, "%lld", (long long)i);
#endif
    write(num_str);
  }

  static void write_atom(unsigned long ui) {
    char num_str[64];
    sprintf(num_str, "%lu", ui);
    write(num_str);
  }

  static void write_atom(long i) {
    char num_str[64];
    sprintf(num_str, "%ld", i);
    write(num_str);
  }


  static void write_atom(unsigned int ui) {
    char num_str[64];
    sprintf(num_str, "%u", ui);
    write(num_str);
  }

  static void write_atom(double d) {
    char num_str[64];
    sprintf(num_str, "%g", d);
    write(num_str);
  }

  static void write() {
    // Nothing to do.  
  }


  static void write_info_if_available(const char *s) {
    if (s) {
      write(s);
      write(": ");
    }
  }

  static static_buffer_output_stream<MAX_LOG_ENTRY_LENGTH> &buffer_output_stream() {
    static static_buffer_output_stream<MAX_LOG_ENTRY_LENGTH> sbos;
    return sbos;
  }


};

} // namespaces
}



#endif
