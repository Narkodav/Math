#pragma once
#include <iostream>
#include <cstdlib>
#include <cstdint>
#include <type_traits>
#include <vector>
#include <string>

//#include "Common.h"
#include "BigNum/BigNumArithmetic.h"
#include "BigNum/BigUint.h"

namespace Math::BigNum {
	class BigFloat;

	class BigInt
	{
		friend class BigUint;
		friend class BigFloat;
	public:
		using StorageType = BigUint::StorageType;
		static const size_t STORAGE_SIZE = sizeof(StorageType) * 8;
		static const bool POSITIVE = false;
		static const bool NEGATIVE = true;

	private:
		BigUint m_num;
		bool m_sign = POSITIVE;	

	public:

		BigInt() : m_num(), m_sign(POSITIVE) {};

		BigInt(const std::string& str)
		{
			initFromString(str);
		}

		BigInt(std::vector<StorageType>&& num, bool sign) noexcept : m_num(std::move(num)), m_sign(sign) {};

		~BigInt() {};

		BigInt(const BigInt& other) noexcept : m_num(other.m_num), m_sign(other.m_sign) {};

		BigInt(BigInt&& other) noexcept : m_num(std::exchange(other.m_num, BigUint())), m_sign(other.m_sign) {};

		BigInt(const BigUint& other, bool sign = POSITIVE) noexcept;
		BigInt(BigUint&& other, bool sign = POSITIVE) noexcept;

		template<typename T>
		BigInt(T&& other) noexcept requires (std::is_unsigned_v<T>)
		{
			m_sign = POSITIVE;
			m_num = std::forward<T>(other);
		}

		template<typename T>
		BigInt(T&& other) noexcept requires (std::is_signed_v<T>)
		{
			if (other < 0)
				m_sign = NEGATIVE;
			else m_sign = POSITIVE;
			m_num = std::forward<T>(other);
		}

		BigInt& operator=(const BigInt& other) noexcept
		{
			if (this != &other)
			{
				m_num = other.m_num;
				m_sign = other.m_sign;
			}
			return *this;
		}

		BigInt& operator=(BigInt&& other) noexcept
		{
			if (this != &other)
			{
				m_num = std::exchange(other.m_num, BigUint());
				m_sign = other.m_sign;
			}
			return *this;
		}

		BigInt& operator=(const BigUint& other) noexcept;
		BigInt& operator=(BigUint&& other) noexcept;

		template<typename T>
		BigInt& operator=(T&& other) noexcept requires (std::is_unsigned_v<T> && (sizeof(T) <= sizeof(StorageType)))
		{
			m_num.raw().clear();
			m_sign = POSITIVE;
			m_num = std::forward<T>(other);
			return *this;
		}

		template<typename T>
		BigInt& operator=(T&& other) noexcept requires (std::is_signed_v<T> && (sizeof(T) <= sizeof(StorageType)))
		{
			if (other < 0)
				m_sign = NEGATIVE;
			else m_sign = POSITIVE;
			m_num = std::forward<T>(other);
			return *this;
		}

		BigInt operator+(const BigInt& other) const noexcept
		{
			if (m_sign == other.m_sign) {
				return BigInt(m_num + other.m_num, m_sign);
			}
			else {
				if (m_num >= other.m_num)
					return BigInt(m_num - other.m_num, m_sign);
				else return BigInt(other.m_num - m_num, other.m_sign);
			}
		}

		BigInt operator-(const BigInt& other) const noexcept
		{
			if (m_sign == other.m_sign) {
				if (m_num >= other.m_num)
					return BigInt(m_num - other.m_num, m_sign);
				else return BigInt(other.m_num - m_num, !m_sign);
			}
			else {
				return BigInt(m_num + other.m_num, m_sign);
			}
		}

		BigInt operator*(const BigInt& other) const noexcept
		{
			return BigInt(m_num * other.m_num, m_sign == other.m_sign ? POSITIVE : NEGATIVE);
		}

		BigInt operator/(const BigInt& other) const
		{
			return BigInt(m_num / other.m_num, m_sign == other.m_sign ? POSITIVE : NEGATIVE);
		}

		BigInt operator%(const BigInt& other) const
		{
			return BigInt(m_num % other.m_num, m_sign);
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
			if (m_sign == NEGATIVE && other.m_sign == POSITIVE)
				return true;
			else if (m_sign != other.m_sign)
				return false;
			if (m_sign == POSITIVE)
				return m_num < other.m_num;
			else return m_num > other.m_num;
		}

		bool operator>(const BigInt& other) const noexcept
		{
			if (m_sign == POSITIVE && other.m_sign == NEGATIVE)
				return true;
			else if (m_sign != other.m_sign)
				return false;
			if (m_sign == POSITIVE)
				return m_num > other.m_num;
			else return m_num < other.m_num;
		}

		bool operator<=(const BigInt& other) const noexcept
		{
			if (m_sign == NEGATIVE && other.m_sign == POSITIVE)
				return true;
			else if (m_sign != other.m_sign)
				return false;
			if (m_sign == POSITIVE)
				return m_num <= other.m_num;
			else return m_num >= other.m_num;
		}

		bool operator>=(const BigInt& other) const noexcept
		{
			if (m_sign == POSITIVE && other.m_sign == NEGATIVE)
				return true;
			else if (m_sign != other.m_sign)
				return false;
			if (m_sign == POSITIVE)
				return m_num >= other.m_num;
			else return m_num <= other.m_num;
		}

		bool operator!=(const BigInt& other) const noexcept
		{
			return m_sign != other.m_sign || m_num != other.m_num;
		}

		bool operator==(const BigInt& other) const noexcept
		{
			return m_sign == other.m_sign && m_num == other.m_num;
		}

		BigInt& operator>>=(size_t shift) {
			m_num >>= shift;
			return *this;
		}

		BigInt& operator<<=(size_t shift) {
			m_num <<= shift;
			return *this;
		}

		BigInt operator>>(size_t shift) const {
			BigInt newInt = *this;
			newInt >>= shift;
			return newInt;
		}

		BigInt operator<<(size_t shift) const {
			BigInt newInt = *this;
			newInt <<= shift;
			return newInt;
		}
		
		void initFromString(const std::string& str)
		{
			size_t start = 0;
			if(str[0] == '-')
			{
				start = 1;
				m_sign = NEGATIVE;
			}
			else 
			{
				if(str[0] == '+')
					start = 1;
				m_sign = POSITIVE;
			}
			if(str[start] == '0')
			{
				m_sign = POSITIVE;
				m_num = StorageType(0);
				return;
			}
			m_num.initFromString(str.substr(start));
		}

		std::string toString() const
		{
			std::string str = m_num.toString();
			if (m_sign == NEGATIVE)
				str = "-" + str;
			return str;
		}

		bool isZero() const noexcept
		{
			return m_num.isZero();
		}

		bool isOdd() const noexcept
		{
			return m_num.isOdd();
		}

		bool isEven() const noexcept
		{
			return m_num.isEven();
		}

		size_t bitLength() const noexcept
		{
			return m_num.bitLength();
		}

		void eraseMSBs(size_t bitAmount) noexcept;

		std::vector<StorageType>& raw() noexcept { return m_num.raw(); };

		const std::vector<StorageType>& raw() const noexcept { return m_num.raw(); };

		BigUint& magnitude() noexcept { return m_num; };

		const BigUint& magnitude() const noexcept { return m_num; };

		std::string toBinary() const;

		friend BigInt operator""_bi(const char* str);
		friend inline std::ostream& operator<<(std::ostream& os, const BigFloat& num);
		friend inline std::istream& operator>>(std::istream& is, BigFloat& num);

		friend BigInt power(BigInt base, BigInt exponent);

		friend std::ostream& operator<<(std::ostream& os, const BigInt& num) {

			if (num.m_num.size() == 0) {
				os << "0";
				return os;
			}

			std::string base10 = num.toString();
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
	};
}

inline Math::BigNum::BigInt operator""_bi(const char* str) {
	return Math::BigNum::BigInt(std::string(str));
}