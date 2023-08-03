// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_JSON_PARSER_HPP
#define NP1_JSON_PARSER_HPP


#include "np1/simple_types.hpp"
#include "np1/assert.hpp"
#include "np1/str.hpp"
#include "np1/json/tokenizer.hpp"
#include "np1/json/writer.hpp"
#include "np1/rel/rlang/vm_heap.hpp"
#include "np1/io/ext_heap_buffer_output_stream.hpp"


namespace np1 {
namespace json {
  
class parser {
private:
  typedef enum {
    OUTSIDE,
    IN_ARRAY_BEFORE_VALUE,
    IN_ARRAY_AFTER_VALUE,
    IN_OBJECT_BEFORE_KEY,
    IN_OBJECT_AFTER_KEY,
    IN_OBJECT_BEFORE_VALUE,
    IN_OBJECT_AFTER_VALUE,
  } state_type;


  class stack {
  public:
    enum { MAX_DEPTH = 100 };

  public:
    stack() : m_size(0) {}

    state_type top() const {
      NP1_ASSERT(m_size > 0, "JSON stack top: JSON has too many closing ] or }s");
      return m_stack[m_size - 1];
    }

    void push(state_type container_type) {
      NP1_ASSERT(m_size < MAX_DEPTH, "JSON stack depth exceeds maximum of " + str::to_dec_str(MAX_DEPTH));
      m_stack[m_size++] = container_type;
    }

    void pop() {
      NP1_ASSERT(m_size > 0, "JSON stack pop: JSON has too many closing ] or }s");
      m_size--;
    }

    void pop_push(state_type container_type) {
      pop();
      push(container_type);
    }

    bool is_empty() {
      return (0 == m_size);
    }

  public:
    state_type m_stack[MAX_DEPTH];
    size_t m_size;
  };


  template <typename Next>
  struct raw_token_handler {
    explicit raw_token_handler(Next &next) : m_next(next) { m_stack.push(OUTSIDE); }

    void on_number(const str::ref &tok) { 
      switch (m_stack.top()) {
        case OUTSIDE:
          m_next.on_number(tok);
          break;

        case IN_ARRAY_BEFORE_VALUE:
          m_next.on_number(tok);
          m_stack.pop_push(IN_ARRAY_AFTER_VALUE);
          break;

        case IN_ARRAY_AFTER_VALUE:
          NP1_ASSERT(false, "JSON number found, expected , or ]");
          break;

        case IN_OBJECT_BEFORE_KEY:
          NP1_ASSERT(false, "JSON object keys cannot be numbers");
          break;

        case IN_OBJECT_AFTER_KEY:
          NP1_ASSERT(false, "JSON number found, expected :");
          break;

        case IN_OBJECT_BEFORE_VALUE:
          m_next.on_number(tok);
          m_stack.pop_push(IN_OBJECT_AFTER_VALUE);
          break;

        case IN_OBJECT_AFTER_VALUE:
          NP1_ASSERT(false, "JSON number found, expected , or }");
          break;
      }
    }

    void on_string(const str::ref &tok) { 
      switch (m_stack.top()) {
        case OUTSIDE:
          m_next.on_string(tok);
          break;

        case IN_ARRAY_BEFORE_VALUE:
          m_next.on_string(tok);
          m_stack.pop_push(IN_ARRAY_AFTER_VALUE);
          break;

        case IN_ARRAY_AFTER_VALUE:
          NP1_ASSERT(false, "JSON string found, expected , or ]");
          break;

       case IN_OBJECT_BEFORE_KEY:
          m_next.on_object_element_name(tok);
          m_stack.pop_push(IN_OBJECT_AFTER_KEY);
          break;

        case IN_OBJECT_AFTER_KEY:
          NP1_ASSERT(false, "JSON string found, expected :");
          break;

        case IN_OBJECT_BEFORE_VALUE:
          m_next.on_string(tok);
          m_stack.pop_push(IN_OBJECT_AFTER_VALUE);
          break;

        case IN_OBJECT_AFTER_VALUE:
          NP1_ASSERT(false, "JSON string found, expected , or }");
          break;
      }
    }
 

    void on_open_object(const str::ref &tok) { 
      switch (m_stack.top()) {
        case OUTSIDE:
          m_next.on_open_object(tok);
          m_stack.push(IN_OBJECT_BEFORE_KEY);
          break;

        case IN_ARRAY_AFTER_VALUE:
          NP1_ASSERT(false, "JSON { found, expected , or ]");
          break;

       case IN_ARRAY_BEFORE_VALUE:
          m_next.on_open_object(tok);
          m_stack.pop_push(IN_ARRAY_AFTER_VALUE);
          m_stack.push(IN_OBJECT_BEFORE_KEY);
          break;

        case IN_OBJECT_BEFORE_KEY:
          NP1_ASSERT(false, "JSON object keys cannot be objects");
          break;

        case IN_OBJECT_AFTER_KEY:
          NP1_ASSERT(false, "JSON { found, expected :");
          break;

        case IN_OBJECT_BEFORE_VALUE:
          m_next.on_open_object(tok);
          m_stack.pop_push(IN_OBJECT_AFTER_VALUE);
          m_stack.push(IN_OBJECT_BEFORE_KEY);
          break;

        case IN_OBJECT_AFTER_VALUE:
          NP1_ASSERT(false, "JSON { found, expected , or }");
          break;
      }
    }

    void on_object_name_value_delimiter(const str::ref &tok) {
      NP1_ASSERT(m_stack.top() == IN_OBJECT_AFTER_KEY, "JSON : found, expected almost anything else");
      m_next.on_delimiter(tok);
      m_stack.pop_push(IN_OBJECT_BEFORE_VALUE);
    }

    void on_close_object(const str::ref &tok) { 
      NP1_ASSERT((m_stack.top() == IN_OBJECT_AFTER_VALUE) || (m_stack.top() == IN_OBJECT_BEFORE_KEY), "Found unexpected JSON }");
      m_next.on_close_object(tok);
      m_stack.pop();
    }

    void on_open_array(const str::ref &tok) {
      switch (m_stack.top()) {
        case OUTSIDE:
          m_next.on_open_array(tok);
          m_stack.push(IN_ARRAY_BEFORE_VALUE);
          break;

        case IN_ARRAY_BEFORE_VALUE:
          m_next.on_open_array(tok);
          m_stack.pop_push(IN_ARRAY_AFTER_VALUE);
          m_stack.push(IN_ARRAY_BEFORE_VALUE);
          break;

        case IN_ARRAY_AFTER_VALUE:
          NP1_ASSERT(false, "JSON { found, expected , or ]");
          break;

       case IN_OBJECT_BEFORE_KEY:
          NP1_ASSERT(false, "JSON object keys cannot be arrays");
          break;

        case IN_OBJECT_AFTER_KEY:
          NP1_ASSERT(false, "JSON [ found, expected :");
          break;

        case IN_OBJECT_BEFORE_VALUE:
          m_next.on_open_array(tok);
          m_stack.pop_push(IN_OBJECT_AFTER_VALUE);
          m_stack.push(IN_ARRAY_BEFORE_VALUE);
          break;

        case IN_OBJECT_AFTER_VALUE:
          NP1_ASSERT(false, "JSON [ found, expected , or }");
          break;
      }
    }

    void on_element_delimiter(const str::ref &tok) { 
      switch (m_stack.top()) {
        case IN_ARRAY_AFTER_VALUE:
          m_stack.pop_push(IN_ARRAY_BEFORE_VALUE);
          break;

        case IN_OBJECT_AFTER_VALUE:
          m_stack.pop_push(IN_OBJECT_BEFORE_KEY);
          break;

        default:
          NP1_ASSERT(false, "Unexpected JSON ,");
          break;
      }

      m_next.on_delimiter(tok);
    }

    void on_close_array(const str::ref &tok) { 
      NP1_ASSERT((m_stack.top() == IN_ARRAY_BEFORE_VALUE) || (m_stack.top() == IN_ARRAY_AFTER_VALUE), "Found unexpected JSON ]");
      m_next.on_close_array(tok);
      m_stack.pop();
    }

    void on_special(const str::ref &tok) {
      switch (m_stack.top()) {
        case OUTSIDE:
          m_next.on_special(tok);
          break;

        case IN_ARRAY_BEFORE_VALUE:
          m_next.on_special(tok);
          m_stack.pop_push(IN_ARRAY_AFTER_VALUE);
          break;

        case IN_ARRAY_AFTER_VALUE:
          NP1_ASSERT(false, "JSON special value found, expected , or ]");
          break;

       case IN_OBJECT_BEFORE_KEY:
          NP1_ASSERT(false, "JSON object keys cannot be specials");
          break;

        case IN_OBJECT_AFTER_KEY:
          NP1_ASSERT(false, "JSON special value found, expected :");
          break;

        case IN_OBJECT_BEFORE_VALUE:
          m_next.on_special(tok);
          m_stack.pop_push(IN_OBJECT_AFTER_VALUE);
          break;

        case IN_OBJECT_AFTER_VALUE:
          NP1_ASSERT(false, "JSON number found, expected , or }");
          break;
      }
    }

    Next &m_next;
    stack m_stack;
  };
 
  template <typename Next>
  struct get_member_filter {
    typedef enum {
      OUTSIDE_MEMBER,
      SEEN_MEMBER_NAME,
      IN_MEMBER,
    } state_type;

    get_member_filter(const str::ref &member_name, Next &next) 
      : m_next(next), m_member_name(member_name), m_stack_depth(0), m_state(OUTSIDE_MEMBER) {}

#define GET_MEMBER_FILTER_VALUE_PASSTHROUGH(name__) void name__(const str::ref &tok) { if (IN_MEMBER == m_state) { m_next.name__(tok); if (1 == m_stack_depth) { m_state = OUTSIDE_MEMBER; } } }

    GET_MEMBER_FILTER_VALUE_PASSTHROUGH(on_number)
    GET_MEMBER_FILTER_VALUE_PASSTHROUGH(on_string)
    GET_MEMBER_FILTER_VALUE_PASSTHROUGH(on_special);
  
    void on_open_object(const str::ref &ref) {
      if (IN_MEMBER == m_state) {
        m_next.on_open_object(ref);
      }

      m_stack_depth++;
    }

    void on_close_object(const str::ref &ref) {
      if (IN_MEMBER == m_state) {
        m_next.on_close_object(ref);
      }

      m_stack_depth--;
      if (1 == m_stack_depth) {
        m_state = OUTSIDE_MEMBER;
      }
    }

    void on_object_element_name(const str::ref &ref) {
      if (IN_MEMBER == m_state) {
        m_next.on_object_element_name(ref);
      } else if ((1 == m_stack_depth) && is_member_name(ref)) {
        m_state = SEEN_MEMBER_NAME;
      }
    }

    void on_open_array(const str::ref &ref) {
      if (IN_MEMBER == m_state) {
        m_next.on_open_array(ref);
      }

      m_stack_depth++;
    }

    void on_close_array(const str::ref &ref) {
      if (IN_MEMBER == m_state) {
        m_next.on_close_array(ref);
      }

      m_stack_depth--;
      if (1 == m_stack_depth) {
        m_state = OUTSIDE_MEMBER;
      }        
    }

    void on_delimiter(const str::ref &ref) {
      if (IN_MEMBER == m_state) {
        m_next.on_delimiter(ref);
      } else if (SEEN_MEMBER_NAME == m_state) {
        m_state = IN_MEMBER;
      }
    }

    bool is_member_name(const str::ref &name) {
      //TODO
      NP1_ASSERT(!str::contains(name, '\\'), "String escapes in JSON strings are not currently supported.");

      NP1_ASSERT(str::starts_with(name, '"') && str::ends_with(name, '"') && (name.length() >= 2), "Internal error: JSON strings should start and end with a quote character");
      if (name.length() - 2 != m_member_name.length()) {
        return false;
      }

      return (memcmp(m_member_name.ptr(), name.ptr() + 1, m_member_name.length()) == 0);
    }


    Next &m_next;
    str::ref m_member_name;
    size_t m_stack_depth;
    state_type m_state;
  };

  typedef io::ext_heap_buffer_output_stream<rel::rlang::vm_heap> buffer_output_stream_type;
  typedef writer::parsed_token_writer_handler<buffer_output_stream_type> writer_handler_type;


public:
  static str::ref get_member(rel::rlang::vm_heap &heap, const str::ref &json, const str::ref &member_name) {
    buffer_output_stream_type out(heap, json.length());
    writer_handler_type writer_handler(out);

    typedef get_member_filter<writer_handler_type> filter_type;
    filter_type filter(member_name, writer_handler);

    typedef raw_token_handler<filter_type> raw_token_handler_type;
    raw_token_handler_type token_handler(filter);

    tokenizer::raw_tokenize(json, token_handler);
    return str::ref((const char *)out.ptr(), out.size());
  }

  static str::ref validate_and_normalise(rel::rlang::vm_heap &heap, const str::ref &json) {
    buffer_output_stream_type out(heap, json.length());
    writer_handler_type writer_handler(out);

    typedef raw_token_handler<writer_handler_type> raw_token_handler_type;
    raw_token_handler_type token_handler(writer_handler);

    tokenizer::raw_tokenize(json, token_handler);
    return str::ref((const char *)out.ptr(), out.size());
  }
}; 

} // namespaces
}

#endif
