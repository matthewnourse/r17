// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_IO_PATH_HPP
#define NP1_IO_PATH_HPP


#include "np1/str.hpp"
#include "np1/io/string_input_stream.hpp"
#include "np1/io/text_input_stream.hpp"
#include "np1/io/file.hpp"

namespace np1 {
namespace io {

/// A path, just like the shell's executable path.
class path {
public:
  explicit path(const rstd::string &s) {
    string_input_stream s_in(s);
    text_input_stream<string_input_stream> t_in(s_in);
    t_in.read_all(m_directories, ':');
    rstd::vector<rstd::string>::iterator ii = m_directories.begin();
    while (ii != m_directories.end()) {
      *ii = str::trim(*ii);
      if (ii->length() <= 0) {
        m_directories.erase(ii);
      } else {
        ++ii;
      }
    }
  }

  // Return the empty string if the file could not be found.
  rstd::string find(const rstd::string &file_name) const {
    rstd::vector<rstd::string>::const_iterator ii = m_directories.begin();
    rstd::vector<rstd::string>::const_iterator iz = m_directories.end();
    for (; ii != iz; ++ii) {
      rstd::string full_path = ((*ii) + "/" + file_name);
      if (file::exists(full_path.c_str())) {
        return full_path;
      }
    }
    
    rstd::string empty;
    return empty;
  }
  
  void push_front(const rstd::string &directory) {
    m_directories.push_front(directory);
  }
  
  const rstd::vector<rstd::string> &directories() const {
    return m_directories;
  }
  
  rstd::string to_string() const {
    rstd::string result;
    rstd::vector<rstd::string>::const_iterator ii = m_directories.begin();
    rstd::vector<rstd::string>::const_iterator iz = m_directories.end();
    for (; ii != iz; ++ii) {
      if (result.length() > 0) {
        result.push_back(':');  
      }
      
      result.append(*ii);
    }
    
    return result;
  }

private:
  rstd::vector<rstd::string> m_directories;
};


} // namespaces
}

#endif
