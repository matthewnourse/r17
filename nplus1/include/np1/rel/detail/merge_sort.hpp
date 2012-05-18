// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_NP1_REL_DETAIL_MERGE_SORT_HPP
#define NP1_NP1_REL_DETAIL_MERGE_SORT_HPP


#include "np1/rel/record_ref.hpp"
#include <vector>


namespace np1 {
namespace rel {
namespace detail {

/**
 * Merge sort is useful because it's stable (keeps entries in same relative
 * order), reliable (cf Quicksort, which on some inputs is awful) and has
 * good locality-of-reference properties.  We pretend that the data is a
 * linked list to avoid allocating extra space apart from that which is required
 * by the linked list itself.  This code based on
 * http://www.chiark.greenend.org.uk/~sgtatham/algorithms/listsort.c by Simon
 * Tatham.
 */
class merge_sort {
private:
  struct list_element {
    list_element() : next(0) {}
    explicit list_element(const record_ref &rec) : next(0), r(rec) {}
    list_element *next;
    record_ref r;
  };

public:
  merge_sort() : m_head(0) {}

  /// Add an element into the sorter's internal workings.
  void insert(const record_ref &r) { m_list.push_back(list_element(r)); }

  /// Sort the internally-stored list.
  template <typename Less_Than>
  void sort(Less_Than less_than) {
    if (!fixup_list_pointers()) {
      // No work to do.
      return;
    }

    list_element *p = 0;
    list_element *q = 0;
    list_element *e = 0;
    list_element *tail = 0;
    ssize_t insize, nmerges, psize, qsize, i;

    insize = 1;

    while (1) {
      p = m_head;
      m_head = 0;
      tail = 0;

      nmerges = 0;  /* count number of merges we do in this pass */

      while (p) {
        nmerges++;  /* there exists a merge to be done */
        /* step `insize' places along from p */
        q = p;
        psize = 0;
        for (i = 0; i < insize; i++) {
          psize++;
          q = q->next;
          if (!q) {
            break;
          }
        }

        /* if q hasn't fallen off end, we have two lists to merge */
        qsize = insize;

        /* now we have two lists; merge them */
        while ((psize > 0) || (qsize > 0 && q)) {

          /* decide whether next element of merge comes from p or q */
          if (psize == 0) {
            /* p is empty; e must come from q. */
            e = q; q = q->next; qsize--;        
          } else if (qsize == 0 || !q) {
            /* q is empty; e must come from p. */
            e = p; p = p->next; psize--;
          } else if (!less_than(q->r, p->r)) {
            /* First element of p is lower (or same);
             * e must come from p. */
            e = p; p = p->next; psize--;
          } else {
            /* First element of q is lower; e must come from q. */
            e = q; q = q->next; qsize--;
          }

          /* add the next element to the merged list */
          if (tail) {
            tail->next = e;
          } else {
            m_head = e;
          }

          tail = e;
        }

        /* now p has stepped `insize' places along, and q has too */
        p = q;
      }

      tail->next = NULL;
    
      /* If we have done only one merge, we're finished. */
      if (nmerges <= 1)   /* allow for nmerges==0, the empty list case */
        return;

      /* Otherwise repeat, merging lists twice the size */
      insize *= 2;
    }    
  }

  /**
   * Must ONLY be called after sort().
   */
  template <typename Callback>
  void walk_sorted(Callback callback) const {
    const list_element *list = m_head;
    while (list) {
      callback(list->r);
      list = list->next;
    }
  }

  void clear() {
    m_list.clear();
    m_head = 0;
  }

  bool empty() { return m_list.empty(); }

private:
  /// Disable copy.
  merge_sort(const merge_sort &other);
  merge_sort &operator = (const merge_sort &other);

private:
  bool fixup_list_pointers() {
    if (m_list.empty()) {
      m_head = 0;
      return false;
    }

    std::vector<list_element>::iterator i = m_list.begin();
    std::vector<list_element>::iterator iz = m_list.end();
    std::vector<list_element>::iterator next_i;

    m_head = i;

    while (true) {
      next_i = i + 1;
      if (next_i != iz) {
        i->next = next_i;
      } else {
        return true;      
      }

      i = next_i;
    }

    NP1_ASSERT(false, "Unreachable");
    return false;
  }

private:
  list_element *m_head;
  std::vector<list_element> m_list; 
};

} // namespaces
}
}


#endif

