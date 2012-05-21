// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_NP1_REL_DETAIL_QUICK_SORT_HPP
#define NP1_NP1_REL_DETAIL_QUICK_SORT_HPP


#include "np1/rel/record_ref.hpp"
#include "rstd/vector.hpp"


namespace np1 {
namespace rel {
namespace detail {

//TODO: to make this a stable sort, use the record number as a tiebreaker.
class quick_sort {
public:
  quick_sort() {}

  /// Add an element into the sorter's internal workings.
  void insert(const record_ref &r) { m_victims.push_back(r); }

  /// Sort the internally-stored list.
  template <typename Less_Than>
  void sort(Less_Than less_than) {
    if (m_victims.size() <= 0) {
      return;
    }

    sort(less_than, 0, m_victims.size() - 1);
  }

  /**
   * Must ONLY be called after sort().
   */
  template <typename Callback>
  void walk_sorted(Callback callback) const {
    rstd::vector<record_ref>::const_iterator i = m_victims.begin();
    rstd::vector<record_ref>::const_iterator iz = m_victims.end();

    for (; i < iz; ++i) {
      callback(*i);
    }
  }

  void clear() { m_victims.clear(); }
  bool empty() { return m_victims.empty(); }


private:
  /// Disable copy.
  quick_sort(const quick_sort &other);
  quick_sort &operator = (const quick_sort &other);

private:
  template <typename Less_Than>
  void sort(Less_Than &less_than, size_t left, size_t right) {
  label_tailcall:
    size_t pivot;
    size_t left_idx = left;
    size_t right_idx = right;

    if (right - left + 1 > 1) {
      pivot = (left + right) / 2;
      while ((left_idx <= pivot) && (right_idx >= pivot)) {
        while ((left_idx <= pivot) && less_than(m_victims[left_idx], m_victims[pivot])) {
          ++left_idx;
        }

        while ((right_idx >= pivot) && less_than(m_victims[pivot], m_victims[right_idx])) {
          --right_idx;
        }

        rstd::swap(m_victims[left_idx], m_victims[right_idx]);
        ++left_idx;
        --right_idx;
        if (left_idx - 1 == pivot) {
            pivot = ++right_idx;
        } else if (right_idx + 1 == pivot) {
            pivot = --left_idx;
        }
      }

      // Recurse into the smaller side first then tail-call into the other one.
      // This is the R. Sedgewick optimization.
      if (pivot - 1 - left > right - pivot + 1) {
        sort(less_than, left, pivot - 1);
        // This is the equivalent of a tail call to sort(less_than, pivot + 1, right)
        left = pivot + 1;
        goto label_tailcall;
      } else {
        sort(less_than, pivot + 1, right);
        // This is the equivalent of a tail call to sort(less_than, left, pivot - 1)
        right = pivot - 1;
        goto label_tailcall;
      }
    }
  }

private:
  rstd::vector<record_ref> m_victims; 
};

} // namespaces
}
}


#endif

