// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_GLOBAL_INFO_HPP
#define NP1_GLOBAL_INFO_HPP


namespace np1 {

namespace global_info_detail {

enum { MAX_STREAM_OP_NAME_LENGTH = 100,
       MAX_SCRIPT_FILE_NAME_LENGTH = 1024,
       MAX_LISTENING_ENDPOINT_LENGTH = 100,
       MAX_NUMBER_PRE_CRASH_HANDLERS = 20 };

struct stream_op_details {
  stream_op_details() : m_script_line_number(0) {
    strcpy(m_name, "[unknown]");
    strcpy(m_script_file_name, "[unknown]");
  }
  char m_name[MAX_STREAM_OP_NAME_LENGTH+1];
  char m_script_file_name[MAX_SCRIPT_FILE_NAME_LENGTH+1];
  size_t m_script_line_number;
};

} // namespace


/// Global program information.  This is used by NP1_ASSERT, so needs to be
/// minimalist.
class global_info {
public:

  //NOTE that this destructor CANNOT be virtual because it will lead to
  // "undefined reference to operator delete" linker errors.
  struct pre_crash_handler {
    virtual void call(const char *crash_msg) {}
  };
 

public:
  static void stream_op_details(const char *name, const char *script_file_name, size_t script_line_number) {
    global_info_detail::stream_op_details &details = get_stream_op_details();
    memset(details.m_name, 0, global_info_detail::MAX_STREAM_OP_NAME_LENGTH+1);
    strncpy(details.m_name, name, global_info_detail::MAX_STREAM_OP_NAME_LENGTH);

    memset(details.m_script_file_name, 0, global_info_detail::MAX_SCRIPT_FILE_NAME_LENGTH+1);
    strncpy(details.m_script_file_name, script_file_name, global_info_detail::MAX_SCRIPT_FILE_NAME_LENGTH);

    get_stream_op_details().m_script_line_number = script_line_number;
  }

  static void stream_op_details_reset() {
    get_stream_op_details() = global_info_detail::stream_op_details();
  }

  static const char *stream_op_name() { return get_stream_op_details().m_name; }

  static const char *stream_op_script_file_name() { return get_stream_op_details().m_script_file_name; }

  static size_t stream_op_script_line_number() { return get_stream_op_details().m_script_line_number; }

  static const char *listening_endpoint() { return get_listening_endpoint(); }
  static void listening_endpoint(const char *s) {
    memset(get_listening_endpoint(), 0, global_info_detail::MAX_LISTENING_ENDPOINT_LENGTH+1);
    strncpy(get_listening_endpoint(), s, global_info_detail::MAX_LISTENING_ENDPOINT_LENGTH);
  }

  static void pre_crash_handler_push(pre_crash_handler *pch) {
    return get_pre_crash_handlers().push(pch);
  }

  static pre_crash_handler *pre_crash_handler_pop() {
    return get_pre_crash_handlers().pop();
  }

  static pre_crash_handler *pre_crash_handler_top() {
    return get_pre_crash_handlers().top();
  }


private:
  static global_info_detail::stream_op_details &get_stream_op_details() {
    static global_info_detail::stream_op_details details;
    return details;
  }

  static char *get_listening_endpoint() {
    static char s[global_info_detail::MAX_LISTENING_ENDPOINT_LENGTH+1] = "";
    return s;
  }

  struct pre_crash_handlers {
    pre_crash_handlers() : m_number_handlers(0) {}
    void push(pre_crash_handler *pch) {
      if (m_number_handlers < global_info_detail::MAX_NUMBER_PRE_CRASH_HANDLERS) {
        m_handlers[m_number_handlers++] = pch;
      }
    }

    pre_crash_handler *top() {
      if (m_number_handlers > 0) {
        return m_handlers[m_number_handlers-1];
      }

      return 0;
    }

    pre_crash_handler *pop() {
      if (m_number_handlers > 0) {
        --m_number_handlers;
        return m_handlers[m_number_handlers];
      }

      return 0;
    }

    pre_crash_handler *m_handlers[global_info_detail::MAX_NUMBER_PRE_CRASH_HANDLERS];
    size_t m_number_handlers;
  };

  static pre_crash_handlers &get_pre_crash_handlers() { 
    static pre_crash_handlers pchis;
    return pchis;
  }
};

} // namespace


#endif