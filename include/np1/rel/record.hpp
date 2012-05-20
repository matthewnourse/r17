// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_REL_DETAIL_RECORD_HPP
#define NP1_REL_DETAIL_RECORD_HPP

#include <string>
#include "np1/rel/record_ref.hpp"
#include "std/detail/mem.hpp"


namespace np1 {
namespace rel {

/// This class is a record that owns its own copy of the memory.
class record
{
public:
  /// Default constructor.
  record() : m_buffer(NULL) {}

  /// Construct from someone else's record reference.
  explicit record(const record_ref &ref) { initialize(ref); }
  
  /// Construct from strings.  This is SLOW- use sparingly.
  record(const std::string &s1, size_t record_number) {
    std::vector<std::string> strs;
    strs.push_back(s1);
    initialize(strs, record_number);   
  }

  record(const std::string &s1, const std::string &s2, size_t record_number) {
    std::vector<std::string> strs;
    strs.push_back(s1);
    strs.push_back(s2);
    initialize(strs, record_number);   
  }

  record(const std::string &s1, const std::string &s2, const std::string &s3,
         size_t record_number) {
    std::vector<std::string> strs;
    strs.push_back(s1);
    strs.push_back(s2);
    strs.push_back(s3);
    initialize(strs, record_number);   
  }

  record(const std::string &s1, const std::string &s2, const std::string &s3,
         const std::string &s4, size_t record_number) {
    std::vector<std::string> strs;
    strs.push_back(s1);
    strs.push_back(s2);
    strs.push_back(s3);
    strs.push_back(s4);
    initialize(strs, record_number);   
  }

  record(const std::string &s1, const std::string &s2, const std::string &s3,
         const std::string &s4, const std::string &s5, size_t record_number) {
    std::vector<std::string> strs;
    strs.push_back(s1);
    strs.push_back(s2);
    strs.push_back(s3);
    strs.push_back(s4);
    strs.push_back(s5);
    initialize(strs, record_number);   
  }

  record(const std::vector<std::string> &strs, size_t record_number) {
    initialize(strs, record_number);
  }

  record(const std::vector<str::ref> &strs, size_t record_number) {
    initialize(strs, record_number);
  }

  /// Copy constructor.
  record(const record &other) { initialize(other.m_ref); }
  
  /// Destructor.
  ~record() { uninitialize(); }

  
  /// Get the number of fields in this record.  
  size_t number_fields() const { return m_ref.number_fields(); }
  
  /// Get the field with 0-based offset field_number.
  /**
   * If the field can't be found, the function returns a reference where
   * is_null() == true.  Otherwise, a pointer to the field is returned and the
   * field length is set.  
   */
  str::ref field(size_t field_number) const {
    return m_ref.field(field_number);  
  }
  
  /// Get the field with 0-based offset field_number.
  /**
   * If the field can't be found, crashes.  Otherwise, 
   * a reference to the field is returned and the field length is set.  
   */
  str::ref mandatory_field(size_t field_number) const {
    return m_ref.mandatory_field(field_number);  
  }
  
  /// Find some field contents and return the field number.
  /**
   * TODO: this method is slow!  If you need to use it somewhere critical,
   * you'll have to make it faster.
   */
  size_t find_field(const char *contents) const {
    return m_ref.find_field(contents);
  }

  /// Find some field contents and return the field number.
  /**
   * TODO: this method is slow!  If you need to use it somewhere critical,
   * you'll have to make it faster.
   * 
   * Crash if the contents can't be found.
   */
  size_t mandatory_find_field(const char *contents) const {
    return m_ref.mandatory_find_field(contents);
  }
  
  size_t mandatory_find_field(const std::string &contents) const {
    return mandatory_find_field(contents.c_str());
  }

  /// mandatory_find_field that will find the heading with or without its
  /// type tag.
  size_t mandatory_find_heading(const str::ref &heading) const {
    return m_ref.mandatory_find_heading(heading);  
  }

  size_t mandatory_find_heading(const char *heading) const {
    return m_ref.mandatory_find_heading(heading);  
  }

  size_t mandatory_find_heading(const std::string &heading) const {
    return m_ref.mandatory_find_heading(heading);  
  }

  /// Get a reference to this record.
  const record_ref &ref() const { return m_ref; }
  

   /// Get all the fields in this record.  NOTE that this is VERY slow!
  std::vector<std::string> fields() const { return m_ref.fields(); }

  /// Get a text representation of the record suitable for error messages.  This is VERY slow!
  std::string to_string() const { return m_ref.to_string(); }


    /// Is the record empty?
  bool is_empty() const { return m_ref.is_empty(); }  

  /// Get stuff.
  uint64_t record_number() const { return m_ref.record_number(); }
  uint64_t checksum() const { return m_ref.checksum(); }


  /// Write this record to the output stream.
  template <typename Mandatory_Output_Stream>
  void write(Mandatory_Output_Stream &mos) const {
    m_ref.write(mos);
  }

  
  /// Write a record to the output stream, constructing it on the fly.
  template <typename Mandatory_Output_Stream>
  static void write(Mandatory_Output_Stream &mos, int argc, const char **argv) {
    record_ref::write(mos, argc, argv);
  }

  /// Write the fields to the output stream, ending with a record delimiter.
  template <typename Mandatory_Output_Stream>
  static void write(Mandatory_Output_Stream &mos, const std::vector<str::ref> &refs) {
    record_ref::write(mos, refs); 
  }

  /// Write the record to the output stream, ending with a record delimiter.
  template <typename Mandatory_Output_Stream>
  static void write(Mandatory_Output_Stream &mos, const record_ref &r) {
    record_ref::write(mos, r);
  }  
 

  /// Write multiple arguments to the output stream as a single record.
  template <typename Mandatory_Output_Stream, typename... Fields>
  static void write(Mandatory_Output_Stream &mos,
                    const std::vector<str::ref> &refs,
                    const Fields&... fields) {
    record_ref::write(mos, refs, fields...);
  }


  template <typename Mandatory_Output_Stream, typename... Fields>
  static void write(Mandatory_Output_Stream &mos, const record_ref &r,
                    const Fields&... fields) {
    record_ref::write(mos, r, fields...);
  }

  template <typename Mandatory_Output_Stream, typename... Fields>
  static void write(Mandatory_Output_Stream &mos, const str::ref &s,
                    const Fields&... fields) {
    record_ref::write(mos, s, fields...);
  }

  template <typename Mandatory_Output_Stream, typename... Fields>
  static void write(Mandatory_Output_Stream &mos, const char *s,
                    const Fields&... fields) {
    record_ref::write(mos, s, fields...);
  }  

  
  /// Swap.
  void swap(record &other) {
    m_ref.swap(other.m_ref);
    std::swap(m_buffer, other.m_buffer);
  }

  /// Assign using a record reference.
  record &assign(const record_ref &other) {
    record r(other);
    swap(r);
    return *this;
  }

  /// Assignment operator.
  record &operator = (const record &other) {
    record r(other);
    swap(r);
    return *this;  
  }



private:
  /// Initialize this object using the reference.
  void initialize(const record_ref &ref) {
    //+1 to include the record delimiter.
    size_t record_data_size = ref.byte_size();     
    m_buffer = (unsigned char *)std::detail::mem::alloc(record_data_size);
                  
    memcpy(m_buffer, ref.start(), record_data_size);    
    m_ref.initialize(m_buffer, m_buffer + record_data_size, ref.record_number());      
  }

  /// Initialize using the vector of strings.
  template <typename T>
  void initialize(const std::vector<T> &strs, size_t record_number) {
    size_t record_data_size = record_ref::required_record_data_size(strs);
    m_buffer = (unsigned char *)std::detail::mem::alloc(record_data_size);
    np1::io::ext_static_buffer_output_stream temp_stream(m_buffer, record_data_size);
    record_ref::write(temp_stream, strs);
    m_ref.initialize(m_buffer, m_buffer + record_data_size, record_number);      
  }

  
  /// Free the memory for this object.
  void uninitialize() {
    std::detail::mem::free(m_buffer);
    m_buffer = NULL;
    m_ref.initialize(NULL, NULL, 0);
  }
  
private:
  record_ref m_ref;
  unsigned char *m_buffer;
};
 
  
} // namespaces
}


#endif
