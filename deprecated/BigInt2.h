#pragma once
#include <iostream>
#include <cstdlib>
#include <cstdint>
#include <type_traits>

#include "Common.h"

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

public:

	BigInt() : m_num(new StorageType[START_SIZE]), m_size(START_SIZE)
	{
		for (int i = 0; i < START_SIZE; i++)
			m_num[i] = 0;
	};

	~BigInt() { delete[] m_num; };

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
	BigInt& operator=(T&& other) noexcept
	{
		static_assert(std::is_arithmetic_v<T>, "Must be numeric type!");

		if constexpr (std::is_signed_v<T>)
			assignSigned(std::forward<T>(other));
		else if constexpr (std::is_unsigned_v<T>)
			assignUnsigned(std::forward<T>(other));

		return *this;
	}

	template<typename T>
	BigInt& operator+(const T& other) noexcept
	{
		static_assert(std::is_arithmetic_v<T>, "Must be numeric type!");

		if constexpr (std::is_signed_v<T>)

		else if constexpr (std::is_unsigned_v<T>)

		else if constexpr (std::is_same_v<std::decay_t<T>, BigInt>)
			if (m_negative == other.m_negative) {
				// Same signs: just add
				addBigInt(other);
			}
			else {
				// Different signs: effectively subtraction
				subtractBigInt(other);
			}
		return *this;
	}

	template<typename T>
	BigInt& operator<(const T& other) noexcept
	{
		static_assert(std::is_arithmetic_v<T>, "Must be numeric type!");

		if constexpr (std::is_signed_v<T>)

		else if constexpr (std::is_unsigned_v<T>)

		else if constexpr (std::is_same_v<std::decay_t<T>, BigInt>)
			if (m_negative == other.m_negative) {
				return compareLessBigInt(other) ^ m_negative;
			}
			else {
				return m_negative;
			}
		return *this;
	}

private:

	void addBigInt(const BigInt& other) noexcept {
		if (other.m_size > m_size)
			grow(other.m_size);

		StorageType carry = 0;
		size_t i = 0;
		for (; i < other.m_size; i++) {
			StorageType sum = m_num[i] + other.m_num[i] + carry;
			m_num[i] = sum;
			carry = sum >> (STORAGE_TYPE_BYTE_SIZE * 8 - 1);
		}
		for (; i < m_size && carry; i++) {
			StorageType sum = m_num[i] + carry;
			m_num[i] = sum;
			carry = sum >> (STORAGE_TYPE_BYTE_SIZE * 8 - 1);
		}
		if (carry)
			grow(m_size * GROWTH_COEFFICIENT);
	}

	// assumes this is bigger
	void substractBigInt(const BigInt& other) noexcept {
		if (other.m_size > m_size)
			grow(other.m_size);

		StorageType borrow = 0;
		size_t i = 0;
		for (; i < other.m_size; i++) {
			StorageType diff;
			if (m_num[i] >= other.m_num[i] + borrow) {
				diff = m_num[i] - other.m_num[i] - borrow;
				borrow = 0;
			}
			else {
				diff = (std::numeric_limits<StorageType>::max() - other.m_num[i] - borrow + 1) + m_num[i];
				borrow = 1;
			}
			m_num[i] = diff;
		}

		// Propagate remaining borrow if any
		while (i < m_size && borrow) {
			if (m_num[i] >= borrow) {
				m_num[i] -= borrow;
				borrow = 0;
			}
			else {
				m_num[i] = std::numeric_limits<StorageType>::max();
				borrow = 1;
			}
			i++;
		}
	}

	bool compareLessBigInt(const BigInt& other) const noexcept {
		size_t limit;
		if (m_size > other.m_size)
		{
			if (m_num[other.m_size] != 0)
				return false;
			limit = other.m_size;
		}
		else if (m_size < other.m_size)
		{
			if (other.m_num[m_size] != 0)
				return true;
			limit = m_size;
		}

		// Compare common digits from most significant
		for (size_t i = limit - 1; i != std::numeric_limits<size_t>::max(); i--) {
			if (m_num[i] < other.m_num[i]) return true;
			if (m_num[i] > other.m_num[i]) return false;
		}

		return false; // Equal numbers
	}
	
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
		if(numBytes[size - 1] >> 7)
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
};

class BigInt
{
};
