// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_TEST_UNIT_NP1_TEST_SKIP_LIST_HPP
#define NP1_TEST_UNIT_NP1_TEST_SKIP_LIST_HPP

#include "np1/skip_list.hpp"

namespace test {
namespace unit {
namespace np1 {

typedef ::np1::skip_list<int> skip_list_type;

void test_skip_list_insert() {
  static const int NUMBER_INSERTIONS = 1000000;
  skip_list_type lst;  

  int i;
  for (i = NUMBER_INSERTIONS - 1; i >= 0; --i) {
    NP1_TEST_ASSERT(lst.insert(i));
  }

  NP1_TEST_ASSERT(lst.size() == (size_t)NUMBER_INSERTIONS);

  NP1_TEST_ASSERT(!lst.insert(NUMBER_INSERTIONS/2));

  NP1_TEST_ASSERT(lst.size() == (size_t)NUMBER_INSERTIONS);

  skip_list_type::const_iterator iter = lst.begin();
  for (i = 0; i < NUMBER_INSERTIONS; ++i, ++iter) {
    NP1_TEST_ASSERT(i == *iter);
  }

  NP1_TEST_ASSERT(iter == lst.end());
}


void test_skip_list_insert_random() {
  skip_list_type lst;  

  int i;
  NP1_TEST_ASSERT(lst.insert(9));
  NP1_TEST_ASSERT(lst.insert(2));
  NP1_TEST_ASSERT(lst.insert(3));
  NP1_TEST_ASSERT(lst.insert(7));
  NP1_TEST_ASSERT(lst.insert(1));
  NP1_TEST_ASSERT(lst.insert(4));
  NP1_TEST_ASSERT(lst.insert(6));
  NP1_TEST_ASSERT(lst.insert(5));
  NP1_TEST_ASSERT(lst.insert(0));
  NP1_TEST_ASSERT(lst.insert(8));
  NP1_TEST_ASSERT(lst.size() == 10);


  NP1_TEST_ASSERT(!lst.insert(3));
  NP1_TEST_ASSERT(lst.size() == 10);

  skip_list_type::const_iterator iter = lst.begin();
  for (i = 0; i < 10; ++i, ++iter) {
    NP1_TEST_ASSERT(i == *iter);
  }

  NP1_TEST_ASSERT(iter == lst.end());
}





void test_skip_list_lower_bound() {
  static const int NUMBER_INSERTIONS = 1000000;
  skip_list_type lst;  

  int i;
  // -= 2 so we can leave holes.
  for (i = NUMBER_INSERTIONS - 1; i >= 0; i -= 2) {
    NP1_TEST_ASSERT(lst.insert(i));
  }

  NP1_TEST_ASSERT(lst.size() == NUMBER_INSERTIONS/2);

  skip_list_type::iterator iter = lst.lower_bound(10);
  NP1_TEST_ASSERT(iter != lst.end());
  NP1_TEST_ASSERT(11 == *iter);
  ++iter;
  NP1_TEST_ASSERT(iter != lst.end());
  NP1_TEST_ASSERT(13 == *iter);

  iter = lst.lower_bound(1021);
  NP1_TEST_ASSERT(iter != lst.end());
  NP1_TEST_ASSERT(1021 == *iter);
  ++iter;
  NP1_TEST_ASSERT(iter != lst.end());
  NP1_TEST_ASSERT(1023 == *iter);

  // 0 is not in the list because the list contains only odd numbers.
  iter = lst.lower_bound(0);
  NP1_TEST_ASSERT(iter != lst.end());
  NP1_TEST_ASSERT(1 == *iter);

  iter = lst.lower_bound(NUMBER_INSERTIONS);
  NP1_TEST_ASSERT(iter == lst.end());
}


void test_skip_list_find() {
  static const int NUMBER_INSERTIONS = 1000000;
  skip_list_type lst;  

  int i;
  // -= 2 so we can leave holes.
  for (i = NUMBER_INSERTIONS - 1; i >= 0; i -= 2) {
    NP1_TEST_ASSERT(lst.insert(i));
  }

  NP1_TEST_ASSERT(lst.size() == NUMBER_INSERTIONS/2);

  skip_list_type::iterator iter = lst.find(10);
  NP1_TEST_ASSERT(iter == lst.end());

  iter = lst.find(1021);
  NP1_TEST_ASSERT(iter != lst.end());
  NP1_TEST_ASSERT(1021 == *iter);
  ++iter;
  NP1_TEST_ASSERT(iter != lst.end());
  NP1_TEST_ASSERT(1023 == *iter);

  // 0 is not in the list because it contains only odd numbers.
  iter = lst.find(0);
  NP1_TEST_ASSERT(iter == lst.end());

  iter = lst.find(NUMBER_INSERTIONS);
  NP1_TEST_ASSERT(iter == lst.end());
}


void test_skip_list_erase() {
  static const int NUMBER_INSERTIONS = 1000000;
  skip_list_type lst;  

  int i;
  // -= 2 so we can leave holes.
  for (i = NUMBER_INSERTIONS - 1; i >= 0; i -= 2) {
    NP1_TEST_ASSERT(lst.insert(i));
  }

  size_t starting_size = NUMBER_INSERTIONS/2;
  NP1_TEST_ASSERT(lst.size() == starting_size);

  // Check that erasing a not-found element fails.
  NP1_TEST_ASSERT(!lst.erase(10));
  NP1_TEST_ASSERT(lst.size() == starting_size);
  NP1_TEST_ASSERT(lst.find(9) != lst.end());
  NP1_TEST_ASSERT(lst.find(11) != lst.end());

  // Test erasing an element somewhere in the middle of the list.
  NP1_TEST_ASSERT(lst.find(1021) != lst.end());
  NP1_TEST_ASSERT(lst.erase(1021));
  NP1_TEST_ASSERT(lst.size() == starting_size-1);
  NP1_TEST_ASSERT(lst.find(1021) == lst.end());

  // Check that the elements either side of the erased element look ok.
  skip_list_type::iterator iter = lst.lower_bound(1021);
  NP1_TEST_ASSERT(iter != lst.end());
  NP1_TEST_ASSERT(1023 == *iter);

  iter = lst.lower_bound(1019);
  NP1_TEST_ASSERT(iter != lst.end());
  NP1_TEST_ASSERT(1019 == *iter);

  // Test erasing an element at the end of the list.
  NP1_TEST_ASSERT(lst.lower_bound(NUMBER_INSERTIONS-1) != lst.end());
  NP1_TEST_ASSERT(lst.erase(NUMBER_INSERTIONS-1));
  iter = lst.lower_bound(NUMBER_INSERTIONS-1);
  NP1_TEST_ASSERT(iter == lst.end());   
  NP1_TEST_ASSERT(lst.lower_bound(NUMBER_INSERTIONS-3) != lst.end());
  NP1_TEST_ASSERT(lst.size() == starting_size-2);


  // Test erasing the element at the front of the list.
  NP1_TEST_ASSERT(lst.lower_bound(1) == lst.begin());
  NP1_TEST_ASSERT(lst.erase(1));
  NP1_TEST_ASSERT(lst.lower_bound(1) == lst.begin());
  NP1_TEST_ASSERT(*lst.lower_bound(1) == 3);
  NP1_TEST_ASSERT(lst.size() == starting_size-3);
}


void test_skip_list_clear() {
  static const int NUMBER_INSERTIONS = 1000;
  skip_list_type lst;  

  int i;
  for (i = 0; i < NUMBER_INSERTIONS; ++i) {
    NP1_TEST_ASSERT(lst.insert(i));
  }

  NP1_TEST_ASSERT(lst.begin() != lst.end());
  
  lst.clear();

  NP1_TEST_ASSERT(lst.begin() == lst.end());

  NP1_TEST_ASSERT(lst.size() == 0);
}

void test_skip_list_pop_front() {
  static const int NUMBER_INSERTIONS = 1000;
  skip_list_type lst;  

  int i;
  for (i = 0; i < NUMBER_INSERTIONS; ++i) {
    NP1_TEST_ASSERT(lst.insert(i));
  }

  NP1_TEST_ASSERT(lst.size() == (size_t)NUMBER_INSERTIONS);
  
  for (i = 0; i < NUMBER_INSERTIONS; ++i) {
    NP1_TEST_ASSERT(*(lst.begin()) == i);
    lst.pop_front();
  }

  NP1_TEST_ASSERT(lst.begin() == lst.end());

  NP1_TEST_ASSERT(lst.size() == 0);
}


void test_skip_list() {
  NP1_TEST_RUN_TEST(test_skip_list_insert);
  NP1_TEST_RUN_TEST(test_skip_list_insert_random);
  NP1_TEST_RUN_TEST(test_skip_list_lower_bound);
  NP1_TEST_RUN_TEST(test_skip_list_find);
  NP1_TEST_RUN_TEST(test_skip_list_erase);
  NP1_TEST_RUN_TEST(test_skip_list_clear);
  NP1_TEST_RUN_TEST(test_skip_list_pop_front);
}

} // namespaces
}
}

#endif
