// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_HASH_SHA256_HPP
#define NP1_HASH_SHA256_HPP

/*
 *  FIPS-180-2 compliant SHA-256 implementation
 *  based on the md5deep implementation by Christophe Devine.
 */

namespace np1 {
namespace hash {

class sha256 {
public:
  sha256() {
    m_total[0] = 0;
    m_total[1] = 0;
  
    m_state[0] = 0x6A09E667;
    m_state[1] = 0xBB67AE85;
    m_state[2] = 0x3C6EF372;
    m_state[3] = 0xA54FF53A;
    m_state[4] = 0x510E527F;
    m_state[5] = 0x9B05688C;
    m_state[6] = 0x1F83D9AB;
    m_state[7] = 0x5BE0CD19;
  }

  static void hash(const unsigned char *buf, size_t len, unsigned char *digest) {
    sha256 ctx;
    ctx.update(buf, len);
    ctx.final(digest);
  }

  void update(const unsigned char *input, size_t length) {
    do_update(input, (uint32_t)length);
  }

  
  void final(unsigned char *digest) {
    static uint8_t sha256_padding[64] = {
      0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };
  
    uint32_t last, padn;
    uint32_t high, low;
    uint8_t msglen[8];
  
    high = (m_total[0] >> 29) | (m_total[1] << 3);
    low  = (m_total[0] <<  3);
  
    put32(high, msglen, 0);
    put32(low, msglen, 4);
  
    last = m_total[0] & 0x3F;
    padn = (last < 56) ? (56 - last) : (120 - last);
  
    update(sha256_padding, padn);
    update(msglen, 8);
  
    put32(m_state[0], digest,  0);
    put32(m_state[1], digest,  4);
    put32(m_state[2], digest,  8);
    put32(m_state[3], digest, 12);
    put32(m_state[4], digest, 16);
    put32(m_state[5], digest, 20);
    put32(m_state[6], digest, 24);
    put32(m_state[7], digest, 28);
  }
  
private:  
  void do_update(const unsigned char *input, uint32_t length) {
    uint32_t left, fill;

    if (!length) {
      return;
    }

    left = m_total[0] & 0x3F;
    fill = 64 - left;

    m_total[0] += length;
    m_total[0] &= 0xFFFFFFFF;

    if (m_total[0] < length) {
      m_total[1]++;
    }

    if (left && (length >= fill)) {
      memcpy((m_buffer + left), input, fill);
      process(m_buffer);
      length -= fill;
      input  += fill;
      left = 0;
    }

    while (length >= 64) {
      process(input);
      length -= 64;
      input += 64;
    }

    if (length) {
      memcpy((m_buffer + left), input, length);
    }
  }


  static inline uint32_t get32(const uint8_t *b, uint32_t i) {
    return ((uint32_t)b[i] << 24 )       
          | ((uint32_t)b[i + 1] << 16)       
          | ((uint32_t)b[i + 2] <<  8)       
          | ((uint32_t)b[i + 3]);
  }

  static inline void put32(uint32_t n, uint8_t *b, uint32_t i) {
    b[i] = (uint8_t)(n >> 24);
    b[i + 1] = (uint8_t)(n >> 16);
    b[i + 2] = (uint8_t)(n >>  8);
    b[i + 3] = (uint8_t)n;
  }
    
  
  void process(const uint8_t data[64]) {
    uint32_t temp1, temp2, W[64];
    uint32_t A, B, C, D, E, F, G, H;
  
    W[0] = get32(data, 0);
    W[1] = get32(data, 4);
    W[2] = get32(data, 8);
    W[3] = get32(data, 12);
    W[4] = get32(data, 16);
    W[5] = get32(data, 20);
    W[6] = get32(data, 24);
    W[7] = get32(data, 28);
    W[8] = get32(data, 32);
    W[9] = get32(data, 36);
    W[10] = get32(data, 40);
    W[11] = get32(data, 44);
    W[12] = get32(data, 48);
    W[13] = get32(data, 52);
    W[14] = get32(data, 56);
    W[15] = get32(data, 60);
  
  
#define SHA256_R(t)                             \
(                                               \
    W[t] = s1(W[t -  2]) + W[t -  7] +          \
           s0(W[t - 15]) + W[t - 16]            \
)

#define SHA256_P(a,b,c,d,e,f,g,h,x,K)           \
{                                               \
    temp1 = h + s3(e) + f1(e,f,g) + K + x;      \
    temp2 = s2(a) + f0(a,b,c);                  \
    d += temp1; h = temp1 + temp2;              \
}
  
    A = m_state[0];
    B = m_state[1];
    C = m_state[2];
    D = m_state[3];
    E = m_state[4];
    F = m_state[5];
    G = m_state[6];
    H = m_state[7];
  
    SHA256_P(A, B, C, D, E, F, G, H, W[ 0], 0x428A2F98);
    SHA256_P(H, A, B, C, D, E, F, G, W[ 1], 0x71374491);
    SHA256_P(G, H, A, B, C, D, E, F, W[ 2], 0xB5C0FBCF);
    SHA256_P(F, G, H, A, B, C, D, E, W[ 3], 0xE9B5DBA5);
    SHA256_P(E, F, G, H, A, B, C, D, W[ 4], 0x3956C25B);
    SHA256_P(D, E, F, G, H, A, B, C, W[ 5], 0x59F111F1);
    SHA256_P(C, D, E, F, G, H, A, B, W[ 6], 0x923F82A4);
    SHA256_P(B, C, D, E, F, G, H, A, W[ 7], 0xAB1C5ED5);
    SHA256_P(A, B, C, D, E, F, G, H, W[ 8], 0xD807AA98);
    SHA256_P(H, A, B, C, D, E, F, G, W[ 9], 0x12835B01);
    SHA256_P(G, H, A, B, C, D, E, F, W[10], 0x243185BE);
    SHA256_P(F, G, H, A, B, C, D, E, W[11], 0x550C7DC3);
    SHA256_P(E, F, G, H, A, B, C, D, W[12], 0x72BE5D74);
    SHA256_P(D, E, F, G, H, A, B, C, W[13], 0x80DEB1FE);
    SHA256_P(C, D, E, F, G, H, A, B, W[14], 0x9BDC06A7);
    SHA256_P(B, C, D, E, F, G, H, A, W[15], 0xC19BF174);
    SHA256_P(A, B, C, D, E, F, G, H, SHA256_R(16), 0xE49B69C1);
    SHA256_P(H, A, B, C, D, E, F, G, SHA256_R(17), 0xEFBE4786);
    SHA256_P(G, H, A, B, C, D, E, F, SHA256_R(18), 0x0FC19DC6);
    SHA256_P(F, G, H, A, B, C, D, E, SHA256_R(19), 0x240CA1CC);
    SHA256_P(E, F, G, H, A, B, C, D, SHA256_R(20), 0x2DE92C6F);
    SHA256_P(D, E, F, G, H, A, B, C, SHA256_R(21), 0x4A7484AA);
    SHA256_P(C, D, E, F, G, H, A, B, SHA256_R(22), 0x5CB0A9DC);
    SHA256_P(B, C, D, E, F, G, H, A, SHA256_R(23), 0x76F988DA);
    SHA256_P(A, B, C, D, E, F, G, H, SHA256_R(24), 0x983E5152);
    SHA256_P(H, A, B, C, D, E, F, G, SHA256_R(25), 0xA831C66D);
    SHA256_P(G, H, A, B, C, D, E, F, SHA256_R(26), 0xB00327C8);
    SHA256_P(F, G, H, A, B, C, D, E, SHA256_R(27), 0xBF597FC7);
    SHA256_P(E, F, G, H, A, B, C, D, SHA256_R(28), 0xC6E00BF3);
    SHA256_P(D, E, F, G, H, A, B, C, SHA256_R(29), 0xD5A79147);
    SHA256_P(C, D, E, F, G, H, A, B, SHA256_R(30), 0x06CA6351);
    SHA256_P(B, C, D, E, F, G, H, A, SHA256_R(31), 0x14292967);
    SHA256_P(A, B, C, D, E, F, G, H, SHA256_R(32), 0x27B70A85);
    SHA256_P(H, A, B, C, D, E, F, G, SHA256_R(33), 0x2E1B2138);
    SHA256_P(G, H, A, B, C, D, E, F, SHA256_R(34), 0x4D2C6DFC);
    SHA256_P(F, G, H, A, B, C, D, E, SHA256_R(35), 0x53380D13);
    SHA256_P(E, F, G, H, A, B, C, D, SHA256_R(36), 0x650A7354);
    SHA256_P(D, E, F, G, H, A, B, C, SHA256_R(37), 0x766A0ABB);
    SHA256_P(C, D, E, F, G, H, A, B, SHA256_R(38), 0x81C2C92E);
    SHA256_P(B, C, D, E, F, G, H, A, SHA256_R(39), 0x92722C85);
    SHA256_P(A, B, C, D, E, F, G, H, SHA256_R(40), 0xA2BFE8A1);
    SHA256_P(H, A, B, C, D, E, F, G, SHA256_R(41), 0xA81A664B);
    SHA256_P(G, H, A, B, C, D, E, F, SHA256_R(42), 0xC24B8B70);
    SHA256_P(F, G, H, A, B, C, D, E, SHA256_R(43), 0xC76C51A3);
    SHA256_P(E, F, G, H, A, B, C, D, SHA256_R(44), 0xD192E819);
    SHA256_P(D, E, F, G, H, A, B, C, SHA256_R(45), 0xD6990624);
    SHA256_P(C, D, E, F, G, H, A, B, SHA256_R(46), 0xF40E3585);
    SHA256_P(B, C, D, E, F, G, H, A, SHA256_R(47), 0x106AA070);
    SHA256_P(A, B, C, D, E, F, G, H, SHA256_R(48), 0x19A4C116);
    SHA256_P(H, A, B, C, D, E, F, G, SHA256_R(49), 0x1E376C08);
    SHA256_P(G, H, A, B, C, D, E, F, SHA256_R(50), 0x2748774C);
    SHA256_P(F, G, H, A, B, C, D, E, SHA256_R(51), 0x34B0BCB5);
    SHA256_P(E, F, G, H, A, B, C, D, SHA256_R(52), 0x391C0CB3);
    SHA256_P(D, E, F, G, H, A, B, C, SHA256_R(53), 0x4ED8AA4A);
    SHA256_P(C, D, E, F, G, H, A, B, SHA256_R(54), 0x5B9CCA4F);
    SHA256_P(B, C, D, E, F, G, H, A, SHA256_R(55), 0x682E6FF3);
    SHA256_P(A, B, C, D, E, F, G, H, SHA256_R(56), 0x748F82EE);
    SHA256_P(H, A, B, C, D, E, F, G, SHA256_R(57), 0x78A5636F);
    SHA256_P(G, H, A, B, C, D, E, F, SHA256_R(58), 0x84C87814);
    SHA256_P(F, G, H, A, B, C, D, E, SHA256_R(59), 0x8CC70208);
    SHA256_P(E, F, G, H, A, B, C, D, SHA256_R(60), 0x90BEFFFA);
    SHA256_P(D, E, F, G, H, A, B, C, SHA256_R(61), 0xA4506CEB);
    SHA256_P(C, D, E, F, G, H, A, B, SHA256_R(62), 0xBEF9A3F7);
    SHA256_P(B, C, D, E, F, G, H, A, SHA256_R(63), 0xC67178F2);
  
    m_state[0] += A;
    m_state[1] += B;
    m_state[2] += C;
    m_state[3] += D;
    m_state[4] += E;
    m_state[5] += F;
    m_state[6] += G;
    m_state[7] += H;
  }

  static uint32_t shr(uint32_t x, uint32_t n) { return (x & 0xFFFFFFFF) >> n; }
  static uint32_t rotr(uint32_t x, uint32_t n) { return shr(x, n) | x << (32 - n); }
  
  static uint32_t s0(uint32_t x) { return (rotr(x, 7) ^ rotr(x,18) ^  shr(x, 3)); }
  static uint32_t s1(uint32_t x) { return (rotr(x,17) ^ rotr(x,19) ^  shr(x,10)); }
  static uint32_t s2(uint32_t x) { return (rotr(x, 2) ^ rotr(x,13) ^ rotr(x,22)); }
  static uint32_t s3(uint32_t x) { return (rotr(x, 6) ^ rotr(x,11) ^ rotr(x,25)); }
  
  static uint32_t f0(uint32_t x, uint32_t y, uint32_t z) { return ((x & y) | (z & (x | y))); }
  static uint32_t f1(uint32_t x, uint32_t y, uint32_t z) { return (z ^ ( x & (y ^ z))); }
  
private:
  uint32_t m_total[2];
  uint32_t m_state[8];
  uint8_t m_buffer[64];
};


} // namespaces
}

#endif
