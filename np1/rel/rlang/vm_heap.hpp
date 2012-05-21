// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_REL_RLANG_VM_HEAP_HPP
#define NP1_REL_RLANG_VM_HEAP_HPP


namespace np1 {
namespace rel {
namespace rlang {


/// The heap for generated string variables.
class vm_heap {
public:
  enum { DEFAULT_CHUNK_SIZE = (1024 * 1024) - 1024 }; // Allow some space for malloc overhead.

public:
  vm_heap() : m_chunk_iter(m_chunks.end()) {
    add_chunk(1);
  }

  char *alloc(size_t size) {
    if ((size_t)(m_chunk_end - m_unused_p) < size) {
      while ((++m_chunk_iter) != m_chunks.end()) {
        if (m_chunk_iter->size() >= size) {
          m_unused_p = m_chunk_iter->begin() + size;
          m_chunk_end = m_chunk_iter->end();
          return m_chunk_iter->begin();
        }
      }

      add_chunk(size);
    }

    char *p = m_unused_p;
    m_unused_p += size;
    return p;
  }

  /// This is a no-op, just here so that this heap can conform to a generic heap interface.
  void free(char *p) {}

  /// Don't deallocate, just set the free pointer back to the start of the
  /// heap.
  void reset() {
    m_chunk_iter = m_chunks.begin();
    m_unused_p = m_chunk_iter->begin();
    m_chunk_end = m_chunk_iter->end();
  }

private:
  /// Disable copy.
  vm_heap(const vm_heap &);
  vm_heap &operator = (const vm_heap &);

private:
  void add_chunk(size_t size_requirement) {
    rstd::vector<char> empty;
    m_chunks.push_back(empty);
    rstd::vector<char> chunk;
    chunk.resize(size_requirement > DEFAULT_CHUNK_SIZE
                  ? size_requirement : DEFAULT_CHUNK_SIZE);

    m_chunks.back().swap(chunk);

    m_chunk_iter = m_chunks.last();
    
    m_unused_p = m_chunk_iter->begin();
    m_chunk_end = m_chunk_iter->end();
  }

private:
  rstd::list<rstd::vector<char> > m_chunks;
  rstd::list<rstd::vector<char> >::iterator m_chunk_iter;
  char *m_unused_p;
  char *m_chunk_end;
};



} // namespaces
}
}


#endif
