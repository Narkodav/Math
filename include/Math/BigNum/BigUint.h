#pragma once
#include <iostream>
#include <cstdlib>
#include <cstdint>
#include <type_traits>
#include <vector>
#include <string>
#include <utility>
#include <limits>

#include "Math/BigNum/BigNumArithmetic.h"

namespace Math::BigNum {
	class BigFloat;

	class BigUint
	{
		friend class BigFloat;
	public:
		using StorageType = uint64_t;
		static inline const size_t s_storageSize = sizeof(StorageType) * 8;

	private:
		std::vector<StorageType> m_num;

	public:

		BigUint() : m_num() {};

		BigUint(const std::string& str)
		{
			initFromString(str);
		}

		BigUint(std::vector<StorageType>&& num) noexcept : m_num(std::move(num)) {};

		~BigUint() {};

		BigUint(const BigUint& other) noexcept : m_num(other.m_num.size())
		{
			for (size_t i = 0; i < m_num.size(); i++)
				m_num[i] = other.m_num[i];
		}

		BigUint(BigUint&& other) noexcept : m_num(std::exchange(other.m_num, std::vector<StorageType>())) {};

		template<typename T>
		BigUint(T&& other) noexcept requires (std::is_unsigned_v<T> && (sizeof(T) <= sizeof(StorageType)))
		{
			if (other != 0)
				m_num.push_back(std::forward<T>(other));
		}

		template<typename T>
		BigUint(T&& other) noexcept requires (std::is_signed_v<T> && (sizeof(T) <= sizeof(StorageType)))
		{
			T num = std::forward<T>(other);
			if (other < 0)
				num = -num;
			if (other != 0)
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

		template<typename T>
		BigUint& operator=(T&& other) noexcept requires (std::is_unsigned_v<T> && (sizeof(T) <= sizeof(StorageType)))
		{
			m_num.clear();
			if (other != 0)
				m_num.push_back(std::forward<T>(other));
			return *this;
		}

		template<typename T>
		BigUint& operator=(T&& other) noexcept requires (std::is_signed_v<T> && (sizeof(T) <= sizeof(StorageType)))
		{
			T num = std::forward<T>(other);
			if (other < 0)
				num = -num;
			if (other != 0)
				m_num.push_back(std::move(num));
			return *this;
		}

		template<typename T>
		BigUint& operator=(T&& other) noexcept requires (std::is_unsigned_v<T> && (sizeof(T) > sizeof(StorageType)))
		{
			T num = std::forward<T>(other);
			m_num.clear();		
			if (num != 0)
			{
				m_num.reserve(sizeof(T) / sizeof(StorageType));
				while (num != 0)
				{
					m_num.push_back(num & (T)std::numeric_limits<StorageType>::max());
					num >>= s_storageSize;
				}
			}
			return *this;
		}

		template<typename T>
		BigUint& operator=(T&& other) noexcept requires (std::is_signed_v<T> && (sizeof(T) > sizeof(StorageType)))
		{
			T num = std::forward<T>(other);
			if (other < 0)
				num = -num;
			m_num.clear();
			if (other != 0)
			{
				m_num.reserve(sizeof(T) / sizeof(StorageType));
				while (num != 0)
				{
					m_num.push_back(num & (T)std::numeric_limits<StorageType>::max());
					num >>= s_storageSize;
				}
			}
			return *this;
		}

		BigUint operator+(const BigUint& other) const noexcept
		{
			if (m_num.size() > other.m_num.size())
				return BigUint(add(m_num, other.m_num));
			else return BigUint(add(other.m_num, m_num));
		}

		BigUint operator-(const BigUint& other) const noexcept
		{
			if (compare(m_num, other.m_num) == CompareResult::Less)
				return 0;
			return BigUint(subtract(m_num, other.m_num));
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

		//naive placeholders
		BigUint& operator+=(const BigUint& other) {
			*this = *this + other;
			return *this;
		}

		BigUint& operator-=(const BigUint& other) {
			*this = *this - other;
			return *this;
		}

		BigUint& operator*=(const BigUint& other) {
			*this = *this * other;
			return *this;
		}

		BigUint& operator/=(const BigUint& other) {
			*this = *this / other;
			return *this;
		}

		BigUint& operator%=(const BigUint& other) {
			*this = *this % other;
			return *this;
		}

		bool operator<(const BigUint& other) const noexcept
		{
			return compare(m_num, other.m_num) == CompareResult::Less;
		}

		bool operator>(const BigUint& other) const noexcept
		{
			return compare(m_num, other.m_num) == CompareResult::Greater;
		}

		bool operator<=(const BigUint& other) const noexcept
		{
			return compare(m_num, other.m_num) != CompareResult::Greater;
		}

		bool operator>=(const BigUint& other) const noexcept
		{
			return compare(m_num, other.m_num) != CompareResult::Less;
		}

		bool operator!=(const BigUint& other) const noexcept
		{
			return compare(m_num, other.m_num) != CompareResult::Equal;
		}

		bool operator==(const BigUint& other) const noexcept
		{
			return compare(m_num, other.m_num) == CompareResult::Equal;
		}

		BigUint& operator>>=(size_t shift) {
			if (shift > m_num.size() * s_storageSize)
			{
				m_num.clear();
				return *this;
			}

			size_t limbsToRemove = shift / s_storageSize;
			size_t bitsToRemove = shift % s_storageSize;
			StorageType removeMask = (StorageType(1) << bitsToRemove) - 1;

			m_num.erase(m_num.begin(), m_num.begin() + limbsToRemove);
			for (size_t i = 0; i < m_num.size() - 1; ++i)
			{
				m_num[i] >>= bitsToRemove;
				m_num[i] |= (m_num[i + 1] & removeMask) << (s_storageSize - bitsToRemove);
			}
			m_num[m_num.size() - 1] >>= bitsToRemove;
			if (m_num.back() == 0)
				m_num.pop_back();
			return *this;
		}

		BigUint& operator<<=(size_t shift) {
			if (shift == 0)
				return *this;
			size_t limbsToAdd = shift / s_storageSize;
			size_t bitsToAdd = shift % s_storageSize;
			size_t remainingBits = s_storageSize - bitsToAdd;
			StorageType addMask = ~((StorageType(1) << remainingBits) - 1);

			m_num.insert(m_num.begin(), limbsToAdd, 0);
			if (bitsToAdd == 0)
				return *this;

			m_num.push_back(0);
			for (size_t i = m_num.size() - 1; i > limbsToAdd; --i)
			{
				m_num[i] <<= bitsToAdd;
				m_num[i] |= (m_num[i - 1] & addMask) >> remainingBits;
			}
			m_num[limbsToAdd] <<= bitsToAdd;
			if (m_num.back() == 0)
				m_num.pop_back();
			return *this;
		}

		BigUint operator>>(size_t shift) const {
			BigUint newInt = *this;
			newInt >>= shift;
			return newInt;
		}

		BigUint operator<<(size_t shift) const {
			BigUint newInt = *this;
			newInt <<= shift;
			return newInt;
		}

		StorageType& operator[](size_t index) noexcept { return m_num[index]; };
		const StorageType& operator[](size_t index) const noexcept { return m_num[index]; };

		size_t size() const noexcept { return m_num.size(); };

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

		void initFromString(const std::string& str)
		{
			if (str[0] < '0' || str[0] > '9')
				throw std::invalid_argument("Init string for BigUint must only contain digits");
			if (str[0] == '0')
			{
				m_num.clear();
				return;
			}
			m_num = stringToBigInt<StorageType>(str);
		}

		std::string toString() const
		{
			std::string str = bigIntToString(m_num);
			return str;
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

		size_t bitLength() const noexcept
		{
			if (m_num.size() == 0)
				return 0;
			size_t result = (m_num.size() - 1) * s_storageSize;
			StorageType msb = m_num.back();
			while (msb != 0)
			{
				msb >>= 1;
				result++;
			}
			return result;
		}

		void eraseMSBs(size_t bitAmount) noexcept
		{
			StorageType msb = m_num.back();
			size_t lastAmount = 0;
			while (msb != 0)
			{
				msb >>= 1;
				lastAmount++;
			}

			if (bitAmount < lastAmount)
			{
				size_t shift = s_storageSize - lastAmount + bitAmount;
				m_num.back() <<= shift;
				m_num.back() >>= shift;
			}
			else if(bitAmount == lastAmount)
				m_num.pop_back();
			else
			{
				size_t limbsToErase = (bitAmount - lastAmount) / s_storageSize;
				size_t rightBitsToErase = (bitAmount - lastAmount) % s_storageSize;

				m_num.erase(m_num.end() - limbsToErase - 1, m_num.end());
				m_num.back() <<= rightBitsToErase;
				m_num.back() >>= rightBitsToErase;
			}	
		}

		std::vector<StorageType>& raw() noexcept { return m_num; };

		const std::vector<StorageType>& raw() const noexcept { return m_num; };

		std::string toBinary() const
		{
			std::string binary;
			StorageType msbMask = StorageType(1) << (s_storageSize - 1);
			for (size_t i = m_num.size() - 1; i != std::numeric_limits<size_t>::max(); i--)
			{
				for (size_t j = 0; j < s_storageSize; j++)
					binary.push_back((m_num[i] << j) & msbMask ? '1' : '0');
			}

			size_t toErase;

			for (toErase = 0; toErase < binary.size(); toErase++)
			{
				if (binary[toErase] != '0')
					break;
			}
			binary.erase(0, toErase);
			return binary;
		}

		friend BigUint operator""_bui(const char* str);
	};

	BigUint power(BigUint base, BigUint exponent)
	{
		if (exponent < BigUint::StorageType(0))
			throw std::invalid_argument("Negative exponent is not supported on BigUints, use BigFloats instead");
		if (exponent == BigUint::StorageType(0)) {
			return BigUint(BigUint::StorageType(1));
		}
		if (exponent == BigUint::StorageType(0)) return BigUint(BigUint::StorageType(1));
		if (base == BigUint::StorageType(0) || base == BigUint::StorageType(1)) return base;

		BigUint result = BigUint::StorageType(1);

		while (exponent.raw().size() != 0) {
			if (exponent.raw().front() & 1) {
				result = result * base;  // Multiply if bit is set
			}
			base = base * base;  // Square the base
			exponent >>= 1;       // Move to next bit
		}

		return result;
	}
}

inline Math::BigNum::BigUint operator""_bui(const char* str) {
	return Math::BigNum::BigUint(std::string(str));
}