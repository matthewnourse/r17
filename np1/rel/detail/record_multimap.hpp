// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef RELOPT_RECORD_MULTIMAP_HPP
#define RELOPT_RECORD_MULTIMAP_HPP

#include "np1/rel/detail/compare_specs.hpp"
#include <multimap>
#include <stack>


namespace np1 {
namespace rel {
namespace detail {


// A record comparison function that takes two compatible lists of compare specs, one for each record.  The compare
// specs must be in the same order. 
template <typename Record1, typename Record2>
int hetero_record_compare(const Record1 &r1, const compare_specs &specs1,
                          const Record2 &r2, const compare_specs &specs2) {
  NP1_ASSERT(specs1.size() == specs2.size(), "compare spec sizes are not equal");
  
  compare_specs::const_iterator spec1 = specs1.begin();
  compare_specs::const_iterator spec1_iz = specs1.end();
  compare_specs::const_iterator spec2 = specs2.begin();
  
  for (; spec1 != spec1_iz; ++spec1, ++spec2) {        
    str::ref field1 = r1.field(spec1->field_number());    
    str::ref field2 = r2.field(spec2->field_number());
    
    if (field1.is_null() || field2.is_null()) {
      NP1_ASSERT(false, "Unable to find field while comparing hetero records");      
    }
        
    if (spec1->compare_function() == spec2->compare_function()) {
      int compare_result = spec1->compare_function()(field1.ptr(),
                                                      field1.length(),
                                                      field2.ptr(),
                                                      field2.length());                      
      if (compare_result != 0) {
        return compare_result;
      }
    } else {
      NP1_ASSERT(false, "Incompatible compare functions while comparing hetero records");     
    }
  }
  
  // If we get to here then all specs have returned equal.
  return 0;
}


/// A record comparison function that takes a list of compare specs.
template <typename Record1, typename Record2>
int record_compare(const Record1 &r1, const Record2 &r2, 
                    const compare_specs &specs) {
  return hetero_record_compare(r1, specs, r2, specs);
}




// A multimap where record_refs are the key.
// The record_refs are backed by "real" records.  record_refs are used as the key so that we don't have to construct a
// record just to do a lookup.
template <typename Value>
class record_multimap { 
private:
  // A key for the record multimap.
  struct record_multimap_key {
    record_multimap_key(const record_ref &ref, const compare_specs *specs)
    : m_ref(ref), m_specs(specs) {}
    
    void update_ref(const record_ref &ref) {
      m_ref = ref;
    }
     
    record_ref m_ref;
    const compare_specs *m_specs;
  };
  

  // A comparison function object, returns true if r1 < r2.
  struct lt_operator {
    bool operator()(const record_multimap_key &k1, 
                    const record_multimap_key &k2) const {
      return (hetero_record_compare(k1.m_ref, *k1.m_specs, 
                      k2.m_ref, *k2.m_specs) < 0);
    }
  };
  

  /// A payload for the record multimap.
  struct payload {
    payload(const record &r, const Value &v) : m_record(r), m_value(v) {}    
    record m_record;
    Value m_value;
  };

  /// The underlying map type.
  typedef typename std::multimap<record_multimap_key, payload, lt_operator> map_type;
                    
public:  
  /// Iterators.
  class iterator {
  public:
    iterator() {}
    iterator(typename map_type::iterator i) : m_i(i) {}
    typename map_type::iterator map_iterator() const { return m_i; }    
    const record_ref &key() const { return m_i->first.m_ref; }
    Value &value() const { return m_i->second.m_value; }
    iterator &operator++() { ++m_i; return *this; }
    iterator &operator--() { --m_i; return *this; }
    bool operator == (const iterator &o) { return m_i == o.m_i; }
    bool operator != (const iterator &o) { return m_i != o.m_i; }
  private:
    typename map_type::iterator m_i;
  };
  
  class const_iterator {
  public:
    const_iterator() {}
    const_iterator(typename map_type::const_iterator i) : m_i(i) {}
    const_iterator(iterator other) : m_i(other.map_iterator()) {}
    typename map_type::const_iterator map_iterator() const { return m_i; }    
    const record_ref &key() const { return m_i->first.m_ref; }
    const Value &value() const { return m_i->second.m_value; }
    const_iterator &operator++() { ++m_i; return *this; }
    const_iterator &operator--() { --m_i; return *this; }
    bool operator == (const const_iterator &o) { return m_i == o.m_i; }
    bool operator != (const const_iterator &o) { return m_i != o.m_i; }
  private:
    typename map_type::const_iterator m_i;
  };
  
    
public:
  record_multimap(const compare_specs &specs)
    : m_map(lt_operator()), m_specs(specs) {}
  
  ~record_multimap() {}
  
  // Find the first matching record.
  const_iterator find(const record_ref &ref) const { return find(ref, m_specs); }
  iterator find(const record_ref &ref) { return find(ref, m_specs); }
  
  // Find first matching record in this map even if records are hetero.
  const_iterator find(const record_ref &ref, const compare_specs &specs) const {
    record_multimap_key key(ref, &specs);
    return m_map.find(key);
  }

  iterator find(const record_ref &ref, const compare_specs &specs) {
    record_multimap_key key(ref, &specs);
    return m_map.find(key);
  }

  const_iterator lower_bound(const record_ref &ref) const {
    return lower_bound(ref, m_specs);
  }

  const_iterator lower_bound(const record_ref &ref,
                              const compare_specs &specs) const {
    record_multimap_key key(ref, &specs);
    return m_map.lower_bound(key);
  }

  const_iterator upper_bound(const record_ref &ref) const {
    return upper_bound(ref, m_specs);
  }

  const_iterator upper_bound(const record_ref &ref,
                              const compare_specs &specs) const {
    record_multimap_key key(ref, &specs);
    return m_map.upper_bound(key);
  }

  
  void insert(const record_ref &k, const Value &v) {
    typename map_type::iterator i = 
      m_map.insert(
        std::pair<record_multimap_key, payload>(
          record_multimap_key(k, &m_specs),
          payload(record(k), v)));
          
    // We need to update the record reference so it points to the 
    // record in the map rather than whatever memory is owned
    // by the caller.
    // We have to cast away const here because (understandably) we're
    // not meant to modify the key in-place.  However we're replacing
    // the key with another key that's semantically the same.
    ((record_multimap_key &)i->first).update_ref(i->second.m_record.ref());           
  }
  
  size_t size() const { return m_map.size(); }
    
  const_iterator begin() const { return m_map.begin(); }
  iterator begin() { return m_map.begin(); }
  
  const_iterator end() const { return m_map.end(); }  
  iterator end() { return m_map.end(); }
  
private:
  map_type m_map;
  compare_specs m_specs;  
};
  
} // namespaces
}
}


#endif
