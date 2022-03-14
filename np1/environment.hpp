// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_ENVIRONMENT_HPP
#define NP1_ENVIRONMENT_HPP



#define NP1_ENVIRONMENT_MAX_RECORD_HASH_TABLE_SIZE "NP1_MAX_RECORD_HASH_TABLE_SIZE"
#define NP1_ENVIRONMENT_DEFAULT_MAX_RECORD_HASH_TABLE_SIZE "9223372036854775807"
#define NP1_ENVIRONMENT_SORT_CHUNK_SIZE_NAME "NP1_SORT_CHUNK_SIZE"
#define NP1_ENVIRONMENT_DEFAULT_SORT_CHUNK_SIZE "104857600"
#define NP1_ENVIRONMENT_SORT_INITIAL_NUMBER_THREADS "NP1_SORT_INITIAL_NUMBER_THREADS"
#define NP1_ENVIRONMENT_DEFAULT_SORT_INITIAL_NUMBER_THREADS "5"
#define NP1_ENVIRONMENT_R17_PATH "NP1_R17_PATH"

namespace np1 {

/// Environment variables.
class environment {
public:
  static uint64_t max_hash_table_size() {
    const char *value = getenv(NP1_ENVIRONMENT_MAX_RECORD_HASH_TABLE_SIZE);
    return str::dec_to_int64(value ? value : NP1_ENVIRONMENT_DEFAULT_MAX_RECORD_HASH_TABLE_SIZE);
  }

  static size_t sort_chunk_size() {
    const char *value = getenv(NP1_ENVIRONMENT_SORT_CHUNK_SIZE_NAME);
    return str::dec_to_int64(value ? value : NP1_ENVIRONMENT_DEFAULT_SORT_CHUNK_SIZE);    
  }
  
  static size_t sort_initial_number_threads() {
    const char *value = getenv(NP1_ENVIRONMENT_SORT_INITIAL_NUMBER_THREADS);
    return str::dec_to_int64(value ? value : NP1_ENVIRONMENT_DEFAULT_SORT_INITIAL_NUMBER_THREADS);    
  }
  
  static rstd::string r17_path() {
    const char *value = getenv(NP1_ENVIRONMENT_R17_PATH);
    return rstd::string(value);
  }
  
  static void r17_path(const rstd::string &new_path) {
    if (new_path.empty()) {
      NP1_ASSERT((unsetenv(NP1_ENVIRONMENT_R17_PATH) == 0),
                 "Unable to unset " NP1_ENVIRONMENT_R17_PATH " environment variable");
    } else {
      NP1_ASSERT((setenv(NP1_ENVIRONMENT_R17_PATH, new_path.c_str(), 1) == 0),
                 "Unable to set " NP1_ENVIRONMENT_R17_PATH " environment variable");
    }
  }

private:
  static const char *mandatory_get_env(const char *name) {
    const char *value = getenv(name);
    NP1_ASSERT(value, "Environment variable not defined: " + rstd::string(name));
    return value;
  }

};


} // namespace



#endif
