// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_IO_FILE_MAPPING_HPP
#define NP1_IO_FILE_MAPPING_HPP


#include "np1/io/file.hpp"


#ifndef _WIN32
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/mman.h>
#endif


namespace np1 {
namespace io {

/// A mapping of a file into RAM.
class file_mapping {
public:
  // Construct from an open file handle, crash the process on error.
  // Note that this does NOT take ownership of the handle!
  explicit file_mapping(file::handle_type h, bool read_only = true)
    : m_ptr(0), m_size(0) {
    initialize(h, read_only);
  }

  explicit file_mapping(FILE *fp, bool read_only = true)
    : m_ptr(0), m_size(0) {
    file::handle_type h;
#ifdef _WIN32
#error not implemented on Windows yet!
#else
    h = fileno(fp);
#endif
    
    initialize(h, read_only);
  }

  ~file_mapping() { unmap(); }

  void *ptr() { return m_ptr; }
  const void *ptr() const { return m_ptr; }
  size_t size() const { return m_size; }

  void unmap() {    
#ifdef _WIN32
#error not implemented on Windows yet!
#else
    if (m_ptr) {
      munmap(m_ptr, m_size);    
    }
#endif
  }

private:
  /// Disable copy.
  file_mapping(const file_mapping &);
  file_mapping &operator = (const file_mapping &);

private:
  void initialize(file::handle_type h, bool read_only) {
    size_t map_size = get_file_size(h);

    int prot = PROT_READ;
    if (!read_only) {
      prot |= PROT_WRITE;
    }

    void *p = mmap(0, map_size, prot, MAP_SHARED, h, 0);
    NP1_ASSERT(p, "Unable to map file");
    m_ptr = p;
    m_size = map_size;
  }


  size_t get_file_size(file::handle_type h) {
    int result;
    uint64_t file_size;

    // On Linux, fstat is a deliberately-unresolvable symbol that's resolved by
    // some compiler/glibc juju.  The only way that I can get it to link in
    // debug mode is by linking with libstdc++, which we don't want to do
    // for licensing reasons.  So we need to make the kernel syscall directly.
#ifdef NP1_NEED_FSTAT_AS_SYSCALL
    struct stat64 stat_buf;
    int syscall_id;
#if __WORDSIZE == 64
    syscall_id = SYS_fstat;
#else
    syscall_id = SYS_fstat64;
#endif
    result = syscall(syscall_id, h, &stat_buf);
    NP1_PREPROC_STATIC_ASSERT(sizeof(uint64_t) >= sizeof(stat_buf.st_size));
    file_size = stat_buf.st_size;
#else
    struct stat stat_buf;
    result = fstat(h, &stat_buf);
    NP1_PREPROC_STATIC_ASSERT(sizeof(uint64_t) >= sizeof(stat_buf.st_size));
    file_size = stat_buf.st_size;
#endif

    NP1_ASSERT(0 == result, "Unable to get size of file for mapping");

    // On 32-bit linux (+others?) off_t is only 32 bits wide unless we do
    // some mucking around with special #defines, which I'm scared to do because
    // we're bypassing fstat for Linux.  But we won't be
    // able to map more than 2GB on most 32-bit systems anyway so it's moot.
    
    NP1_ASSERT(file_size <= NP1_SSIZE_T_MAX, "File is too large to be mapped");
    return (size_t)file_size;
  }

private:
  void *m_ptr;  
  size_t m_size;
};


} // namespaces
}



#endif
