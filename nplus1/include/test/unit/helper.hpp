// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_TEST_UNIT_HELPER_HPP
#define NP1_TEST_UNIT_HELPER_HPP



#define NP1_TEST_ASSERT(assertion__) \
NP1_ASSERT(assertion__, std::string(__FILE__) + ": " + ::np1::str::to_dec_str(__LINE__))

#define NP1_TEST_RUN_TEST(test__) \
do { printf("%s\n", #test__); fflush(stdout); test__(); } while (0)


#endif
