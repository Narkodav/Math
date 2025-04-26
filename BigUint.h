#pragma once
#include <iostream>
#include <cstdlib>
#include <cstdint>
#include <type_traits>
#include <vector>
#include <string>

#include "Common.h"
#include "BigNumArithmetic.h"
#include "BigInt.h"

//should be faster than BigInt because of no sign checks, but throws if smaller - bigger
class BigUint
{
	friend class BigInt;
public:
	using StorageType = uint64_t;

private:
	std::vector<StorageType> m_num;

	BigUint(std::vector<StorageType>&& num) noexcept : m_num(std::move(num)) {};
	BigUint(const char* str)
	{
		initFromString(str);
	}

public:

	BigUint() : m_num() {};

	~BigUint() {};

	BigUint(const BigUint& other) noexcept : m_num(other.m_num.size())
	{
		for (size_t i = 0; i < m_num.size(); i++)
			m_num[i] = other.m_num[i];
	}

	BigUint(BigUint&& other) noexcept : m_num(std::exchange(other.m_num, std::vector<StorageType>())) {};

	BigUint(const BigInt& other) noexcept : m_num(other.m_num.size())
	{
		for (size_t i = 0; i < m_num.size(); i++)
			m_num[i] = other.m_num[i];
	}

	BigUint(BigInt&& other) noexcept : m_num(std::exchange(other.m_num, std::vector<StorageType>())) {};

	template<typename T>
	BigUint(T&& other) noexcept requires (!std::is_same_v<std::decay_t<T>, BigUint>&&
		std::is_unsigned_v<T> && (sizeof(T) <= sizeof(StorageType)))
	{
		m_num.push_back(std::forward<T>(other));
	}

	template<typename T>
	BigUint(T&& other) noexcept requires (!std::is_same_v<std::decay_t<T>, BigUint>&&
		std::is_signed_v<T> && (sizeof(T) <= sizeof(StorageType)))
	{
		T num = std::forward<T>(other);
		if (other < 0)
			num = -num;
		m_num.push_back(std::move(num));
	}

	BigUint& operator=(const BigUint& other) noexcept
	{
		if (this != &other)
		{
			m_num.resize(other.m_num.size());
			for (size_t i = 0; i < m_num.size(); i++)
				m_num[i] = other.m_num[i];
		}
		return *this;
	}

	BigUint& operator=(BigUint&& other) noexcept
	{
		if (this != &other)
			m_num = std::exchange(other.m_num, std::vector<StorageType>());
		return *this;
	}

	BigUint& operator=(const BigInt& other) noexcept
	{
		m_num.resize(other.m_num.size());
		for (size_t i = 0; i < m_num.size(); i++)
			m_num[i] = other.m_num[i];
		return *this;
	}

	BigUint& operator=(BigInt&& other) noexcept
	{
		m_num = std::exchange(other.m_num, std::vector<StorageType>());
		return *this;
	}

	template<typename T>
	BigUint& operator=(T&& other) noexcept requires (!std::is_same_v<std::decay_t<T>, BigUint>&&
		std::is_unsigned_v<T> && (sizeof(T) <= sizeof(StorageType)))
	{
		m_num.clear();
		m_num.push_back(std::forward<T>(other));
		return *this;
	}

	template<typename T>
	BigUint& operator=(T&& other) noexcept requires (!std::is_same_v<std::decay_t<T>, BigUint>&&
		std::is_signed_v<T> && (sizeof(T) <= sizeof(StorageType)))
	{
		T num = std::forward<T>(other);
		if (other < 0)
			num = -num;
		m_num.push_back(std::move(num));
		return *this;
	}

	void shrinkToFit() // will shrink by growth coefficient
	{
		m_num.shrink_to_fit();
	}

	BigUint operator+(const BigUint& other) const noexcept
	{
		if (m_num.size() > other.m_num.size())
			return BigUint(add(m_num, other.m_num));
		else return BigUint(add(other.m_num, m_num));
	}

	BigUint operator-(const BigUint& other) const
	{
		if (compare(m_num, other.m_num) != CompareResult::LESS)
			return BigUint(subtract(m_num, other.m_num));
		else throw std::underflow_error("Result of unsigned subtraction would be negative");
	}

	BigUint operator*(const BigUint& other) const noexcept
	{
		if (m_num.size() >= other.m_num.size())
			return BigUint(multiply(m_num, other.m_num));
		else return BigUint(multiply(other.m_num, m_num));
	}

	BigUint operator/(const BigUint& other) const
	{
		return BigUint(divide(m_num, other.m_num));
	}

	BigUint operator%(const BigUint& other) const
	{
		return BigUint(modulo(m_num, other.m_num));
	}

	BigInt operator-() const {
		BigInt result(*this);
		result.m_sign = BigInt::POSITIVE;
		return result;
	}

	bool operator<(const BigUint& other) const noexcept
	{
		return compare(m_num, other.m_num) == CompareResult::LESS;
	}

	bool operator>(const BigUint& other) const noexcept
	{
		return compare(m_num, other.m_num) == CompareResult::GREATER;
	}

	bool operator<=(const BigUint& other) const noexcept
	{
		return compare(m_num, other.m_num) != CompareResult::GREATER;
	}

	bool operator>=(const BigUint& other) const noexcept
	{
		return compare(m_num, other.m_num) != CompareResult::LESS;
	}

	bool operator!=(const BigUint& other) const noexcept
	{
		return compare(m_num, other.m_num) != CompareResult::EQUAL;
	}

	bool operator==(const BigUint& other) const noexcept
	{
		return compare(m_num, other.m_num) == CompareResult::EQUAL;
	}

	friend std::ostream& operator<<(std::ostream& os, const BigUint& num) {

		if (num.m_num.size() == 0) {
			os << "0";
			return os;
		}

		std::string base10 = bigIntToString(num.m_num);
		os << base10;

		return os;
	}

	friend std::istream& operator>>(std::istream& is, BigUint& num) {

		std::string base10;
		if (is >> base10) {
			num.initFromString(base10.c_str());
		}

		return is;
	}

	void initFromString(const char* c_str)
	{
		size_t size = strlen(c_str);
		size_t end;

		std::string str(c_str);
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

	friend BigUint operator""_bui(const char* str);
};

inline BigUint operator""_bui(const char* str) {
	return BigUint(str);
}