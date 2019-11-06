#ifndef LONG_H
#define LONG_H

#include <cstdint>
#include <type_traits>

template<uint64_t P>
using Long = std::integral_constant<uint64_t, P>;

#endif

