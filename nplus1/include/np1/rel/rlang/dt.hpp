// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_REL_RLANG_DT_HPP
#define NP1_REL_RLANG_DT_HPP



namespace np1 {
namespace rel {
namespace rlang {
namespace dt {

/// Data type ids.

typedef enum {
  TYPE_STRING,
  TYPE_ISTRING,
  TYPE_INT,
  TYPE_UINT,
  TYPE_DOUBLE,
  TYPE_BOOL,
  TYPE_IPADDRESS
} data_type;


/// The C++ implementations of those data types.
struct string : public str::ref {
  string() {}
  explicit string(const char *p) : str::ref(p) {}
  explicit string(const std::string &s) : str::ref(s) {}
  string(const char *p, size_t n) : str::ref(p, n) {}
  string(const str::ref &s) : str::ref(s) {}
};

struct istring : public str::ref {
  istring() {}
  explicit istring(const char *p) : str::ref(p) {}
  explicit istring(const std::string &s) : str::ref(s) {}
  istring(const char *p, size_t n) : str::ref(p, n) {}
  istring(const str::ref &s) : str::ref(s) {}
};

typedef int64_t integer;
typedef uint64_t uinteger;
typedef bool boolean;
typedef double fdouble;
struct ipaddress : public str::ref {
  ipaddress() {}
  ipaddress(const str::ref &s) : str::ref(s) {}
};


/// Functions to translate between C++ data types and their ids.

template <typename Data_Type> struct to_data_type_enum;

template <> struct to_data_type_enum<string> {
  static const data_type value = TYPE_STRING; 
};

template <> struct to_data_type_enum<istring>  {
  static const data_type value = TYPE_ISTRING; 
};

template <> struct to_data_type_enum<integer> {
  static const data_type value = TYPE_INT; 
};

template <> struct to_data_type_enum<uinteger> {
  static const data_type value = TYPE_UINT;
};

template <> struct to_data_type_enum<fdouble> {
  static const data_type value = TYPE_DOUBLE;
};

template <> struct to_data_type_enum<boolean> {
  static const data_type value = TYPE_BOOL; 
};

template <> struct to_data_type_enum<ipaddress> {
  static const data_type value = TYPE_IPADDRESS; 
};




template <typename Data_Type>
struct is_cpp_implementation_of {
  static bool f(data_type dt) { return to_data_type_enum<Data_Type>::value == dt; }
};




static const char *to_string(data_type type) {
  switch (type) {
  case TYPE_STRING: return "string";
  case TYPE_ISTRING: return "istring";
  case TYPE_INT: return "int";
  case TYPE_UINT: return "uint";
  case TYPE_DOUBLE: return "double";
  case TYPE_BOOL: return "bool";
  case TYPE_IPADDRESS: return "ipaddress";    
  }

  NP1_ASSERT(false, "Unreachable");
  return 0;
}

static bool is_valid_tag(const str::ref &type_tag) {
  return ((str::cmp(type_tag, "string") == 0)
          || (str::cmp(type_tag, "istring") == 0)
          || (str::cmp(type_tag, "int") == 0) 
          || (str::cmp(type_tag, "uint") == 0)
          || (str::cmp(type_tag, "double") == 0) 
          || (str::cmp(type_tag, "bool") == 0) 
          || (str::cmp(type_tag, "ipaddress") == 0));
}


static data_type mandatory_from_string(const str::ref &type_tag) {
  if (str::cmp(type_tag, "string") == 0) {
    return TYPE_STRING;
  } else if (str::cmp(type_tag, "istring") == 0) {
    return TYPE_ISTRING;
  } else if (str::cmp(type_tag, "int") == 0) {
    return TYPE_INT;
  } else if (str::cmp(type_tag, "uint") == 0) {
    return TYPE_UINT;
  } else if (str::cmp(type_tag, "double") == 0) {
    return TYPE_DOUBLE;
  } else if (str::cmp(type_tag, "bool") == 0) {
    return TYPE_BOOL;
  } else if (str::cmp(type_tag, "ipaddress") == 0) {
    return TYPE_IPADDRESS;
  } 

  NP1_ASSERT(false, "Unknown type: " + type_tag.to_string());
  return TYPE_STRING;
}



template <typename Iterator>
static void for_each(Iterator iter) {
  iter(TYPE_STRING, to_string(TYPE_STRING), "Case-sensitive string.");
  iter(TYPE_ISTRING, to_string(TYPE_ISTRING), "Case-insensitive string.");
  iter(TYPE_INT, to_string(TYPE_INT), "64-bit signed integer.");
  iter(TYPE_UINT, to_string(TYPE_UINT), "64-bit unsigned integer.");
  iter(TYPE_DOUBLE, to_string(TYPE_DOUBLE), "Double-precision floating point number.");
  iter(TYPE_BOOL, to_string(TYPE_BOOL), "Boolean.");
  iter(TYPE_IPADDRESS, to_string(TYPE_IPADDRESS), "IPv4 IP address.");
}

// Return a NULL-like value.
//TODO: do we need a "NULL" value to do this?
static str::ref empty_value(data_type type) {
  switch (type) {
  case rlang::dt::TYPE_STRING:
  case rlang::dt::TYPE_ISTRING:
    return str::ref("");

  case rlang::dt::TYPE_INT: 
  case rlang::dt::TYPE_UINT: 
    return str::ref("0");

  case rlang::dt::TYPE_DOUBLE: 
    return str::ref("0.0");

  case rlang::dt::TYPE_BOOL:
    return str::ref("false");

  case rlang::dt::TYPE_IPADDRESS:
    return str::ref("0.0.0.0");
  }

  NP1_ASSERT(false, "Unreachable");
  return str::ref("");
}


}
}
}
}

#endif

