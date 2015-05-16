// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_IO_FILE_HPP
#define NP1_IO_FILE_HPP



#include "np1/simple_types.hpp"
#include "np1/io/detail/stream_helper.hpp"
#include "np1/preproc.hpp"
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/syscall.h>
#include "rstd/string.hpp"
#include "np1/str.hpp"



#include <limits.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/file.h>
#endif



namespace np1 {
namespace io {

/// A class that deals with lowest-level file functions. 
class file : public detail::stream_helper<file> {
public:
  // Wrapping classes can use this to ensure that the thing they are wrapping
  // is unbuffered.
  struct is_unbuffered {};

public:
#ifdef _WIN32
  typedef HANDLE handle_type;
#else
  typedef int handle_type;
#endif

public:
  /// Default constructor.
  file() : detail::stream_helper<file>(*this) { m_handle = invalid_handle_value(); }

  /// Automatically close if not a std handle.
  ~file() {
    close();        
  }

  /// Returns false on error.
  bool create_or_open_wo_trunc(const char *file_name) {
    close();
#ifdef _WIN32
    m_handle = CreateFileA(file_name, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
                            FILE_ATTRIBUTE_NORMAL, NULL);
#else
    m_handle = open(file_name, O_CREAT | O_WRONLY | O_TRUNC, 0644);
#endif
    if (invalid_handle_value() != m_handle) {
      m_file_name = file_name;
      return true;
    }

    return false;
  }

  /// Returns false on error.
  bool create_or_open_wo_append(const char *file_name) {
    close();
#ifdef _WIN32
#error Not implemented on Windows.
#else
    m_handle = open(file_name, O_CREAT | O_WRONLY, 0644);
    if (invalid_handle_value() != m_handle) {
      if (lseek(m_handle, 0, SEEK_END) == (off_t)-1) {
        close();
      }
    }
#endif

    if (invalid_handle_value() != m_handle) {
      m_file_name = file_name;
      return true;
    }

    return false;
  }


  /// Returns false on error.
  bool open_wo(const char *file_name) {
    close();
#ifdef _WIN32
    m_handle = CreateFileA(file_name, GENERIC_WRITE, 0, NULL, OPEN_EXISTING,
                            FILE_ATTRIBUTE_NORMAL, NULL);
#else
    m_handle = open(file_name, O_WRONLY, 0);
#endif

    if (invalid_handle_value() != m_handle) {
      m_file_name = file_name;
      return true;
    }

    return false;
  }

  /// Returns false on error.
  bool create_wo(const char *file_name) {
    close();
#ifdef _WIN32
    m_handle = CreateFileA(file_name, GENERIC_WRITE, 0, NULL, CREATE_NEW,
                            FILE_ATTRIBUTE_NORMAL, NULL);
#else
    m_handle = open(file_name, O_CREAT | O_EXCL | O_WRONLY, 0644);
#endif

    if (invalid_handle_value() != m_handle) {
      m_file_name = file_name;
      return true;
    }

    return false;
  }


  /// Returns false on error.
  bool open_ro(const char *file_name) {
    close();
#ifdef _WIN32
    m_handle = CreateFileA(file_name, GENERIC_READ, FILE_SHARE_WRITE,
                            NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
#else
    m_handle = open(file_name, O_RDONLY, 0);
#endif

    if (invalid_handle_value() != m_handle) {
      m_file_name = file_name;
      return true;
    }

    return false;
  }

  /// Returns false on error.
  bool open_rw(const char *file_name) {
    close();
#ifdef _WIN32
    m_handle = CreateFileA(file_name, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_NONE,
                            NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
#else
    m_handle = open(file_name, O_RDWR, 0);
#endif

    if (invalid_handle_value() != m_handle) {
      m_file_name = file_name;
      return true;
    }

    return false;
  }


  // Initialize from std handle.
  void from_stdin() { from_handle(stdin_handle()); m_file_name = "[stdin]"; }
  void from_stdout() { from_handle(stdout_handle()); m_file_name = "[stdout]"; }
  void from_stderr() { from_handle(stderr_handle()); m_file_name = "[stderr]"; }

  // Initialize from another handle.
  void from_handle(handle_type h) { close(); m_handle = h; m_file_name = "[handle]"; }
  void from_handle(FILE *fp) {
    close();
#ifdef _WIN32
#error not defined on Windows.
#else
    m_handle = fileno(fp);
#endif
    m_file_name = "[FILE*]";
  }

  // Get the underlying handle- use sparingly.
  handle_type handle() { return m_handle; }

  /// Is the file open?
  bool is_open() const { return (m_handle != invalid_handle_value()); }


  /**
   * Returns false and sets *bytes_read_p=0 on error.  Returns true
   * and sets *bytes_read_p=0 on EOF.
   */
  bool read_some(void *buf, size_t bytes_to_read, size_t *bytes_read_p) {
#ifdef _WIN32
    if (bytes_to_read > NP1_DWORD_MAX) {
      bytes_to_read = NP1_DWORD_MAX;
    }
  
    DWORD bytes_read;
    BOOL result = ReadFile(m_handle, buf, bytes_to_read, &bytes_read, NULL);
    if (!result) {
      *bytes_read_p = 0;
    } else {
      *bytes_read_p = bytes_read;
    }
  
    return !!result;
#else
    if (bytes_to_read > NP1_SSIZE_T_MAX) {
      bytes_to_read = NP1_SSIZE_T_MAX;
    }
  
    ssize_t bytes_read = ::read(m_handle, buf, bytes_to_read);
    if (bytes_read < 0) {
      *bytes_read_p = 0;
      return false;
    }
  
    *bytes_read_p = bytes_read;
    return true;
#endif
  }


  /// Returns false and sets *bytes_written_p=0 on error. 
  bool write_some(const void *buf, size_t bytes_to_write, size_t *bytes_written_p) {
#ifdef _WIN32
    if (bytes_to_write > NP1_DWORD_MAX) {
      bytes_to_write = NP1_DWORD_MAX;
    }
  
    DWORD bytes_written;
    BOOL result = WriteFile(m_handle, buf, bytes_to_write, &bytes_written, NULL);
    if (!result) {
      *bytes_written_p = 0;
    } else {
      *bytes_written_p = bytes_written;
    }
  
    return !!result;
#else
    if (bytes_to_write > NP1_SSIZE_T_MAX) {
      bytes_to_write = NP1_SSIZE_T_MAX;
    }
  
    ssize_t bytes_written = ::write(m_handle, buf, bytes_to_write);
    if (bytes_written < 0) {
      *bytes_written_p = 0;
      return false;
    }
  
    *bytes_written_p = bytes_written;
    return true;
#endif
  }

  /// Go back to the start of the stream.
  bool rewind() {
#ifdef _WIN32
#error need to define rewind for win32
#else
    return (lseek(m_handle, 0, SEEK_SET) != (off_t)-1);
#endif
  }


  /// Just to satisfy the "stream" concept.
  bool soft_flush() {
    return true;
  }

  /**
   * Flush the file.  This will be an fsync-style flush-to-disk-controller-or-disk
   * flush.  It's not guaranteed that the data has hit the actual device when
   * this returns, but it's as close as we can get across all supported OSs and
   * disks.  Returns false on error.
   */
  bool hard_flush() {
#ifdef _WIN32
    return !!FlushFileBuffers(m_handle);
#else
    return (fsync(m_handle) == 0);
#endif
  }

  bool lock_exclusive() {
#ifdef _WIN32
#error Define the locking primitive for win32
#else
    return (::flock(m_handle, LOCK_EX) == 0);
#endif
  }

  bool unlock() {
#ifdef _WIN32
#error Define the locking primitive for win32
#else
    return (::flock(m_handle, LOCK_UN) == 0);
#endif    
  }

  /// Close the file handle, noop if file handle is invalid or a std handle.
  bool close() {
    bool result = true;
    if (!is_std() && (m_handle != invalid_handle_value())) {
#ifdef _WIN32
      result = CloseHandle(m_handle);
#else
      result = (::close(m_handle) == 0);
#endif
      m_handle = invalid_handle_value();
      m_file_name = rstd::string();
    }

    return result;
  }

  /// Release the handle so that it's not closed on destruction.  This will
  /// set the handle to invalid_handle_value().
  void release() {
    m_handle = invalid_handle_value();  
  }

  /// Get the mtime in microseconds, size in bytes, and whether or not this file is actually a directory.
  static bool get_info(const char *file_name, uint64_t &mtime, uint64_t &sz, bool &is_directory) {
#ifdef NP1_NEED_FSTAT_AS_SYSCALL
    struct stat64 stat_buf;
    int syscall_id;
#if __WORDSIZE == 64
    syscall_id = SYS_stat;
#else
    syscall_id = SYS_stat64;
#endif
    if (syscall(syscall_id, file_name, &stat_buf) != 0) {
      return false;
    }
    
    mtime = stat_buf.st_mtime;
    sz = stat_buf.st_size;
    is_directory = !!S_ISDIR(stat_buf.st_mode);
#else
    struct stat stat_buf;
    if (stat(file_name, &stat_buf) != 0) {
      return false;
    }

    mtime = stat_buf.st_mtime;
    sz = stat_buf.st_size;
    is_directory = !!S_ISDIR(stat_buf.st_mode);
#endif
    mtime *= 1000000;
    return true;

  }

  /// Get the mtime in microseconds, returns false on failure.
  static bool get_mtime_usec(const char *file_name, uint64_t &mtime) {
    uint64_t sz;
    bool is_directory;
    return get_info(file_name, mtime, sz, is_directory);
  }


  /// Get the file size in bytes, returns false on error.
  static bool get_size(const char *file_name, uint64_t &sz) {
    uint64_t mtime;
    bool is_directory;
    return get_info(file_name, mtime, sz, is_directory);
  }


  /// Returns true if the file exists.
  static bool exists(const char *file_name) {
    uint64_t sz;
    return get_size(file_name, sz);
  }


  /// Delete the supplied file, returns false on error.
  static bool erase(const char *file_name) {
#ifdef _WIN32
    return DeleteFileA(file_name);
#else
    return (unlink(file_name) == 0);
#endif
  }


  /// Create a directory, returns false on error.
  static bool mkdir(const char *directory_name) {
#ifdef _WIN32
    return CreateDirectoryA(directory_name, NULL);
#else
    return (::mkdir(directory_name, 0755) == 0);
#endif
  }


  /// Rename a file.
  static bool rename(const char *from, const char *to) {
    return (::rename(from, to) == 0);
  }

  /// Get the directory component of a file name.  Returns a string that means "the current directory" if there
  /// is no directory component.  The return value includes the trailing slash.
  static rstd::string parse_directory(const rstd::string &file_name) {
#ifdef _WIN32
#error parse_directory not implemented for Windows
#else
    const char *last_slash = str::find_last(file_name, '/');
    if (!last_slash) {
      return rstd::string("./");
    }
    
    return rstd::string(file_name, last_slash - file_name.c_str() + 1); // include the /
#endif    
  }
  
  /// Is the file actually a std file?
  bool is_std() {
    return ((stdout_handle() == m_handle)
            || (stdin_handle() == m_handle)
            || (stderr_handle() == m_handle));
  }

  const rstd::string &name() const { return m_file_name; }

private:
  /// Disable copy.
  file(const file &);
  file &operator = (const file &);

private:
  /// Get an invalid handle value.
  static inline handle_type invalid_handle_value() {
#ifdef _WIN32
    return INVALID_HANDLE_VALUE;
#else
    return -1;
#endif
  }


  /// Get a std handle. 
  inline handle_type stdin_handle() {
#ifdef _WIN32
    return GetStdHandle(STD_INPUT_HANDLE);
#else
    return 0;
#endif
  }

  inline handle_type stdout_handle() {
#ifdef _WIN32
    return GetStdHandle(STD_OUTPUT_HANDLE);
#else
    return 1;
#endif
  }

  inline handle_type stderr_handle() {
#ifdef _WIN32
    return GetStdHandle(STD_ERROR_HANDLE);
#else
    return 2;
#endif
  }


private:
  handle_type m_handle;
  rstd::string m_file_name;
};


} // namespaces
}

#endif
