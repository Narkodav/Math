#pragma once
#include <iostream>
#include <cstdlib>
#include <cstdint>
#include <type_traits>
#include <concepts>

#include "Common.h"

template<typename T>
	requires std::is_arithmetic_v<T>
void addArraysToThis(T*& thisArray, size_t& thisSize, T* otherArray, size_t otherSize)
{
	if (thisSize < otherSize || thisArray[thisSize - 1] != 0 &&
		thisArray[thisSize - 1] != std::numeric_limits<T>::max() ||
		otherSize == thisSize && otherArray[otherSize - 1] != 0 &&
		thisArray[otherSize - 1] != std::numeric_limits<T>::max())
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

	if ((otherArray[otherSize - 1] >> (std::numeric_limits<T>::digits - 1)) == 1)
	{
		for (size_t i = otherSize; i < thisSize; i++)
		{
			thisArray[i] = thisArray[i] + std::numeric_limits<T>::max() + leftovers;
			leftovers = 1;
		}
	}
	else
	{
		for (size_t i = otherSize; i < thisSize; i++)
		{
			T buffer = thisArray[i];
			thisArray[i] = thisArray[i] + leftovers;
			leftovers = std::numeric_limits<T>::max() - buffer > leftovers;
		}
	}

	// we can ignore leftovers, as they are ignored in normal numerical operation in this case
	// for example in 4 bit numbers -6+7 = 1010+0111=10001, last 1 is ignored, 0001 = 1
	// we dont need to fill the end of thisArray because it was already filled, since it stored a valid number already
}

template<typename T>
	requires std::is_arithmetic_v<T>
void addArraysToNew(T* thisArray, size_t thisSize, T* otherArray, size_t otherSize, T*& newArray, size_t& newSize) //assumes thisSize >= otherSize
{
	if (thisArray[thisSize - 1] != 0 && thisArray[thisSize - 1] != std::numeric_limits<T>::max() ||
		otherSize == thisSize && otherArray[otherSize - 1] != 0 && otherArray[otherSize - 1] != std::numeric_limits<T>::max())
	{
		newSize = thisSize * 2;
	}
	else newSize = thisSize;
	newArray = new T[newSize];
	T leftovers = 0;

	for (size_t i = 0; i < otherSize; i++)
	{
		newArray[i] = thisArray[i] + otherArray[i] + leftovers;
		leftovers = std::numeric_limits<T>::max() - thisArray[i] < otherArray[i];
	}

	if ((otherArray[otherSize - 1] >> (std::numeric_limits<T>::digits - 1)) == 1)
	{
		for (size_t i = otherSize; i < thisSize; i++)
		{
			newArray[i] = thisArray[i] + std::numeric_limits<T>::max() + leftovers;
			leftovers = 1;
		}
	}
	else
	{
		for (size_t i = otherSize; i < thisSize; i++)
		{
			newArray[i] = thisArray[i] + leftovers;
			leftovers = std::numeric_limits<T>::max() - thisArray[i] > leftovers;
		}
	}

	// we can ignore leftovers, as they are ignored in normal numerical operation in this case
	// for example in 4 bit numbers -6+7 = 1010+0111=10001, last 1 is ignored, 0001 = 1 overflow causes a sign change to positive
	T value = (newArray[thisSize - 1] >> (std::numeric_limits<T>::digits - 1)) == 1 ? std::numeric_limits<T>::max() : 0;

	for (size_t i = thisSize; i < newSize; i++)
		newArray[i] = value;
}

//template<typename T>
//	requires std::is_arithmetic_v<T>
//void copyArray(T* thisArray, size_t thisSize, T* otherArray, size_t otherSize) //assumes thisSize >= otherSize
//{
//	if (thisArray[thisSize - 1] != 0 && thisArray[thisSize - 1] != std::numeric_limits<T>::max() ||
//		otherSize == thisSize && otherArray[otherSize - 1] != 0 && otherArray[otherSize - 1] != std::numeric_limits<T>::max())
//	{
//		newSize = thisSize * 2;
//	}
//	else newSize = thisSize;
//	newArray = new T[newSize];
//	T leftovers = 0;
//
//	for (size_t i = 0; i < otherSize; i++)
//	{
//		newArray[i] = thisArray[i] + otherArray[i] + leftovers;
//		leftovers = std::numeric_limits<T>::max() - thisArray[i] < otherArray[i];
//	}
//
//	if ((otherArray[otherSize - 1] >> (std::numeric_limits<T>::digits - 1)) == 1)
//	{
//		for (size_t i = otherSize; i < thisSize; i++)
//		{
//			newArray[i] = thisArray[i] + std::numeric_limits<T>::max() + leftovers;
//			leftovers = 1;
//		}
//	}
//	else
//	{
//		for (size_t i = otherSize; i < thisSize; i++)
//		{
//			newArray[i] = thisArray[i] + leftovers;
//			leftovers = std::numeric_limits<T>::max() - thisArray[i] > leftovers;
//		}
//	}
//
//	// we can ignore leftovers, as they are ignored in normal numerical operation in this case
//	// for example in 4 bit numbers -6+7 = 1010+0111=10001, last 1 is ignored, 0001 = 1 overflow causes a sign change to positive
//	T value = (newArray[thisSize - 1] >> (std::numeric_limits<T>::digits - 1)) == 1 ? std::numeric_limits<T>::max() : 0;
//
//	for (size_t i = thisSize; i < newSize; i++)
//		newArray[i] = value;
//}

class BigInt
{
public:
	using StorageType = uint64_t;
	static const size_t STORAGE_TYPE_BYTE_SIZE = sizeof(StorageType);
	static const size_t STORAGE_TYPE_BIT_SIZE = sizeof(StorageType) * 8;
	static const size_t START_SIZE = 1;				// start size is 1 StorageType
	static const size_t GROWTH_COEFFICIENT = 2;		// will grow twice the size

private:
	StorageType* m_num;
	size_t m_size;

	BigInt(StorageType* num, size_t size) noexcept : m_num(num), m_size(size) {};

public:

	BigInt() noexcept : m_num(new StorageType[START_SIZE]), m_size(START_SIZE)
	{
		for (size_t i = 0; i < m_size; i++)
			m_num[i] = 0;
	};

	~BigInt() noexcept { delete[] m_num; };

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
	BigInt(T&& other) requires (!std::is_same_v<std::decay_t<T>, BigInt>&&
		std::is_arithmetic_v<T> && (sizeof(T) <= sizeof(StorageType)))
	{
		static_assert(std::is_arithmetic_v<T>, "Must be numeric type!");

		if constexpr (std::is_unsigned_v<T>)
		{
			if ((other >> (std::numeric_limits<T>::digits - 1)) == 1)
			{
				m_num = new StorageType[2];
				m_size = 2;
			}
			else
			{
				m_num = new StorageType[1];
				m_size = 1;
			}
			m_num[0] = std::forward<T>(other);
			for (size_t i = 1; i < m_size; i++)
				m_num[i] = 0;
		}
		else
		{
			m_num = new StorageType[1];
			m_size = 1;
			m_num[0] = std::forward<T>(other);
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

		if constexpr (std::is_unsigned_v<T>)
		{
			if (m_size == 1 && (other >> (std::numeric_limits<T>::digits - 1)) == 1)
			{
				size_t newSize = m_size * 2;
				StorageType* newNum = new StorageType[newSize];
				delete[] m_num;
				m_num = newNum;
				m_size = newSize;
			}
			m_num[0] = std::forward<T>(other);
			for (size_t i = 1; i < m_size; i++)
				m_num[i] = 0;
		}
		else
		{
			m_num[0] = std::forward<T>(other);
			StorageType value = (m_num[0] >> (std::numeric_limits<StorageType>::digits - 1)) == 1 ?
				std::numeric_limits<StorageType>::max() : 0;
			for (size_t i = 1; i < m_size; i++)
				m_num[i] = value;
		}
		return *this;
	}

	template<typename T>
	BigInt operator+(const T& other) noexcept
	{
		static_assert(std::is_arithmetic_v<T> || std::is_same_v<std::decay_t<T>, BigInt>, "Must be numeric type or BigInt!");

		if constexpr (std::is_signed_v<T>)
			return BigInt();
		else if constexpr (std::is_unsigned_v<T>)
			return BigInt();
		else if constexpr (std::is_same_v<std::decay_t<T>, BigInt>)
		{
			StorageType* newArray;
			size_t newSize;
			if(m_size < other.m_size)
				addArraysToNew(other.m_num, other.m_size, m_num, m_size, newArray, newSize);
			else addArraysToNew(m_num, m_size, other.m_num, other.m_size, newArray, newSize);
			
			return BigInt(newArray, newSize);
		}
			
	}

	template<typename T>
	BigInt operator-(const T& other) noexcept
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

	//void substractBigInt(const BigInt& other) noexcept {
	//	StorageType borrow = 0;
	//	StorageType otherVal;
	//	StorageType temp;

	//	if (other.m_size > m_size)
	//		grow(other.m_size);

	//	size_t i = 0;
	//	for (; i < other.m_size; i++) {
	//		temp = m_num[i];
	//		otherVal = other.m_num[i];

	//		m_num[i] = temp - otherVal - borrow;
	//		borrow = (m_num[i] > temp) || (temp < otherVal + borrow);
	//	}

	//	// Handle remaining borrow if any
	//	while (borrow && i < m_size) {
	//		temp = m_num[i];
	//		m_num[i] = temp - borrow;
	//		borrow = (temp == 0);
	//		i++;
	//	}
	//}

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
};

