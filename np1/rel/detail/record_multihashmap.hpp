// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef RELOPT_RECORD_MULTIHASHMAP_HPP
#define RELOPT_RECORD_MULTIHASHMAP_HPP


#include "rstd/vector.hpp"
#include "rstd/list.hpp"
#include "rstd/pair.hpp"
#include "np1/rel/detail/helper.hpp"
#include "np1/rel/detail/compare_specs.hpp"


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
  
// A non-unique hash table of records.
template <typename Value>
class record_multihashmap {
public:
  /// Hash table tuning stuff.
  enum {
    INITIAL_APPROX_MAX_CHAIN_SIZE = 20,
    
    // The hash algorithm likes power-of-two tables.    
    INITIAL_HASH_TABLE_SIZE = 65536    
  };
  
  // A list of records that compare equal.
  typedef rstd::list<rstd::pair<record, Value> > equal_list_type;
  
  // A chain of records that hash to the same value.
  typedef rstd::list<equal_list_type> hash_chain_type;
  
  // The hash table itself - a list of chains.
  typedef rstd::vector<hash_chain_type> hash_map_type;
  
public:
  record_multihashmap(const compare_specs &specs,
                      size_t initial_size_hint = INITIAL_HASH_TABLE_SIZE)
    : m_specs(specs), m_approx_max_chain_size(INITIAL_APPROX_MAX_CHAIN_SIZE),
      m_max_hash_table_size(environment::max_hash_table_size()) {
    if (initial_size_hint > m_max_hash_table_size) {
      initial_size_hint = m_max_hash_table_size;
    }

    NP1_ASSERT(try_init(initial_size_hint, 0) == hash_map_type::SEARCH_CAPACITY_SUCCESS,
                "Unable to initialize hash map");
  }


  ~record_multihashmap() {}
  
  // Insert a value into this hash map.
  void insert(const record_ref &r, const Value &v) {
    NP1_ASSERT(m_map.size() > 0, "Attempt to insert into an invalid multihashmap");
    hash_chain_type *hash_chain = find_hash_chain(r, m_specs);
    
    // Try to find a list of records in this hash chain that compare
    // equal to the new record.
    equal_list_type *equal_list = find_equal_list(r, *hash_chain, m_specs);
    
    if (equal_list) {
      equal_list->push_back(rstd::make_pair(record(r), v));
    } else {
      // There are no matching records, just make a new list.
      equal_list_type new_list;
      new_list.push_back(rstd::make_pair(record(r), v));
      hash_chain->push_back(equal_list_type());
      hash_chain->back().swap(new_list);
    }
            
    if (hash_chain->size() > m_approx_max_chain_size) {
      // The chain is too long.  Ideally we'll make a new & bigger hash map and 
      // copy all the elements into it.  TODO: take table utilisation
      // into account to avoid DOS where someone deliberately adds
      // values that hash to the same number.
      // The hash algorithm likes power-of-two tables.  If we can't easily
      // allocate a new hash table or we've exceeded the user-specified maximum
      // then just increase the max chain size.
      if (m_map.size() >= m_max_hash_table_size) {
        ++m_approx_max_chain_size;
      } else {
        size_t desired_hash_table_size = m_map.size() * 2;
        if (desired_hash_table_size > m_max_hash_table_size) {
          desired_hash_table_size = m_max_hash_table_size;
        }

        record_multihashmap new_hash_map(m_specs);
        switch (new_hash_map.try_init(desired_hash_table_size, m_map.size())) {
        case hash_map_type::SEARCH_CAPACITY_SUCCESS:
          for_each(insertion_iterator(new_hash_map));
          swap(new_hash_map);
          break;
  
        case hash_map_type::SEARCH_CAPACITY_SUCCESS_WITH_RETRY:
          for_each(insertion_iterator(new_hash_map));
          swap(new_hash_map);
          ++m_approx_max_chain_size;
          break;
  
        case hash_map_type::SEARCH_CAPACITY_FAIL:
          ++m_approx_max_chain_size;
          break;
        }
      }
    }
  }

  
  // Find a record in the hash map.  
  // Returns a pointer to the list of matching record/value pairs or NULL if 
  // there is no matching value.
  equal_list_type *find(const record_ref &r) { return find(r, m_specs); }
  
  equal_list_type *find(const record_ref &r, const compare_specs &specs) {
     return find_equal_list(r, *find_hash_chain(r, specs), specs);
  }
  
  const equal_list_type *find(const record_ref &r) const {
    return find(r, m_specs); 
  }

  const equal_list_type *find(const record_ref &r, 
                              const compare_specs &specs) const {
    return ((record_multihashmap *)this)->find(r, specs); 
  }

  // Iterate over all the values.
  /**
   * The iterator must have the following prototype:
   * bool iter(const record &r, const Value &v);
   * 
   * If iter returns true then iteration will continue, otherwise it will 
   * stop.
   * 
   * @return true if iteration completes normally.
   */
  template <typename Iterator>
  bool for_each(Iterator iter) const {
    typename hash_map_type::const_iterator chain_i;
    typename hash_map_type::const_iterator chain_iz = m_map.end();
        
    for (chain_i = m_map.begin(); chain_i != chain_iz; ++chain_i) {
      typename hash_chain_type::const_iterator equal_list_i;
      typename hash_chain_type::const_iterator equal_list_iz = chain_i->end();      
      for (equal_list_i = chain_i->begin(); 
        (equal_list_i != equal_list_iz);
        ++equal_list_i) {
        if (!for_each(iter, *equal_list_i)) {
          return false;
        }
      }
    }
    
    return true;
  }


  // Iterate over all the values in the supplied equals list.
  template <typename Iterator>
  bool for_each(Iterator iter, const equal_list_type &equal_list) const {
    typename equal_list_type::const_iterator entry_i; 
    typename equal_list_type::const_iterator entry_iz = equal_list.end();
    for (entry_i = equal_list.begin(); (entry_i != entry_iz); ++entry_i) {
      if (!iter(entry_i->first, entry_i->second)) {
        return false;
      }
    }
    
    return true;
  }


  // Iterate over all the values that match the supplied record.
  template <typename Iterator>
  bool for_each(Iterator iter, const record_ref &r, 
                const compare_specs &specs) const {
    const equal_list_type *equal_list = find(r, specs);
    if (equal_list) {
      return for_each(iter, *equal_list);
    }
    
    return true;
  }
  
  template <typename Iterator>
  bool for_each(Iterator iter, const record_ref &r) {
    return for_each(iter, r, m_specs);
  }
  
  
  void swap(record_multihashmap &other) {
    m_map.swap(other.m_map);
    m_specs.swap(other.m_specs);
  }

private:
  hash_chain_type *find_hash_chain(const record_ref &r, const compare_specs &specs) {
    size_t hash_offset = hash(r, specs);
    return &m_map[hash_offset];
  }
  
  // Returns NULL if there is no such equal list.
  equal_list_type *find_equal_list(const record_ref &r, const hash_chain_type &hash_chain, const compare_specs &specs) {
    typename hash_chain_type::const_iterator equal_list_i;
    typename hash_chain_type::const_iterator equal_list_iz 
                            = hash_chain.end();
    for (equal_list_i = hash_chain.begin(); 
        (equal_list_i != equal_list_iz);
        ++equal_list_i) {
      NP1_ASSERT(equal_list_i->size(), "Empty equal_list");
      if (hetero_record_compare(equal_list_i->front().first, m_specs,
                                r, specs) == 0) {
        return (equal_list_type *)&(*equal_list_i);
      }
    }

    return NULL;
  }
  

  // An iterator to insert records into a new hash map.
  struct insertion_iterator {
    insertion_iterator(record_multihashmap &m) : m_new_hash_map(m) {}
    bool operator()(const record &r, const Value &v) const {
      m_new_hash_map.insert(r.ref(), v);
      return true;    
    }
    
    record_multihashmap &m_new_hash_map;
  };
  

  // Hash a record and modulate the hash for the size of the hash table.
  template <typename Record>
  size_t hash(const Record &r, const compare_specs &specs) const {
    compare_specs::const_iterator spec = specs.begin();
    compare_specs::const_iterator spec_iz = specs.end();
    uint64_t hval = helper::hash_init();
    
    for (; spec != spec_iz; ++spec) {        
      const str::ref f = r.field(spec->field_number());    
            
      if (f.is_null()) {
        rstd::string error_message =
          "Unable to find field " + str::to_dec_str(spec->field_number())
          + " while hashing.  Record number: "
          + str::to_dec_str(r.record_number());
            
        NP1_ASSERT(false, error_message);
      }
      
      hval = spec->hash_function()(f.ptr(), f.length(), hval);               
    }

    return (size_t)(((size_t)hval) % m_map.size());
  }
  
  typename hash_map_type::search_capacity_result_type
  try_init(size_t initial_size_hint, size_t old_map_size) {
    if (initial_size_hint <= 0) {
      initial_size_hint = 1;
    }

    size_t min_capacity = old_map_size * 1.25;
    if (0 == min_capacity) {
      min_capacity = 1;
    }

    typename hash_map_type::search_capacity_result_type result =
      m_map.search_capacity(initial_size_hint, old_map_size+1);
        
    m_map.resize(m_map.capacity());

    return result;
  }
  
private:
  // A vector of hash chains where each exact-matching entry is a list of
  // records.
  hash_map_type m_map;
  compare_specs m_specs;
  size_t m_approx_max_chain_size;
  uint64_t m_max_hash_table_size;
};
  
  
} // namespaces
}
}



#endif
