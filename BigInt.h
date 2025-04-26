#pragma once
#include <iostream>
#include <cstdlib>
#include <cstdint>
#include <type_traits>
#include <vector>
#include <string>

#include "Common.h"
#include "BigNumArithmetic.h"

class BigUint;
class BigFloat;

class BigInt
{
	friend class BigUint;
	friend class BigFloat;
public:
	using StorageType = uint64_t;
	static const size_t STORAGE_SIZE = sizeof(StorageType) * 8;
	static const bool POSITIVE = false;
	static const bool NEGATIVE = true;

private:
	std::vector<StorageType> m_num;
	bool m_sign = POSITIVE;

	BigInt(std::vector<StorageType>&& num, bool sign) noexcept : m_num(std::move(num)), m_sign(sign) {};
	BigInt(const char* str)
	{
		initFromString(str);
	}

public:

	BigInt() : m_num(), m_sign(POSITIVE) {};

	~BigInt() {};

	BigInt(const BigInt& other) noexcept : m_num(other.m_num.size()), m_sign(other.m_sign)
	{
		for (size_t i = 0; i < m_num.size(); i++)
			m_num[i] = other.m_num[i];
	}

	BigInt(BigInt&& other) noexcept : m_num(std::exchange(other.m_num, std::vector<StorageType>())), m_sign(other.m_sign) {};

	BigInt(const BigUint& other) noexcept;
	BigInt(BigUint&& other) noexcept;

	template<typename T>
	BigInt(T&& other) noexcept requires (!std::is_same_v<std::decay_t<T>, BigInt> &&
		std::is_unsigned_v<T> && (sizeof(T) <= sizeof(StorageType)))
	{
		m_sign = POSITIVE;
		if (other != 0)
			m_num.push_back(std::forward<T>(other));
	}

	template<typename T>
	BigInt(T&& other) noexcept requires (!std::is_same_v<std::decay_t<T>, BigInt>&&
		std::is_signed_v<T> && (sizeof(T) <= sizeof(StorageType)))
	{
		T num = std::forward<T>(other);
		if (other <= 0)
		{
			m_sign = NEGATIVE;
			num = -num;
		}
		else m_sign = POSITIVE;
		if (other != 0)
			m_num.push_back(std::move(num));
	}

	BigInt& operator=(const BigInt& other) noexcept
	{
		if (this != &other)
		{
			m_num.resize(other.m_num.size());
			m_sign = other.m_sign;
			for (size_t i = 0; i < m_num.size(); i++)
				m_num[i] = other.m_num[i];
		}
		return *this;
	}

	BigInt& operator=(BigInt&& other) noexcept
	{
		if (this != &other)
		{
			m_num = std::exchange(other.m_num, std::vector<StorageType>());
			m_sign = other.m_sign;
		}
		return *this;
	}

	BigInt& operator=(const BigUint& other) noexcept;
	BigInt& operator=(BigUint&& other) noexcept;

	template<typename T>
	BigInt& operator=(T&& other) noexcept requires (!std::is_same_v<std::decay_t<T>, BigInt>&&
		std::is_unsigned_v<T> && (sizeof(T) <= sizeof(StorageType)))
	{
		m_num.clear();
		m_sign = POSITIVE;
		if (other != 0)
			m_num.push_back(std::forward<T>(other));
		return *this;
	}

	template<typename T>
	BigInt& operator=(T&& other) noexcept requires (!std::is_same_v<std::decay_t<T>, BigInt>&&
		std::is_signed_v<T> && (sizeof(T) <= sizeof(StorageType)))
	{
		T num = std::forward<T>(other);
		if (other < 0)
		{
			m_sign = NEGATIVE;
			num = -num;
		}
		else m_sign = POSITIVE;
		if (other != 0)
			m_num.push_back(std::move(num));
		return *this;
	}

	void shrinkToFit() // will shrink by growth coefficient
	{
		m_num.shrink_to_fit();
	}

	BigInt operator+(const BigInt& other) const noexcept
	{
		if (m_sign == other.m_sign) {
			if (m_num.size() > other.m_num.size())
				return BigInt(add(m_num, other.m_num), m_sign);
			else return BigInt(add(other.m_num, m_num), m_sign);
		}
		else {
			if (compare(m_num, other.m_num) != CompareResult::LESS)
				return BigInt(subtract(m_num, other.m_num), m_sign);
			else return BigInt(subtract(other.m_num, m_num), other.m_sign);
		}
	}

	BigInt operator-(const BigInt& other) const noexcept
	{
		if (m_sign == other.m_sign) {
			if (compare(m_num, other.m_num) != CompareResult::LESS)
				return BigInt(subtract(m_num, other.m_num), m_sign);
			else return BigInt(subtract(other.m_num, m_num), !m_sign);
		}
		else {
			if (m_num.size() > other.m_num.size())
				return BigInt(add(m_num, other.m_num), m_sign);
			else return BigInt(add(other.m_num, m_num), m_sign);
		}
	}

	BigInt operator*(const BigInt& other) const noexcept
	{
		if (m_num.size() >= other.m_num.size())
			return BigInt(multiply(m_num, other.m_num), m_sign == other.m_sign ? POSITIVE : NEGATIVE);
		else return BigInt(multiply(other.m_num, m_num), m_sign == other.m_sign ? POSITIVE : NEGATIVE);
	}

	BigInt operator/(const BigInt& other) const
	{
		return BigInt(divide(m_num, other.m_num), m_sign == other.m_sign ? POSITIVE : NEGATIVE);
	}

	BigInt operator%(const BigInt& other) const
	{
		return BigInt(modulo(m_num, other.m_num), m_sign);
	}
	
	BigInt operator-() const {
		BigInt result(*this);
		result.m_sign = !m_sign;
		return result;
	}

	//naive placeholders
	BigInt& operator+=(const BigInt& other) {
		*this = *this + other;
		return *this;
	}

	BigInt& operator-=(const BigInt& other) {
		*this = *this - other;
		return *this;
	}

	BigInt& operator*=(const BigInt& other) {
		*this = *this * other;
		return *this;
	}

	BigInt& operator/=(const BigInt& other) {
		*this = *this / other;
		return *this;
	}

	BigInt& operator%=(const BigInt& other) {
		*this = *this % other;
		return *this;
	}

	bool operator<(const BigInt& other) const noexcept
	{
		return compare(m_num, other.m_num) == CompareResult::LESS;
	}

	bool operator>(const BigInt& other) const noexcept
	{
		return compare(m_num, other.m_num) == CompareResult::GREATER;
	}

	bool operator<=(const BigInt& other) const noexcept
	{
		return compare(m_num, other.m_num) != CompareResult::GREATER;
	}

	bool operator>=(const BigInt& other) const noexcept
	{
		return compare(m_num, other.m_num) != CompareResult::LESS;
	}

	bool operator!=(const BigInt& other) const noexcept
	{
		return compare(m_num, other.m_num) != CompareResult::EQUAL;
	}

	bool operator==(const BigInt& other) const noexcept
	{
		return compare(m_num, other.m_num) == CompareResult::EQUAL;
	}

	BigInt& operator>>(size_t shift) {
		if (shift > m_num.size() * STORAGE_SIZE)
		{
			m_num.clear();
			m_sign = POSITIVE;
			return *this;
		}

		size_t limbsToRemove = shift / STORAGE_SIZE;
		size_t bitsToRemove = shift % STORAGE_SIZE;
		StorageType removeMask = (StorageType(1) << bitsToRemove) - 1;

		m_num.erase(m_num.begin(), m_num.begin() + limbsToRemove);
		for (size_t i = 0; i < m_num.size() - 1; ++i)
		{
			m_num[i] >>= bitsToRemove;
			m_num[i] |= (m_num[i + 1] & removeMask) << (STORAGE_SIZE - bitsToRemove);
		}
		m_num[m_num.size() - 1] >>= bitsToRemove;
		if (m_num.back() == 0)
			m_num.pop_back();
		return *this;
	}

	BigInt& operator<<(size_t shift) {
		if (shift > m_num.size() * STORAGE_SIZE)
		{
			m_num.clear();
			m_sign = POSITIVE;
			return *this;
		}

		size_t limbsToAdd = shift / STORAGE_SIZE;
		size_t bitsToAdd = shift % STORAGE_SIZE;
		StorageType addMask = (StorageType(1) << bitsToAdd) - 1;

		m_num.insert(m_num.begin(), limbsToAdd, 0);
		if (bitsToAdd == 0)
			return *this;

		m_num.push_back(0);
		for (size_t i = m_num.size() - 1; i > limbsToAdd; --i)
		{
			m_num[i] <<= bitsToAdd;
			m_num[i] |= (m_num[i - 1] & addMask) >> (STORAGE_SIZE - bitsToAdd);
		}
		m_num[limbsToAdd] <<= bitsToAdd;
		if (m_num.back() == 0)
			m_num.pop_back();
		return *this;
	}

	friend std::ostream& operator<<(std::ostream& os, const BigInt& num) {

		if (num.m_num.size() == 0) {
			os << "0";
			return os;
		}

		if (num.m_sign == NEGATIVE)
			os << "-";

		std::string base10 = bigIntToString(num.m_num);
		os << base10;

		return os;
	}

	friend std::istream& operator>>(std::istream& is, BigInt& num) {

		std::string base10;
		if (is >> base10) {
			num.initFromString(base10.c_str());
		}

		return is;
	}

	void initFromString(const char* c_str)
	{
		size_t size = strlen(c_str);
		size_t start = 0;
		if(c_str[0] == '-')
		{
			start = 1;
			m_sign = NEGATIVE;
		}
		else 
		{
			if(c_str[0] == '+')
				start = 1;
			m_sign = POSITIVE;
		}
		std::string str(c_str + start);
		m_num = stringToBigInt<StorageType>(str);
	}

	bool isZero() const noexcept
	{
		return m_num.size() == 0;
	}

	bool isOdd() const noexcept
	{
		return m_num.size() != 0 && (m_num[0] & 1) == 1;
	}

	bool isEven() const noexcept
	{
		return m_num.size() == 0 || (m_num[0] & 1) == 0;
	}

	friend BigInt operator""_bi(const char* str);
	friend inline std::ostream& operator<<(std::ostream& os, const BigFloat& num);
	friend inline std::istream& operator>>(std::istream& is, BigFloat& num);
};

inline BigInt operator""_bi(const char* str) {
	return BigInt(str);
}