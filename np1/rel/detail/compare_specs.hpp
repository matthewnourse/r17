// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_REL_DETAIL_COMPARE_SPECS_HPP
#define NP1_REL_DETAIL_COMPARE_SPECS_HPP


#include "np1/rel/detail/compare_spec.hpp"
#include "rstd/vector.hpp"

namespace np1 {
namespace rel {
namespace detail {
  
class compare_specs {
public:
  typedef const compare_spec *const_iterator;
  
public:
  // The specs in spec vector will be in the same order as the headings.
  template <typename Record>
  compare_specs(const Record &headings, const char **selected_heading_names, 
                size_t number_selected_heading_names) {
    size_t i;
    for (i = 0; i < number_selected_heading_names; ++i) {
      m_specs.push_back(
        compare_spec(headings, selected_heading_names[i]));      
    }    
  }
  
  template <typename Record>
  compare_specs(const Record &headings, 
          const rstd::vector<rstd::string> &selected_heading_names) {
    size_t i;
    for (i = 0; i < selected_heading_names.size(); ++i) {
      m_specs.push_back(
        compare_spec(headings, selected_heading_names[i].c_str()));      
    }    
  }
  
  template <typename Record>
  explicit compare_specs(const Record &headings) {
    rstd::vector<rstd::string> heading_names = headings.fields();
    
    size_t i;
    for (i = 0; i < heading_names.size(); ++i) {
      m_specs.push_back(compare_spec(headings, heading_names[i].c_str()));
    }    
  }
  
  ~compare_specs() {}
  
  size_t size() const { return m_specs.size(); }
  
  const_iterator begin() const { return &m_specs[0]; }
  const_iterator end() const { return begin() + m_specs.size(); }
  
  void swap(compare_specs &other) { m_specs.swap(other.m_specs); }
  
  const compare_spec &at(size_t n) const {
    NP1_ASSERT(n < size(), "compare_spec.at(n): n out of bounds");
    return *(begin() + n);
  }
  
  bool has_double() const {
    const_iterator ii = begin();
    const_iterator iz = end();
    for (; ii != iz; ++ii) {
      if (ii->is_double()) {
        return true;
      }
    }
    
    return false;
  }
  
private:
  rstd::vector<compare_spec> m_specs;
};


#define NP1_REL_DETAIL_COMPARE_SPECS_SORT_OPERATOR(name__, op__, reverse_op__) \
  struct name__ { \
    explicit name__(const detail::compare_specs &cs) : m_compare_specs(cs) {}  \
    bool operator()(const record_ref &r1, const record_ref &r2) { \
      detail::compare_specs::const_iterator spec = m_compare_specs.begin(); \
      detail::compare_specs::const_iterator spec_iz = m_compare_specs.end(); \
      for (; spec != spec_iz; ++spec) { \
        const str::ref f1 = r1.field(spec->field_number()); \
        const str::ref f2 = r2.field(spec->field_number()); \
        int result = spec->compare_function()(f1.ptr(), f1.length(), f2.ptr(), f2.length()); \
        if (result op__ 0) { \
          return true; \
        } \
        if (result reverse_op__ 0) { \
          return false; \
        } \
      } \
      return (r1.record_number() op__ r2.record_number()); \
    } \
    detail::compare_specs m_compare_specs; \
  }

NP1_REL_DETAIL_COMPARE_SPECS_SORT_OPERATOR(compare_specs_less_than_sort_operator, <, >);
NP1_REL_DETAIL_COMPARE_SPECS_SORT_OPERATOR(compare_specs_greater_than_sort_operator, >, <);


  
} // namespaces
}
}




#endif
