// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_ASSERT_HPP
#define NP1_ASSERT_HPP

#include "np1/global_info.hpp"
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>


/*TODO: compare the performance of this against assert().
 * I believe that this is now the same implementation as assert() in the
 * GCC standard library.  The details of the implementation (eg, the void cast
 * or lack thereof) have a significant impact on performance. */
#define NP1_ASSERT(must_be_true__, crash_explanation__) \
(void)((must_be_true__) ? (void)0 : ::np1::assert::crash(#must_be_true__, crash_explanation__))

namespace np1 {
namespace assert {

namespace detail {

#define NP1_MAX_NUMBER_C_STACK_BACKTRACE_RETURN_ADDRESSES ((size_t)100)


/* Use our own to-string conversion functions to avoid any issues where a string
   conversion function calls NP1_ASSERT. */
void assert_safe_str_uint64_to_hex_str(char *num_str, uint64_t ui) {
#ifdef _WIN32
  sprintf(num_str, "%I64x", ui);
#else
  sprintf(num_str, "%llx", (unsigned long long)ui);
#endif
}

void assert_safe_str_uint64_to_dec_str(char *num_str, uint64_t ui) {
#ifdef _WIN32
  sprintf(num_str, "%I64u", ui);
#else
  sprintf(num_str, "%llu", (unsigned long long)ui);
#endif
}


// Use our own file-writing functions to avoid a dependency on io::file.
bool assert_safe_real_write_some(const void *buf, size_t bytes_to_write, size_t *bytes_written_p) {
#ifdef _WIN32
  HANDLE handle = return GetStdHandle(STD_ERROR_HANDLE);

  if (bytes_to_write > NP1_DWORD_MAX) {
    bytes_to_write = NP1_DWORD_MAX;
  }

  DWORD bytes_written;
  BOOL result = WriteFile(handle, buf, bytes_to_write, &bytes_written, NULL);
  if (!result) {
    *bytes_written_p = 0;
  } else {
    *bytes_written_p = bytes_written;
  }

  return !!result && FlushFileBuffers(handle);
;
#else
  int handle = 2;

  if (bytes_to_write > NP1_SSIZE_T_MAX) {
    bytes_to_write = NP1_SSIZE_T_MAX;
  }

  ssize_t bytes_written = ::write(handle, buf, bytes_to_write);
  if (bytes_written < 0) {
    *bytes_written_p = 0;
    return false;
  }

  *bytes_written_p = bytes_written;
  return (fsync(handle) == 0);
#endif
}


/// Writes all the buffer or returns false. 
bool assert_safe_real_write(const void *buf, size_t bytes_to_write) {
  unsigned char *p = (unsigned char *)buf;
  unsigned char *end = p + bytes_to_write;

  while (p < end) {
    size_t bytes_written;
    if (!assert_safe_real_write_some(p, end - p, &bytes_written)) {
      return false;
    }

    p += bytes_written;
  }

  return true;
}

unsigned char global_assert_buffer[4096];
unsigned char *global_assert_buffer_p = global_assert_buffer;
unsigned char *global_assert_buffer_end = global_assert_buffer_p + sizeof(global_assert_buffer);

void assert_safe_buffer_flush() {
  assert_safe_real_write(global_assert_buffer, global_assert_buffer_p - global_assert_buffer);
  global_assert_buffer_p = global_assert_buffer;
}


void assert_safe_buffered_write(const void *data, size_t bytes_to_write) {
  if (bytes_to_write > (size_t)(global_assert_buffer_end - global_assert_buffer_p)) {
    assert_safe_buffer_flush();
    assert_safe_real_write(data, bytes_to_write);
  } else {
    memcpy(global_assert_buffer_p, data, bytes_to_write);
    global_assert_buffer_p += bytes_to_write;
  }
}



void assert_safe_write(const char *s) {
  assert_safe_buffered_write(s, strlen(s));
}



#if defined(_WIN32) || !defined(NP1_DEBUG)
static void write_stack_trace() {}
#else
/* This stack tracer works only with GCC. */
#include <execinfo.h>
static void write_stack_trace() {
  void *backtrace_return_addresses[
          NP1_MAX_NUMBER_C_STACK_BACKTRACE_RETURN_ADDRESSES];

  int number_backtrace_return_addresses;

  /* Get the backtrace. */
  number_backtrace_return_addresses =
      backtrace(backtrace_return_addresses,
                NP1_MAX_NUMBER_C_STACK_BACKTRACE_RETURN_ADDRESSES);

  /* The gnu backtrace_symbols_fd stuff doesn't supply function names
     unless you link with -rdynamic, which is expensive.  So we'll just
     direct the user to use addr2line. */
  assert_safe_write("addr2line command line:\n");
  assert_safe_write("addr2line -e [path to r17 exe] ");
  int i;
  for (i = 0; (i < number_backtrace_return_addresses); ++i) {
    assert_safe_write(" ");
    char addr_str[32];
    assert_safe_str_uint64_to_hex_str(addr_str,
                                      (size_t)backtrace_return_addresses[i]);
    assert_safe_write(addr_str);
  }

  assert_safe_write("\n");
}
#endif

} // namespace detail.



void write_pre_crash_message(const char *failed_assertion_text, const char *explanation) {
  if (global_info::stream_op_name()) {
    detail::assert_safe_write("In op '");
    detail::assert_safe_write(global_info::stream_op_name());
    detail::assert_safe_write("'");

    if (global_info::stream_op_script_file_name()) {
      detail::assert_safe_write(" called at ");
      detail::assert_safe_write(global_info::stream_op_script_file_name());
      detail::assert_safe_write(":");
      char num_str[64];
      detail::assert_safe_str_uint64_to_dec_str(num_str, global_info::stream_op_script_line_number());
      detail::assert_safe_write(num_str);
    }

    if (strlen(global_info::listening_endpoint()) > 0) {
      detail::assert_safe_write(" listening on '");
      detail::assert_safe_write(global_info::listening_endpoint());
      detail::assert_safe_write("'");
    }

    detail::assert_safe_write("  ");
  }

  char pid_str[64];
  detail::assert_safe_str_uint64_to_dec_str(pid_str, getpid());
  detail::assert_safe_write("pid: ");
  detail::assert_safe_write(pid_str);
  detail::assert_safe_write("\nError: ");
#ifdef NP1_DEBUG
  detail::assert_safe_write("Assertion: ");
  detail::assert_safe_write(failed_assertion_text);
  detail::assert_safe_write("\n");
#endif
  detail::assert_safe_write(explanation);
  detail::assert_safe_write("\n");
  detail::write_stack_trace();
  detail::assert_safe_write("\n");

  detail::assert_safe_buffer_flush();
}

/// Crash unconditionally.
void crash(const char *failed_assertion_text, const char *explanation) {
  write_pre_crash_message(failed_assertion_text, explanation);
  global_info::pre_crash_handlers_call("Assertion failure");
  exit(-1);
}

void crash(const char *failed_assertion_text, char *explanation) {
  crash(failed_assertion_text, (const char *)explanation);
}


template <typename String>
void crash(const char *failed_assertion_text, const String &s) {
  crash(failed_assertion_text, s.c_str());
}


} // namespaces 
}


#endif
