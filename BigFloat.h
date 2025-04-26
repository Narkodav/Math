#pragma once
#include <cmath>
#include "BigInt.h"

class BigFloat
{
public:
	using StorageType = BigInt::StorageType;
	static inline const size_t STORAGE_SIZE = BigInt::STORAGE_SIZE; //exponents are stored in base 2
	static inline const size_t base = 2; //exponents are stored in base 2
	// the scale by which the mantissa is expanded during division
	// can be adjusted depending on desired precision or performance
	static inline const float divisionPrecisionScale = 1.618; 
	static inline const float divisionPrecisionAddition = 10;
	static inline const double logBase_10 = std::log(10.0) / std::log(static_cast<double>(base));
	static inline const double log10_base = std::log(static_cast<double>(base)) / std::log(10.0);

private:
	BigInt m_mantissa;
	int64_t m_exponent = 0;

	BigFloat(BigInt&& mantissa, StorageType exponent) : m_mantissa(std::move(mantissa)), m_exponent(exponent) {
		size_t leadingZeros = 0;
		while (leadingZeros < m_mantissa.m_num.size() && m_mantissa.m_num[leadingZeros] == 0) {
			leadingZeros++;
		}

		if (leadingZeros > 0) {
			m_mantissa.m_num.erase(m_mantissa.m_num.begin(), m_mantissa.m_num.begin() + leadingZeros);
		}
		m_exponent += leadingZeros * STORAGE_SIZE;
	};

public:
	BigFloat() : m_mantissa(0), m_exponent(0) {};

	BigFloat(const char* str) {
		initFromString(str);
	};
	
	BigFloat operator+(const BigFloat& other) const
	{
		if (m_mantissa.isZero()) return other;
		if (other.m_mantissa.isZero()) return *this;

		if (m_exponent == other.m_exponent) {
			return BigFloat(m_mantissa + other.m_mantissa, m_exponent);
		}

		// Align to the larger exponent
		else if (m_exponent > other.m_exponent) {
			BigInt shiftedMantissa = other.m_mantissa;
			shiftedMantissa = shiftedMantissa << (m_exponent - other.m_exponent);
			return BigFloat(m_mantissa + shiftedMantissa, m_exponent);
		}
		else {
			BigInt shiftedMantissa = m_mantissa;
			shiftedMantissa = shiftedMantissa << (other.m_exponent - m_exponent);
			return BigFloat(other.m_mantissa + shiftedMantissa, other.m_exponent);
		}
	}

	BigFloat operator-() const {
		BigFloat result(*this);
		result.m_mantissa.m_sign = !result.m_mantissa.m_sign;
		return result;
	}

	BigFloat operator-(const BigFloat& other) const
	{
		if (m_mantissa.isZero()) return -other;
		if (other.m_mantissa.isZero()) return *this;

		if (m_exponent == other.m_exponent) {
			return BigFloat(m_mantissa - other.m_mantissa, m_exponent);
		}

		// Align to the larger exponent
		else if (m_exponent > other.m_exponent) {
			BigInt shiftedMantissa = other.m_mantissa;
			shiftedMantissa = shiftedMantissa << (m_exponent - other.m_exponent);
			return BigFloat(m_mantissa - shiftedMantissa, m_exponent);
		}
		else {
			BigInt shiftedMantissa = m_mantissa;
			shiftedMantissa = shiftedMantissa << (other.m_exponent - m_exponent);
			return BigFloat(shiftedMantissa - other.m_mantissa, other.m_exponent);
		}
	}

	BigFloat operator*(const BigFloat& other) const
	{
		if (m_mantissa.isZero() || other.m_mantissa.isZero()) return BigFloat();
		return BigFloat(m_mantissa * other.m_mantissa, m_exponent + other.m_exponent);
	}

	BigFloat operator/(const BigFloat& other) const
	{
		if (other.m_mantissa.isZero()) throw std::invalid_argument("Division by zero");
		if (m_mantissa.isZero()) return BigFloat();

		BigInt grownThisMantissa = m_mantissa;
		BigInt grownOtherMantissa = other.m_mantissa;

		// Ensure dividend is larger if needed
		if (m_mantissa.m_num.size() <= other.m_mantissa.m_num.size()) {
			growMantissa(grownThisMantissa, (1 + other.m_mantissa.m_num.size() - m_mantissa.m_num.size()));
		}

		// Apply precision scaling
		growMantissa(grownThisMantissa, divisionPrecisionAddition);
		growMantissa(grownOtherMantissa, divisionPrecisionAddition);

		int64_t grownThisExponent = m_exponent - (grownThisMantissa.m_num.size() - m_mantissa.m_num.size()) * STORAGE_SIZE;
		int64_t grownOtherExponent = other.m_exponent - (grownOtherMantissa.m_num.size() - other.m_mantissa.m_num.size()) * STORAGE_SIZE;

		return BigFloat(grownThisMantissa / grownOtherMantissa, grownThisExponent - grownOtherExponent);
	}

	void floor()
	{
		if (m_exponent < 0)
		{
			m_mantissa = m_mantissa >> -m_exponent;
			if(m_mantissa.m_sign == BigInt::NEGATIVE)
				m_mantissa = m_mantissa + BigInt(1);
		}
	}

	void ceiling()
	{
		if (m_exponent < 0)
		{
			size_t limit = std::max(-m_exponent / STORAGE_SIZE, m_mantissa.m_num.size() - 1);
			for(size_t i = 0; i < limit; ++i)
				if (m_mantissa.m_num[i] != 0)
				{
					m_mantissa = m_mantissa >> -m_exponent;
					if (m_mantissa.m_sign == BigInt::POSITIVE)
						m_mantissa = m_mantissa + BigInt(1);
					return;
				}

			if (m_mantissa.m_num[limit] << (STORAGE_SIZE - (-m_exponent - limit * STORAGE_SIZE)) != 0)
			{
				m_mantissa = m_mantissa >> -m_exponent;
				if (m_mantissa.m_sign == BigInt::POSITIVE)
					m_mantissa = m_mantissa + BigInt(1);
				return;
			}
			m_mantissa = m_mantissa >> -m_exponent;
		}
	}

	BigFloat operator%(const BigFloat& other) const {
		if (other.m_mantissa.isZero()) throw std::invalid_argument("Modulo by zero");
		if (m_mantissa.isZero()) return BigFloat();

		// First divide
		BigFloat quotient = *this / other;

		// Floor the quotient (remove fractional part)
		quotient.floor();

		return *this - (other * quotient);
	}

	friend inline std::ostream& operator<<(std::ostream& os, const BigFloat& num);
	friend inline std::istream& operator>>(std::istream& is, BigFloat& num);

	friend BigFloat operator""_bf(const char* str);

private:
	static void growMantissa(BigInt& mantissa, size_t growthAmount) {
		mantissa.m_num.insert(mantissa.m_num.begin(), growthAmount, 0);
	}

	void initFromString(const char* c_str)
	{
		size_t size = strlen(c_str);
		size_t start = 0;
		int64_t decimalBase10;
		if (c_str[0] == '-')
		{
			start = 1;
			m_mantissa.m_sign = BigInt::NEGATIVE;
		}
		else
		{
			if (c_str[0] == '+')
				start = 1;
			m_mantissa.m_sign = BigInt::POSITIVE;
		}
		std::string str(c_str + start);
		decimalBase10 = str.size() - 1;
		for (int i = str.size() - 1; i != start - 1; --i)
			if (str[i] == '.')
			{
				decimalBase10 = str.size() - 1 - i;
				str.erase(str.begin() + i);
				break;
			}
		if (decimalBase10 == str.size() - 1)
			m_exponent = 0;
		else
		{
			double decimalBase = -(double)decimalBase10 * logBase_10;
			m_exponent = decimalBase;
			if (decimalBase + 0.5 > m_exponent)
				m_exponent -= 1;
		}

		m_mantissa.m_num = stringToBigInt<StorageType>(str);

		size_t leadingZeros = 0;
		while (leadingZeros < m_mantissa.m_num.size() && m_mantissa.m_num[leadingZeros] == 0) {
			leadingZeros++;
		}

		if (leadingZeros > 0) {
			m_mantissa.m_num.erase(m_mantissa.m_num.begin(), m_mantissa.m_num.begin() + leadingZeros);
		}
		m_exponent += leadingZeros * STORAGE_SIZE;
	}
};

inline BigFloat operator""_bf(const char* str) {
	return BigFloat(str);
}

std::ostream& operator<<(std::ostream& os, const BigFloat& num) {

	if (num.m_mantissa.m_num.size() == 0) {
		os << "0";
		return os;
	}

	if (num.m_mantissa.m_sign == BigInt::NEGATIVE)
		os << "-";

	std::string base10 = bigIntToString(num.m_mantissa.m_num);
	if (num.m_exponent == 0)
	{
		os << base10;
		return os;
	}
	
	static const double reverseConversion = std::log(static_cast<double>(BigFloat::base) / std::log(10.0)); //log_10(base)
	double decimal = (double)num.m_exponent * BigFloat::log10_base;
	int64_t decimalInt = decimal;
	if (abs(decimal - (double)decimalInt) > 0.5)
	{
		if (decimalInt > 0)
			decimalInt++;
		else decimalInt--;
	}

	//base10.insert(base10.begin() + 1, '.');
	base10 += "e" + std::to_string(decimalInt);
	os << base10;

	return os;
}

std::istream& operator>>(std::istream& is, BigFloat& num) {

	std::string base10;
	if (is >> base10) {
		num.initFromString(base10.c_str());
	}

	return is;
}