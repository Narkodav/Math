#pragma once
#include <cstdlib>
#include <cstdint>
#include <type_traits>
#include <vector>
#include <string>
#include <limits>
#include <bit>
#include <span>

#include "Math/Utility/ArrayBase.h"

// Intrinsic like helpers
namespace Math::Utility {
		template <typename T>
		requires std::is_unsigned_v<T>
	static inline std::pair<T, T> multFallback(T a, T b) {
		static const size_t halfBits = sizeof(T) * 4;
		static const T lowMask = (T(1) << halfBits) - 1;
	
		std::cout << halfBits << std::endl;
	
		T aHigh = a >> halfBits;
		T bHigh = b >> halfBits;
	
		T aLow = a & lowMask;
		T bLow = b & lowMask;
	
		T x = aLow * bLow;
		T s0 = x & lowMask;
	
		x = aHigh * bLow + (x >> halfBits);
	
		T s1 = x & lowMask;
		T s2 = x >> halfBits;
	
		x = s1 + aLow * bHigh;
		s1 = x & lowMask;
	
		x = s2 + aHigh * bHigh + (x >> halfBits);
		s2 = x & lowMask;
		T s3 = x >> halfBits;
	
		T high = s3 << halfBits | s2;
		T low = s1 << halfBits | s0;
	
		return { low, high };
	}

	std::pair<uint64_t, uint64_t> divModFallback(uint64_t high, uint64_t low, uint64_t divisor) {
		std::pair<uint64_t, uint64_t> res;
		// Handle division by zero
		if (divisor == 0) {
			// Division by zero - behavior is undefined, return 0 and remainder 0
			return {0, 0};
		}
		
		// Special case: if high < divisor, we can use standard 64-bit division
		if (high < divisor) {
			uint64_t quotient;
			
			// Combine high and low into a 128-bit number and divide
			// Using binary long division algorithm
			uint64_t remainder_high = high;
			uint64_t result = 0;
			
			for (int i = 0; i < 64; i++) {
				// Shift result left by 1
				result <<= 1;
				
				// Shift the high bit of low into remainder_high
				remainder_high = (remainder_high << 1) | ((low >> 63) & 1);
				low <<= 1;
				
				// If remainder_high >= divisor, subtract and set result bit
				if (remainder_high >= divisor) {
					remainder_high -= divisor;
					result |= 1;
				}
			}
			
			return {result, remainder_high};
		}
		else {
			// When high >= divisor, we need full 128-bit division
			// Using binary long division algorithm for 128 bits
			
			uint64_t result = 0;
			
			// Combine high and low into a 128-bit number by shifting and adding bits
			// Process all 128 bits
			for (int i = 0; i < 128; i++) {
				// Shift result left by 1
				result <<= 1;
				
				// Get the most significant bit of the current dividend
				uint64_t msb = (high >> 63) & 1;
				
				// Shift the 128-bit dividend left by 1
				high = (high << 1) | ((low >> 63) & 1);
				low <<= 1;
				
				// Check if we can subtract divisor from high
				// For the first 64 bits, we use high as the remainder
				// For the last 64 bits, we need to be careful about overflow
				
				if (i < 64) {
					// First 64 bits: high contains the working remainder
					if (high >= divisor) {
						high -= divisor;
						result |= 1;
					}
				} else {
					// Last 64 bits: high might be large, but we can still compare
					// We need to handle the case where high >= divisor
					if (high >= divisor || (high == divisor && msb)) {
						high -= divisor;
						result |= 1;
					}
				}
			}
			
			return {result, high};
		}
	}

	//helper struct to get the next greater sized type for example if T = uint16_t, NextSizedInteger::Type = uint32_t
	template<typename T>
		requires std::is_unsigned_v<T>
	struct NextSizedInteger {
		static_assert(false, "Unknown integer type");
	};

	template<>
	struct NextSizedInteger<uint8_t> {
		using Type = uint16_t;
	};

	template<>
	struct NextSizedInteger<uint16_t> {
		using Type = uint32_t;
	};

	template<>
	struct NextSizedInteger<uint32_t> {
		using Type = uint64_t;
	};

	template <typename T>
		requires std::is_unsigned_v<T>
	static inline bool compareGreater(T hiLeft, T loLeft, T hiRight, T loRight) {		
		if constexpr (sizeof(T) == 8) {
	#if defined(__SIZEOF_INT128__)
			__uint128_t left = (static_cast<__uint128_t>(hiLeft) << 64) | loLeft;
			__uint128_t right = (static_cast<__uint128_t>(hiRight) << 64) | loRight;
			return left > right;
	#else
			return hiLeft > hiRight || ((hiLeft == hiRight) && (loLeft > loRight));
	#endif
		}
		static const size_t bits = sizeof(T) * 8;
		using WideT = typename NextSizedInteger<T>::Type;
		WideT left = (static_cast<WideT>(hiLeft) << bits) | loLeft;
		WideT right = (static_cast<WideT>(hiRight) << bits) | loRight;
		return left > right;
	}

	template <typename T>
		requires std::is_unsigned_v<T>
	static inline std::pair<T, T> mult(T a, T b) {
		static const size_t bits = sizeof(T) * 8;
		using WideT = typename NextSizedInteger<T>::Type;
		WideT res = static_cast<WideT>(a) * b;
		return { static_cast<T>(res), static_cast<T>(res >> bits) };
	}

	template<>
	inline std::pair<uint64_t, uint64_t> mult<uint64_t>(uint64_t a, uint64_t b) {
    // ---------------- MSVC intrinsic ----------------
	#if defined(_MSC_VER) && !defined(__clang__)
		uint64_t hi;
		uint64_t lo = _umul128(a, b, &hi);
		return { lo, hi };
	#endif
	
		// ---------------- GCC / Clang __uint128_t ----------------
	#if defined(__SIZEOF_INT128__)
		__uint128_t res = static_cast<__uint128_t>(a) * b;
		uint64_t lo = static_cast<uint64_t>(res);
		uint64_t hi = static_cast<uint64_t>(res >> 64);
		return { lo, hi };
	#endif

		// ---------------- Branchless fallback (manual 64-bit split) ----------------
		return multFallback(a, b);
	}

	// ================== Addition with carry ==================
	template <typename T>
	requires std::is_unsigned_v<T>
	static inline std::pair<T, T> addWithCarry(T a, T b, bool carry) // returns a pair where first is result and second is carry
	{
		// ---------------- MSVC intrinsics ----------------
	#if defined(_MSC_VER) && !defined(__clang__)
		if constexpr (sizeof(T) == 8) {
			T res;
			carry = _addcarry_u64(static_cast<unsigned char>(carry), a, b, &res);
			return { res, carry };
		}
		else if constexpr (sizeof(T) == 4) {
			T res;
			carry = _addcarry_u32(static_cast<unsigned char>(carry), a, b, &res);
			return { res, carry };
		}
	#endif

		// ---------------- __uint128_t ----------------	
		if constexpr (sizeof(T) == 8) {
	#if defined(__SIZEOF_INT128__)
			__uint128_t sum = static_cast<__uint128_t>(a) + b + carry;
			carry = static_cast<bool>(sum >> 64);
			return { static_cast<uint64_t>(sum), carry };
		// ---------------- Fallback: GCC / Clang builtins ----------------
	#elif defined(__GNUC__) || defined(__clang__)
			T res;
			auto carryIntermediate = __builtin_uaddll_overflow(a, b, &res);
			carry = __builtin_uaddll_overflow(res, static_cast<T>(carry), &res) | carryIntermediate;
			return { res, carry };
	#else
			// ---------------- Branchless fallback ----------------
			T res = a + b + carry;
			carry = (res < a) || (carry && res == a);
			return { res, carry };
	#endif
		}
		else {
			static const size_t bits = sizeof(T) * 8;
			using WideT = typename NextSizedInteger<T>::Type;
			WideT res = static_cast<WideT>(a) + b + carry;
			carry = static_cast<bool>(res >> bits);
			return { static_cast<T>(res), carry };
		}
	}

	template <typename T>
	requires std::is_unsigned_v<T>
	static inline std::pair<T, T> addWithBigCarry(T a, T b, T carry) // returns a pair where first is result and second is carry
	{
		if constexpr (sizeof(T) == 8) {
	#if defined(__SIZEOF_INT128__)
			__uint128_t sum = static_cast<__uint128_t>(a) + b + carry;
			return { static_cast<uint64_t>(sum), static_cast<uint64_t>(sum >> 64) };
	#else
			// ---------------- Branchless fallback ----------------
			auto [ sum1, carry1 ] = addWithCarry(a, b, 0);
			auto [ sum2, carry2 ] = addWithCarry(sum1, carry, 0);
			return { sum2, carry1 + carry2 };
	#endif
		}
		else {
			static const size_t bits = sizeof(T) * 8;
			using WideT = typename NextSizedInteger<T>::Type;
			WideT sum = static_cast<WideT>(a) + b + carry;
			return { static_cast<T>(sum), static_cast<T>(sum >> bits) };
		}
	}

	// ================== Subtraction with borrow ==================
	template <typename T>
	requires std::is_unsigned_v<T>
	static inline std::pair<T, T> subWithBorrow(T a, T b, bool borrow) // returns a pair where first is result and second is borrow
	{
		// ---------------- MSVC intrinsics ----------------
	#if defined(_MSC_VER) && !defined(__clang__)
		if constexpr (sizeof(T) == 8) {
			T res;
			borrow = _subborrow_u64(static_cast<unsigned char>(borrow), a, b, &res);
			return { res, borrow };
		}
		else if constexpr (sizeof(T) == 4) {
			T res;
			borrow = _subborrow_u32(static_cast<unsigned char>(borrow), a, b, &res);
			return { res, borrow };
		}
	#endif

		// ---------------- GCC / Clang builtins ----------------
	#if defined(__GNUC__) || defined(__clang__)
		if constexpr (sizeof(T) == 8) {
			T res;
			auto borrowIntermediate = __builtin_usubll_overflow(a, b, &res);
			borrow = __builtin_usubll_overflow(res, static_cast<T>(borrow), &res) | borrowIntermediate;
			return { res, borrow };
		}
		else if constexpr (sizeof(T) == 4) {
			T res;
			auto borrowIntermediate = __builtin_usub_overflow(a, b, &res);
			borrow = __builtin_usub_overflow(res, static_cast<T>(borrow), &res) | borrowIntermediate;
			return { res, borrow };
		}
	#endif

		// ---------------- Branchless fallback ----------------
		T t = a - b;
		T res = t - borrow;
		borrow = (a < b) || (borrow && a == b);
		return { res, borrow };
	}

	template <typename T>
	requires std::is_unsigned_v<T>
	static inline std::pair<T, T> subWithBigBorrow(T a, T b, T borrow) // returns a pair where first is result and second is borrow
	{
		auto [ res1, borrow1 ] = subWithBorrow(a, b, 0);
		auto [ res2, borrow2 ] = subWithBorrow(res1, borrow, 0);
		return { res2, borrow1 + borrow2 };
	}

	template <typename T>
	requires std::is_unsigned_v<T>
	static inline T shiftRight(T hi, T lo, uint8_t shift) // returns low
	{
		// ---------------- MSVC intrinsics ----------------
	#if defined(_MSC_VER) && !defined(__clang__)
		if constexpr (sizeof(T) == 8) {
			T res;
			res = __shiftright128(lo, hi, shift);
			return res;
		}
	#endif

		// ---------------- Branchless fallback ----------------
		T res = (lo >> shift) | (hi << (sizeof(T) - shift));
		return res;
	}

	template <typename T>
	requires std::is_unsigned_v<T>
	static inline T shiftLeft(T hi, T lo, uint8_t shift) // returns high
	{
		// ---------------- MSVC intrinsics ----------------
	#if defined(_MSC_VER) && !defined(__clang__)
		if constexpr (sizeof(T) == 8) {
			T res;
			res = __shiftleft128(lo, hi, shift);
			return res;
		}
	#endif

		// ---------------- Branchless fallback ----------------
		T res = (lo >> (sizeof(T) - shift)) | (hi << shift);
		return res;
	}

	// Convenience wrappers around std utilities
	template <typename T>
	requires std::is_unsigned_v<T>
	static inline uint8_t countLeadingZeroes(T num) {
		return std::countl_zero(num);
	}

	template <typename T>
	requires std::is_unsigned_v<T>
	static inline uint8_t countLeadingOnes(T num) {
		return std::countl_one(num);
	}

	template <typename T>
	requires std::is_unsigned_v<T>
	static inline uint8_t countRightZeroes(T num) {
		return std::countr_zero(num);
	}

	template <typename T>
	requires std::is_unsigned_v<T>
	static inline uint8_t countRightOnes(T num) {
		return std::countr_one(num);
	}

	template <typename T>
	requires std::is_unsigned_v<T>
	static inline std::pair<T, T> divMod(T hi, T lo, T divisor) // returns pair where first is quotient and second is remainder
	{
		if constexpr (sizeof(T) == 8) {
	#if defined(_MSC_VER) && !defined(__clang__)
			std::pair<T, T> res;
			res.first = _udiv128(hi, lo, &res.second);
			return res;
	#elif defined(__SIZEOF_INT128__)
			std::pair<T, T> res;
			__uint128_t divident = static_cast<__uint128_t>(hi << 64) | lo;			
			res.first = divident / divisor;
			res.second = divident % divisor;
			return res;
	#else
			return divModFallback(hi, lo, divisor);
	#endif
		}
		static const size_t bits = sizeof(T) * 8;
		using WideT = typename NextSizedInteger<T>::Type;
		std::pair<T, T> res;
		WideT divident = static_cast<WideT>(hi << bits) | lo;			
		res.first = divident / divisor;
		res.second = divident % divisor;
		return res;
	}
}

namespace Math::StaticBigNum {

	template<typename Container>
	requires std::is_unsigned_v<typename Container::ValueType>
	void shiftThisLeftOneLimb(Container& val, uint8_t shift)  // shift from 0 to sizeof(ValueType) * 8 * 2
	{
		using ValueType = typename Container::ValueType;
		if (shift == 0) return;

		ValueType carry = 0;
		ValueType newCarry = 0;

		for (size_t i = 0; i < val.size(); ++i) {
			newCarry = val[i] >> (sizeof(ValueType) * 8 - shift);
			val[i] = (val[i] << shift) | carry;
			carry = newCarry;
		}
	}

	template<typename Container>
	requires std::is_unsigned_v<typename Container::ValueType>
	void shiftThisRightOneLimb(Container& val, uint8_t shift) // shift from 0 to sizeof(ValueType) * 8 * 2
	{
		using ValueType = typename Container::ValueType;
		if (shift == 0) return;

		ValueType carry = 0;
		ValueType newCarry = 0;

		for (size_t i = val.size() - 1; i != std::numeric_limits<size_t>::max(); --i) {
			newCarry = val[i] << (sizeof(ValueType) * 8 - shift);
			val[i] = (val[i] >> shift) | carry;
			carry = newCarry;
		}
	}

	enum class CompareResult
	{
		Less = -1,
		Equal = 0,
		Greater = 1
	};

	// -1 if x < y : 1 if x > y : 0 if x == y
	template<typename ContainerLeft, typename ContainerRight>
	requires (std::is_unsigned_v<typename ContainerLeft::ValueType> 
		&& std::is_unsigned_v<typename ContainerRight::ValueType> &&
		sizeof(typename ContainerLeft::ValueType) == sizeof(typename ContainerRight::ValueType))
	CompareResult compare(const ContainerLeft& x, const ContainerRight& y)
	{
		size_t i;
		if constexpr (ContainerLeft::size() > ContainerRight::size()) {
            i = x.size() - 1;
            for (; i != y.size() - 1; --i)
                if (x[i] != 0)
                    return CompareResult::Greater;
        }
		else if constexpr (ContainerLeft::size() < ContainerRight::size()) {
			i = y.size() - 1;
            for (; i != x.size() - 1; --i)
                if (y[i] != 0)
                    return CompareResult::Less;
		}
		else {
			i = x.size() - 1;
		}
		for (; i != std::numeric_limits<size_t>::max(); --i)
			if (x[i] > y[i])
				return CompareResult::Greater;
			else if (x[i] < y[i])
				return CompareResult::Less;
		return CompareResult::Equal;
	}

	template<typename ContainerLeft, typename ContainerRight>
	requires (std::is_unsigned_v<typename ContainerLeft::ValueType> 
		&& std::is_unsigned_v<typename ContainerRight::ValueType> &&
		sizeof(typename ContainerLeft::ValueType) == sizeof(typename ContainerRight::ValueType))
	using BiggerArray = std::conditional_t<ContainerLeft::size() >= ContainerRight::size(), 
	Math::Utility::ArrayBase<typename ContainerLeft::ValueType, ContainerLeft::size()>, 
	Math::Utility::ArrayBase<typename ContainerLeft::ValueType, ContainerRight::size()>
	>;

	template<typename ContainerLeft, typename ContainerRight>
	requires (std::is_unsigned_v<typename ContainerLeft::ValueType> 
		&& std::is_unsigned_v<typename ContainerRight::ValueType> &&
		sizeof(typename ContainerLeft::ValueType) == sizeof(typename ContainerRight::ValueType))
	BiggerArray<ContainerLeft, ContainerRight> add(const ContainerLeft& x, const ContainerRight& y, bool carry = 0)
	{
		BiggerArray<ContainerLeft, ContainerRight> result;
		using ValueType = typename ContainerLeft::ValueType;
		size_t i = 0;
		if constexpr (ContainerLeft::size() > ContainerRight::size()) {
			for (; i < ContainerRight::size(); i++)
				std::tie(result[i], carry) = Math::Utility::addWithCarry(x[i], y[i], carry);
			for (; i < ContainerLeft::size(); i++)
				std::tie(result[i], carry) = Math::Utility::addWithCarry(x[i], static_cast<ValueType>(0), carry);
		}
		else if constexpr (ContainerLeft::size() < ContainerRight::size()) {
			for (; i < ContainerLeft::size(); i++)
				std::tie(result[i], carry) = Math::Utility::addWithCarry(x[i], y[i], carry);
			for (; i < ContainerRight::size(); i++)
				std::tie(result[i], carry) = Math::Utility::addWithCarry(static_cast<ValueType>(0), y[i], carry);
		}
		else {
			for (; i < ContainerLeft::size(); i++)
				std::tie(result[i], carry) = Math::Utility::addWithCarry(x[i], y[i], carry);
		}
		return result;
	}

	template<typename ContainerLeft, typename ContainerRight>
	requires (std::is_unsigned_v<typename ContainerLeft::ValueType> 
		&& std::is_unsigned_v<typename ContainerRight::ValueType> &&
		sizeof(typename ContainerLeft::ValueType) == sizeof(typename ContainerRight::ValueType))
	BiggerArray<ContainerLeft, ContainerRight> subtract(const ContainerLeft& x, const ContainerRight& y, bool borrow = 0)
	{
		BiggerArray<ContainerLeft, ContainerRight> result;
		using ValueType = typename ContainerLeft::ValueType;
		size_t i = 0;
		if constexpr (ContainerLeft::size() > ContainerRight::size()) {
			for (; i < ContainerRight::size(); i++)
				std::tie(result[i], borrow) = Math::Utility::subWithBorrow(x[i], y[i], borrow);
			for (; i < ContainerLeft::size(); i++)
				std::tie(result[i], borrow) = Math::Utility::subWithBorrow(x[i], static_cast<ValueType>(0), borrow);
		}
		else if constexpr (ContainerLeft::size() < ContainerRight::size()) {
			for (; i < ContainerLeft::size(); i++)
				std::tie(result[i], borrow) = Math::Utility::subWithBorrow(x[i], y[i], borrow);
			for (; i < ContainerRight::size(); i++)
				std::tie(result[i], borrow) = Math::Utility::subWithBorrow(static_cast<ValueType>(0), y[i], borrow);
		}
		else {
			for (; i < ContainerLeft::size(); i++)
				std::tie(result[i], borrow) = Math::Utility::subWithBorrow(x[i], y[i], borrow);
		}
		return result;
	}

	template<typename ContainerLeft, typename ContainerRight>
	requires (std::is_unsigned_v<typename ContainerLeft::ValueType> 
		&& std::is_unsigned_v<typename ContainerRight::ValueType> &&
		sizeof(typename ContainerLeft::ValueType) == sizeof(typename ContainerRight::ValueType))
	BiggerArray<ContainerLeft, ContainerRight> multiply(const ContainerLeft& x, const ContainerRight& y)
	{
		BiggerArray<ContainerLeft, ContainerRight> result;
		using ValueType = typename ContainerLeft::ValueType;
		if constexpr (ContainerLeft::size() > ContainerRight::size()) {
			for (size_t i = 0; i < ContainerRight::size(); ++i)
			{
				ValueType carry = 0;
				for (size_t j = 0; j < ContainerLeft::size() - i; ++j)
				{
					auto [res, carryInternal] = Math::Utility::mult(y[i], x[j]);
					std::tie(result[i + j], carry) = Math::Utility::addWithBigCarry(result[i + j], res, carry);
					carry += carryInternal;
				}
			}
		}
		else {
			for (size_t i = 0; i < ContainerLeft::size(); ++i)
			{
				ValueType carry = 0;
				for (size_t j = 0; j < ContainerRight::size() - i; ++j)
				{
					auto [res, carryInternal] = Math::Utility::mult(x[i], y[j]);
					std::tie(result[i + j], carry) = Math::Utility::addWithBigCarry(result[i + j], res, carry);
					carry += carryInternal;
				}
			}
		}
		return result;
	}

	template<typename ContainerLeft, typename ContainerRight>
	requires (std::is_unsigned_v<typename ContainerLeft::ValueType> 
		&& std::is_unsigned_v<typename ContainerRight::ValueType> &&
		sizeof(typename ContainerLeft::ValueType) == sizeof(typename ContainerRight::ValueType))
	struct DivModResult {
		BiggerArray<ContainerLeft, ContainerRight> remainder;
		BiggerArray<ContainerLeft, ContainerRight> quotient;
	};

	template<typename ContainerLeft, typename ContainerRight>
	requires (std::is_unsigned_v<typename ContainerLeft::ValueType> &&
		std::is_unsigned_v<typename ContainerRight::ValueType> &&
		sizeof(typename ContainerLeft::ValueType) ==
		sizeof(typename ContainerRight::ValueType))
	DivModResult<ContainerLeft, ContainerRight> divMod(const ContainerLeft& x, const ContainerRight& y)
	{
		using ValueType = typename ContainerLeft::ValueType;

		constexpr size_t N = ContainerRight::size();

		DivModResult<ContainerLeft, ContainerRight> res{};

		size_t n = N;

		while (n > 0 && y[n - 1] == 0)
			--n;

		if (n == 0)
			throw std::runtime_error("division by zero");

		// normalization
		ValueType shift = countLeadingZeroes(y[n - 1]);

		BiggerArray<ContainerLeft, ContainerRight> u = x;
		BiggerArray<ContainerLeft, ContainerRight> v = y;

		shiftThisLeftOneLimb(u, shift);
		shiftThisLeftOneLimb(v, shift);

		Math::Utility::ArrayBase<ValueType,
			BiggerArray<ContainerLeft, ContainerRight>::size() + 1> un{};

		for (size_t i = 0; i < u.size(); ++i)
			un[i] = u[i];

		un.back() = 0;

		size_t m = u.size() - n;

		for (size_t j = m; j != std::numeric_limits<size_t>::max(); --j)
		{
			//-----------------------------------------
			// D3: estimate quotient
			//-----------------------------------------

			auto [ qhat, rhat ] = Math::Utility::divMod(un[j+n], un[j+n-1], v[n-1]);

			if (qhat == std::numeric_limits<uint64_t>::max())
				qhat--;

			if (n > 1)
			{
				while (true)
				{
					auto [plo, phi] = Math::Utility::mult(qhat, v[n-2]);					
					if (!Math::Utility::compareGreater(phi, plo, rhat, un[j+n-2]))
						break;

					qhat--;

					auto [sum, carry] = Math::Utility::addWithCarry(rhat, v[n-1], 0);
					rhat = sum;

					if (carry)
						break;
				}
			}

			//-----------------------------------------
			// D4: multiply and subtract
			//-----------------------------------------

			ValueType borrow = 0;

			for (size_t i = 0; i < n; ++i)
			{
				auto [plo, phi] = Math::Utility::mult(qhat, v[i]);

				auto s1 = Math::Utility::subWithBigBorrow(un[i+j], plo, borrow);
				auto s2 = Math::Utility::subWithBorrow(s1.diff, phi, 0);

				un[i+j] = s1.first;

				borrow = s1.second + s2.second;
			}

			bool negative;
			std::tie(un[j+n], negative) = Math::Utility::subWithBorrow(un[j+n], borrow, 0);

			//-----------------------------------------
			// D5: correction
			//-----------------------------------------

			if (negative)
			{
				qhat--;

				ValueType carry = 0;

				for (size_t i = 0; i < n; ++i) {
					std::tie(un[i+j], carry) = Math::Utility::addWithBigCarry(un[i+j], v[i], carry);
				}

				std::tie(un[j+n], std::ignore) = Math::Utility::addWithCarry(un[j+n], carry, 0);
			}

			res.quotient[j] = qhat;
		}

		//-----------------------------------------
		// remainder
		//-----------------------------------------

		for (size_t i = 0; i < n; ++i)
			res.remainder[i] = un[i];

		shiftThisRightOneLimb(res.remainder, shift);

		return res;
	}

	template<size_t s_digitCount>
	static inline constexpr size_t estimateByteCount() {
		return (s_digitCount * 3321928ULL / 1000000ULL + 7) / 8;
	}

	template<typename T, size_t s_digitCount>
	static inline constexpr size_t estimateLimbCount() {
		return (estimateByteCount<s_digitCount>() - 1) / sizeof(T) + 1;
	}


	template<typename T, size_t N>
		requires std::is_unsigned_v<T>
	constexpr auto stringToBigInt(std::span<const char, N> str) {
		Math::Utility::ArrayBase<T, estimateLimbCount<T, N>()> num(0);

		{
			T digit = str[0] - '0';
			num[0] = digit;
		}

		for (size_t i = 1; i < str.size(); ++i) {
			if (!std::isdigit(str[i])) {
				return Math::Utility::ArrayBase<T, 1>{0};
			}

			// Multiply current number by 10
			auto temp = multiply(num, Math::Utility::ArrayBase<T, 1>{ 10 });

			// Add current digit
			T digit = str[i] - '0';
			if (digit != 0) {
				temp = add(temp, Math::Utility::ArrayBase<T, 1>{ digit });
			}

			num = temp;
		}

		return num;
	}

	template<typename T, size_t N>
		requires std::is_unsigned_v<T>
	Math::Utility::ArrayBase<T, N> stringToBigInt(std::span<const char> str) {
		Math::Utility::ArrayBase<T, N> num(0);

		{
			T digit = str[0] - '0';
			num[0] = digit;
		}

		for (size_t i = 1; i < str.size(); ++i) {
			if (!std::isdigit(str[i])) {
				return {0};
			}

			// Multiply current number by 10
			auto temp = multiply(num, Math::Utility::ArrayBase<T, 1>{ 10 });

			// Add current digit
			T digit = str[i] - '0';
			if (digit != 0) {
				temp = add(temp, Math::Utility::ArrayBase<T, 1>{ digit });
			}

			num = temp;
		}

		return num;
	}

	template<typename T>
	struct Base10Divisor {
		static_assert(false, "Type not implemented");
	};

	template<>
	struct Base10Divisor<uint64_t> {
		static inline const uint64_t s_value = 1000000000000000000; //10^18
	};

	template<>
	struct Base10Divisor<uint32_t> {
		static inline const uint32_t s_value = 1000000000; //10^9
	};

	// template<typename T, size_t s_size>
	// uint32_t div_mod_base(std::vector<Limb>& limbs) {
	// 	uint64_t remainder = 0;

	// 	for (int i = limbs.size() - 1; i >= 0; --i) {
	// 		uint64_t cur = (remainder << 32) | limbs[i];
	// 		limbs[i] = static_cast<Limb>(cur / BASE);
	// 		remainder = cur % BASE;
	// 	}

	// 	while (!limbs.empty() && limbs.back() == 0)
	// 		limbs.pop_back();

	// 	return static_cast<uint32_t>(remainder);
	// }

	// template<typename T, size_t s_size>
	// 	requires std::is_unsigned_v<T>
	// std::string bigIntToString(const Utility::ArrayBase<T, s_size>& num) {

	// static inline const T s_divisor = Base10Divisor<T>::s_value;

	// std::string base10;
	
	

	// }
}