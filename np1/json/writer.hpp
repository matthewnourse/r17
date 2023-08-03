// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_JSON_WRITER_HPP
#define NP1_JSON_WRITER_HPP


#include "np1/json/tokenizer.hpp"

namespace np1 {
namespace json {
  
class writer {
public:
  template <typename Mandatory_Output_Stream>
  struct raw_token_writer_handler {
    explicit raw_token_writer_handler(Mandatory_Output_Stream &os) : m_os(os) {}
    
    void on_number(const str::ref &tok) { writer::mandatory_write_raw(m_os, tok); }
    void on_string(const str::ref &tok) { writer::mandatory_write_raw(m_os, tok); }
    void on_open_object(const str::ref &tok) { writer::mandatory_write_raw(m_os, tok); }
    void on_object_name_value_delimiter(const str::ref &tok) { writer::mandatory_write_raw(m_os, tok); }
    void on_close_object(const str::ref &tok) { writer::mandatory_write_raw(m_os, tok); }
    void on_open_array(const str::ref &tok) { writer::mandatory_write_raw(m_os, tok); }
    void on_element_delimiter(const str::ref &tok) { writer::mandatory_write_raw(m_os, tok); }
    void on_close_array(const str::ref &tok) { writer::mandatory_write_raw(m_os, tok); }
    void on_special(const str::ref &tok) { writer::mandatory_write_raw(m_os, tok); }

    Mandatory_Output_Stream &m_os;
  };

  template <typename Mandatory_Output_Stream>
  struct parsed_token_writer_handler {
    explicit parsed_token_writer_handler(Mandatory_Output_Stream &os) : m_os(os) {}

    void on_number(const str::ref &tok) { writer::mandatory_write_raw(m_os, tok); } 
    void on_string(const str::ref &tok) { writer::mandatory_write_raw(m_os, tok); }
    void on_special(const str::ref &tok) { writer::mandatory_write_raw(m_os, tok); }
    void on_open_object(const str::ref &tok) { writer::mandatory_write_raw(m_os, tok); } 
    void on_close_object(const str::ref &tok) { writer::mandatory_write_raw(m_os, tok); }
    void on_object_element_name(const str::ref &tok) { writer::mandatory_write_raw(m_os, tok); }
    void on_open_array(const str::ref &tok) { writer::mandatory_write_raw(m_os, tok); }
    void on_close_array(const str::ref &tok) { writer::mandatory_write_raw(m_os, tok); }
    void on_delimiter(const str::ref &tok) { writer::mandatory_write_raw(m_os, tok); }

    Mandatory_Output_Stream &m_os;
  };


public:
  template <typename Mandatory_Output_Stream>
  static void mandatory_write_raw(Mandatory_Output_Stream &os, const str::ref &token) {
    // The raw token is ready to write, no work to do.
    os.write(token.ptr(), token.length());
  }

  template <typename Mandatory_Output_Stream>
  static raw_token_writer_handler<Mandatory_Output_Stream> make_raw_token_writer_handler(Mandatory_Output_Stream &os) {
    return raw_token_writer_handler<Mandatory_Output_Stream>(os);
  }

  template <typename Mandatory_Output_Stream>
  static parsed_token_writer_handler<Mandatory_Output_Stream> make_parsed_token_writer_handler(Mandatory_Output_Stream &os) {
    return parsed_token_writer_handler<Mandatory_Output_Stream>(os);
  }
};


} // namespaces
}



#endif
