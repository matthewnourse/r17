// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_IO_NAMED_TEMP_FILE_HPP
#define NP1_IO_NAMED_TEMP_FILE_HPP



#include "np1/io/file.hpp"




namespace np1 {
namespace io {

/// A temporary file that's viewable to the outside world. 
class named_temp_file {
private:
  struct temp_file_info {
    temp_file_info(const char *file_name, int fd) : m_file_name(file_name), m_fd(fd) {}
    const rstd::string m_file_name;
    const int m_fd;
  };

public:
  named_temp_file() : m_info(create_temp_file()), m_pch(m_info.m_file_name) {
    global_info::pre_crash_handler_push(&m_pch);
    m_file.from_handle(m_info.m_fd);
  }

  ~named_temp_file() {
    io::file::erase(m_info.m_file_name.c_str());
    global_info::pre_crash_handler_pop();        
  }

  const rstd::string &file_name() const { return m_info.m_file_name; }
  file &real_file() { return m_file; }

private:
  /// Disable copy.
  named_temp_file(const named_temp_file &);
  named_temp_file &operator = (const named_temp_file &);

private:
  static temp_file_info create_temp_file() {
    char temp_file_name[256];
    strcpy(temp_file_name, "/tmp/tmpXXXXXX");
    int temp_fd;
    NP1_ASSERT(((temp_fd = mkstemp(temp_file_name)) != -1), "Unable to create named temporary file");
    return temp_file_info(temp_file_name, temp_fd);
  }

  struct temp_file_erase_pre_crash_handler : public global_info::pre_crash_handler {
    temp_file_erase_pre_crash_handler(const rstd::string &temp_file_name) : m_file_name(temp_file_name) {}

    virtual void call(const char *crash_msg) {
      io::file::erase(m_file_name.c_str());
    }

    rstd::string m_file_name;
  };

private:
  const temp_file_info m_info;
  file m_file;
  temp_file_erase_pre_crash_handler m_pch;
};


} // namespaces
}

#endif
