// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_TEST_UNIT_NP1_IO_TEST_GZFILE_HPP
#define NP1_TEST_UNIT_NP1_IO_TEST_GZFILE_HPP



namespace test {
namespace unit {
namespace np1 {
namespace io {

typedef ::np1::io::gzfile gzfile_type;


void test_gzfile_lifecycle() {
  static const char *FILE_NAME = "/tmp/gzfile_test.gz";
  ::np1::io::file::erase(FILE_NAME);
  rstd::string test_data = make_alphabet_test_data_string();

  // Create & write.
  {
    gzfile_type f;
    NP1_TEST_ASSERT(f.create_wo(FILE_NAME));
  
    ::np1::io::buffered_output_stream<gzfile_type> buffered_f(f);
    NP1_TEST_ASSERT(buffered_f.write(test_data.c_str(), test_data.length()));
  }

  // Open & read.
  {
    gzfile_type f;
    NP1_TEST_ASSERT(f.open_ro(FILE_NAME));
    char buffer[256 * 1024];
    size_t bytes_read = 1;
    rstd::string result;

    while (bytes_read > 0) {
      NP1_TEST_ASSERT(f.read(buffer, sizeof(buffer), &bytes_read));
      result.append(buffer, bytes_read);
    }
  
    NP1_TEST_ASSERT(result == test_data);
  }
}




void test_gzfile() {
  NP1_TEST_RUN_TEST(test_gzfile_lifecycle);
}

} // namespaces
}
}
}

#endif
