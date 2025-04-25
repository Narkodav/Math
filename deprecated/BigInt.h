#pragma once
#include <iostream>
#include <cstdlib>
#include <cstdint>
#include <type_traits>
#include <vector>
#include <string>

#include "Common.h"

template<typename T>
	requires std::is_arithmetic_v<T>
void divideArraysToThis(T*& thisArray, size_t& thisSize, const T* otherArray, size_t otherSize)
{
	T* newArray;
	size_t newSize;
	divideArraysToNew(thisArray, thisSize, otherArray, otherSize, newArray, newSize);
	delete[] thisArray;
	thisArray = newArray;
	thisSize = newSize;
}

template<typename T>
	requires std::is_arithmetic_v<T>
void divideArraysToNew(const T* thisArray, size_t thisSize, const T* otherArray, size_t otherSize, T*& newArray, size_t& newSize)
{
	newSize = thisSize - otherSize + 1; // good enough estimate
	newArray = new T[newSize]();

	// Find actual sizes by skipping leading zeros
	while (thisSize > 0 && thisArray[thisSize - 1] == 0) thisSize--;
	while (otherSize > 0 && otherArray[otherSize - 1] == 0) otherSize--;

	// Handle special cases
	if (otherSize == 0) {
		throw std::invalid_argument("Division by zero");
	}
	if (thisSize < otherSize) {
		newSize = 1;
		newArray[0] = 0;
		return;
	}

	// Create temporary array for remainder calculation
	T* temp = new T[thisSize + 1]();
	std::copy(thisArray, thisArray + thisSize, temp);

	// Main division loop
	for (size_t i = thisSize - otherSize; i != static_cast<size_t>(-1); i--) {
		// Find quotient digit
		T dividend_high = (i + otherSize < thisSize) ? temp[i + otherSize] : 0;
		T dividend_low = temp[i + otherSize - 1];
		T quotient = 0;

		// If we have a full double-word dividend
		if (dividend_high != 0) {
			// Estimate quotient using highest word
			quotient = std::numeric_limits<T>::max();
		}
		else {
			quotient = dividend_low / otherArray[otherSize - 1];
		}

		// Adjust quotient if necessary
		while (true) {
			T carry = 0;
			bool needAdjust = false;

			// Multiply and subtract
			for (size_t j = 0; j < otherSize; j++) {
				T product_low = quotient * otherArray[j];
				T subtrahend = product_low + carry;
				carry = (product_low < carry) ? 1 : 0;  // Handle overflow

				if (temp[i + j] < subtrahend) {
					carry++;
				}
				temp[i + j] -= subtrahend;
			}

			if (carry > dividend_high) {
				quotient--;
				// Add back
				T carry2 = 0;
				for (size_t j = 0; j < otherSize; j++) {
					T sum = temp[i + j] + otherArray[j] + carry2;
					carry2 = (sum < temp[i + j]) ? 1 : 0;
					temp[i + j] = sum;
				}
			}
			else {
				break;
			}
		}

		newArray[i] = quotient;
	}

	delete[] temp;
}

template<typename T>
	requires std::is_arithmetic_v<T>
void multiplyArraysToThis(T*& thisArray, size_t& thisSize, const T* otherArray, size_t otherSize)
{
	T* newArray;
	size_t newSize;
	multiplyArraysToNew(thisArray, thisSize, otherArray, otherSize, newArray, newSize);
	delete[] thisArray;
	thisArray = newArray;
	thisSize = newSize;
}

template<typename T>
	requires std::is_arithmetic_v<T>
void multiplyArraysToNew(const T* thisArray, size_t thisSize, const T* otherArray, size_t otherSize, T*& newArray, size_t& newSize)
{
	for (size_t i = thisSize - 1; i > 0; i--)
		if (thisArray[i] != 0)
		{
			thisSize = i + 1;
			break;
		}

	for (size_t i = otherSize - 1; i > 0; i--)
		if (otherArray[i] != 0)
		{
			otherSize = i + 1;
			break;
		}

	newSize = getNextPowerOf2((thisSize + otherSize) * 64 / sizeof(T));
	newArray = new T[newSize]();  // Initialize to zero

	for (size_t i = 0; i < thisSize; i++) {
		for (size_t j = 0; j < otherSize; j++) {
			// Calculate product and carry
			T product = thisArray[i] * otherArray[j];
			T carry = 0;

			// Add to existing value at position i+j
			T sum = newArray[i + j] + product;
			if (sum < newArray[i + j] || sum < product) { // Check for overflow
				carry = 1;
			}
			newArray[i + j] = sum;

			// Handle carry
			if (carry && i + j + 1 < newSize) {
				newArray[i + j + 1] += carry;
			}
		}
	}

	for (size_t i = thisSize + otherSize; i < newSize; i++)
		newArray[i] = 0;
}

template<typename T>
	requires std::is_arithmetic_v<T>
void addArraysToThis(T*& thisArray, size_t& thisSize, const T* otherArray, size_t otherSize)
{
	if ((thisArray[thisSize - 1] >> (std::numeric_limits<T>::digits - 1)) == 1 ||
		otherSize == thisSize &&
		(otherArray[otherSize - 1] >> (std::numeric_limits<T>::digits - 1)) == 1)
	{
		T* newArray = new T[otherSize];
		addArraysToNew(thisArray, thisSize, otherArray, otherSize, newArray, otherSize);
		delete[] thisArray;
		thisArray = newArray;
		thisSize = otherSize;
		return;
	}
	T leftovers = 0;

	for (size_t i = 0; i < otherSize; i++)
	{
		T buffer = thisArray[i];
		thisArray[i] = thisArray[i] + otherArray[i] + leftovers;
		leftovers = std::numeric_limits<T>::max() - buffer < otherArray[i];
	}

	for (size_t i = otherSize; i < thisSize; i++)
	{
		T buffer = thisArray[i];
		thisArray[i] = thisArray[i] + leftovers;
		leftovers = std::numeric_limits<T>::max() - buffer == 0;
	}
}

template<typename T>
	requires std::is_arithmetic_v<T>
void addArraysToNew(const T* thisArray, size_t thisSize, const T* otherArray, size_t otherSize, T*& newArray, size_t& newSize) //assumes thisSize >= otherSize
{
	if ((thisArray[thisSize - 1] >> (std::numeric_limits<T>::digits - 1)) == 1 ||
		otherSize == thisSize &&
		(otherArray[otherSize - 1] >> (std::numeric_limits<T>::digits - 1)) == 1)
		newSize = thisSize * 2 * 64 / sizeof(T);
	else newSize = thisSize;
	newArray = new T[newSize];
	T leftovers = 0;

	for (size_t i = 0; i < otherSize; i++)
	{
		newArray[i] = thisArray[i] + otherArray[i] + leftovers;
		leftovers = std::numeric_limits<T>::max() - thisArray[i] < otherArray[i];
	}

	for (size_t i = otherSize; i < thisSize; i++)
	{
		newArray[i] = thisArray[i] + leftovers;
		leftovers = std::numeric_limits<T>::max() - thisArray[i] == 0;
	}

	for (size_t i = thisSize; i < newSize; i++)
		newArray[i] = 0;
}

template<typename T>
	requires std::is_arithmetic_v<T>
void substractArraysToThis(T*& thisArray, size_t& thisSize, const T* otherArray, size_t otherSize)
{
	T leftovers = 0;

	for (size_t i = 0; i < otherSize; i++)
	{
		T buffer = thisArray[i];
		thisArray[i] = thisArray[i] - otherArray[i] - leftovers;
		if (leftovers)
			leftovers = otherArray[i] >= buffer;
		else leftovers = otherArray[i] > buffer;
	}

	for (size_t i = otherSize; i < thisSize; i++)
	{
		T buffer = thisArray[i];
		thisArray[i] = thisArray[i] - leftovers;
		leftovers = buffer < leftovers;
	}
}

template<typename T>
	requires std::is_arithmetic_v<T>
void substractArraysToNew(const T* thisArray, size_t thisSize, const T* otherArray, size_t otherSize, T*& newArray, size_t& newSize) //assumes thisSize >= otherSize
{
	newSize = thisSize;
	newArray = new T[newSize];
	T leftovers = 0;

	for (size_t i = 0; i < otherSize; i++)
	{
		newArray[i] = thisArray[i] - otherArray[i] - leftovers;
		if (leftovers)
			leftovers = otherArray[i] >= thisArray[i];
		else leftovers = otherArray[i] > thisArray[i];
	}

	for (size_t i = otherSize; i < thisSize; i++)
	{
		newArray[i] = thisArray[i] - leftovers;
		leftovers = thisArray[i] < leftovers;
	}
}

template<typename T>
	requires std::is_arithmetic_v<T>
bool compareMagnitutedsLess(const T* thisArray, size_t thisSize, const T* otherArray, size_t otherSize)
{
	size_t limit = thisSize;
	if (thisSize < otherSize)
	{
		for (size_t i = otherSize - 1; i != thisSize; i--)
			if (otherArray[i] != 0)
				return true;
	}
	else if (thisSize > otherSize)
	{
		for (size_t i = thisSize - 1; i != otherSize; i--)
			if (thisArray[i] != 0)
				return false;
		limit = otherSize;
	}

	for (size_t i = limit - 1; i != std::numeric_limits<size_t>::max(); i--)
	{
		if (thisArray[i] < otherArray[i])
			return true;
		else if (thisArray[i] > otherArray[i])
			return false;
	}
	return false;
}

template<typename T>
	requires std::is_arithmetic_v<T>
bool compareMagnitutedsMore(const T* thisArray, size_t thisSize, const T* otherArray, size_t otherSize)
{
	size_t limit = thisSize;
	if (thisSize < otherSize)
	{
		for (size_t i = otherSize - 1; i != thisSize; i--)
			if (otherArray[i] != 0)
				return false;
	}
	else if (thisSize > otherSize)
	{
		for (size_t i = thisSize - 1; i != otherSize; i--)
			if (thisArray[i] != 0)
				return true;
		limit = otherSize;
	}

	for (size_t i = limit - 1; i != std::numeric_limits<size_t>::max(); i--)
	{
		if (thisArray[i] < otherArray[i])
			return false;
		else if (thisArray[i] > otherArray[i])
			return true;
	}
	return false;
}

template<typename T>
	requires std::is_arithmetic_v<T>
bool compareMagnitutedsEqual(const T* thisArray, size_t thisSize, const T* otherArray, size_t otherSize)
{
	size_t limit = thisSize;
	if (thisSize < otherSize)
	{
		for (size_t i = otherSize - 1; i != thisSize; i--)
			if (otherArray[i] != 0)
				return false;
	}
	else if (thisSize > otherSize)
	{
		for (size_t i = thisSize - 1; i != otherSize; i--)
			if (thisArray[i] != 0)
				return false;
		limit = otherSize;
	}

	for (size_t i = limit - 1; i != std::numeric_limits<size_t>::max(); i--)
	{
		if (thisArray[i] != otherArray[i])
			return false;
	}
	return true;
}

class BigInt
{
public:
	using StorageType = uint64_t;
	static const size_t STORAGE_TYPE_BYTE_SIZE = sizeof(StorageType);
	static const size_t START_SIZE = 1;				// start size is 1 StorageType
	static const size_t GROWTH_COEFFICIENT = 2;		// will grow twice the size
	static const bool POSITIVE = false;
	static const bool NEGATIVE = true;

private:
	StorageType* m_num;
	size_t m_size;
	bool m_sign = POSITIVE;

	BigInt(StorageType* num, size_t size, bool sign) noexcept : m_num(num), m_size(size), m_sign(sign) {};
	BigInt(const char* str)
	{
		initFromString(str);
	}

public:

	BigInt() : m_num(new StorageType[START_SIZE]), m_size(START_SIZE)
	{
		for (int i = 0; i < START_SIZE; i++)
			m_num[i] = 0;
	};

	~BigInt() { delete[] m_num; };

	BigInt(const BigInt& other) noexcept
	{
		m_num = new StorageType[other.m_size];
		m_size = other.m_size;

		for (size_t i = 0; i < m_size; i++)
			m_num[i] = other.m_num[i];
	}

	BigInt(BigInt&& other) noexcept
	{
		m_num = other.m_num;
		m_size = other.m_size;

		other.m_num = nullptr;
		other.m_size = 0;
	}

	template<typename T>
	BigInt(T&& other) noexcept requires (!std::is_same_v<std::decay_t<T>, BigInt>&&
		std::is_arithmetic_v<T> && (sizeof(T) <= sizeof(StorageType)))
	{
		static_assert(std::is_arithmetic_v<T>, "Must be numeric type!");

		if constexpr (std::is_unsigned_v<T>)
		{
			m_num = new StorageType[1];
			m_size = 1;
			m_num[0] = std::forward<T>(other);
			m_sign = POSITIVE;
		}
		else
		{
			m_num = new StorageType[1];
			m_size = 1;
			if (other > 0)
			{
				m_num[0] = std::forward<T>(other);
				m_sign = POSITIVE;
			}
			else
			{
				m_num[0] = std::forward<T>(-other);
				m_sign = NEGATIVE;
			}
		}
	}

	BigInt& operator=(const BigInt& other) noexcept
	{
		if (this != &other)
		{
			delete[] m_num;
			m_num = new StorageType[other.m_size];
			m_size = other.m_size;

			for (int i = 0; i < m_size; i++)
				m_num[i] = other.m_num[i];
		}
		return *this;
	}

	BigInt& operator=(BigInt&& other) noexcept
	{
		if (this != &other)
		{
			delete[] m_num;
			m_num = other.m_num;
			m_size = other.m_size;

			other.m_num = nullptr;
			other.m_size = 0;
		}
		return *this;
	}

	template<typename T>
	BigInt& operator=(T&& other) noexcept requires (!std::is_same_v<std::decay_t<T>, BigInt>&&
		std::is_arithmetic_v<T> && (sizeof(T) <= sizeof(StorageType)))
	{
		static_assert(std::is_arithmetic_v<T>, "Must be numeric type!");
		delete[] m_num;

		if constexpr (std::is_unsigned_v<T>)
		{
			m_num = new StorageType[1];
			m_size = 1;
			m_num[0] = std::forward<T>(other);
			m_sign = POSITIVE;
		}
		else
		{
			m_num = new StorageType[1];
			m_size = 1;
			if (other > 0)
			{
				m_num[0] = std::forward<T>(other);
				m_sign = POSITIVE;
			}
			else
			{
				m_num[0] = std::forward<T>(-other);
				m_sign = NEGATIVE;
			}
		}

		return *this;
	}

	void shrinkToFit() // will shrink by growth coefficient
	{
		if (m_size <= 1) return;  // Always keep at least 1 byte

		StorageType lastBit = 1 << (STORAGE_TYPE_BYTE_SIZE * 8 - 1);
		StorageType sign = (m_num[m_size - 1] & lastBit) ? ~StorageType(0) : 0;

		// Find last non-redundant num
		uint64_t lastSignificantNum = m_size;
		while (lastSignificantNum > 1) {  // Keep at least 1 byte
			if (m_num[lastSignificantNum - 1] != sign ||
				(m_num[lastSignificantNum - 2] & lastBit) != (sign & lastBit)) {
				break;
			}
			lastSignificantNum--;
		}

		// Calculate new size as next power of 2 that fits the number
		size_t newSize = getNextPowerOf2(lastSignificantNum);

		if (newSize < m_size) {
			StorageType* newNum = new StorageType[newSize];
			std::memcpy(newNum, m_num, lastSignificantNum);
			// Fill remaining bytes with sign extension
			for (size_t i = lastSignificantNum; i < newSize; i++) {
				newNum[i] = sign;
			}
			delete[] m_num;
			m_num = newNum;
			m_size = newSize;
		}
	}

	template<typename T>
	BigInt operator+(const T& other) noexcept
	{
		if constexpr (std::is_signed_v<T>)
			//work in progress
			return BigInt();
		else if constexpr (std::is_unsigned_v<T>)
			//work in progress
			return BigInt();
		else if constexpr (std::is_same_v<std::decay_t<T>, BigInt>)
		{
			StorageType* num;
			size_t size;

			if (m_sign == other.m_sign) {
				if (m_size > other.m_size)
					addArraysToNew(m_num, m_size, other.m_num, other.m_size, num, size);
				else
					addArraysToNew(other.m_num, other.m_size, m_num, m_size, num, size);
				return BigInt(num, size, m_sign);
			}
			else {
				if (compareMagnitutedsLess(m_num, m_size, other.m_num, other.m_size))
				{
					substractArraysToNew(other.m_num, other.m_size, m_num, m_size, num, size);
					return BigInt(num, size, other.m_sign);
				}
				else
				{
					substractArraysToNew(m_num, m_size, other.m_num, other.m_size, num, size);
					return BigInt(num, size, m_sign);
				}
			}
		}
		return BigInt();
	}

	template<typename T>
	BigInt operator-(const T& other) noexcept
	{
		if constexpr (std::is_signed_v<T>)
			//work in progress
			return BigInt();
		else if constexpr (std::is_unsigned_v<T>)
			//work in progress
			return BigInt();
		else if constexpr (std::is_same_v<std::decay_t<T>, BigInt>)
		{
			StorageType* num;
			size_t size;

			if (m_sign == other.m_sign) {
				if (compareMagnitutedsLess(m_num, m_size, other.m_num, other.m_size))
				{
					substractArraysToNew(other.m_num, other.m_size, m_num, m_size, num, size);
					return BigInt(num, size, !m_sign);
				}
				else
				{
					substractArraysToNew(m_num, m_size, other.m_num, other.m_size, num, size);
					return BigInt(num, size, m_sign);
				}
			}
			else {
				if (m_size > other.m_size)
					addArraysToNew(m_num, m_size, other.m_num, other.m_size, num, size);
				else
					addArraysToNew(other.m_num, other.m_size, m_num, m_size, num, size);
				return BigInt(num, size, m_sign);
			}
		}
		return BigInt();
	}

	template<typename T>
	BigInt operator*(const T& other) noexcept
	{
		if constexpr (std::is_signed_v<T>)
		{
			bool resultSign;
			T* num;
			size_t size;
			T* thisNum = reinterpret_cast<T*>(m_num);
			size_t thisSize = m_size * sizeof(StorageType) / sizeof(T);
			if (other < 0)
			{
				other = -other;
				resultSign = POSITIVE == m_sign ? NEGATIVE : POSITIVE;
			}
			else resultSign = NEGATIVE == m_sign ? NEGATIVE : POSITIVE;
			multiplyArraysToNew(thisNum, thisSize, &other, 1, num, size);
			return BigInt(reinterpret_cast<StorageType*>(num), size / sizeof(StorageType) * sizeof(T), resultSign);
		}
		else if constexpr (std::is_unsigned_v<T>)
		{
			bool resultSign;
			T* num;
			size_t size;
			T* thisNum = reinterpret_cast<T*>(m_num);
			size_t thisSize = m_size * sizeof(StorageType) / sizeof(T);
			multiplyArraysToNew(thisNum, thisSize, &other, 1, num, size);
			return BigInt(reinterpret_cast<StorageType*>(num), size / sizeof(StorageType) * sizeof(T), m_sign);
		}
		else if constexpr (std::is_same_v<std::decay_t<T>, BigInt>)
		{
			StorageType* num;
			size_t size;
			multiplyArraysToNew(m_num, m_size, other.m_num, other.m_size, num, size);
			return BigInt(num, size, m_sign == other.m_sign ? POSITIVE : NEGATIVE);
		}
		return BigInt();
	}

	BigInt operator-() const {
		BigInt result(*this);
		result.m_sign = !m_sign;
		return result;
	}

	template<typename T>
	bool operator<(const T& other) noexcept
	{
		if constexpr (std::is_signed_v<T>)
			return false;
		else if constexpr (std::is_unsigned_v<T>)
			return false;
		else if constexpr (std::is_same_v<std::decay_t<T>, BigInt>)
			return compareMagnitutedsLess(m_num, m_size, other.m_num, other.m_size);
		return false;
	}

	friend std::ostream& operator<<(std::ostream& os, const BigInt& num) {

		if (num.m_size == 1 && num.m_num[0] == 0) {
			os << "0";
			return os;
		}

		if (num.m_sign == NEGATIVE)
			os << "-";

		// Convert to base 10 by repeatedly dividing by 10 and using remainders
		std::string base10 = "";
		StorageType* temp = new StorageType[num.m_size];
		std::memcpy(temp, num.m_num, num.m_size * sizeof(StorageType));
		size_t tempSize = num.m_size;

		while (tempSize > 1 || temp[0] != 0) {
			StorageType remainder = 0;
			for (size_t i = tempSize - 1; i != std::numeric_limits<size_t>::max(); i--) {
				StorageType current = temp[i] + (remainder << (sizeof(StorageType) * 8));
				temp[i] = current / 10;
				remainder = current % 10;
			}

			base10.insert(base10.begin(), '0' + remainder);

			// Adjust size if leading zeros appear
			while (tempSize > 0 && temp[tempSize - 1] == 0)
				tempSize--;
		}

		delete[] temp;

		os << base10;

		return os;
	}

	friend std::istream& operator>>(std::istream& is, BigInt& num) {
		delete[] num.m_num;
		num.m_size = 0;

		std::string base10;
		if (is >> base10) {
			num.initFromString(base10.c_str());
		}

		return is;
	}

private:

	template<typename T>
	void assignSigned(T&& other) noexcept
	{
		uint8_t* bytes = reinterpret_cast<uint8_t*>(&other);
		uint64_t size = sizeof(other);
		delete[] m_num;
		m_size = getNextPowerOf2(size);
		m_num = new StorageType[m_size];
		uint8_t* numBytes = reinterpret_cast<uint8_t*>(m_num);

		for (int i = 0; i < size; i++)
			numBytes[i] = bytes[i];
		if (numBytes[size - 1] >> 7)
			for (int i = size; i < m_size; i++)
				numBytes[i] = ~StorageType(0);
		else
			for (int i = size; i < m_size; i++)
				numBytes[i] = 0;
	}

	template<typename T>
	void assignUnsigned(T&& other) noexcept
	{
		uint8_t* bytes = reinterpret_cast<uint8_t*>(&other);
		uint64_t size = sizeof(other);
		m_size = getNextPowerOf2(size) * 2; //in case the unsigned value occupies the last bit
		delete[] m_num;
		m_num = new StorageType[m_size];
		uint8_t* numBytes = reinterpret_cast<uint8_t*>(m_num);

		for (int i = 0; i < size; i++)
			numBytes[i] = bytes[i];
		for (int i = size; i < m_size; i++)
			numBytes[i] = 0;
	}

	void grow(size_t size)
	{
		StorageType* newNum = new StorageType[size];
		for (size_t i = 0; i < m_size; i++)
			newNum[i] = m_num[i];
		for (int i = m_size; i < size; i++)
			newNum[i] = 0;
		delete[] m_num;
		m_num = newNum;
		m_size = size;
	}

	void initFromString(const char* str)
	{
		size_t size = strlen(str);
		size_t end;
		if (str[0] == '-')
		{
			m_sign = NEGATIVE;
			end = 0;
		}
		else
		{
			m_sign = POSITIVE;
			if (str[0] == '+')
				end = 0;
			else end = std::numeric_limits<size_t>::max();
		}
		m_num = new StorageType[1];
		m_size = 1;
		m_num[0] = 0;
		size_t mult = 1;
		for (size_t i = size - 1; i != end; i--, mult *= 10)
			*this = *this + BigInt(str[i] - '0') * mult;
	}

	friend BigInt operator""_bi(const char* str);
};

inline BigInt operator""_bi(const char* str) {
	return BigInt(str);
}