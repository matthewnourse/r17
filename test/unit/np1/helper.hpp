// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_TEST_UNIT_NP1_HELPER_HPP
#define NP1_TEST_UNIT_NP1_HELPER_HPP

namespace test {
namespace unit {
namespace np1 {


void make_test_data_record_string(rstd::string &test_data, size_t number_records) {
  test_data.clear();

  test_data.append("string:mul1_str\tint:mul1_int\tstring:mul7_str\tint:mul7_int\n");

  size_t counter;
  for (counter = 0; counter < number_records; ++counter) {
    char num_str[32];
    test_data.append("name_");
    ::np1::str::to_dec_str(num_str, counter);
    test_data.append(num_str);
    test_data.append("\t");
    test_data.append(::np1::str::to_dec_str(counter));
    test_data.append("\t");
    
    ::np1::str::to_dec_str(num_str, counter % 7);
    test_data.append("name_");
    test_data.append(num_str);
    test_data.append("\t");
    test_data.append(::np1::str::to_dec_str(counter));
    test_data.append("\n");
  }

  printf("  test data length is %zuK\n", test_data.length()/1024);
}


void make_large_test_data_record_string(rstd::string &test_data) {
  make_test_data_record_string(test_data, 1000000);  
}

void make_very_large_test_data_record_string(rstd::string &test_data) {
  make_test_data_record_string(test_data, 10000000);  
}


} // namespaces
}
}

#endif
