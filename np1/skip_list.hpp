// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_SKIP_LIST_HPP
#define NP1_SKIP_LIST_HPP

#include <math.h>
#include <stdlib.h>
#include <time.h>

namespace np1 {

namespace detail {

template <typename V>
struct skip_list_less_than {
  bool operator()(const V &v1, const V &v2) const { return v1 < v2; }
};


struct skip_list_default_heap {
  void *alloc(size_t sz) {
    return rstd::detail::mem::alloc(sz);
  }
  
  void free(void *p) {
    return rstd::detail::mem::free(p);
  }
};

template <typename T, typename Heap>
class skip_list_typed_heap {
public:
  explicit skip_list_typed_heap(Heap &h) : m_heap(h) {}
  
  template <typename... Arguments>
  T *alloc_construct(const Arguments& ...arguments) {
    T *p = (T *)m_heap.alloc(sizeof(T));
    new (p) T(arguments...);
    return p;
  }
  
  void destruct_and_free(T *p) {
    rstd::detail::mem::destruct(p);
    m_heap.free(p);
  }
  
private:
  // Disable copy.
  skip_list_typed_heap(const skip_list_typed_heap &);
  skip_list_typed_heap &operator = (const skip_list_typed_heap &);
  
private:
  Heap &m_heap;
};

} // namespace detail




// See http://en.wikipedia.org/wiki/Skip_list.
template <typename V, typename Less_Than = detail::skip_list_less_than<V>,
          typename Heap = detail::skip_list_default_heap>
class skip_list {
public:
  enum { MAX_LEVEL = 7 };

  struct node {
    node() { initialize_forwards(m_forwards); }

    explicit node(const V &v) : m_value(v) {
      initialize_forwards(m_forwards);
    }

    ssize_t find_highest_level() {
      ssize_t i = MAX_LEVEL - 1;
      for (; i >= 0; --i) {
        if (m_forwards[i]) {
          return i;
        }
      }

      return -1;
    }

    V m_value;
    node *m_forwards[MAX_LEVEL];
  };


  /// Iterators.
  class const_iterator {
  public:
    const_iterator() : m_n(0) {}
    explicit const_iterator(node *n) : m_n(n) {}
    const V &operator*() const { return m_n->m_value; }
    const V *operator->() const { return &m_n->m_value; }
    const_iterator &operator++() { m_n = m_n->m_forwards[0]; return *this; }
    bool operator == (const const_iterator &o) { return m_n == o.m_n; }
    bool operator != (const const_iterator &o) { return m_n != o.m_n; }
  private:
    node *m_n;
  };

  class iterator : public const_iterator {
  public:
    iterator() : const_iterator(0) {}
    explicit iterator(node *n) : const_iterator(n) {}
    V &operator*() { return (V &)const_iterator::operator*(); }
    V *operator->() { return (V *)const_iterator::operator->(); }
    iterator &operator++() { const_iterator::operator++(); return *this; }
    bool operator == (const iterator &o) { return const_iterator::operator==(o); }
    bool operator != (const iterator &o) { return const_iterator::operator!=(o); }
  private:
  };


public:
  explicit skip_list(Heap &heap = get_default_heap()) : m_typed_heap(heap), m_size(0) {}

  ~skip_list() {
    clear();
  }

  // Get an iterator to the lowest node that is greater than or equal to k.
  template <typename K>
  iterator lower_bound(const K &k) {
    node *curr = &m_head;
    Less_Than lt;
    for (ssize_t i = MAX_LEVEL-1; i >= 0; --i) { 
      while (curr->m_forwards[i] && lt(curr->m_forwards[i]->m_value, k)) { 
        curr = curr->m_forwards[i];
      } 
    }

    curr = curr->m_forwards[0]; 
    return iterator(curr);
  }

  template <typename K>
  const_iterator lower_bound(const K &k) const {
    return ((skip_list *)this)->lower_bound(k);
  }



  // Find the node that compares equal to k, returns end() if not found.
  template <typename K>
  iterator find(const K &k) {
    iterator lb = lower_bound(k);

    if (end() == lb) {
      return lb;      
    }

    if (is_equal(*lb, k)) {
      return lb;
    }

    return end();
  }

  template <typename K>
  const_iterator find(const K &k) const {
    return ((skip_list *)this)->find(k);
  }


  // Insert a new node, returns false if there is already a node that compares
  // equal to v.
  bool insert(const V &v) {
    node *updates[MAX_LEVEL];
    node *lb = lower_bound_and_record_updates(v, updates);
    if (lb && is_equal(lb->m_value, v)) {
      return false;
    }

    ssize_t level_to_update = random_level();
    // Update the head node if necessary.
    ssize_t current_level;
    if ((current_level = m_head.find_highest_level()) < level_to_update) {
      for (ssize_t i = current_level + 1; i <= level_to_update; ++i) {
        updates[i] = &m_head;
      }
    }

    // Now do the actual insert.
    node *new_node = m_typed_heap.alloc_construct(v);
    for (ssize_t i = 0; i <= level_to_update; ++i) {
      new_node->m_forwards[i] = updates[i]->m_forwards[i];
      updates[i]->m_forwards[i] = new_node;
    }

    ++m_size;
    return true;        
  }
  
  // Remove the node that matches k from the list, returns false if not found.
  template <typename K>
  bool erase(const K &k) {
    node *updates[MAX_LEVEL];

    // Find the node in the list.
    node *n = lower_bound_and_record_updates(k, updates);
    if (!n || !is_equal(n->m_value, k)) {
      return false;
    }

    // Remove the node from the list.
    ssize_t current_level = m_head.find_highest_level();
    for (ssize_t i = 0; (i <= current_level) && (updates[i]->m_forwards[i] == n); ++i) {
      updates[i]->m_forwards[i] = n->m_forwards[i];   
    }
    
    m_typed_heap.destruct_and_free(n);
    --m_size;
    return true;
  }

  
  // Remove all nodes from the list.
  void clear() {
    node *curr = m_head.m_forwards[0];
    while (curr) {
      node *n = curr;
      curr = curr->m_forwards[0];
      m_typed_heap.destruct_and_free(n);
    }

    m_head = node();
    m_size = 0;
  }
  
  // Remove the first node from the list.
  void pop_front() {
    NP1_ASSERT(m_size > 0, "pop_front() on empty skip list");
    size_t i;
    node *old_first = m_head.m_forwards[0];    
    for (i = 0; i < MAX_LEVEL; ++i) {
      m_head.m_forwards[i] = old_first->m_forwards[i];
    }
    
    --m_size;
    m_typed_heap.destruct_and_free(old_first);
  }

  size_t size() const { return m_size; }  

  // Iterator functions.
  iterator begin() { return iterator(m_head.m_forwards[0]); }
  const_iterator begin() const { return const_iterator(m_head.m_forwards[0]); }

  iterator end() { return iterator(0); }
  const_iterator end() const { return const_iterator(0); }

  // Get the first node in the list.
  const V &front() const {
    NP1_ASSERT(m_size > 0, "front() on empty skip list");
    return m_head->m_value;
  }

private:
  /// Disable copy.
  skip_list(const skip_list &);
  skip_list &operator = (const skip_list &);

private:
  // Get the lower bound and record nodes that would require updating if
  // we insert.
  template <typename K>
  node *lower_bound_and_record_updates(const K &k, node *updates[]) {
    node *curr = &m_head;
    Less_Than lt;
    initialize_forwards(updates);

    for (ssize_t i = MAX_LEVEL-1; i >= 0; --i) { 
      while (curr->m_forwards[i] && lt(curr->m_forwards[i]->m_value, k)) { 
        curr = curr->m_forwards[i];
      }

      updates[i] = curr;
    }

    curr = curr->m_forwards[0]; 
    return curr;
  }

  static void initialize_forwards(node *forwards[]) {
    node **p = forwards;
    node **end = &forwards[MAX_LEVEL];
    for (; p < end; ++p) {
      *p = 0;
    }
  }

  template <typename V1, typename V2>
  static inline bool is_equal(const V1 &v1, const V2 &v2) {
    Less_Than lt;
    return !lt(v1, v2) && !lt(v2, v1);
  }

  static int random_level() {
    static bool first = true;
    static const float P = 0.5;

    if (first) {
      time_t now = ::time(0);
      srand(now);
      first = false;
    }

    int lvl = (int)(log((double)rand()/RAND_MAX)/log(1.-P));
    return lvl < MAX_LEVEL ? lvl : MAX_LEVEL-1;
  }

  static detail::skip_list_default_heap &get_default_heap() {
    static detail::skip_list_default_heap h;
    return h;
  }

private:
  detail::skip_list_typed_heap<node, Heap> m_typed_heap;
  node m_head;
  size_t m_size;
};

} // namespace


#endif
