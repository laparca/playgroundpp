#ifndef __LONG_H__
#define __LONG_H__

#include <cstdint>
#include <type_traits>

template<uint64_t P>
using Long = std::integral_constant<uint64_t, P>;

#endif

