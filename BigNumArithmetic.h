#pragma once
#include <cstdlib>
#include <cstdint>
#include <type_traits>
#include <vector>

template<typename T>
	requires std::is_unsigned_v<T>
std::string bigIntToString(const std::vector<T>& num) {
	std::vector<bool> bits;
	bits.reserve(num.size() * sizeof(T) * 8);

	for (size_t i = num.size() - 1; i != std::numeric_limits<size_t>::max(); i--) {
		for (size_t j = sizeof(T) * 8 - 1; j != std::numeric_limits<size_t>::max(); j--) {  // MSB first
			bits.push_back((num[i] >> j) & 1);
		}
	}

	if (bits.empty()) return "0";

	std::vector<uint8_t> decimal{ 0 }; // Store decimal digits

	// Process each bit from MSB to LSB
	for (size_t i = 0; i < bits.size(); i++) {
		// Multiply current number by 2
		uint8_t carry = 0;
		for (size_t j = 0; j < decimal.size(); j++) {
			uint16_t current = decimal[j] * 2 + carry;
			decimal[j] = current % 10;
			carry = current / 10;
		}
		if (carry) {
			decimal.push_back(carry);
		}

		// Add current bit
		if (bits[i]) {
			// Add 1 to the result
			carry = 1;
			for (size_t j = 0; j < decimal.size() && carry; j++) {
				uint8_t sum = decimal[j] + carry;
				decimal[j] = sum % 10;
				carry = sum / 10;
			}
			if (carry) {
				decimal.push_back(carry);
			}
		}
	}

	// Convert to string
	std::string result;
	result.reserve(decimal.size());
	for (auto it = decimal.rbegin(); it != decimal.rend(); ++it) {
		result.push_back('0' + *it);
	}

	return result;
}

//helper struct to get the nest greater sized type for example if T = uint16_t, Greater::Type = uint32_t
template<typename T>
	requires std::is_unsigned_v<T>
struct Greater {
	using Type = std::conditional_t<
		std::is_same_v<T, unsigned char>, unsigned short,
		std::conditional_t<
		std::is_same_v<T, unsigned short>, unsigned int,
		std::conditional_t<
		std::is_same_v<T, unsigned int>, unsigned long,
		std::conditional_t<
		std::is_same_v<T, unsigned long>, unsigned long long,
		void
		>
		>
		>
	>;
};

//template <typename T>
//	requires std::is_unsigned_v<T>
//static inline std::pair<T, T> multWithCarry(T a, T b) {
//	static const size_t halfBits = sizeof(T) * 4;
//	static const T lowMask = (T(1) << halfBits) - 1;
//
//	std::cout << halfBits << std::endl;
//
//	T aHigh = a >> halfBits;
//	T bHigh = b >> halfBits;
//
//	T aLow = a & lowMask;
//	T bLow = b & lowMask;
//
//	T x = aLow * bLow;
//	T s0 = x & lowMask;
//
//	x = aHigh * bLow + (x >> halfBits);
//
//	T s1 = x & lowMask;
//	T s2 = x >> halfBits;
//
//	x = s1 + aLow * bHigh;
//	s1 = x & lowMask;
//
//	x = s2 + aHigh * bHigh + (x >> halfBits);
//	s2 = x & lowMask;
//	T s3 = x >> halfBits;
//
//	T high = s3 << halfBits | s2;
//	T low = s1 << halfBits | s0;
//
//	return { low, high };
//}

template <typename T>
	requires std::is_unsigned_v<T>
static inline std::pair<T, T> multWithCarry(T a, T b) {
	static const size_t bits = sizeof(T) * 8;
	typename Greater<T>::Type res = static_cast<typename Greater<T>::Type>(a) * b;
	return { static_cast<T>(res), static_cast<T>(res >> bits) };
}

template<>
std::pair<uint64_t, uint64_t> multWithCarry<uint64_t>(uint64_t a, uint64_t b) {
	uint64_t hi, lo;
#ifdef _MSC_VER
	lo = _umul128(a, b, &hi);
#else
	__uint128_t res = static_cast<__uint128_t>(a) * b;
	lo = static_cast<uint64_t>(res);
	hi = static_cast<uint64_t>(res >> 64);
#endif
	return { lo, hi };
}

static enum class CompareResult
{
	LESS = -1,
	EQUAL = 0,
	GREATER = 1
};

//-1 if x < y 1 if x > y 0 if x == y
template<typename T>
	requires std::is_unsigned_v<T>
CompareResult compare(const std::vector<T>& x, const std::vector<T>& y)
{
	if (x.size() > y.size())
		return CompareResult::GREATER;
	else if (y.size() > x.size())
		return CompareResult::LESS;

	for (size_t i = x.size() - 1; i != std::numeric_limits<size_t>::max(); --i)
		if (x[i] > y[i])
			return CompareResult::GREATER;
		else if (y[i] > x[i])
			return CompareResult::LESS;
	return CompareResult::EQUAL;
}

template<typename T>
	requires std::is_unsigned_v<T>
std::vector<T> add(const std::vector<T>& x, const std::vector<T>& y) //assumes thisSize >= otherSize
{
	//if (x.size() < y.size())
	//	return add(y, x);
	std::vector<T> result(x.size() + 1);
	T carry = 0;

	for (size_t i = 0; i < y.size(); i++)
	{
		result[i] = x[i] + y[i] + carry;
		if (carry)
			carry = std::numeric_limits<T>::max() - x[i] <= y[i];
		else carry = std::numeric_limits<T>::max() - x[i] < y[i];		
	}

	for (size_t i = y.size(); i < x.size(); i++)
	{
		result[i] = x[i] + carry;
		carry = std::numeric_limits<T>::max() - x[i] == 0;
	}
	if (carry)
		result[x.size()] = 1;
	else result.pop_back();
	return result;
}

template<typename T>
	requires std::is_unsigned_v<T>
std::vector<T> subtract(const std::vector<T>& x, const std::vector<T>& y) //assumes x >= y
{
	//if (compare(x, y) != CompareResult::GREATER)
	//	return std::vector<T>();

	std::vector<T> result(x.size());
	T borrow = 0;

	for (size_t i = 0; i < y.size(); i++)
	{
		result[i] = x[i] - y[i] - borrow;
		if (borrow)
			borrow = y[i] >= x[i];
		else borrow = y[i] > x[i];
	}

	for (size_t i = y.size(); i < x.size(); i++)
	{
		result[i] = x[i] - borrow;
		borrow = x[i] < borrow;
	}

	for (size_t i = result.size() - 1; i != std::numeric_limits<size_t>::max(); i--)
	{
		if (result[i] != 0)
			break;
		result.pop_back();
	}

	return result;
}

template<typename T>
	requires std::is_unsigned_v<T>
std::vector<T> multiply(const std::vector<T>& x, const std::vector<T>& y)
{
	//if (y.size() > x.size())
	//	return multiply(y, x);
	std::vector<T> result(x.size() * y.size() + 1, 0);

	for (size_t i = 0; i < x.size(); ++i)
	{
		T carry = 0;
		for (size_t j = 0; j < y.size(); ++j)
		{
			auto [res, carryInternal] = multWithCarry(x[i], y[j]);
			T buffer = res + carry;

			if (buffer < res)
				carryInternal++;
			res = buffer;

			buffer = result[i + j] + res;
			if (buffer < result[i + j])
				carryInternal++;
			carry = carryInternal;
			result[i + j] = buffer;
		}
		result[i + y.size()] = carry;
	}

	for (size_t i = result.size() - 1; i != std::numeric_limits<size_t>::max(); i--)
	{
		if (result[i] != 0)
			break;
		result.pop_back();
	}

	return result;
}

template<typename T>
	requires std::is_unsigned_v<T>
std::vector<T> divide(const std::vector<T>& x, const std::vector<T>& y) {
	if (y.size() == 0)
		throw std::invalid_argument("Division by zero");
	if (x.size() == 0)
		return std::vector<T>();
	CompareResult res = compare(x, y);
	if (res != CompareResult::GREATER)
		return res == CompareResult::EQUAL ? std::vector<T>() : std::vector<T>{ 1 };

	std::vector<T> quotient(y.size() + 1, 0);
	std::vector<T> remainder;

	for (size_t i = x.size() - 1; i != std::numeric_limits<size_t>::max(); --i) {
		// Shift R left by one limb (multiply by base 2^64)
		remainder.insert(remainder.begin(), x[i]);

		if (compare(remainder, y) != CompareResult::LESS)
		{
			// Binary search for the quotient digit q
			T q_low = 0, q_high = std::numeric_limits<T>::max();
			T q = 0;

			while (q_low <= q_high) {
				T q_mid = q_low + (q_high - q_low) / 2;
				std::vector<T> product = multiply(y, { q_mid });

				if (compare(remainder, product) != CompareResult::LESS) {
					q_low = q_mid + 1;
				}
				else {
					q_high = q_mid - 1;
				}
			}
			q = q_high;

			if (q > 0) {
				remainder = subtract(remainder, multiply(y, { q }));
				quotient[i] = q;
			}
		}
	}

	for (size_t i = x.size() - 1; i != std::numeric_limits<size_t>::max(); i--)
	{
		if (quotient[i] != 0)
			break;
		quotient.pop_back();
	}

	return quotient;
}

template<typename T>
	requires std::is_unsigned_v<T>
std::vector<T> modulo(const std::vector<T>& x, const std::vector<T>& y) {
	if (y.size() == 0)
		throw std::invalid_argument("Division by zero");
	if (x.size() == 0)
		return std::vector<T>();
	CompareResult res = compare(x, y);
	if (res == CompareResult::LESS)
		return x;
	else if (res == CompareResult::EQUAL)
		return std::vector<T>();

	std::vector<T> remainder;

	for (size_t i = x.size() - 1; i != std::numeric_limits<size_t>::max(); --i) {
		// Shift R left by one limb (multiply by base 2^64)
		remainder.insert(remainder.begin(), x[i]);

		if (compare(remainder, y) != CompareResult::LESS)
		{
			// Binary search for the quotient digit q
			T q_low = 0, q_high = std::numeric_limits<T>::max();
			T q = 0;

			while (q_low <= q_high) {
				T q_mid = q_low + (q_high - q_low) / 2;
				std::vector<T> product = multiply(y, { q_mid });

				if (compare(remainder, product) != CompareResult::LESS) {
					q_low = q_mid + 1;
				}
				else {
					q_high = q_mid - 1;
				}
			}
			q = q_high;

			if (q > 0) {
				remainder = subtract(remainder, multiply(y, { q }));
			}
		}
	}

	return remainder;
}

//template<typename T>
//	requires std::is_unsigned_v<T>
//std::vector<T> power(const std::vector<T>& base, const std::vector<T>& exponent) {
//
//	std::vector<T> result(base.size(), 1);
//	std::vector<T> current_power = base;
//	std::vector<T> exp = exponent;
//
//	// Continue while any exponent is non-zero
//	while (std::any_of(exp.begin(), exp.end(), [](T e) { return e != 0; })) {
//		// For each element where current bit is 1, multiply result by current_power
//		for (size_t i = 0; i < exp.size(); i++) {
//			if (exp[i] & 1) {  // Check if odd
//				result[i] *= current_power[i];
//			}
//		}
//
//		// Square current_power for next iteration
//		for (size_t i = 0; i < current_power.size(); i++) {
//			current_power[i] *= current_power[i];
//		}
//
//		// Shift right all exponents (divide by 2)
//		for (size_t i = 0; i < exp.size(); i++) {
//			exp[i] >>= 1;
//		}
//	}
//
//	return result;
//}

template<typename T>
	requires std::is_unsigned_v<T>
std::vector<T> stringToBigInt(const std::string& str) {
	std::vector<T> num{ 0 };  // Start with 0

	{
		T digit = str[0] - '0';
		num[0] = digit;
	}

	for (size_t i = 1; i < str.size(); ++i) {
		if (!std::isdigit(str[i])) {
			throw std::invalid_argument("Invalid character in string");
		}

		// Multiply current number by 10
		std::vector<T> temp = multiply(num, { 10 });

		// Add current digit
		T digit = str[i] - '0';
		if (digit != 0) {
			temp = add(temp, { digit });
		}

		num = std::move(temp);
	}

	return num;
}