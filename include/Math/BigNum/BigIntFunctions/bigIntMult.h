#pragma once
#include <cstdlib>
#include <cstdint>
#include <type_traits>
#include <vector>

#include "Math/bigIntAddAndSub.h"

template <typename T>
	requires std::is_unsigned_v<T>
static inline std::pair<T, T> multWithCarry(T a, T b) {
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

template<typename T>
	requires std::is_arithmetic_v<T>&& std::is_unsigned_v<T>
std::vector<T> multiplyMagnitudes(const std::vector<T>& x, const T& y) //assumes x.size() >= y.size()
{
	std::vector<T> newNum(x.size() + 1);
	T trailingCarry = 0;
	for (int i = 0; i < x.size(); i++)
	{
		auto [res, carry] = multWithCarry(x[i], y);
		trailingCarry += carry;
		newNum[i] = x[i] * y;
	}

	if (trailingCarry == 0)
		newNum.pop_back();
	else newNum.back() = trailingCarry;
	return newNum;
}



////naive approach
//template<typename T>
//	requires std::is_arithmetic_v<T>&& std::is_unsigned_v<T>
//std::vector<T> multiplyArraysToNewNaive(const std::vector<T>& thisArray, const std::vector<T>& otherArray)
//{
//	T carry = 0;
//	std::vector<T> newArray(thisArray.size() + otherArray.size() + 1);
//	for (size_t i = 0; i < thisArray.size; i++) {
//		for (size_t j = 0; j < otherArray.size; j++) {
//			T res = thisArray[i] * otherArray[j];
//			if (res / thisArray[i] != otherArray[j]) //thisArray[i] cannot be 0
//				carry += getMultiplicationOverflow(thisArray[i], otherArray[j]);
//
//			// Add low to current position, track carry
//			T sum = newArray[i + j] + res;
//			T addition_carry = (sum < newArray[i + j]) ? 1 : 0;
//			newArray[i + j] = sum;
//
//			// Add high and any carries
//			carry += addition_carry;
//		}
//		newArray[i + otherArray.size] = carry;
//	}
//	if (carry == 0)
//		newArray.pop_back();
//	return newArray;
//}

//kartasuba
template<typename T>
	requires std::is_arithmetic_v<T>&& std::is_unsigned_v<T>
std::vector<T> multiplyMagnitudes(const std::vector<T>& x, const std::vector<T>& y) //assumes x.size() >= y.size()
{
	if (x.size() == 0 || y.size() == 0)
		return { 0 };

	// Base case: Use naive multiplication for small numbers
	if (x.size() <= 1) {
		auto [lo, hi] = multWithCarry(x[0], y[0]);
		if (hi == 0) return { lo };
		else return { lo, hi };
	}

	size_t xMiddle = x.size() / 2;
	size_t yMiddle = std::min(xMiddle, y.size());

	// Split x and y into lower and upper halves
	std::vector<T> x0(x.begin(), x.begin() + xMiddle);
	std::vector<T> x1(x.begin() + xMiddle, x.end());
	std::vector<T> y0(y.begin(), y.begin() + yMiddle);
	std::vector<T> y1(y.begin() + yMiddle, y.end());

	// Recursive steps
	std::vector<T> z0 = multiplyMagnitudes(x0, y0);       // z0 = x0 * y0
	std::vector<T> z2 = multiplyMagnitudes(x1, y1);       // z2 = x1 * y1

	// z1 = (x1 + x0) * (y1 + y0) - z0 - z2
	std::vector<T> x01 = addMagnitudes(x1, x0);
	std::vector<T> y01 = addMagnitudes(y1, y0);
	std::vector<T> z1 = substractMagnitudes(substractMagnitudes(multiplyMagnitudes(x01, y01), z0), z2);

	// Combine results: z2 * B^(2m) + z1 * B^m + z0
	std::vector<T> res(z0.size() + 2 * xMiddle, 0);
	for (size_t i = 0; i < z0.size(); ++i) res[i] = z0[i];
	for (size_t i = 0; i < z1.size(); ++i) res[i + xMiddle] += z1[i];
	for (size_t i = 0; i < z2.size(); ++i) res[i + 2 * xMiddle] += z2[i];

	// Propagate carries
	T carry = 0;
	for (size_t i = 0; i < res.size(); ++i) {
		T sum = res[i] + carry;
		carry = sum < res[i] ? 1 : 0;
		res[i] = sum;
	}
	if (carry) res.push_back(carry);

	// Remove leading zeros
	while (res.size() > 1 && res.back() == 0) res.pop_back();
	return res;
}

//// Evaluates a bigint at x = k (for k = 0, 1, -1, 2, ∞)
//std::vector<uint64_t> evaluateAt(const std::vector<uint64_t>& num, int k) {
//	if (k == 0) return { num.empty() ? 0 : num[0] };  // x=0 → return x₀
//	if (k == std::numeric_limits<int>::max()) {  // x=∞ → return x₂ (highest part)
//		if (num.size() < 3) return { 0 };
//		return { num.back() };
//	}
//
//	std::vector<uint64_t> res;
//	uint64_t carry = 0;
//	for (uint64_t limb : num) {
//		uint64_t val = limb + carry * k;
//		res.push_back(val);
//		carry = val >> 64;
//	}
//	if (carry) res.push_back(carry);
//	return res;
//}

//// Toom-3 multiplication
//std::vector<uint64_t> toom3(const std::vector<uint64_t>& x, const std::vector<uint64_t>& y) {
//	size_t n = std::max(x.size(), y.size());
//	if (n <= 16) return multiplyMagnitudesKartasuba(x, y);  // Base case: Use Karatsuba for small numbers
//
//	size_t m = (n + 2) / 3;  // Split into 3 parts
//
//	// Split x and y into x₂, x₁, x₀ (each of size m)
//	auto x0 = std::vector<uint64_t>(x.begin(), x.begin() + std::min(m, x.size()));
//	auto x1 = std::vector<uint64_t>(x.begin() + std::min(m, x.size()), x.begin() + std::min(2 * m, x.size()));
//	auto x2 = std::vector<uint64_t>(x.begin() + std::min(2 * m, x.size()), x.end());
//
//	auto y0 = std::vector<uint64_t>(y.begin(), y.begin() + std::min(m, y.size()));
//	auto y1 = std::vector<uint64_t>(y.begin() + std::min(m, y.size()), y.begin() + std::min(2 * m, y.size()));
//	auto y2 = std::vector<uint64_t>(y.begin() + std::min(2 * m, y.size()), y.end());
//
//	// Evaluate at 0, 1, -1, 2, ∞
//	auto x0_val = evaluateAt(x, 0);
//	auto x1_val = evaluateAt(x, 1);
//	auto x_neg1_val = evaluateAt(x, -1);
//	auto x2_val = evaluateAt(x, 2);
//	auto x_inf_val = evaluateAt(x, INT_MAX);
//
//	auto y0_val = evaluateAt(y, 0);
//	auto y1_val = evaluateAt(y, 1);
//	auto y_neg1_val = evaluateAt(y, -1);
//	auto y2_val = evaluateAt(y, 2);
//	auto y_inf_val = evaluateAt(y, INT_MAX);
//
//	// Multiply evaluated points
//	auto r0 = multiplyMagnitudesKartasuba(x0_val, y0_val);  // r(0) = x(0)*y(0)
//	auto r1 = multiplyMagnitudesKartasuba(x1_val, y1_val);  // r(1) = x(1)*y(1)
//	auto r_neg1 = multiplyMagnitudesKartasuba(x_neg1_val, y_neg1_val);  // r(-1) = x(-1)*y(-1)
//	auto r2 = multiplyMagnitudesKartasuba(x2_val, y2_val);  // r(2) = x(2)*y(2)
//	auto r_inf = multiplyMagnitudesKartasuba(x_inf_val, y_inf_val);  // r(∞) = x(∞)*y(∞)
//
//	// Interpolate to reconstruct r = x × y
//	auto c0 = r0;  // c0 = r(0)
//	auto c4 = r_inf;  // c4 = r(∞)
//
//	auto tmp1 = addMagnitudes(r1, r_neg1);
//	auto tmp2 = addMagnitudes(tmp1, addMagnitudes(r0, r0));
//	auto c2 = div_bigint_by_small(tmp2, 2);  // c2 = (r(1) + r(-1) - 2r(0)) / 2
//
//	tmp1 = substractMagnitudes(r1, r_neg1);
//	auto c3 = div_bigint_by_small(tmp1, 2);  // c3 = (r(1) - r(-1)) / 2
//
//	tmp1 = substractMagnitudes(r2, r0);
//	tmp2 = substractMagnitudes(tmp1, multiply_by_small(c2, 4));
//	tmp1 = substractMagnitudes(tmp2, multiply_by_small(c3, 2));
//	auto c1 = div_bigint_by_small(tmp1, 3);  // c1 = (r(2) - r(0) - 4c2 - 2c3) / 3
//
//	// Recombine: r = c4·B⁴ⁿ + c3·B³ⁿ + c2·B²ⁿ + c1·Bⁿ + c0
//	auto res = addMagnitudes(c0, multiply_by_power_of_B(c1, m));
//	res = addMagnitudes(res, multiply_by_power_of_B(c2, 2 * m));
//	res = addMagnitudes(res, multiply_by_power_of_B(c3, 3 * m));
//	res = addMagnitudes(res, multiply_by_power_of_B(c4, 4 * m));
//
//	return res;
//}