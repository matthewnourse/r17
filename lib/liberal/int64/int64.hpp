#ifndef LIB_INT64_HPP
#define LIB_INT64_HPP

#include <stdint.h>

namespace lib {
namespace int64 {

#ifndef __LP64__
int64_t divide(int64_t numerator, int64_t denominator);
uint64_t divide(uint64_t numerator, uint64_t denominator);

int64_t modulus(int64_t numerator, int64_t denominator);
uint64_t modulus(uint64_t numerator, uint64_t denominator);
#else
inline int64_t divide(int64_t numerator, int64_t denominator) { return numerator/denominator; }
inline uint64_t divide(uint64_t numerator, uint64_t denominator) { return numerator/denominator; }

inline int64_t modulus(int64_t numerator, int64_t denominator) { return numerator % denominator; }
inline uint64_t modulus(uint64_t numerator, uint64_t denominator) { return numerator % denominator; }
#endif

} // namespaces
}


#endif
