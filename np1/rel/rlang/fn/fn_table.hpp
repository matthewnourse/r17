// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_REL_RLANG_FN_FN_TABLE_HPP
#define NP1_REL_RLANG_FN_FN_TABLE_HPP


#include "np1/rel/rlang/fn/fn.hpp"
#include "np1/rel/rlang/fn/op.hpp"
#include "np1/preproc.hpp"

namespace np1 {
namespace rel {
namespace rlang {
namespace fn {


/// The base for all wrappers.
template <typename Return, typename Target, size_t Number_Arguments>
struct wrap_base {
  typedef Target target_type;
  static const size_t number_arguments = Number_Arguments;
  static const dt::data_type return_data_type_enum = dt::to_data_type_enum<Return>::value;
  static const bool is_push_this = false;
  static const bool is_push_other = false;

  static bool is_name_match(const str::ref &name) {
    return ((str::cmp(name, target_type::name()) == 0)
            || (target_type::synonym() && (str::cmp(name, target_type::synonym()) == 0)));
  }

  static bool is_name_and_arg_count_match(
                  const str::ref &name, const simulated_stack &available_args,
                  size_t called_arg_count) {
    if (!is_name_match(name) || (available_args.size() < number_arguments)) {
      return false;
    }

    return (number_arguments == called_arg_count);
  }

  static bool name_starts_with(const str::ref &partial_name) {
        return (str::starts_with(target_type::name(), partial_name)
            || (target_type::synonym()
                && str::starts_with(target_type::synonym(), partial_name)));
  }
};


/// Wrappers for functions that take no arguments.
template <typename Return, typename Target>
struct wrap0_base : public wrap_base<Return, Target, 0> {
  template <typename Receiver>  
  static void get_info(Receiver &receiver) {
    receiver(Target::since(), Target::name(), Target::synonym(), Target::description(),
              Target::is_operator, Target::precedence, Target::is_left_assoc,
              0, 0, dt::to_data_type_enum<Return>::value);
  }

  static bool is_match(const str::ref &name, const simulated_stack &available_args,
                        size_t called_arg_count) {      
    return wrap_base<Return, Target, 0>::is_name_and_arg_count_match(
                                          name, available_args, called_arg_count);
  }
};

template <typename Return, typename Target>
struct wrap0_noheap : public wrap0_base<Return, Target> {
  inline static size_t call(vm_stack &stk, vm_heap &heap, const vm_literals &lit,
                            const record_ref &this_r, const record_ref &other_r,
                            size_t record_or_literal_number) {    
    stk.push(Target::call()); return 1;
  }  
};

template <typename Return, typename Target>
struct wrap0_heap : public wrap0_base<Return, Target> {
  inline static size_t call(vm_stack &stk, vm_heap &heap, const vm_literals &lit,
                            const record_ref &this_r, const record_ref &other_r,
                            size_t record_or_literal_number) {    
    stk.push(Target::call(heap)); return 1;
  }  
};


/// Wrappers for unary functions.
template <typename Return, typename Target, typename A1>
struct wrap1_base : public wrap_base<Return, Target, 1> {
  typedef A1 arg1_type;

  template <typename Receiver>
  static void get_info(Receiver &receiver) {
    dt::data_type arg_type_enum = dt::to_data_type_enum<arg1_type>::value;

    receiver(Target::since(), Target::name(), Target::synonym(), Target::description(),
              Target::is_operator, Target::precedence, Target::is_left_assoc,
              &arg_type_enum, 1, dt::to_data_type_enum<Return>::value);
  }


  static bool is_match(const str::ref &name, const simulated_stack &available_args,
                        size_t called_arg_count) {
      
    return (wrap_base<Return, Target, 1>::is_name_and_arg_count_match(
                                            name, available_args, called_arg_count)
            && dt::is_cpp_implementation_of<arg1_type>::f(available_args.top()));
  }
};

template <typename Return, typename Target, typename A1>
struct wrap1_noheap : public wrap1_base<Return, Target, A1> {
  inline static size_t call(vm_stack &stk, vm_heap &heap, const vm_literals &lit,
                            const record_ref &this_r, const record_ref &other_r,
                            size_t record_or_literal_number) {    
    A1 v1; stk.pop(v1); stk.push(Target::call(v1));  return 1;
  }  
};

template <typename Return, typename Target, typename A1>
struct wrap1_heap : public wrap1_base<Return, Target, A1> {
  inline static size_t call(vm_stack &stk, vm_heap &heap, const vm_literals &lit,
                            const record_ref &this_r, const record_ref &other_r,
                            size_t record_or_literal_number) {    
    A1 v1; stk.pop(v1); stk.push(Target::call(heap, v1)); return 1;
  }  
};


/// Wrappers for binary functions.
template <typename Return, typename Target, typename A1, typename A2>
struct wrap2_base : public wrap_base<Return, Target, 2> {
  typedef A1 arg1_type;
  typedef A2 arg2_type;

  template <typename Receiver>
  static void get_info(Receiver &receiver) {
    dt::data_type arg_type_enums[2];
    arg_type_enums[0] = dt::to_data_type_enum<arg1_type>::value;
    arg_type_enums[1] = dt::to_data_type_enum<arg2_type>::value;

    receiver(Target::since(), Target::name(), Target::synonym(), Target::description(),
              Target::is_operator, Target::precedence, Target::is_left_assoc,
              arg_type_enums, 2, dt::to_data_type_enum<Return>::value);
  }

  static bool is_match(const str::ref &name, const simulated_stack &available_args,
                        size_t called_arg_count) {      
    return (wrap_base<Return, Target, 2>::is_name_and_arg_count_match(
                                              name, available_args, called_arg_count)
            && dt::is_cpp_implementation_of<arg1_type>::f(available_args.top_minus_1())
            && dt::is_cpp_implementation_of<arg2_type>::f(available_args.top()));
  }
};

template <typename Return, typename Target, typename A1, typename A2>
struct wrap2_noheap : public wrap2_base<Return, Target, A1, A2> {
  inline static size_t call(vm_stack &stk, vm_heap &heap, const vm_literals &lit,
                            const record_ref &this_r, const record_ref &other_r,
                            size_t record_or_literal_number) {    
    A1 v1; A2 v2; stk.pop(v2); stk.pop(v1); stk.push(Target::call(v1, v2)); return 1;
  }  
};

template <typename Return, typename Target, typename A1, typename A2>
struct wrap2_heap : public wrap2_base<Return, Target, A1, A2> {
  inline static size_t call(vm_stack &stk, vm_heap &heap, const vm_literals &lit,
                            const record_ref &this_r, const record_ref &other_r,
                            size_t record_or_literal_number) {    
    A1 v1; A2 v2; stk.pop(v2); stk.pop(v1); stk.push(Target::call(heap, v1, v2)); return 1;
  }  
};


/// Wrappers for 3-ary functions.
template <typename Return, typename Target, typename A1, typename A2, typename A3>
struct wrap3_base : public wrap_base<Return, Target, 3> {
  typedef A1 arg1_type;
  typedef A2 arg2_type;
  typedef A3 arg3_type;

  template <typename Receiver>
  static void get_info(Receiver &receiver) {
    dt::data_type arg_type_enums[3];
    arg_type_enums[0] = dt::to_data_type_enum<arg1_type>::value;
    arg_type_enums[1] = dt::to_data_type_enum<arg2_type>::value;
    arg_type_enums[2] = dt::to_data_type_enum<arg3_type>::value;

    receiver(Target::since(), Target::name(), Target::synonym(), Target::description(),
              Target::is_operator, Target::precedence, Target::is_left_assoc,
              arg_type_enums, 3, dt::to_data_type_enum<Return>::value);
  }

  static bool is_match(const str::ref &name, const simulated_stack &available_args,
                        size_t called_arg_count) {      
    return (wrap_base<Return, Target, 3>::is_name_and_arg_count_match(
                                              name, available_args, called_arg_count)
            && dt::is_cpp_implementation_of<arg1_type>::f(available_args.top_minus_2())
            && dt::is_cpp_implementation_of<arg2_type>::f(available_args.top_minus_1())
            && dt::is_cpp_implementation_of<arg3_type>::f(available_args.top()));
  }
};

template <typename Return, typename Target, typename A1, typename A2, typename A3>
struct wrap3_noheap : public wrap3_base<Return, Target, A1, A2, A3> {
  inline static size_t call(vm_stack &stk, vm_heap &heap, const vm_literals &lit,
                            const record_ref &this_r, const record_ref &other_r,
                            size_t record_or_literal_number) {    
    A1 v1; A2 v2; A3 v3; stk.pop(v3); stk.pop(v2); stk.pop(v1);
    stk.push(Target::call(v1, v2, v3));
    return 1;
  }  
};

template <typename Return, typename Target, typename A1, typename A2, typename A3>
struct wrap3_heap : public wrap3_base<Return, Target, A1, A2, A3> {
  inline static size_t call(vm_stack &stk, vm_heap &heap, const vm_literals &lit,
                            const record_ref &this_r, const record_ref &other_r,
                            size_t record_or_literal_number) {    
    A1 v1; A2 v2; A3 v3; stk.pop(v3); stk.pop(v2); stk.pop(v1);
    stk.push(Target::call(heap, v1, v2, v3));
    return 1;
  }  
};


/// Wrappers for pushing literals.
template <typename Return, typename Target>
struct wrap_push_literal : public wrap_base<Return, Target, 0> {
  template <typename Receiver>
  static void get_info(Receiver &receiver) {
    receiver(Target::since(), Target::name(), Target::synonym(), Target::description(),
              Target::is_operator, Target::precedence, Target::is_left_assoc,
              0, 0, dt::to_data_type_enum<Return>::value);
  }

  static bool is_match(const str::ref &name, const simulated_stack &available_args,
                        size_t called_arg_count) {      
    return wrap_base<Return, Target, 0>::is_name_and_arg_count_match(
                                          name, available_args, called_arg_count);
  }

  inline static size_t call(vm_stack &stk, vm_heap &heap, const vm_literals &lit,
                            const record_ref &this_r, const record_ref &other_r,
                            size_t record_or_literal_number) {    
    stk.push(Target::call(lit, record_or_literal_number)); return 1;
  }  
};


/// Wrappers for pushing variables from "this" record.
template <typename Return, typename Target>
struct wrap_push_this : public wrap_base<Return, Target, 0> {
  static const bool is_push_this = true;
  
  template <typename Receiver>
  static void get_info(Receiver &receiver) {
    receiver(Target::since(), Target::name(), Target::synonym(), Target::description(),
              Target::is_operator, Target::precedence, Target::is_left_assoc,
              0, 0, dt::to_data_type_enum<Return>::value);
  }

  static bool is_match(const str::ref &name, const simulated_stack &available_args,
                        size_t called_arg_count) {      
    return wrap_base<Return, Target, 0>::is_name_and_arg_count_match(
                                            name, available_args, called_arg_count);
  }

  inline static size_t call(vm_stack &stk, vm_heap &heap, const vm_literals &lit,
                            const record_ref &this_r, const record_ref &other_r,
                            size_t record_or_literal_number) {    
    stk.push(Target::call(this_r, record_or_literal_number)); return 1;   
  }  
};


/// Wrappers for pushing variables from the "other" record.
template <typename Return, typename Target>
struct wrap_push_other : public wrap_base<Return, Target, 0> {
  static const bool is_push_other = false;
  
  template <typename Receiver>
  static void get_info(Receiver &receiver) {
    receiver(Target::since(), Target::name(), Target::synonym(), Target::description(),
              Target::is_operator, Target::precedence, Target::is_left_assoc,
              0, 0, dt::to_data_type_enum<Return>::value);
  }

  static bool is_match(const str::ref &name, const simulated_stack &available_args,
                        size_t called_arg_count) {      
    return wrap_base<Return, Target, 0>::is_name_and_arg_count_match(
                                            name, available_args, called_arg_count);
  }

  inline static size_t call(vm_stack &stk, vm_heap &heap, const vm_literals &lit,
                            const record_ref &this_r, const record_ref &other_r,
                            size_t record_or_literal_number) {    
    stk.push(Target::call(other_r, record_or_literal_number)); return 1;
  }  
};


/// Wrapper for internal branch expressions.
template <typename Return, typename Target>
struct wrap_branch : public wrap_base<Return, Target, 0> {
  template <typename Receiver>
  static void get_info(Receiver &receiver) {
    receiver(Target::since(), Target::name(), Target::synonym(), Target::description(),
              Target::is_operator, Target::precedence, Target::is_left_assoc,
              0, 0, dt::to_data_type_enum<Return>::value);
  }

  static bool is_match(const str::ref &name, const simulated_stack &available_args,
                        size_t called_arg_count) {      
    return wrap_base<Return, Target, 0>::is_name_and_arg_count_match(
                                            name, available_args, called_arg_count);
  }

  inline static size_t call(vm_stack &stk, vm_heap &heap, const vm_literals &lit,
                            const record_ref &this_r, const record_ref &other_r,
                            size_t offset) {    
    return Target::call(stk, offset);
  }  
};






#define NP1_REL_RLANG_FN_SUPPORTED_EQUALITY_COMPARISON_OPERATOR_WRAPPED_FNS(name__) \
  (wrap2_noheap<dt::boolean, name__, dt::string, dt::string>), \
  (wrap2_noheap<dt::boolean, name__, dt::string, dt::istring>), \
  (wrap2_noheap<dt::boolean, name__, dt::istring, dt::string>), \
  (wrap2_noheap<dt::boolean, name__, dt::istring, dt::istring>), \
  (wrap2_noheap<dt::boolean, name__, dt::integer, dt::integer>), \
  (wrap2_noheap<dt::boolean, name__, dt::uinteger, dt::uinteger>), \
  (wrap2_noheap<dt::boolean, name__, dt::boolean, dt::boolean>), \
  (wrap2_noheap<dt::boolean, name__, dt::ipaddress, dt::ipaddress>), \
  (wrap2_noheap<dt::boolean, name__, dt::string, dt::ipaddress>), \
  (wrap2_noheap<dt::boolean, name__, dt::ipaddress, dt::string>), \
  (wrap2_noheap<dt::boolean, name__, dt::istring, dt::ipaddress>), \
  (wrap2_noheap<dt::boolean, name__, dt::ipaddress, dt::istring>) 

#define NP1_REL_RLANG_FN_SUPPORTED_NON_EQUALITY_COMPARISON_OPERATOR_WRAPPED_FNS(name__) \
  NP1_REL_RLANG_FN_SUPPORTED_EQUALITY_COMPARISON_OPERATOR_WRAPPED_FNS(name__), \
  (wrap2_noheap<dt::boolean, name__, dt::fdouble, dt::fdouble>)


#define NP1_REL_RLANG_FN_SUPPORTED_BINARY_ARITHMETIC_OPERATOR_WRAPPED_FNS(name__) \
  (wrap2_noheap<dt::integer, name__, dt::integer, dt::integer>), \
  (wrap2_noheap<dt::uinteger, name__, dt::uinteger, dt::uinteger>)


#define NP1_REL_RLANG_FN_SUPPORTED_STRING_MATCH_FNS(name__) \
  (wrap2_noheap<dt::boolean, name__, dt::string, dt::string>), \
  (wrap2_noheap<dt::boolean, name__, dt::string, dt::istring>), \
  (wrap2_noheap<dt::boolean, name__, dt::istring, dt::string>), \
  (wrap2_noheap<dt::boolean, name__, dt::istring, dt::istring>)


/// The one true list of supported functions.
/**
 * Overloads MUST be grouped together by function/operator name.
 */
#define NP1_REL_RLANG_FN_TABLE \
  NP1_REL_RLANG_FN_SUPPORTED_EQUALITY_COMPARISON_OPERATOR_WRAPPED_FNS(op::eq), \
  NP1_REL_RLANG_FN_SUPPORTED_EQUALITY_COMPARISON_OPERATOR_WRAPPED_FNS(op::ne), \
  NP1_REL_RLANG_FN_SUPPORTED_NON_EQUALITY_COMPARISON_OPERATOR_WRAPPED_FNS(op::lt), \
  NP1_REL_RLANG_FN_SUPPORTED_NON_EQUALITY_COMPARISON_OPERATOR_WRAPPED_FNS(op::gt), \
  NP1_REL_RLANG_FN_SUPPORTED_NON_EQUALITY_COMPARISON_OPERATOR_WRAPPED_FNS(op::le), \
  NP1_REL_RLANG_FN_SUPPORTED_NON_EQUALITY_COMPARISON_OPERATOR_WRAPPED_FNS(op::ge), \
\
  (wrap2_heap<dt::string, op::string_plus, dt::string, dt::string>), \
  (wrap2_heap<dt::istring, op::string_plus, dt::istring, dt::string>), \
  (wrap2_heap<dt::istring, op::string_plus, dt::string, dt::istring>), \
  (wrap2_heap<dt::istring, op::string_plus, dt::istring, dt::istring>), \
\
  NP1_REL_RLANG_FN_SUPPORTED_BINARY_ARITHMETIC_OPERATOR_WRAPPED_FNS(op::plus), \
  (wrap2_noheap<dt::fdouble, op::plus, dt::fdouble, dt::fdouble>), \
  NP1_REL_RLANG_FN_SUPPORTED_BINARY_ARITHMETIC_OPERATOR_WRAPPED_FNS(op::minus), \
  (wrap2_noheap<dt::fdouble, op::minus, dt::fdouble, dt::fdouble>), \
  (wrap1_noheap<dt::integer, op::unary_minus, dt::integer>), \
  (wrap1_noheap<dt::fdouble, op::unary_minus, dt::fdouble>), \
  NP1_REL_RLANG_FN_SUPPORTED_BINARY_ARITHMETIC_OPERATOR_WRAPPED_FNS(op::multiply), \
  (wrap2_noheap<dt::fdouble, op::multiply, dt::fdouble, dt::fdouble>), \
  NP1_REL_RLANG_FN_SUPPORTED_BINARY_ARITHMETIC_OPERATOR_WRAPPED_FNS(op::divide), \
  (wrap2_noheap<dt::fdouble, op::divide, dt::fdouble, dt::fdouble>), \
  NP1_REL_RLANG_FN_SUPPORTED_BINARY_ARITHMETIC_OPERATOR_WRAPPED_FNS(op::modulus), \
  NP1_REL_RLANG_FN_SUPPORTED_BINARY_ARITHMETIC_OPERATOR_WRAPPED_FNS(op::bitwise_and), \
  NP1_REL_RLANG_FN_SUPPORTED_BINARY_ARITHMETIC_OPERATOR_WRAPPED_FNS(op::bitwise_or), \
  (wrap1_noheap<dt::integer, op::bitwise_not, dt::integer>), \
  (wrap1_noheap<dt::uinteger, op::bitwise_not, dt::uinteger>), \
\
  (wrap2_noheap<dt::boolean, op::logical_and, dt::boolean, dt::boolean>), \
  (wrap2_noheap<dt::boolean, op::logical_or, dt::boolean, dt::boolean>), \
  (wrap1_noheap<dt::boolean, op::logical_not, dt::boolean>), \
\
  (wrap_branch<dt::boolean, internal_if>), \
  (wrap_branch<dt::boolean, internal_goto>), \
\
  (wrap_push_literal<dt::string, internal_push_literal_string>), \
  (wrap_push_literal<dt::integer, internal_push_literal_integer>), \
  (wrap_push_literal<dt::integer, internal_push_literal_uinteger>), \
  (wrap_push_literal<dt::integer, internal_push_literal_double>), \
  (wrap_push_literal<dt::boolean, internal_push_literal_boolean_true>), \
  (wrap_push_literal<dt::boolean, internal_push_literal_boolean_false>), \
\
  (wrap_push_this<dt::string, internal_push_field_string>), \
  (wrap_push_this<dt::istring, internal_push_field_istring>), \
  (wrap_push_this<dt::integer, internal_push_field_integer>), \
  (wrap_push_this<dt::uinteger, internal_push_field_uinteger>), \
  (wrap_push_this<dt::fdouble, internal_push_field_double>), \
  (wrap_push_this<dt::boolean, internal_push_field_boolean>), \
  (wrap_push_this<dt::ipaddress, internal_push_field_ipaddress>), \
  (wrap_push_this<dt::uinteger, internal_push_rownum>), \
\
  (wrap_push_other<dt::string, internal_push_field_string>), \
  (wrap_push_other<dt::istring, internal_push_field_istring>), \
  (wrap_push_other<dt::integer, internal_push_field_integer>), \
  (wrap_push_other<dt::uinteger, internal_push_field_uinteger>), \
  (wrap_push_other<dt::fdouble, internal_push_field_double>), \
  (wrap_push_other<dt::boolean, internal_push_field_boolean>), \
  (wrap_push_other<dt::ipaddress, internal_push_field_ipaddress>), \
  (wrap_push_other<dt::uinteger, internal_push_rownum>), \
\
  (wrap1_noheap<dt::string, to_string, dt::string>), \
  (wrap1_noheap<dt::string, to_string, dt::istring>), \
  (wrap1_heap<dt::string, to_string, dt::integer>), \
  (wrap1_heap<dt::string, to_string, dt::uinteger>), \
  (wrap1_heap<dt::string, to_string, dt::fdouble>), \
  (wrap1_noheap<dt::string, to_string, dt::boolean>), \
  (wrap1_noheap<dt::string, to_string, dt::ipaddress>), \
\
  (wrap1_noheap<dt::istring, to_istring, dt::string>), \
  (wrap1_noheap<dt::istring, to_istring, dt::istring>), \
  (wrap1_heap<dt::istring, to_istring, dt::integer>), \
  (wrap1_heap<dt::istring, to_istring, dt::uinteger>), \
  (wrap1_heap<dt::istring, to_istring, dt::fdouble>), \
  (wrap1_noheap<dt::istring, to_istring, dt::boolean>), \
  (wrap1_noheap<dt::istring, to_istring, dt::ipaddress>), \
\
  (wrap1_heap<dt::string, str_to_upper_case, dt::string>), \
  (wrap1_heap<dt::istring, str_to_upper_case, dt::istring>), \
  (wrap1_heap<dt::string, str_to_lower_case, dt::string>), \
  (wrap1_heap<dt::istring, str_to_lower_case, dt::istring>), \
\
  NP1_REL_RLANG_FN_SUPPORTED_STRING_MATCH_FNS(str_regex_match), \
  (wrap3_heap<dt::string, str_regex_replace, dt::string, dt::string, dt::string>), \
  (wrap3_heap<dt::string, str_regex_replace, dt::string, dt::string, dt::istring>), \
  (wrap3_heap<dt::istring, str_regex_replace, dt::string, dt::istring, dt::string>), \
  (wrap3_heap<dt::istring, str_regex_replace, dt::string, dt::istring, dt::istring>), \
  (wrap3_heap<dt::string, str_regex_replace_empty_on_no_match, dt::string, dt::string, dt::string>), \
  (wrap3_heap<dt::string, str_regex_replace_empty_on_no_match, dt::string, dt::string, dt::istring>), \
  (wrap3_heap<dt::istring, str_regex_replace_empty_on_no_match, dt::string, dt::istring, dt::string>), \
  (wrap3_heap<dt::istring, str_regex_replace_empty_on_no_match, dt::string, dt::istring, dt::istring>), \
  (wrap3_heap<dt::string, str_regex_replace_all, dt::string, dt::string, dt::string>), \
  (wrap3_heap<dt::string, str_regex_replace_all, dt::string, dt::string, dt::istring>), \
  (wrap3_heap<dt::istring, str_regex_replace_all, dt::string, dt::istring, dt::string>), \
  (wrap3_heap<dt::istring, str_regex_replace_all, dt::string, dt::istring, dt::istring>), \
\
  NP1_REL_RLANG_FN_SUPPORTED_STRING_MATCH_FNS(str_starts_with), \
  NP1_REL_RLANG_FN_SUPPORTED_STRING_MATCH_FNS(str_ends_with), \
  NP1_REL_RLANG_FN_SUPPORTED_STRING_MATCH_FNS(str_contains), \
\
  (wrap0_heap<dt::string, str_uuidgen>), \
\
  (wrap1_heap<dt::string, str_sha256, dt::string>), \
  (wrap1_heap<dt::string, str_sha256, dt::istring>), \
  (wrap1_heap<dt::string, str_sha256, dt::integer>), \
  (wrap1_heap<dt::string, str_sha256, dt::uinteger>), \
  (wrap1_heap<dt::string, str_sha256, dt::boolean>), \
  (wrap1_heap<dt::string, str_sha256, dt::ipaddress>), \
\
  (wrap0_noheap<dt::uinteger, math_rand64>), \
\
  (wrap0_noheap<dt::uinteger, time_now_epoch_usec>), \
  (wrap1_noheap<dt::uinteger, time_usec_to_msec, dt::uinteger>), \
  (wrap1_noheap<dt::integer, time_usec_to_msec, dt::integer>), \
  (wrap1_noheap<dt::uinteger, time_msec_to_usec, dt::uinteger>), \
  (wrap1_noheap<dt::integer, time_msec_to_usec, dt::integer>), \
  (wrap1_noheap<dt::uinteger, time_usec_to_sec, dt::uinteger>), \
  (wrap1_noheap<dt::integer, time_usec_to_sec, dt::integer>), \
  (wrap1_noheap<dt::uinteger, time_sec_to_usec, dt::uinteger>), \
  (wrap1_noheap<dt::integer, time_sec_to_usec, dt::integer>), \
  (wrap1_noheap<dt::uinteger, time_extract_year, dt::uinteger>), \
  (wrap1_noheap<dt::integer, time_extract_year, dt::integer>), \
  (wrap1_noheap<dt::uinteger, time_extract_month, dt::uinteger>), \
  (wrap1_noheap<dt::integer, time_extract_month, dt::integer>), \
  (wrap1_noheap<dt::uinteger, time_extract_day, dt::uinteger>), \
  (wrap1_noheap<dt::integer, time_extract_day, dt::integer>), \
  (wrap1_noheap<dt::uinteger, time_extract_hour, dt::uinteger>), \
  (wrap1_noheap<dt::integer, time_extract_hour, dt::integer>), \
  (wrap1_noheap<dt::uinteger, time_extract_minute, dt::uinteger>), \
  (wrap1_noheap<dt::integer, time_extract_minute, dt::integer>), \
  (wrap1_noheap<dt::uinteger, time_extract_second, dt::uinteger>), \
  (wrap1_noheap<dt::integer, time_extract_second, dt::integer>), \
  (wrap2_noheap<dt::uinteger, time_parse, dt::string, dt::string>), \
  (wrap2_noheap<dt::uinteger, time_parse, dt::string, dt::istring>), \
  (wrap2_noheap<dt::uinteger, time_parse, dt::istring, dt::string>), \
  (wrap2_noheap<dt::uinteger, time_parse, dt::istring, dt::istring>), \
  (wrap2_heap<dt::string, time_format, dt::uinteger, dt::string>), \
  (wrap2_heap<dt::string, time_format, dt::uinteger, dt::istring>), \
\
  (wrap1_heap<dt::string, io_net_url_get, dt::string>), \
  (wrap1_heap<dt::string, io_file_read, dt::string>), \
  (wrap1_noheap<dt::boolean, io_file_erase, dt::string>), \
  (wrap1_heap<dt::string, meta_shell, dt::string>)




// The number of supported functions.
#define NP1_REL_RLANG_FN_NUMBER_SUPPORTED_FUNCTIONS NP1_PREPROC_ARG_COUNT(NP1_REL_RLANG_FN_TABLE)



// Macros to help get function info.
#define NP1_REL_RLANG_FN_TABLE_GET_FUNCTION_INFO_CASE(n__, wrap__, unused__) \
case n__: return fn_info::create<NP1_PREPROC_REMOVE_PAREN wrap__ >(); 


#define NP1_REL_RLANG_FN_TABLE_GET_FUNCTION_INFO_HELPER(function_offset_variable_name__) \
switch (function_offset_variable_name__) { \
NP1_PREPROC_FOR_EACH(NP1_REL_RLANG_FN_TABLE_GET_FUNCTION_INFO_CASE, unused__, \
                      NP1_REL_RLANG_FN_TABLE) \
}



/// Helper for figuring out if two types are the same.
template <typename T1, typename T2>
struct is_same_type {
  static const bool value = false;
};

template <typename T>
struct is_same_type<T, T> {
  static const bool value = true;
};



/****************** start UNUSED ******************************/
// This switch-based dispatch code is not currently used because it is marginally
// slower than the computed goto dispatch code below.  It's left here in case
// the g++ optimizer improves so that switch-based dispatch becomes faster.

#if 0
#define NP1_REL_RLANG_FN_TABLE_DYNAMIC_DISPATCH_CASE_CALL(n__, wrap__, call__) \
case n__: used_in_macro_FCALL_P__ += NP1_PREPROC_REMOVE_PAREN wrap__::call__; break;

/// The code to call a function is a macro to force inlining- it's the only path
/// here which absolutely must be fast.  NOTE that this macro uses an
/// externally-defined variable called used_in_macro_FCALL_P__
#define NP1_REL_RLANG_FN_TABLE_CALL(function_id__, stk__, heap__, lit__, this_r__, other_r__, data__) \
{ \
  switch (function_id__) { \
  NP1_PREPROC_FOR_EACH(NP1_REL_RLANG_FN_TABLE_DYNAMIC_DISPATCH_CASE_CALL, \
                        call(stk__, heap__, lit__, this_r__, other_r__, data__), \
                        NP1_REL_RLANG_FN_TABLE) \
  case NP1_REL_RLANG_FN_NUMBER_SUPPORTED_FUNCTIONS: \
    NP1_ASSERT(!m_stack.empty(), "Unexpected final stack pointer"); \
    return m_stack; \
  } \
}

#define NP1_REL_RLANG_FN_TABLE_CALL_ALL(function_calls__, stk__, heap__, lit__, this_r__, other_r__) \
{ \
  const vm_function_call *used_in_macro_FCALL_P__ = function_calls__.begin(); \
  const vm_function_call *fcall_end__ = function_calls__.end(); \
  while (used_in_macro_FCALL_P__ < fcall_end__) { \
    NP1_REL_RLANG_FN_TABLE_CALL(used_in_macro_FCALL_P__->id(), stk__, heap__, lit__, this_r__, other_r__, used_in_macro_FCALL_P__->data()) \
  } \
  NP1_ASSERT(false, "Unreachable!"); \
  return m_stack; \
}
#endif
/****************** end UNUSED ******************************/


/// Computed-goto-based dispatch code.  This is a threaded "interpreter" in
/// the FORTH sense.
#define NP1_REL_RLANG_MAKE_LABEL(n__) label__##n__##__

#define NP1_REL_RLANG_MAKE_ADDRESSOF_LABEL_AND_COMMA(n__, unused__) \
  &&NP1_REL_RLANG_MAKE_LABEL(n__),


#if (NP1_TRACE_ON == 1)
#define NP1_REL_RLANG_FN_TABLE_CALL_LABEL_BODY_TRACE(n__, wrap__) \
fprintf(stderr, "called %d (%s), going to offset=%zu, id=%zu\n", n__, wrap__::target_type::name(), fcall_p__ - fcall_start__, fcall_p__->id());
#else
#define NP1_REL_RLANG_FN_TABLE_CALL_LABEL_BODY_TRACE(n__, wrap__) 
#endif

// Note that after calling the function, this just jumps to the next function...
// just like a real FORTH interpreter :D.
#define NP1_REL_RLANG_FN_TABLE_CALL_LABEL_BODY(n__, wrap__, call__) \
NP1_REL_RLANG_MAKE_LABEL(n__): \
fcall_p__ += NP1_PREPROC_REMOVE_PAREN wrap__::call__; \
NP1_REL_RLANG_FN_TABLE_CALL_LABEL_BODY_TRACE(n__, NP1_PREPROC_REMOVE_PAREN wrap__) \
goto *labels__[fcall_p__->id()];


/// This code calls all the functions in the function list. It's a macro to force inlining.
/// The last function in the list MUST be a reference to label_end__ so that the VM
/// exits correctly.
#define NP1_REL_RLANG_FN_TABLE_CALL_ALL(function_calls__, stk__, heap__, lit__, this_r__, other_r__) \
{ \
  using namespace ::np1::rel::rlang::fn; \
  static const void *labels__[] = { \
    NP1_PREPROC_REPEAT_N(NP1_REL_RLANG_MAKE_ADDRESSOF_LABEL_AND_COMMA, \
                          NP1_REL_RLANG_FN_NUMBER_SUPPORTED_FUNCTIONS, \
                          unused__) \
    &&label_end__ \
  }; \
  const vm_function_call *fcall_start__ = function_calls__.begin(); \
  const vm_function_call *fcall_p__ = fcall_start__; \
  goto *labels__[fcall_p__->id()]; \
  NP1_PREPROC_FOR_EACH(NP1_REL_RLANG_FN_TABLE_CALL_LABEL_BODY, \
                        call(stk__, heap__, lit__, this_r__, other_r__, fcall_p__->data()), \
                        NP1_REL_RLANG_FN_TABLE) \
  label_end__: \
    NP1_ASSERT(!m_stack.empty(), "Unexpected final stack pointer"); \
    return m_stack; \
}
  




/// Operations on the one true list of supported functions.
struct fn_table {
  class fn_info {
  public:
    fn_info()
      : m_name(0), m_synonym(0), m_description(0), m_precedence(0),
        m_is_left_assoc(false), m_is_push_this(false), m_is_push_other(false), m_is_operator(false),
        m_number_arguments(0), m_return_type(dt::TYPE_STRING) {}


    template <typename Wrap>
    static fn_info create() {
      fn_info info;
      info.m_name = Wrap::target_type::name();
      info.m_synonym = Wrap::target_type::synonym();
      info.m_description = Wrap::target_type::description();
      info.m_precedence = Wrap::target_type::precedence;
      info.m_is_left_assoc = Wrap::target_type::is_left_assoc;
      info.m_is_push_this = Wrap::is_push_this;
      info.m_is_push_other = Wrap::is_push_this;
      info.m_is_operator = Wrap::target_type::is_operator;
      info.m_number_arguments = Wrap::number_arguments;
      info.m_return_type = Wrap::return_data_type_enum;
      return info;
    }

    const char *name() const { return m_name; }
    const char *synonym() const { return m_synonym; }
    const char *description() const { return m_description; }
    size_t precedence() const { return m_precedence; }
    bool is_left_assoc() const { return m_is_left_assoc; }
    bool is_push_this() const { return m_is_push_this; }
    bool is_push_other() const { return m_is_push_other; }
    bool is_operator() const { return m_is_operator; }
    size_t number_arguments() const { return m_number_arguments; }
    dt::data_type return_type() const { return m_return_type; }


  private:
    const char *m_name;
    const char *m_synonym;
    const char *m_description;
    size_t m_precedence;
    bool m_is_left_assoc;
    bool m_is_push_this;
    bool m_is_push_other;
    bool m_is_operator;
    size_t m_number_arguments;
    dt::data_type m_return_type;
  };

  /// Get information about a function.
  static fn_info get_info(size_t function_id) {
    NP1_REL_RLANG_FN_TABLE_GET_FUNCTION_INFO_HELPER(function_id)  
  
    NP1_ASSERT(false, "Unreachable");
    return fn_info();
  }
  
  /// Find a function, returns (size_t)-1 if not found.
  static size_t find(const str::ref &name, const simulated_stack &available_args,
                     size_t called_arg_count) {
#define NP1_REL_RLANG_FN_TABLE_FIND_EXACT_ITER(n__, wrap__, unused__) \
if (NP1_PREPROC_REMOVE_PAREN wrap__ ::is_match(name, available_args, called_arg_count)) { return n__; }

    NP1_PREPROC_FOR_EACH(NP1_REL_RLANG_FN_TABLE_FIND_EXACT_ITER, unused__, NP1_REL_RLANG_FN_TABLE);
    return -1;
  }
  
  /// Find the first function with the supplied name.
  static size_t find_first(const str::ref &name) {
#define NP1_REL_RLANG_FN_TABLE_FIND_FIRST_ITER(n__, wrap__, unused__) \
if (NP1_PREPROC_REMOVE_PAREN wrap__ ::is_name_match(name)) { return n__; }

    NP1_PREPROC_FOR_EACH(NP1_REL_RLANG_FN_TABLE_FIND_FIRST_ITER, unused__, NP1_REL_RLANG_FN_TABLE);
    return -1;
  }
  
  /// Find the first function whose name starts with the supplied characters,
  /// returns (size_t)-1 if not found.
  static size_t find_first_starts_with(const str::ref &partial_name) {
#define NP1_REL_RLANG_FN_TABLE_FIND_FIRST_STARTS_WTIH_ITER(n__, wrap__, unused__) \
if (NP1_PREPROC_REMOVE_PAREN wrap__ ::name_starts_with(partial_name)) { return n__; }

    NP1_PREPROC_FOR_EACH(NP1_REL_RLANG_FN_TABLE_FIND_FIRST_STARTS_WTIH_ITER, unused__, NP1_REL_RLANG_FN_TABLE);
    return -1;
  }
  
  static bool is_valid_partial_match(const str::ref &partial_name) {
    return find_first_starts_with(partial_name) != (size_t)-1;
  }

  /// Iterate over the wrappers.
  template <typename Receiver>
  static void for_each(Receiver &r) {
#define NP1_REL_RLANG_FN_TABLE_FOR_EACH_ITER(n__, wrap__, unused__) \
NP1_PREPROC_REMOVE_PAREN wrap__ ::get_info(r);

    NP1_PREPROC_FOR_EACH(NP1_REL_RLANG_FN_TABLE_FOR_EACH_ITER, unused__, NP1_REL_RLANG_FN_TABLE);    
  }


  /// Find the function that matches with the inner function.
  template <typename Inner>
  static size_t mandatory_find_by_inner() {
#define NP1_REL_RLANG_FN_TABLE_FIND_BY_INNER_ITER(n__, wrap__, unused__) \
if (is_same_type<Inner, NP1_PREPROC_REMOVE_PAREN wrap__ ::target_type>::value) { return n__; }

    NP1_PREPROC_FOR_EACH(NP1_REL_RLANG_FN_TABLE_FIND_BY_INNER_ITER, unused__, NP1_REL_RLANG_FN_TABLE);
    NP1_ASSERT(false, "mandatory_find_by_inner failed!");
    return -1;
  }


  /// Unary minus- a special and annoying case.
  static size_t find_unary_minus() {
    return mandatory_find_by_inner<op::unary_minus>();
  }

  /// Find branch functions.
  static size_t find_if() {
    return mandatory_find_by_inner<internal_if>();
  }

  static size_t find_goto() {
    return mandatory_find_by_inner<internal_goto>();
  }
  

  /// Find literal-pushing functions.
  static size_t find_push_literal_string() {
    return mandatory_find_by_inner<internal_push_literal_string>();
  }

  static size_t find_push_literal_integer() {
    return mandatory_find_by_inner<internal_push_literal_integer>();
  }

  static size_t find_push_literal_uinteger() {
    return mandatory_find_by_inner<internal_push_literal_uinteger>();
  }

  static size_t find_push_literal_double() {
    return mandatory_find_by_inner<internal_push_literal_double>();
  }

  static size_t find_push_literal_boolean_true() {
    return mandatory_find_by_inner<internal_push_literal_boolean_true>();
  }

  static size_t find_push_literal_boolean_false() {
    return mandatory_find_by_inner<internal_push_literal_boolean_false>();
  }


  /// Find the function that matches with the wrapper.
  template <typename Wrap>
  static size_t mandatory_find_by_wrap() {
#define NP1_REL_RLANG_FN_TABLE_FIND_BY_WRAP_ITER(n__, wrap__, unused__) \
if (is_same_type<Wrap, NP1_PREPROC_REMOVE_PAREN wrap__ >::value) { return n__; }

    NP1_PREPROC_FOR_EACH(NP1_REL_RLANG_FN_TABLE_FIND_BY_WRAP_ITER, unused__, NP1_REL_RLANG_FN_TABLE);
    NP1_ASSERT(false, "mandatory_find_by_wrap failed!");
    return -1;
  }
  

  // Find field-pushing functions.
  static size_t find_push_this_field_string() {
    return mandatory_find_by_wrap<wrap_push_this<dt::string, internal_push_field_string> >();
  }

  static size_t find_push_this_field_istring() {
    return mandatory_find_by_wrap<wrap_push_this<dt::istring, internal_push_field_istring> >();
  }

  static size_t find_push_this_field_integer() {
    return mandatory_find_by_wrap<wrap_push_this<dt::integer, internal_push_field_integer> >();
  }

  static size_t find_push_this_field_uinteger() {
    return mandatory_find_by_wrap<wrap_push_this<dt::uinteger, internal_push_field_uinteger> >();
  }

  static size_t find_push_this_field_double() {
    return mandatory_find_by_wrap<wrap_push_this<dt::fdouble, internal_push_field_double> >();
  }

  static size_t find_push_this_field_boolean() {
    return mandatory_find_by_wrap<wrap_push_this<dt::boolean, internal_push_field_boolean> >();
  }

  static size_t find_push_this_field_ipaddress() {
    return mandatory_find_by_wrap<wrap_push_this<dt::ipaddress, internal_push_field_ipaddress> >();
  }

  static size_t find_push_this_rownum() {
    return mandatory_find_by_wrap<wrap_push_this<dt::uinteger, internal_push_rownum> >();
  }


  static size_t find_push_other_field_string() {
    return mandatory_find_by_wrap<wrap_push_other<dt::string, internal_push_field_string> >();
  }

  static size_t find_push_other_field_istring() {
    return mandatory_find_by_wrap<wrap_push_other<dt::istring, internal_push_field_istring> >();
  }

  static size_t find_push_other_field_integer() {
    return mandatory_find_by_wrap<wrap_push_other<dt::integer, internal_push_field_integer> >();
  }

  static size_t find_push_other_field_uinteger() {
    return mandatory_find_by_wrap<wrap_push_other<dt::uinteger, internal_push_field_uinteger> >();
  }

  static size_t find_push_other_field_double() {
    return mandatory_find_by_wrap<wrap_push_other<dt::fdouble, internal_push_field_double> >();
  }

  static size_t find_push_other_field_boolean() {
    return mandatory_find_by_wrap<wrap_push_other<dt::boolean, internal_push_field_boolean> >();
  }

  static size_t find_push_other_field_ipaddress() {
    return mandatory_find_by_wrap<wrap_push_other<dt::ipaddress, internal_push_field_ipaddress> >();
  }

  static size_t find_push_other_rownum() {
    return mandatory_find_by_wrap<wrap_push_other<dt::uinteger, internal_push_rownum> >();
  }

};

} // namespaces
}
}
}


#endif
