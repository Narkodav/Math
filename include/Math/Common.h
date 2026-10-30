#pragma once
#include <cstdlib>
#include <cstdint>
#include <array>
#include <stdexcept>
#include <cassert>
#include <concepts>
#include <array>
#include <numbers>
#include <algorithm>

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

namespace Math {
    template<typename T>
    constexpr auto s_pi = std::numbers::pi_v<T>;
    template<typename T>
    constexpr auto s_piDiv180 = s_pi<T> / 180.f;
    template<typename T>
    constexpr auto s_180DivPi = 180.f / s_pi<T>;

    template<typename T>
    constexpr T radians(T val) { return val * s_piDiv180<T>; }

    template<typename T>
    constexpr T degrees(T val) { return val * s_180DivPi<T>; }

    // Passing 0 is UB
    template<typename T>
    constexpr inline size_t getNextPowerOf2(T value)
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
    constexpr inline size_t getNextPowerOf2<uint64_t>(uint64_t value)
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
    constexpr inline size_t getNextPowerOf2<uint32_t>(uint32_t value)
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

	template<typename T>
	constexpr inline T gaussCircleApproximation(T radius)
	{
		return s_pi<T> * radius * radius + radius * sqrt(2 * s_pi<T>) + 1 / 3;
	}

	template<typename T>
	constexpr inline bool isInCircle(T x, T y, T radius)
	{
		return x * x + y * y <= radius * radius;
	}

	template<typename T>
	constexpr inline T lerp(T a, T b, T alpha)
	{
		return a + alpha * (b - a);
	}

	template<typename T>
	constexpr inline T fade(T alpha)
	{
		return ((6 * alpha - 15) * alpha + 10) * alpha * alpha * alpha;
	}

}