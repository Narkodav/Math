#pragma once
#include <iostream>
#include <cstdlib>
#include <cstdint>
#include <type_traits>
#include <concepts>

#include "Common.h"

class BigInt
{
public:
	using StorageType = uint64_t;
	static const size_t STORAGE_TYPE_BYTE_SIZE = sizeof(StorageType);
	static const size_t STORAGE_TYPE_BIT_SIZE = sizeof(StorageType) * 8;
	//static const size_t START_SIZE = 1;				// start size is 1 StorageType
	//static const size_t GROWTH_COEFFICIENT = 2;		// will grow twice the size

private:
	StorageType* m_num;
	size_t m_size;

public:

	BigInt() noexcept : m_num(new StorageType[1]), m_size(1)
	{
		m_num[0] = 0;
	};

	~BigInt() noexcept { delete[] m_num; };

	BigInt(const BigInt& other) noexcept
	{
		m_num = new StorageType[other.m_size];
		m_size = other.m_size;

		for (int i = 0; i < m_size; i++)
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
	BigInt(T&& other) requires !std::is_same_v<std::decay_t<T>, BigInt> &&
		std::is_arithmetic_v<T> && (sizeof(T) <= sizeof(StorageType))
		: m_num(new StorageType[1]), m_size(1)
	{
		//if constexpr (std::is_signed_v<T>)
		//	assignSigned(std::forward<T>(other));
		//else if constexpr (std::is_unsigned_v<T>)
		//	assignUnsigned(std::forward<T>(other));
		m_num = other;
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
		static_assert(std::is_arithmetic_v<T> || std::is_same_v<std::decay_t<T>, BigInt>, "Must be numeric type or BigInt!");

		if constexpr (std::is_signed_v<T>)
			return *this;
		else if constexpr (std::is_unsigned_v<T>)
			return *this;
		else if constexpr (std::is_same_v<std::decay_t<T>, BigInt>)
			addBigInt(other);
		return *this;
	}

	template<typename T>
	BigInt& operator-(const T& other) noexcept
	{
		static_assert(std::is_arithmetic_v<T> || std::is_same_v<std::decay_t<T>, BigInt>, "Must be numeric type or BigInt!");

		if constexpr (std::is_signed_v<T>)
			return *this;
		else if constexpr (std::is_unsigned_v<T>)
			return *this;
		else if constexpr (std::is_same_v<std::decay_t<T>, BigInt>)
			substractBigInt(other);
		return *this;
	}

	template<typename T>
	BigInt& operator<(const T& other) noexcept
	{
		static_assert(std::is_arithmetic_v<T> || std::is_same_v<std::decay_t<T>, BigInt>, "Must be numeric type or BigInt!");

		if constexpr (std::is_signed_v<T>)
			return *this;
		else if constexpr (std::is_unsigned_v<T>)
			return *this;
		else if constexpr (std::is_same_v<std::decay_t<T>, BigInt>)

		return *this;
	}

	friend std::ostream& operator<<(std::ostream& os, const BigInt& num) {

		//if (num.m_num[0] == 0)
		//	os << "0";
		//num.m_num[num.m_size - 1] >> (STORAGE_TYPE_BYTE_SIZE * 8 - 1);
		for (int i = num.m_size - 1; i >= 0; i--)
		{
			os << num.m_num[i] << " ";
		}

		return os;
	}

	friend std::istream& operator>>(std::istream& is, BigInt& num) {
		
	}

private:

	void addBigInt(const BigInt& other) noexcept {
		StorageType carry = 0;

		if (other.m_size > m_size)
			grow(other.m_size);

		// Get sign extension value for the smaller number
		StorageType lastBit = StorageType(1) << (STORAGE_TYPE_BYTE_SIZE * 8 - 1);
		StorageType signExtension = (other.m_num[other.m_size - 1] & lastBit) ? ~StorageType(0) : 0;

		// Add common parts
		size_t i = 0;
		for (; i < other.m_size; i++) {
			StorageType sum = m_num[i] + other.m_num[i] + carry;
			m_num[i] = sum;
			carry = sum >> (STORAGE_TYPE_BYTE_SIZE * 8 - 1);
		}

		// Continue with sign extension
		for (; i < m_size; i++) {
			StorageType sum = m_num[i] + signExtension + carry;
			m_num[i] = sum;
			carry = sum >> (STORAGE_TYPE_BYTE_SIZE * 8 - 1);
		}

		// Check for signed overflow
		bool thisSign = (m_num[m_size - 1] & lastBit) != 0;
		bool otherSign = (other.m_num[other.m_size - 1] & lastBit) != 0;
		bool resultSign = (m_num[m_size - 1] & lastBit) != 0;

		// If operands have same sign but result has different sign, we need to grow
		if (thisSign == otherSign && thisSign != resultSign) {
			grow(m_size * GROWTH_COEFFICIENT);
			m_num[m_size - 1] = thisSign ? ~StorageType(0) : 0;
		}
		else if (carry) {
			grow(m_size * GROWTH_COEFFICIENT);
			m_num[m_size - 1] = carry;
		}
	}

	void substractBigInt(const BigInt& other) noexcept {
		StorageType borrow = 0;
		StorageType otherVal;
		StorageType temp;

		if (other.m_size > m_size)
			grow(other.m_size);

		size_t i = 0;
		for (; i < other.m_size; i++) {
			temp = m_num[i];
			otherVal = other.m_num[i];

			m_num[i] = temp - otherVal - borrow;
			borrow = (m_num[i] > temp) || (temp < otherVal + borrow);
		}

		// Handle remaining borrow if any
		while (borrow && i < m_size) {
			temp = m_num[i];
			m_num[i] = temp - borrow;
			borrow = (temp == 0);
			i++;
		}
	}

	bool compareLessBigInt(const BigInt& other) const noexcept {
		// Get the sign bits (most significant bit of the last storage)
		bool thisNeg = m_num[m_size - 1] >> (STORAGE_TYPE_BIT_SIZE - 1);
		bool otherNeg = other.m_num[other.m_size - 1] >> (STORAGE_TYPE_BIT_SIZE - 1);

		// If signs differ, negative is less than positive
		if (thisNeg != otherNeg)
			return thisNeg;

		if (m_num[0] == m_num[m_size - 1])
		{
			if (other.m_num[0] == other.m_num[other.m_size - 1])
				return false; // Equal at this point
			return !thisNeg;
		}
		if (other.m_num[0] == other.m_num[other.m_size - 1])
			return thisNeg;

		size_t limit = std::min(m_size, other.m_size);

		for (int i = 1; i < limit - 1; i++)
		{
			if (m_num[i] == m_num[m_size - 1])
			{
				if (other.m_num[i] == other.m_num[other.m_size - 1])
					return thisNeg ? (m_num[i - 1] > other.m_num[i - 1]) : (m_num[i - 1] < other.m_num[i - 1]);
				return !thisNeg;
			}
			if (other.m_num[i] == other.m_num[other.m_size - 1])
				return thisNeg;
		}
		return thisNeg ? (m_num[limit - 1] > other.m_num[limit - 1]) : (m_num[limit - 1] < other.m_num[limit - 1]);
	}
	
	template<typename T>
	void assignSigned(T&& other) noexcept
	{
		uint8_t* bytes = reinterpret_cast<uint8_t*>(&other);
		uint64_t size = sizeof(other);
		uint64_t size64 = (size - 1) / 8 + 1;
		delete[] m_num;
		m_size = getNextPowerOf2(size64);
		m_num = new StorageType[m_size];
		uint8_t* numBytes = reinterpret_cast<uint8_t*>(m_num);

		for (int i = 0; i < size; i++)
		{
			numBytes[i] = bytes[i];
			std::cout << int(numBytes[i]) << " " << int(bytes[i]) << std::endl;
		}
		if(numBytes[size - 1] >> 7)
			for (int i = size; i < m_size * 8; i++)
				numBytes[i] = ~uint8_t(0);
		else
			for (int i = size; i < m_size * 8; i++)
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
		for (int i = size * 8; i < m_size * 8; i++)
			numBytes[i] = 0;
	}

	void grow()
	{
		StorageType* newNum = new StorageType[size];
		for (size_t i = 0; i < m_size; i++)
			newNum[i] = m_num[i];
		if (newNum[m_size - 1] >> STORAGE_TYPE_BIT_SIZE - 1)
			for (int i = m_size; i < size; i++)
				newNum[i] = ~uint8_t(0);
		else
			for (int i = m_size; i < size; i++)
				newNum[i] = 0;
		delete[] m_num;
		m_num = newNum;
		m_size = size;
	}
};