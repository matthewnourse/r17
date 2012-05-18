// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_FIXED_HOMOGENOUS_HEAP_HPP
#define NP1_FIXED_HOMOGENOUS_HEAP_HPP


namespace np1 {
  
class fixed_homogenous_heap {
private:
  struct block_header {
    block_header *m_next;
    // The block's data follows the block.
  };
  
public:
  fixed_homogenous_heap(size_t number_blocks, size_t block_size_bytes)
    : m_block_size(block_size_bytes),
      m_number_blocks(number_blocks) {
    size_t block_size_in_block_header_units = bytes_to_block_header_units(block_size_bytes);
    m_heap.resize(number_blocks * block_size_in_block_header_units);
    m_free_list = m_heap.begin();
    std::vector<block_header>::iterator ii = m_heap.begin();
    std::vector<block_header>::iterator iz = m_heap.end();
    std::vector<block_header>::iterator next = 0;
    for (; ii != iz; ii = next) {
      next = ii + block_size_in_block_header_units;
      if (next >= m_heap.end()) {
        ii->m_next = 0;
      } else {
        ii->m_next = next;
      }
    }
  }

  void *alloc(size_t sz) {
    NP1_ASSERT(m_free_list, "fixed_homogenous_heap out of space!");
    
    void *p = (void *)(m_free_list + 1);
    m_free_list = m_free_list->m_next;
    return p;
  }
  
  void free(void *p) {
    block_header *bh = ((block_header *)p) - 1;
    bh->m_next = m_free_list;
    m_free_list = bh;
  }

private:
  // Disable copy.
  fixed_homogenous_heap(const fixed_homogenous_heap &);
  fixed_homogenous_heap &operator = (const fixed_homogenous_heap &);
    
private:
  static size_t bytes_to_block_header_units(size_t bytes) {
    if ((bytes % sizeof(block_header)) == 0) {
      return bytes/sizeof(block_header) + 1;  // allow space for the actual header.
    }
    
    return (bytes/sizeof(block_header)) + 2;
  }
  
private:
  std::vector<block_header> m_heap;
  size_t m_block_size;
  size_t m_number_blocks;
  std::vector<block_header>::iterator m_free_list;
};
  
} // namespace


#endif
