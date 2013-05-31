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

    detail::compare_specs_less_than_sort_operator lt(comp_specs);
    detail::compare_specs_greater_than_sort_operator gt(comp_specs);
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
    record_output_walker<Output> output_walker(output);
    sorter.finalize(output_walker);
  }

  template <typename Output_Stream>
  struct record_output_walker {
    explicit record_output_walker(Output_Stream &o) : m_output(o) {}
    void operator()(const record_ref &r) {
      r.write(m_output);
    }

    Output_Stream &m_output;
  };

};


} //namespaces
}


#endif
