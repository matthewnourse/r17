// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_IO_DIRECTORY_HPP
#define NP1_IO_DIRECTORY_HPP



#include "np1/simple_types.hpp"
#include <sys/types.h>
#include <dirent.h>


namespace np1 {
namespace io {

/// A class that deals with file system directories.
//TODO: this class makes several deep copies of strings because I figured that the disk is the most likely
// bottleneck, we may want to revisit that choice.
class directory {
public:
  class entry {
  public:
    entry(const rstd::string &directory_name, const char *file_name)
      : m_directory_name(directory_name), m_file_name(file_name),
        m_relative_path(directory_name + (str::ends_with(directory_name, slash_char()) ? rstd::string() : slash_char()) + file_name),
        m_size_bytes(0), m_mtime_usec(0), m_is_directory(false) {
      // If this fails then just leave the values as the defaults.
      file::get_info(m_relative_path.c_str(), m_mtime_usec, m_size_bytes, m_is_directory);
    }

    const rstd::string &directory_name() const { return m_directory_name; }  
    const rstd::string &file_name() const { return m_file_name; }
    const rstd::string &relative_path() const { return m_relative_path; }
    uint64_t size_bytes() const { return m_size_bytes; }
    uint64_t mtime_usec() const { return m_mtime_usec; }
    bool is_directory() const { return m_is_directory; }

  private:
    rstd::string m_directory_name;
    rstd::string m_file_name;
    rstd::string m_relative_path;
    uint64_t m_size_bytes;
    uint64_t m_mtime_usec;
    bool m_is_directory;
  };

  // Iterate over the directory, calling the iterator for each entry.  Returns false on failure.
  template <typename Iterator>
  static bool iterate(const rstd::string &directory_name, Iterator iter) {
    DIR *directory = opendir(directory_name.c_str());
    if (!directory) {
      return false;
    }

    struct dirent *directory_entry;
    while ((directory_entry = readdir(directory))) {
      if ((str::cmp(directory_entry->d_name, "..") != 0) && (str::cmp(directory_entry->d_name, ".") != 0)) {
        iter(entry(directory_name, directory_entry->d_name));
      }
    }

    closedir(directory);
    return true;
  }

  template <typename Iterator>
  static void mandatory_iterate(const rstd::string &directory_name, const Iterator &iter) {
    NP1_ASSERT(iterate(directory_name, iter), "Unable to iterate over directory '" + directory_name + "'");      
  }


  static const rstd::string &slash_char() {
    static rstd::string s("/");
    return s;
  }
};

} // namespace
}


#endif
