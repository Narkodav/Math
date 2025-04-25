#pragma once
#include <iostream>
#include <cstdlib>
#include <cstdint>
#include <type_traits>
#include <vector>
#include <string>

//template <typename T>
//	requires std::is_unsigned_v<T>
//T hi(T x) {
//	static const size_t halfBits = sizeof(T) * 4;
//	return x >> halfBits;
//}
//
//template <typename T>
//	requires std::is_unsigned_v<T>
//T lo(T x) {
//	static const size_t halfBits = sizeof(T) * 4;
//	return ((T(1) << halfBits) - 1) & x;
//}
//
//template <typename T>
//	requires std::is_unsigned_v<T>
//std::pair<T, T> multiply_with_overflow(T a, T b) {
//	T s0, s1, s2, s3;
//
//	T x = lo(a) * lo(b);
//	s0 = lo(x);
//
//	x = hi(a) * lo(b) + hi(x);
//	s1 = lo(x);
//	s2 = hi(x);
//
//	x = s1 + lo(a) * hi(b);
//	s1 = lo(x);
//
//	x = s2 + hi(a) * hi(b) + hi(x);
//	s2 = lo(x);
//	s3 = hi(x);
//
//	T result = s1 << (sizeof(T) * 4) | s0;
//	T carry = s3 << (sizeof(T) * 4) | s2;
//
//	return { carry, result };
//}

template <typename T>
	requires std::is_unsigned_v<T>
static inline T getMultiplicationOverflow(T a, T b) {
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

	T carry = s3 << halfBits | s2;

	return carry;
}

//doesnt own memory and can be easily set
//all functions assume the spans store tightly packed data that doesn't have trailing zeroes
template <typename T>
struct MagnitudeSpan
{
	T* array = nullptr;
	size_t size = 0;

	T& operator[](size_t index) { return array[index]; };
	const T& operator[](size_t index) const { return array[index]; };
};

//template<typename T>
//	requires std::is_arithmetic_v<T>
//MagnitudeSpan<T> divideArraysToThis(MagnitudeSpan<T>& thisArray, const MagnitudeSpan<T>& otherArray)
//{
//	T* newArray;
//	size_t newSize;
//	divideArraysToNew(thisArray, thisSize, otherArray, otherSize, newArray, newSize);
//	delete[] thisArray;
//	thisArray = newArray;
//	thisSize = newSize;
//}

//template<typename T>
//	requires std::is_arithmetic_v<T>
//MagnitudeSpan<T> divideArraysToNew(const MagnitudeSpan<T>& thisArray, const MagnitudeSpan<T>& otherArray,
//	MagnitudeSpan<T>& newArray)
//{
//	const size_t n = otherArray.size;
//	const size_t m = thisArray.size - otherArray.size;
//
//	// Working storage for partial results
//	T partial[n + 1];
//	T shifted_divisor[n + 1];
//
//	// Main division loop
//	for (int64_t j = m; j >= 0; --j) {
//		// Estimate quotient digit (qhat)
//		T u2 = (j + n < thisArray.size) ? thisArray[j + n] : 0;
//		T u1 = (j + n - 1 < thisArray.size) ? thisArray[j + n - 1] : 0;
//		T v1 = otherArray[n - 1];
//
//		T qhat = (v1 == std::numeric_limits<T>::max())
//			? u2
//			: ((static_cast<__uint128_t>(u2) << 64) | u1) / (v1 + 1);
//
//		qhat = std::min(qhat, static_cast<T>(-1));  // Prevent overflow
//
//		// Multiply and subtract
//		T carry = 0;
//		for (size_t i = 0; i < n; ++i) {
//			__uint128_t product = static_cast<__uint128_t>(qhat) * otherArray[i];
//			T sum_lo = static_cast<T>(product) + carry;
//			carry = static_cast<T>(product >> 64) + (sum_lo < carry ? 1 : 0);
//			partial[i] = sum_lo;
//		}
//		partial[n] = carry;
//
//		// Adjust if estimate was too large
//		bool needs_adjustment = true;
//		while (needs_adjustment) {
//			needs_adjustment = false;
//
//			// Compare partial product with current dividend portion
//			for (size_t i = n; i-- > 0; ) {
//				size_t pos = j + i;
//				T dv = (pos < thisArray.size) ? thisArray[pos] : 0;
//
//				if (partial[i] != dv) {
//					if (partial[i] > dv) {
//						--qhat;
//						T borrow = 0;
//						for (size_t k = 0; k < n; ++k) {
//							T diff = partial[k] - otherArray[k] - borrow;
//							borrow = (diff > partial[k]) ? 1 : 0;
//							partial[k] = diff;
//						}
//						needs_adjustment = true;
//					}
//					break;
//				}
//			}
//		}
//
//		// Store quotient digit
//		if (j < thisArray.size) newArray[j] = qhat;
//
//		// Subtract from dividend
//		T borrow = 0;
//		for (size_t i = 0; i <= n; ++i) {
//			size_t pos = j + i;
//			if (pos >= thisArray.size) break;
//
//			T diff = thisArray[pos] - partial[i] - borrow;
//			borrow = (diff > thisArray[pos]) ? 1 : 0;
//			thisArray[pos] = diff;
//		}
//	}
//
//	//// Remainder is in the dividend array (first 'n' elements)
//	//std::copy_n(thisArray, n, remainder);
//}

template<typename T>
	requires std::is_arithmetic_v<T>
MagnitudeSpan<T> divideArraysToNew(const MagnitudeSpan<T>& thisArray, const MagnitudeSpan<T>& otherArray,
	MagnitudeSpan<T>& newArray)
{
	MagnitudeSpan<T> quotient;
	temp.array = new T[thisArray.size]();
	temp.size = thisArray.size;
	
	vector<int> quotient;
	vector<int> current;

	for (int digit : dividend) {
		current.push_back(digit);

		// Remove leading zeros
		if (current.size() > 1 && current[0] == 0) {
			current.erase(current.begin());
		}

		// If current is smaller than divisor, we just add 0 to quotient
		if (isLessThan(current, divisor)) {
			quotient.push_back(0);
			continue;
		}

		// Find how many times divisor fits into current
		int count = 0;
		vector<int> tempDivisor = divisor;
		while (!isLessThan(current, tempDivisor)) {
			current = subtractVectors(current, tempDivisor);
			count++;
		}

		quotient.push_back(count);
	}
}

template<typename T>
	requires std::is_arithmetic_v<T> && std::is_unsigned_v<T>
void multiplyArraysToNew(const MagnitudeSpan<T>& thisArray, const MagnitudeSpan<T>& otherArray,
	MagnitudeSpan<T>& newArray)
{
	for (size_t i = 0; i < thisArray.size; i++) {
		T carry = 0;
		for (size_t j = 0; j < otherArray.size; j++) {
			T res = thisArray[i] * otherArray[j];
			if (res / thisArray[i] != otherArray[j]) //thisArray[i] cannot be 0
				carry += getMultiplicationOverflow(thisArray[i], otherArray[j]);

			// Add low to current position, track carry
			T sum = newArray[i + j] + res;
			T addition_carry = (sum < newArray[i + j]) ? 1 : 0;
			newArray[i + j] = sum;

			// Add high and any carries
			carry += addition_carry;
		}
		newArray[i + otherArray.size] = carry;
	}
}

template<typename T>
	requires std::is_arithmetic_v<T>
void addArraysToNew(const MagnitudeSpan<T>& thisArray, const MagnitudeSpan<T>& otherArray,
	MagnitudeSpan<T>& newArray) //assumes thisSize >= otherSize and newSize is big enough
{
	T leftovers = 0;

	for (size_t i = 0; i < otherArray.size; i++)
	{
		newArray[i] = thisArray[i] + otherArray[i] + leftovers;
		leftovers = std::numeric_limits<T>::max() - thisArray[i] < otherArray[i];
	}

	for (size_t i = otherArray.size; i < thisArray.size; i++)
	{
		newArray[i] = thisArray[i] + leftovers;
		leftovers = std::numeric_limits<T>::max() - thisArray[i] == 0;
	}
}

template<typename T>
	requires std::is_arithmetic_v<T>
void substractArraysToNew(const MagnitudeSpan<T>& thisArray, const MagnitudeSpan<T>& otherArray,
	MagnitudeSpan<T>& newArray) //assumes thisSize >= otherSize and newArray is big enough
{
	T leftovers = 0;

	for (size_t i = 0; i < otherArray.size; i++)
	{
		newArray[i] = thisArray[i] - otherArray[i] - leftovers;
		if (leftovers)
			leftovers = otherArray[i] >= thisArray[i];
		else leftovers = otherArray[i] > thisArray[i];
	}

	for (size_t i = otherArray.size; i < thisArray.size; i++)
	{
		newArray[i] = thisArray[i] - leftovers;
		leftovers = thisArray[i] < leftovers;
	}
}

template<typename T>
	requires std::is_arithmetic_v<T>
bool compareMagnitutedsLess(const MagnitudeSpan<T>& thisArray, const MagnitudeSpan<T>& otherArray)
{
	if (thisArray.size < otherArray.size)
		return true;
	else if (thisArray.size > otherArray.size)
		return false;
	for (size_t i = thisArray.size - 1; i != std::numeric_limits<size_t>::max(); i--)
	{
		if (thisArray[i] > otherArray[i])
			return false;
		else if (thisArray[i] < otherArray[i])
			return true;
	}
	return false;
}

template<typename T>
	requires std::is_arithmetic_v<T>
bool compareMagnitutedsMore(const MagnitudeSpan<T>& thisArray, const MagnitudeSpan<T>& otherArray)
{
	if (thisArray.size < otherArray.size)
		return false;
	else if (thisArray.size > otherArray.size)
		return true;
	for (size_t i = thisArray.size - 1; i != std::numeric_limits<size_t>::max(); i--)
	{
		if (thisArray[i] > otherArray[i])
			return true;
		else if (thisArray[i] < otherArray[i])
			return false;
	}
	return false;
}

template<typename T>
	requires std::is_arithmetic_v<T>
MagnitudeSpan<T> compareMagnitutedsEqual(const MagnitudeSpan<T>& thisArray, const MagnitudeSpan<T>& otherArray)
{
	if (thisArray.size < otherArray.size)
		return false;
	else if (thisArray.size > otherArray.size)
		return false;
	for (size_t i = thisArray.size - 1; i != std::numeric_limits<size_t>::max(); i--)
	{
		if (thisArray[i] != otherArray[i])
			return false;
	}
	return true;
}