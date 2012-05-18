// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_REL_DETAIL_RECORD_REF_HPP
#define NP1_REL_DETAIL_RECORD_REF_HPP


#include <vector>
#include "np1/assert.hpp"
#include "np1/str.hpp"
#include "np1/compressed_int.hpp"
#include "np1/rel/detail/helper.hpp"
#include "np1/rel/rlang/dt.hpp"


namespace np1 {
namespace rel {


/// Decompression helpers.
namespace detail {
  // Returns 0 if there is no complete int in the buffer, otherwise returns
  // a pointer to the next byte in the buffer.
  inline const unsigned char *decompress_uint64(const unsigned char *buf, const unsigned char *end, uint64_t &result) {
    const unsigned char *p = compressed_int::decompress(buf, end, result);
    if (p) {
      NP1_ASSERT((p - buf) <= compressed_int::MAX_COMPRESSED_INT_SIZE,
                  "Compressed int is weirdly big.  Is the input in the native record format?  Buffer (hex, then raw): "
                    + str::get_as_hex_dump(buf, end));
    } else {
      NP1_ASSERT((end - buf) < compressed_int::MAX_COMPRESSED_INT_SIZE,
                  "Buffer is big enough for compressed int but no compressed int was found.  Is the input in the native record format?  Buffer (hex, then raw): "
                    + str::get_as_hex_dump(buf, end));
    }

    return p;
  }

  template <size_t Size>
  struct decompress_size {
    static inline const unsigned char *f(const unsigned char *buf, const unsigned char *end, uint64_t &sz) {
      NP1_ASSERT(false, "Unreachable decompress_size overload");
      return 0;
    }
  };
  
  template <>
  struct decompress_size <sizeof(uint32_t)> {
    static inline const unsigned char *f(const unsigned char *buf, const unsigned char *end, size_t &sz) {
      uint64_t ui64;
      const unsigned char *p = decompress_uint64(buf, end, ui64);
      if (!p) {
        return p;
      }
  
      NP1_ASSERT(ui64 < 0xffffffff, "Size field in record is too long");
      sz = (uint32_t)ui64;
      return p;
    }
  };
  
  
  template <>
  struct decompress_size <sizeof(uint64_t)> {
    static inline const unsigned char *f(const unsigned char *buf, const unsigned char *end, size_t &sz) {
      uint64_t ui64 = sz;
      const unsigned char *p = decompress_uint64(buf, end, ui64);
      sz = ui64;
      return p;
    }
  };
} // namespace detail


/// This class is a reference to a record.
/**
 * IT DOES NOT OWN THE MEMORY so watch yourself if you copy it or save it 
 * anywhere :).
 */
class record_ref {
public:
  struct raw_record_data;
  
public:
  /// Default constructor.
  record_ref() : m_start(NULL), m_end(NULL), m_record_number(0) {}

  /// Constructor.
  record_ref(const unsigned char *start, const unsigned char *end, uint64_t record_number)
    : m_start(start), m_end(end), m_record_number(record_number) {}
  
  /// Destructor.
  ~record_ref() {}

  
  /// Get the number of fields in this record.  
  size_t number_fields() const {
    if (!m_start) {
      return 0;
    }

    prelude prel;
    mandatory_read_prelude(prel);
    return prel.number_fields;
  }
  
  /// Get the field with 0-based offset field_number.
  /**
   * If the field can't be found, the function returns str::ref where is_null()
   * is true.  Otherwise, a reference to the field is returned and the field
   * length is set.  
   */
  str::ref field(size_t field_number) const {
    if (m_start) {
      prelude prel;
      const unsigned char *start_field_length = mandatory_read_prelude(prel);
      if (field_number >= prel.number_fields) {
        return str::ref();
      }

      size_t field_size = 0;
      size_t field_counter;
      const unsigned char *fields_end = m_end - postlude_size();
      for (field_counter = 0; field_counter < field_number; ++field_counter) {
        const unsigned char *start_field = mandatory_decompress_size(start_field_length, fields_end, field_size);
        start_field_length = start_field + field_size;
      }

      if (start_field_length >= fields_end) {
        NP1_ASSERT(start_field_length == fields_end,
                    "Field lengths extend beyond end of record.  Error found while searching for field "
                      + str::to_dec_str(field_number) + " in record " + str::to_dec_str(m_record_number));
        return str::ref();
      }

      const unsigned char *start_field = mandatory_decompress_size(start_field_length, fields_end, field_size);
      return str::ref((const char *)start_field, field_size);
    }
    
    return str::ref();  
  }
  
  
  /// Find a field, crash on error.
  str::ref mandatory_field(size_t field_number) const {
    str::ref f = field(field_number);
    if (f.is_null()) {
      std::string description =
          "Unable to find field " + str::to_dec_str(field_number)
          + " in record " + str::to_dec_str(m_record_number)
          + "  record: " + to_string();
          
      NP1_ASSERT(false, description);
    }
    
    return f;
  }


  /// Find some field contents and return the field number.
  /**
   * TODO: this method is slow!  If you need to use it somewhere critical,
   * you'll have to make it faster.
   */
  size_t find_field(const str::ref &contents) const {
    if (!m_start) {
      return (size_t)-1;
    }
    
    size_t field_number;
    str::ref current_field;
    
    for (field_number = 0;
        !((current_field = field(field_number)).is_null());
        ++field_number) {
      if (str::cmp(contents, current_field) == 0) {
        return field_number;
      }
    }
    
    // If we get to here then the contents were not found.
    return (size_t)-1;
  }
  
  size_t find_field(const char *contents) const {
    return find_field(str::ref(contents, strlen(contents)));
  }
  
  /// Find some field contents and return the field number.
  /**
   * TODO: this method is slow!  If you need to use it somewhere critical,
   * you'll have to make it faster.
   * 
   * Crash if the contents can't be found.
   */
  size_t mandatory_find_field(const char *contents) const {
    size_t field_pos = find_field(contents);
    NP1_ASSERT(field_pos != (size_t)-1,
                "Unable to find record contents: " + std::string(contents));
    return field_pos;
  }


  /// find_field that will find the heading with or without its
  /// type tag.
  size_t find_heading(const str::ref &heading) const {
    if (!m_start) {
      return (size_t)-1;
    }

    size_t field_number;
    str::ref current_field;
    str::ref heading_without_type_tag =
      detail::helper::get_heading_without_type_tag(heading);    

    // If the heading name was supplied with a type tag then just do a plain
    // find_field, 'cause we need to match the type tag too.
    if (str::cmp(heading_without_type_tag, heading) != 0) {
      return find_field(heading);
    }

    for (field_number = 0;
        !((current_field = field(field_number)).is_null());
        ++field_number) {
      if (str::cmp(heading_without_type_tag,
                    detail::helper::get_heading_without_type_tag(current_field)) == 0) {
        return field_number;
      }
    }

    return (size_t)-1;
  }

  size_t find_heading(const char *heading) const {
    return find_heading(str::ref(heading, strlen(heading)));  
  }

  /// mandatory_find_field that will find the heading with or without its
  /// type tag.
  size_t mandatory_find_heading(const str::ref &heading) const {
    size_t field_pos = find_heading(heading);
    NP1_ASSERT(field_pos != (size_t)-1,
                "Unable to find heading: '" + heading.to_string() + "' in record: " + to_string());
    return field_pos;
  }

  size_t mandatory_find_heading(const char *s) const {
    return mandatory_find_heading(str::ref(s, strlen(s)));  
  }

  size_t mandatory_find_heading(const std::string &s) const {
    return mandatory_find_heading(str::ref(s));  
  }

  
  /// Get all the fields in this record.  NOTE that this is VERY slow!
  std::vector<std::string> fields() const {
    size_t i;
    size_t nfields = number_fields();
    std::vector<std::string> result;
    
    for (i = 0; i < nfields; ++i) {
      str::ref f = field(i);
      NP1_ASSERT(!f.is_null(), "Field " + str::to_dec_str(i) + " not found!");
      result.push_back(f.to_string());
    }
    
    return result;
  }
  
  /// Get a text representation of the record suitable for error messages.  This is VERY slow!
  std::string to_string() const {
    return str::implode(fields(), "|");
  }

  /// Returns 0 if the supplied buffer contains an incomplete record.  NOTE that this does not do as much checking as
  /// contains_record, but is faster.
  static const unsigned char *get_record_end(const unsigned char *start,
                                              size_t length) {
    size_t byte_size = 0;
    const unsigned char *p = detail::decompress_size<sizeof(size_t)>::f(start, start + length, byte_size);
    if (!p) {
      return 0;
    }

    const unsigned char *end = p + byte_size;
    const unsigned char *available_end = start + length;
    return end <= available_end ? end : 0;
  }

  static const char *get_record_end(const char *start, size_t length) {
    return (char *)get_record_end((unsigned char *)start, length);
  }


  /// Does the supplied buffer contain at least one valid record?
  static bool contains_record(const unsigned char *start, size_t length, uint64_t &number_fields) {    
    const unsigned char *end = start + length;
    uint64_t record_length;
    const unsigned char *p = compressed_int::decompress(start, end, record_length);
    if (!p) {
      return false;
    }

    if (length < record_length) {
      return false;
    }

    p = compressed_int::decompress(p, end, number_fields);
    if (!p) {
      return false;
    }

    uint64_t field_counter;
    for (field_counter = 0; (p < end) && (field_counter < number_fields); ++field_counter) {
      uint64_t field_length;
      p = compressed_int::decompress(p, end, field_length);
      p += field_length;
    }

    return ((field_counter == number_fields) && (p + postlude_size() <= end));
  }

  
  /// Is the record empty?
  bool is_empty() const { return m_start == m_end; }

  /// Get stuff.
  uint64_t record_number() const { return m_record_number; }

  const raw_record_data *start() const { return (const raw_record_data *)m_start; }
  const raw_record_data *end() const { return (const raw_record_data *)m_end; }
  size_t byte_size() const { return m_start ? m_end - m_start : 0; }
  uint64_t checksum() const {
    if (!m_start) {
      return 0;  
    }

    const unsigned char *checksum_p = m_start + byte_size() - postlude_size();
    uint64_t checksum;
    memcpy(&checksum, checksum_p, sizeof(checksum));
    return checksum;
  }


  /// Write this record to the output stream.
  template <typename Mandatory_Output_Stream>
  void write(Mandatory_Output_Stream &mos) const {
    write(mos, *this);
  }

  
  /// Write a record to the output stream, constructing it on the fly.
  template <typename Mandatory_Output_Stream>
  static void write(Mandatory_Output_Stream &mos, int argc, const char **argv) {
    write_complete_record(mos, argv, argv + argc);
  }

  /// Write the fields to the output stream as a complete record.
  template <typename Mandatory_Output_Stream>
  static void write(Mandatory_Output_Stream &mos, const std::vector<str::ref> &refs) {
    write_complete_record(mos, refs.begin(), refs.end());
  }

  template <typename Mandatory_Output_Stream>
  static void write(Mandatory_Output_Stream &mos, const std::vector<std::string> &strs) {
    write_complete_record(mos, strs.begin(), strs.end());
  }

  // Write out the supplied vector as headings, setting a default type tag if not supplied.
  template <typename Mandatory_Output_Stream>
  static void write_headings(Mandatory_Output_Stream &mos, const std::vector<std::string> &strs,
                              rlang::dt::data_type default_type_tag) {
    // No need to be fast here, the headings should be written out once.
    std::vector<std::string> final_strs;
    std::vector<std::string>::const_iterator ii = strs.begin();
    std::vector<std::string>::const_iterator iz = strs.end();
    for (; ii != iz; ++ii) {
      if (detail::helper::get_heading_type_tag(*ii).is_null()) {
        final_strs.push_back(detail::helper::make_typed_heading_name(rlang::dt::to_string(default_type_tag), *ii));
      } else {
        final_strs.push_back(*ii);
      }
    }

    write_complete_record(mos, final_strs.begin(), final_strs.end());
  }


  template <typename Mandatory_Output_Stream>
  static void write_headings(Mandatory_Output_Stream &mos, const std::vector<str::ref> &strs,
                              rlang::dt::data_type default_type_tag) {
    // No need to be fast here, the headings should be written out once.
    std::vector<std::string> std_strs;
    std::vector<str::ref>::const_iterator ii = strs.begin();
    std::vector<str::ref>::const_iterator iz = strs.end();
    for (; ii != iz; ++ii) {
      std_strs.push_back(ii->to_string());
    }

    write_headings(mos, std_strs, default_type_tag);
  }


  /// Write the record to the output stream as a complete record.
  template <typename Mandatory_Output_Stream>
  static void write(Mandatory_Output_Stream &mos, const record_ref &r) {
    mos.write(r.m_start, r.byte_size());
  }  
 

  /// Write multiple arguments to the output stream as a single record.
  template <typename Mandatory_Output_Stream, typename Field, typename... Fields>
  static void write(Mandatory_Output_Stream &mos, const Field &field, const Fields&... fields) {
    write_prelude(mos, serialized_size(field, fields...), number_component_fields(field, fields...));
    write_field(mos, field, fields...);
    write_postlude(mos);
  }

  
  // For the use of the 'record' class, ONLY.
  void initialize(const unsigned char *start, const unsigned char *end, uint64_t record_number) {
    m_start = start;
    m_end = end;
    m_record_number = record_number;
  }
  
  // Figure out how much space it would take to store a record that consists of the supplied fields.
  template <typename T>
  static size_t required_record_data_size(const std::vector<T> &fields) {
    return total_record_size(serialized_size(fields), fields.size());
  }

  void swap(record_ref &other) {
    std::swap(m_start, other.m_start);
    std::swap(m_end, other.m_end);
    std::swap(m_record_number, other.m_record_number);
  }

  /// Equality comparison doesn't include record number.
  bool is_equal(const record_ref &other) const {
    size_t sz = byte_size();
    if (sz != other.byte_size()) {
      return false;
    }

    return (memcmp(m_start, other.m_start, sz) == 0);
  }

private:
  // The record prelude- a headerette for a single record.
  struct prelude {
    prelude() : byte_size(0), number_fields(0) {}
    size_t byte_size;
    size_t number_fields;
  };

  // Read the record prelude and return a pointer to the first field in the
  // record.
  const unsigned char *mandatory_read_prelude(prelude &prel) const {
    const unsigned char *p = mandatory_decompress_size(m_start, m_end, prel.byte_size);
    p = mandatory_decompress_size(p, m_end, prel.number_fields);
    NP1_ASSERT(p, "Record does not contain the number of fields");
    return p;
  }

  // Write the prelude out to the supplied stream.
  template <typename Mandatory_Output_Stream>
  static inline void write_prelude(Mandatory_Output_Stream &mos, size_t total_field_byte_size, size_t number_fields) {
    write_compressed_uint64(mos, calculate_byte_size_in_prelude(total_field_byte_size, number_fields));
    write_compressed_uint64(mos, number_fields);
  }

  // Get the total size required to store the record, including all the prelude & postlude.
  static inline size_t total_record_size(size_t total_field_byte_size, size_t number_fields) {
    size_t byte_size_in_prelude = calculate_byte_size_in_prelude(total_field_byte_size, number_fields);
    return compressed_int::compressed_size(byte_size_in_prelude) + byte_size_in_prelude;
  }

  // The footerette for the record.
  struct postlude {
    postlude() : checksum(0) {}
    uint64_t checksum;
  };

  //TODO: actually write a checksum.
  template <typename Mandatory_Output_Stream>
  static inline void write_postlude(Mandatory_Output_Stream &mos) {
    //TODO: checksum calculation.
    uint64_t checksum = 0;
    mos.write((unsigned char *)&checksum, sizeof(checksum));    
  }

  static inline size_t postlude_size() { return sizeof(uint64_t); }

  // Write a field out to the supplied stream.
  template <typename Mandatory_Output_Stream>
  static inline void write_field(Mandatory_Output_Stream &mos, const char *field, size_t length) {
    write_compressed_uint64(mos, length);
    mos.write(field, length);
  }

  template <typename Mandatory_Output_Stream>
  static inline void write_field(Mandatory_Output_Stream &mos, const char *field) {
    write_field(mos, field, strlen(field));
  }
  
  template <typename Mandatory_Output_Stream>
  static inline void write_field(Mandatory_Output_Stream &mos, const str::ref &field) {
    write_field(mos, field.ptr(), field.length());
  }

  template <typename Mandatory_Output_Stream>
  static inline void write_field(Mandatory_Output_Stream &mos, const std::string &field) {
    write_field(mos, field.c_str(), field.length());
  }

  // Write the fields in the record.
  template <typename Mandatory_Output_Stream>
  static inline void write_field(Mandatory_Output_Stream &mos, const record_ref &fields) {
    prelude prel;
    const unsigned char *first_field = fields.mandatory_read_prelude(prel);
    const unsigned char *fields_end = fields.m_end - postlude_size();
    NP1_ASSERT(fields_end > first_field, "Record is missing postlude");
    mos.write(first_field, fields_end - first_field);    
  }

  // Write the fields in the vector.
  template <typename Mandatory_Output_Stream>
  static inline void write_field(Mandatory_Output_Stream &mos, const std::vector<str::ref> &fields) {
    write_fields(mos, fields.begin(), fields.end());
  }

  template <typename Mandatory_Output_Stream>
  static inline void write_field(Mandatory_Output_Stream &mos, const std::vector<std::string> &fields) {
    write_fields(mos, fields.begin(), fields.end());
  }

  template <typename Mandatory_Output_Stream, typename Field, typename... Fields>
  static void write_field(Mandatory_Output_Stream &mos, const Field &f, const Fields&... fields) {
    write_field(mos, f);
    write_field(mos, fields...);
  }


  // Write the values between the iterators as a complete record.
  template <typename Mandatory_Output_Stream, typename Forward_Iterator>
  static void write_complete_record(Mandatory_Output_Stream &mos, Forward_Iterator begin, Forward_Iterator end) {
    write_prelude(mos, total_serialized_size(begin, end), end - begin);
    write_fields(mos, begin, end);
    write_postlude(mos);
  }

  // Write the fields between the iterators.
  template <typename Mandatory_Output_Stream, typename Forward_Iterator>
  static void write_fields(Mandatory_Output_Stream &mos, Forward_Iterator begin, Forward_Iterator end) {
    Forward_Iterator iter;
     
    for (iter = begin; iter < end; ++iter) {
      write_field(mos, *iter);
    }
  }

  // Calculate the size required to store a field of the supplied length.
  static inline size_t serialized_size(size_t field_length) {
    return compressed_int::compressed_size(field_length) + field_length;
  }

  static inline size_t serialized_size(const char *field) {
    return serialized_size(strlen(field));
  }

  static inline size_t serialized_size(const str::ref &field) {
    return serialized_size(field.length());
  }

  static inline size_t serialized_size(const std::string &field) {
    return serialized_size(field.length());
  }

  static inline size_t serialized_size(const std::vector<str::ref> &fields) {
    return total_serialized_size(fields.begin(), fields.end());
  }

  static inline size_t serialized_size(const std::vector<std::string> &fields) {
    return total_serialized_size(fields.begin(), fields.end());
  }
  

  // Get the total byte size of the fields between the iterators.
  template <typename Forward_Iterator>
  static size_t total_serialized_size(Forward_Iterator begin, Forward_Iterator end) {
    Forward_Iterator iter;
     
    size_t total_field_byte_size = 0;
    for (iter = begin; iter < end; ++iter) {
      total_field_byte_size += serialized_size(*iter);
    }

    return total_field_byte_size;    
  }


  // Get the total byte size of the fields in the record.
  static size_t serialized_size(const record_ref &r) {
    prelude prel;
    const unsigned char *first_field = r.mandatory_read_prelude(prel);
    const unsigned char *fields_end = r.m_end - postlude_size();
    NP1_ASSERT(fields_end > first_field, "Record is missing postlude");
    return fields_end - first_field;
  }

  // Get the total byte size of all the fields in the argument list.
  template <typename Field, typename... Fields>
  static size_t serialized_size(const Field &f, const Fields&... fields) {
    return serialized_size(f) + serialized_size(fields...);
  }


  // Get the number of fields in a part of the record.
  static inline size_t number_component_fields(const char *) { return 1; }
  static inline size_t number_component_fields(const str::ref &) { return 1; }
  static inline size_t number_component_fields(const std::string &) { return 1; }
  static inline size_t number_component_fields(const record_ref &r) { return r.number_fields(); }
  
  template <typename T>
  static inline size_t number_component_fields(const std::vector<T> &fields) { return fields.size(); }

  template <typename Field, typename... Fields>
  static size_t number_component_fields(const Field &f, const Fields&... fields) {
    return number_component_fields(f) + number_component_fields(fields...);
  }


  // Calculate the prelude's byte_size field.
  static inline size_t calculate_byte_size_in_prelude(size_t total_field_byte_size, size_t number_fields) {
    size_t sz = total_field_byte_size;
    sz += compressed_int::compressed_size(number_fields);
    sz += postlude_size(); // checksum
    // If this line is not here then the optimizer really does remove the addition!
    NP1_ASSERT(sz > total_field_byte_size, "Addition has been optimized away");  
    return sz;
  }


  static inline const unsigned char *mandatory_decompress_uint64(const unsigned char *buf, const unsigned char *end,
                                                                  uint64_t &result) {
    const unsigned char *next = detail::decompress_uint64(buf, end, result);
    NP1_ASSERT(next, "Unexpected end-of-record in the middle of compressed int");
    return next;
  }

  static inline const unsigned char *mandatory_decompress_size(const unsigned char *buf, const unsigned char *end,
                                                                size_t &sz) {
    const unsigned char *next = detail::decompress_size<sizeof(size_t)>::f(buf, end, sz);
    NP1_ASSERT(next, "Unexpected end-of-record in the middle of compressed size int");
    return next;    
  }

  template <typename Mandatory_Output_Stream>
  static inline void write_compressed_uint64(Mandatory_Output_Stream &mos, uint64_t ui64) {
    unsigned char buf[compressed_int::MAX_COMPRESSED_INT_SIZE];
    unsigned char *end = compressed_int::compress(buf, ui64);
    mos.write(buf, end - buf);
  }
 
private:
  const unsigned char *m_start;
  const unsigned char *m_end;
  uint64_t m_record_number;  // 1-based record number.
};
 
  
} // namespaces
}


#endif
