// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_MATH_HPP
#define NP1_MATH_HPP

#include "np1/io/random.hpp"
#include "np1/hash/sha256.hpp"
#include "np1/time.hpp"



namespace np1 {
namespace math {

class rand64_state {
private:
  enum { SEED_SIZE = 2048 };
  enum { MAX_READS_BEFORE_SEED_INITIALIZATION = 100000 };
  enum { HASH_SIZE = 32 };

  struct state {
    state()
      : m_number_reads_since_seed_initialization(MAX_READS_BEFORE_SEED_INITIALIZATION),
        m_hash_end(m_hash + sizeof(m_hash)),
        m_hash_p(m_hash_end) {}
      
    uint64_t next() {
      if (m_number_reads_since_seed_initialization >= MAX_READS_BEFORE_SEED_INITIALIZATION) {
        static io::random rand_source;
        static io::mandatory_input_stream<io::random> mandatory_rand_source(rand_source);

        mandatory_rand_source.read(m_seed, sizeof(m_seed));

        // Add the pid and the time to reduce the chance of duplicate random numbers in other processes on the
        // same host.
        struct {
          pid_t pid;
          uint64_t now;
        } extra_entropy;
        extra_entropy.pid = getpid();
        extra_entropy.now = time::now_epoch_usec();
        memcpy(m_seed, &extra_entropy, sizeof(extra_entropy));
        m_number_reads_since_seed_initialization = 0;
      }

      uint64_t result;
      if (m_hash_p + sizeof(result) > m_hash_end) {
        m_hasher.update(m_seed, sizeof(m_seed));
        m_hasher.update(m_hash, sizeof(m_hash));
        m_hasher.final(m_hash);
        m_hash_p = m_hash;
      }
      
      memcpy(&result, m_hash_p, sizeof(result));
      m_hash_p += sizeof(result);
      ++m_number_reads_since_seed_initialization;
      return result;
    }

    void trash() {
      m_number_reads_since_seed_initialization = MAX_READS_BEFORE_SEED_INITIALIZATION;
      m_hash_p = m_hash_end;
    }

    uint64_t m_number_reads_since_seed_initialization;
    unsigned char m_seed[SEED_SIZE];
    unsigned char m_hash[HASH_SIZE];
    unsigned char *m_hash_end;  
    unsigned char *m_hash_p;
    hash::sha256 m_hasher;
  };

public:
  static uint64_t next() {
    return get_state().next();
  }
  static void trash() { get_state().trash(); }

private:
  static state &get_state() { static state s; return s; }
};


uint64_t rand64() {
  uint64_t n = rand64_state::next();
  return n;
}


void rand256_hex_encoded(char *data) {
  char *p = data;
  char *end = p + 64;
  for (; p < end; p += 16) {
    uint64_t r = math::rand64();
    str::to_hex_str_pad_16(p, r);
  }
}

} // namespaces
}


#endif
