#pragma once
#include <cstdlib>
#include <cstdint>
#include <array>

#ifdef _MSC_VER
#include <intrin.h>
#define popcnt64 __popcnt64
#else
#define popcnt64 __builtin_popcountll
#endif

#ifdef _MSC_VER
#define popcnt32 __popcnt
#else
#define popcnt32 __builtin_popcount
#endif

template<typename T>
inline size_t getNextPowerOf2(T value)
{
    static_assert(std::is_unsigned<T>::value, "T must be unsigned integer type");
    value--;
    uint64_t shift = 1;
    while (shift < sizeof(T) * 8)
    {
        value |= value >> shift;
        shift *= 2;
    }
    value++;
    return value;
}

template<>
inline size_t getNextPowerOf2<uint64_t>(uint64_t value)
{
    value--;
    value |= value >> 1;
    value |= value >> 2;
    value |= value >> 4;
    value |= value >> 8;
    value |= value >> 16;
    value |= value >> 32;
    value++;
    return value;
}

template<>
inline size_t getNextPowerOf2<uint32_t>(uint32_t value)
{
    value--;
    value |= value >> 1;
    value |= value >> 2;
    value |= value >> 4;
    value |= value >> 8;
    value |= value >> 16;
    value++;
    return value;
}