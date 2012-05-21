// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_REL_ORDER_BY_HPP
#define NP1_REL_ORDER_BY_HPP


#include "np1/rel/detail/quick_sort.hpp"
#include "np1/rel/detail/merge_sort.hpp"
#include "np1/rel/detail/sort_manager.hpp"
#include "np1/rel/rlang/rlang.hpp"

namespace np1 {
namespace rel {


class order_by {
public:
  typedef enum { TYPE_MERGE_SORT, TYPE_QUICK_SORT } sort_type_type;
  typedef enum { ORDER_ASCENDING, ORDER_DESCENDING } sort_order_type;

public:
  template <typename Input_Stream, typename Output_Stream>
  void operator()(Input_Stream &input, Output_Stream &output,
                  const rstd::vector<rel::rlang::token> &tokens,
                  sort_type_type sort_type,
                  sort_order_type sort_order) {
    NP1_ASSERT(tokens.size() > 0, "Unexpected empty stream operator argument list");

    // Read the first line of input, we need it to add meaning to the arguments.
    record headings(input.parse_headings()); 

    rstd::vector<rstd::string> arg_headings;

    rlang::compiler::compile_heading_name_list(
                      tokens, headings.ref(), arg_headings);

    // Create the compare specs.
    detail::compare_specs comp_specs(headings, arg_headings);

    // Write out the headings then do the actual sorting.
    headings.write(output);

    less_than lt(comp_specs);
    greater_than gt(comp_specs);
    switch (sort_type) {
    case TYPE_MERGE_SORT:
      switch (sort_order) {
        case ORDER_ASCENDING:
          sort<detail::merge_sort>(input, output, lt);
          break;
        
        case ORDER_DESCENDING:
          sort<detail::merge_sort>(input, output, gt);
          break;
      }
      break;

    case TYPE_QUICK_SORT:
      //TODO: why the dickens isn't this quick sort and why is quick sort broken?
      switch (sort_order) {
        case ORDER_ASCENDING:
          sort<detail::merge_sort>(input, output, lt);
          break;
        
        case ORDER_DESCENDING:
          sort<detail::merge_sort>(input, output, gt);
          break;
      }
      break;
    }
  }

private:
  struct empty_type {};

  // The actual sorting.
  template <typename Sorter, typename Input, typename Output, typename Sort_Operator>
  void sort(Input &input, Output &output, Sort_Operator &op) {
    typedef detail::sort_manager<Sort_Operator, Sorter> sort_manager_type;
    typename sort_manager_type::sort_state sort_state(op);
    sort_manager_type sorter(sort_state);
    input.parse_records(sorter);
    sorter.finalize(output);
  }

#define NP1_REL_ORDER_BY_SORT_OPERATOR(name__, op__, reverse_op__) \
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

NP1_REL_ORDER_BY_SORT_OPERATOR(less_than, <, >);
NP1_REL_ORDER_BY_SORT_OPERATOR(greater_than, >, <);
};


} //namespaces
}


#endif
