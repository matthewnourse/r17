// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_TEST_UNIT_NP1_TEST_CONSISTENT_HASH_TABLE_HPP
#define NP1_TEST_UNIT_NP1_TEST_CONSISTENT_HASH_TABLE_HPP

#include "np1/consistent_hash_table.hpp"

namespace test {
namespace unit {
namespace np1 {

struct hash_function {
  uint64_t operator()(int i, uint64_t consistent_hash_internal) {
    uint64_t hval = ::np1::hash::fnv1a64::init();
    hval = ::np1::hash::fnv1a64::add(&i, sizeof(i), hval);
    hval = ::np1::hash::fnv1a64::add(&consistent_hash_internal, sizeof(consistent_hash_internal), hval);
    return hval;
  }
};

typedef ::np1::consistent_hash_table<int, hash_function> consistent_hash_table_type;




static void check_number_duplicates(int number_insertions, int number_duplicates,
                                    consistent_hash_table_type::const_iterator iter,
                                    consistent_hash_table_type::const_iterator end) {
  int duplicates[number_insertions];
  memset(duplicates, 0, sizeof(duplicates));

  for (; iter != end; ++iter) {
    NP1_TEST_ASSERT((*iter >= 0) && (*iter < number_insertions));
    duplicates[*iter] = duplicates[*iter] + 1;
  }

  int i;
  for (i = 0; i < number_insertions; ++i) {
    NP1_TEST_ASSERT(number_duplicates == duplicates[i]);
  }
}




void test_consistent_hash_table_insert() {
  static const int NUMBER_INSERTIONS = 1000;
  static const int NUMBER_DUPLICATES = 16;
  consistent_hash_table_type table(16);  

  int i;
  for (i = NUMBER_INSERTIONS - 1; i >= 0; --i) {
    NP1_TEST_ASSERT(table.insert(i));
  }

  NP1_TEST_ASSERT(table.size() == (size_t)NUMBER_INSERTIONS);

  check_number_duplicates(NUMBER_INSERTIONS, NUMBER_DUPLICATES, table.begin(), table.end());
}



void test_consistent_hash_table_insert_random() {
  static const int NUMBER_DUPLICATES = 16;
  consistent_hash_table_type table(NUMBER_DUPLICATES);  

  NP1_TEST_ASSERT(table.insert(9));
  NP1_TEST_ASSERT(table.insert(2));
  NP1_TEST_ASSERT(table.insert(3));
  NP1_TEST_ASSERT(table.insert(7));
  NP1_TEST_ASSERT(table.insert(1));
  NP1_TEST_ASSERT(table.insert(4));
  NP1_TEST_ASSERT(table.insert(6));
  NP1_TEST_ASSERT(table.insert(5));
  NP1_TEST_ASSERT(table.insert(0));
  NP1_TEST_ASSERT(table.insert(8));

  NP1_TEST_ASSERT(table.size() == 10);

  
  int duplicates[10];
  memset(duplicates, 0, sizeof(duplicates));

  check_number_duplicates(10, NUMBER_DUPLICATES, table.begin(), table.end());
}








void test_consistent_hash_table_lower_bound() {
  static const int NUMBER_DUPLICATES = 16;
  static const int NUMBER_INSERTIONS = 100;
  consistent_hash_table_type table(NUMBER_DUPLICATES);  

  int i;
  for (i = 0; i < NUMBER_INSERTIONS; ++i) {
    NP1_TEST_ASSERT(table.insert(i));
  }

  NP1_TEST_ASSERT(table.size() == (size_t)NUMBER_INSERTIONS);


  for (i = 0; i < NUMBER_INSERTIONS * 10; ++i) {
    // Test that we can scan right around the "circle" from a variety of keys.
    //TODO: ensure that we are testing all possible positions on the hash table.
    consistent_hash_table_type::const_iterator iter = table.lower_bound(i);
    int counter = 0;
    for (; iter != table.end(); ++iter, ++counter) {
    }

    NP1_TEST_ASSERT(counter == NUMBER_INSERTIONS * NUMBER_DUPLICATES);
  }
}




void test_consistent_hash_table() {
  NP1_TEST_RUN_TEST(test_consistent_hash_table_insert);
  NP1_TEST_RUN_TEST(test_consistent_hash_table_insert_random);
  NP1_TEST_RUN_TEST(test_consistent_hash_table_lower_bound);
}

} // namespaces
}
}

#endif
