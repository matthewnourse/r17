// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_TEST_UNIT_NP1_IO_TEST_EXT_HEAP_BUFFER_OUTPUT_STREAM_HPP
#define NP1_TEST_UNIT_NP1_IO_TEST_EXT_HEAP_BUFFER_OUTPUT_STREAM_HPP



namespace test {
namespace unit {
namespace np1 {
namespace io {

typedef ::np1::rel::rlang::vm_heap vm_heap_type;


void test_ext_heap_buffer_output_stream_many_sizes() {
  std::string test_data = make_alphabet_test_data_string(100000);

  size_t allocation_size;
  size_t write_size;
  vm_heap_type heap;

  for (allocation_size = 4096; allocation_size < 16385; allocation_size += 4096) {
  
    for (write_size = 1; write_size < 8135; ++write_size) {
      ::np1::io::ext_heap_buffer_output_stream<vm_heap_type> output(heap, allocation_size);
      const char *p = test_data.c_str();
      const char *end = p + test_data.length();
      while (p < end) {
        size_t bytes_to_write = write_size;
        const char *new_p = p + bytes_to_write;
        if (new_p > end) {
          bytes_to_write = end - p;      
        }

        NP1_TEST_ASSERT(output.write(p, bytes_to_write));
        p += bytes_to_write;        
      }

      NP1_TEST_ASSERT(test_data.length() == output.size());
      NP1_TEST_ASSERT(memcmp(test_data.c_str(), output.ptr(), output.size()) == 0);

      heap.reset();
    }
  }
}




void test_ext_heap_buffer_output_stream() {
  NP1_TEST_RUN_TEST(test_ext_heap_buffer_output_stream_many_sizes);
}

} // namespaces
}
}
}

#endif
