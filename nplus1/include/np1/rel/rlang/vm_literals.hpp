// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_REL_RLANG_VM_LITERALS_HPP
#define NP1_REL_RLANG_VM_LITERALS_HPP


namespace np1 {
namespace rel {
namespace rlang {


/// Storage for literals.
class vm_literals {
public:
  enum { MAX_NUMBER_LITERALS = 100, MAX_LITERAL_STRING_COMBINED_SIZE = 4096 };
 
public:
  vm_literals() : m_next_literal_offset(0), m_next_string_offset(0) {}

  size_t push_back(dt::data_type type, const char *s) {
    NP1_ASSERT(m_next_literal_offset < MAX_NUMBER_LITERALS, "Too many literals");

    size_t s_length = strlen(s);

    switch (type) {
    case dt::TYPE_STRING:
    case dt::TYPE_ISTRING:
      NP1_ASSERT(m_next_string_offset + s_length + 1 < MAX_LITERAL_STRING_COMBINED_SIZE,
                  "Too many long string literals");
      memcpy(&m_strings[m_next_string_offset], s, s_length);
      m_literals[m_next_literal_offset].s.m_offset = m_next_string_offset;
      m_literals[m_next_literal_offset].s.m_length = s_length;      
      m_next_string_offset += s_length;
      m_strings[m_next_string_offset] = '\0';
      ++m_next_string_offset;
      break;

    case dt::TYPE_INT:
      m_literals[m_next_literal_offset].i = str::dec_to_int64(s, s_length);
      break;

    case dt::TYPE_UINT:
      m_literals[m_next_literal_offset].ui = str::dec_to_int64(s, s_length);
      break;

    case dt::TYPE_DOUBLE:
      m_literals[m_next_literal_offset].d = str::dec_to_double(s, s_length);
      break;

    case dt::TYPE_BOOL:
      m_literals[m_next_literal_offset].b = str::to_bool(s, s_length);
      break;

    default:
      NP1_ASSERT(false, "Invalid type for literal");
      break;
    }

    size_t offset = m_next_literal_offset;
    ++m_next_literal_offset;
    return offset;
  }

  const dt::string get_string(size_t offset) const {
    const literal *l = &m_literals[offset];
    return dt::string(&m_strings[l->s.m_offset], l->s.m_length);
  }

  dt::integer get_integer(size_t offset) const {
    return m_literals[offset].i;  
  }

  dt::uinteger get_uinteger(size_t offset) const {
    return m_literals[offset].ui;  
  }

  dt::fdouble get_double(size_t offset) const {
    return m_literals[offset].d;  
  }

  dt::boolean get_boolean(size_t offset) const {
    return m_literals[offset].b;  
  }

private:
  union literal {
    struct {
      size_t m_offset;
      size_t m_length;
    } s;
    int64_t i;
    uint64_t ui;
    double d;
    bool b;
  };

  literal m_literals[MAX_NUMBER_LITERALS];
  char m_strings[MAX_LITERAL_STRING_COMBINED_SIZE];
  size_t m_next_literal_offset;
  size_t m_next_string_offset;
};



} // namespaces
}
}


#endif
