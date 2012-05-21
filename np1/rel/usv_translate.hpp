// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_REL_USV_TRANSLATE_HPP
#define NP1_REL_USV_TRANSLATE_HPP

#include "np1/rel/rlang/rlang.hpp"

namespace np1 {
namespace rel {


class usv_translate {
public:
  template <typename Input_Stream, typename Output_Stream>
  void from_usv(Input_Stream &input, Output_Stream &output,
                  const rstd::vector<rlang::token> &args) {
    NP1_ASSERT(args.size() == 0, "rel.from_usv expects no arguments.");
    io::mandatory_record_input_stream<Input_Stream, usv_record, usv_record_ref> mandatory_input(input);
    mandatory_input.parse_records(from_usv_record_callback<Output_Stream>(output));
  }


  template <typename Input_Stream, typename Output_Stream>
  void to_usv(Input_Stream &input, Output_Stream &output,
              const rstd::vector<rlang::token> &args) {
    input.parse_records(to_usv_record_callback<Output_Stream>(output));
  }


private:
  class usv_record_ref {
  public:
    static const char FIELD_DELIMITER = 0x1f;  // US  (unit separator)
    static const char RECORD_DELIMITER = 0;

  public:
    /// Default constructor.
    usv_record_ref() : m_start(NULL), m_end(NULL), m_record_number(0) {}
  
    /// Constructor.
    usv_record_ref(const unsigned char *start, const unsigned char *end, uint64_t record_number)
      // Remember that the end points after the record delimiter.
      : m_start(start), m_end(end != start ? end-1 : end), m_record_number(record_number) {}
    
    /// Destructor.
    ~usv_record_ref() {}

    /// Returns 0 if the supplied buffer contains an incomplete record.
    static const unsigned char *get_record_end(const unsigned char *start, size_t length) {
      const unsigned char *end = (const unsigned char *)memchr(start, RECORD_DELIMITER, length);
      if (end) {
        return end+1;  // Skip over the record delimiter.
      }

      return 0;
    }
  
    static const char *get_record_end(const char *start, size_t length) {
      return (char *)get_record_end((unsigned char *)start, length);
    }

    void field_refs(rstd::vector<str::ref> &fields) const {
      fields.clear();

      const unsigned char *field = m_start;
      while (field < m_end) {
        const unsigned char *field_end = (const unsigned char *)memchr(field, FIELD_DELIMITER, m_end - field);
        if (!field_end) {
          field_end = m_end;
        }

        fields.push_back(str::ref((const char *)field, field_end - field));
        field = field_end + 1;
      }
    }

    template <typename Mandatory_Output_Stream>
    static void write(Mandatory_Output_Stream &output, const record_ref &r) {
      str::ref field;
      size_t number_fields = r.number_fields();
      size_t i = 0;
      while (true) {
        field = r.mandatory_field(i);
        NP1_ASSERT(memchr(field.ptr(), FIELD_DELIMITER, field.length()) == 0, "Field contains US char!");
        NP1_ASSERT(memchr(field.ptr(), RECORD_DELIMITER, field.length()) == 0, "Record contains NUL char!");
        output.write(field.ptr(), field.length());
        if (++i >= number_fields) {
          output.write(RECORD_DELIMITER);
          return;
        }

        output.write(FIELD_DELIMITER);
      }
    }

  private:
    const unsigned char *m_start;
    const unsigned char *m_end;
    uint64_t m_record_number;
  };


  // This is just a dummy implementation to satisfy mandatory_record_input_stream's requirements.  We don't actually
  // use it.
  struct usv_record {
    explicit usv_record(const usv_record_ref &ref) {}
  };


  template <typename Output>
  struct from_usv_record_callback {
    explicit from_usv_record_callback(Output &o) : m_output(o) {}  

    bool operator()(const usv_record_ref &r) {
      r.field_refs(m_field_refs);
      record_ref::write(m_output, m_field_refs);
      return true;
    }  

    Output &m_output;
    rstd::vector<str::ref> m_field_refs;
  };

  template <typename Output>
  struct to_usv_record_callback {
    explicit to_usv_record_callback(Output &o) : m_output(o) {}  

    bool operator()(const record_ref &r) {
      usv_record_ref::write(m_output, r);
      return true;
    }  

    Output &m_output;
  };
};


} // namespaces
}

#endif

