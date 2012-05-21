// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_REL_RLANG_FN_OP_HPP
#define NP1_REL_RLANG_FN_OP_HPP


#include "np1/rel/rlang/fn/fn.hpp"

namespace np1 {
namespace rel {
namespace rlang {
namespace fn {
namespace op {


// Operators can be overloaded with similar rules to C++.  That is, all operator
// overloads must have the same precedence, arity and associativity.



// Precedence order & left/right associativity is taken from C's precedence
// according to
// http://en.wikipedia.org/wiki/Operators_in_C_and_C%2B%2B
// We follow the precedence numbering in that link even though there's gaps for
// operators we don't support.


/// The base for all operators.
struct base {
  static const char *since() { return "1.0"; }
  static const bool is_operator = true;
};



/// The base for all left-associative operators.
struct left_assoc : public base {
  static const bool is_left_assoc = true;
};

/// The base for all right-associative operators.
struct right_assoc : public base {
  static const bool is_left_assoc = false;
};



/// Comparison operators.  We don't allow comparisons between
/// different integer types because of the error-prone nature of such comparisons.
/// See
/// https://www.securecoding.cert.org/confluence/display/seccode/INT02-C.+Understand+integer+conversion+rules
/// for an example discussion about integer conversion rules in C.

/// We also don't allow equality comparisons between floating-point numbers because they make no sense.

#define NP1_REL_RLANG_FN_OP_EQUALITY_COMPARISON_METHODS(real_op__) \
inline static dt::boolean call(const dt::string &one, const dt::string &two) { return (str::cmp(one, two) real_op__ 0); } \
inline static dt::boolean call(const dt::string &one, const dt::istring &two) { return (str::icmp(one, two) real_op__ 0); } \
inline static dt::boolean call(const dt::istring &one, const dt::string &two) { return (str::icmp(one, two) real_op__ 0); } \
inline static dt::boolean call(const dt::istring &one, const dt::istring &two) { return (str::icmp(one, two) real_op__ 0); } \
inline static dt::boolean call(dt::integer one, dt::integer two) { return one real_op__ two; } \
inline static dt::boolean call(dt::uinteger one,  dt::uinteger two) { return one real_op__ two; } \
inline static dt::boolean call(dt::boolean one, dt::boolean two) { return one real_op__ two; } \
inline static dt::boolean call(const dt::ipaddress &one, const dt::ipaddress &two) { return (np1::rel::detail::helper::ipaddress_or_ipnumber_compare(one, two) real_op__ 0); } \
inline static dt::boolean call(const dt::ipaddress &one, const dt::string &two) { return (np1::rel::detail::helper::ipaddress_or_ipnumber_compare(one, two) real_op__ 0); } \
inline static dt::boolean call(const dt::string &one, const dt::ipaddress &two) { return (np1::rel::detail::helper::ipaddress_or_ipnumber_compare(one, two) real_op__ 0); } \
inline static dt::boolean call(const dt::ipaddress &one, const dt::istring &two) { return (np1::rel::detail::helper::ipaddress_or_ipnumber_compare(one, two) real_op__ 0); } \
inline static dt::boolean call(const dt::istring &one, const dt::ipaddress &two) { return (np1::rel::detail::helper::ipaddress_or_ipnumber_compare(one, two) real_op__ 0); } 

#define NP1_REL_RLANG_FN_OP_NON_EQUALITY_COMPARISON_METHODS(real_op__) \
NP1_REL_RLANG_FN_OP_EQUALITY_COMPARISON_METHODS(real_op__) \
inline static dt::boolean call(dt::fdouble one,  dt::fdouble two) { return one real_op__ two; } \




/// The base for all equals operators.
struct eq : public left_assoc {
  static const char *name() { return "="; }
  static const char *synonym() { return 0; }
  static const char *description() { return "Equal to."; }  
  static const size_t precedence = 9;
  NP1_REL_RLANG_FN_OP_EQUALITY_COMPARISON_METHODS(==);
};

/// The bases for all other comparison operators.
struct ne : public left_assoc {
  static const char *name() { return "!="; }
  static const char *synonym() { return "<>"; }
  static const char *description() { return "Not equal to."; }  
  static const size_t precedence = 9;
  NP1_REL_RLANG_FN_OP_EQUALITY_COMPARISON_METHODS(!=);
};

struct lt : public left_assoc {
  static const char *name() { return "<"; }
  static const char *synonym() { return 0; }
  static const char *description() { return "Less than."; }  
  static const size_t precedence = 8;
  NP1_REL_RLANG_FN_OP_NON_EQUALITY_COMPARISON_METHODS(<);
};

struct gt : public left_assoc {
  static const char *name() { return ">"; }
  static const char *synonym() { return 0; }
  static const char *description() { return "Greater than."; }  
  static const size_t precedence = 8;
  NP1_REL_RLANG_FN_OP_NON_EQUALITY_COMPARISON_METHODS(>);
};

struct le : public left_assoc {
  static const char *name() { return "<="; }
  static const char *synonym() { return 0; }
  static const char *description() { return "Less than or equal to."; }  
  static const size_t precedence = 8;
  NP1_REL_RLANG_FN_OP_NON_EQUALITY_COMPARISON_METHODS(<=);
};

struct ge : public left_assoc {
  static const char *name() { return ">="; }
  static const char *synonym() { return 0; }
  static const char *description() { return "Greater than or equal to."; }  
  static const size_t precedence = 8;
  NP1_REL_RLANG_FN_OP_NON_EQUALITY_COMPARISON_METHODS(>=);
};


/// String concatenation.
struct string_plus : public left_assoc {
  static const char *name() { return "+"; }
  static const char *synonym() { return 0; }
  static const char *description() { return "String concatenation or addition."; }  
  static const size_t precedence = 6;

  inline static dt::string call(vm_heap &h, const dt::string &one, const dt::string &two) { 
    return np1::rel::detail::helper::string_concat(h, one, two);  
  }

  inline static dt::istring call(vm_heap &h, const dt::istring &one, const dt::string &two) { 
    return np1::rel::detail::helper::string_concat(h, one, two);  
  }

  inline static dt::istring call(vm_heap &h, const dt::string &one, const dt::istring &two) { 
    return np1::rel::detail::helper::string_concat(h, one, two);  
  }

  inline static dt::istring call(vm_heap &h, const dt::istring &one, const dt::istring &two) { 
    return np1::rel::detail::helper::string_concat(h, one, two);  
  }
};


/// Signatures for functions/operators that take 2 integer arguments and return
/// another.  We don't allow operations between different integer types because
/// of the error-prone nature of such operations.
/// See
/// https://www.securecoding.cert.org/confluence/display/seccode/INT02-C.+Understand+integer+conversion+rules
/// for an example discussion about integer conversion rules in C.



/// Helper macros for binary arithmetic operators.
#define NP1_REL_RLANG_FN_OP_BINARY_ARITHMETIC_METHODS(real_op__) \
inline static dt::integer call(dt::integer one, dt::integer two) { return one real_op__ two; } \
inline static  dt::uinteger call(dt::uinteger one, dt::uinteger two) { return one real_op__ two; } 



/// Addition.
struct plus : public left_assoc {
  static const char *name() { return "+"; }
  static const char *synonym() { return 0; }
  static const char *description() { return "Addition or string concatenation."; }  
  static const size_t precedence = 6;
  NP1_REL_RLANG_FN_OP_BINARY_ARITHMETIC_METHODS(+)
  inline static  dt::fdouble call(dt::fdouble one, dt::fdouble two) { return one + two; } 
};




/// Subtraction.
struct minus : public left_assoc {
  static const char *name() { return "-"; }
  static const char *synonym() { return 0; }
  static const char *description() { return "Subtraction."; }  
  static const size_t precedence = 6;
  NP1_REL_RLANG_FN_OP_BINARY_ARITHMETIC_METHODS(-)
  inline static  dt::fdouble call(dt::fdouble one, dt::fdouble two) { return one - two; } 
};



/// Unary minus.  TODO: allow for unsigned?
struct unary_minus : public right_assoc {
  static const char *name() { return "-"; }
  static const char *synonym() { return 0; }
  static const char *description() { return "Unary minus."; }  
  static const size_t precedence = 3;
  inline static dt::integer call(dt::integer one) { return -one; }
  inline static dt::fdouble call(dt::fdouble one) { return -one; }
};

/// Multiplication.
struct multiply : public left_assoc {
  static const char *name() { return "*"; }
  static const char *synonym() { return 0; }
  static const char *description() { return "Multiplication."; }  
  static const size_t precedence = 5;
  NP1_REL_RLANG_FN_OP_BINARY_ARITHMETIC_METHODS(*)
  inline static  dt::fdouble call(dt::fdouble one, dt::fdouble two) { return one * two; } 
};


/// Division.
struct divide : public left_assoc {
  static const char *name() { return "/"; }
  static const char *synonym() { return 0; }
  static const char *description() { return "Division."; }  
  static const size_t precedence = 5;
  NP1_REL_RLANG_FN_OP_BINARY_ARITHMETIC_METHODS(/)
  inline static  dt::fdouble call(dt::fdouble one, dt::fdouble two) { return one / two; } 
};


/// Modulus.
struct modulus : public left_assoc {
  static const char *name() { return "%"; }
  static const char *synonym() { return "mod"; }
  static const char *description() { return "Modulus."; }  
  static const size_t precedence = 5;
  NP1_REL_RLANG_FN_OP_BINARY_ARITHMETIC_METHODS(%)
};


/// Bitwise AND.
struct bitwise_and : public left_assoc {
  static const char *name() { return "&"; }
  static const char *synonym() { return 0; }
  static const char *description() { return "Bitwise AND."; }  
  static const size_t precedence = 10;
  NP1_REL_RLANG_FN_OP_BINARY_ARITHMETIC_METHODS(&)
};


/// Bitwise OR.
struct bitwise_or : public left_assoc {
  static const char *name() { return "|"; }
  static const char *synonym() { return 0; }
  static const char *description() { return "Bitwise OR."; }  
  static const size_t precedence = 12;
  NP1_REL_RLANG_FN_OP_BINARY_ARITHMETIC_METHODS(|)
};


/// Bitwise NOT
struct bitwise_not : public right_assoc {
  static const char *name() { return "~"; }
  static const char *synonym() { return 0; }
  static const char *description() { return "Bitwise NOT."; }  
  static const size_t precedence = 3;
  inline static dt::integer call(dt::integer one) { return ~one; }
  inline static dt::uinteger call(dt::uinteger one) { return ~one; }
};



/// Logical AND.
struct logical_and : public left_assoc {
  static const char *name() { return "&&"; }
  static const char *synonym() { return "and"; }
  static const char *description() { return "Logical AND.  Note there is currently no \"shortcutting\"- both sides of the && will be executed even if the first expression returns false."; }  
  static const size_t precedence = 13;
  inline static dt::boolean call(dt::boolean one, dt::boolean two) { return one && two; }
};


/// Logical OR.
struct logical_or : public left_assoc {
  static const char *name() { return "||"; }
  static const char *synonym() { return "or"; }
  static const char *description() { return "Logical OR.  Note there is currently no \"shortcutting\"- both sides of the || will be executed even if the first expression returns true."; }  
  static const size_t precedence = 14;
  inline static dt::boolean call(dt::boolean one, dt::boolean two) { return one || two; }
};


/// Logical NOT
struct logical_not : public right_assoc {
  static const char *name() { return "!"; }
  static const char *synonym() { return "not"; }
  static const char *description() { return "Logical NOT."; }  
  static const size_t precedence = 3;
  inline static dt::boolean call(dt::boolean one) { return !one; }
};



} // namespaces.
}
}
}
}
#endif

