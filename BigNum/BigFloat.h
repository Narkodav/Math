#pragma once
#include <cmath>
#include <cassert>

#include "BigInt.h"
#include "BigFloatArithmetic.h"

class BigFloat
{
public:
	using StorageType = BigInt::StorageType;
	static inline const size_t STORAGE_SIZE = BigInt::STORAGE_SIZE; //exponents are stored in base 2
	static inline const size_t base = 2; //exponents are stored in base 2
	// the scale by which the mantissa is expanded during division
	// can be adjusted depending on desired precision or performance
	static inline const float divisionPrecisionScale = 1.618;
	static inline const float divisionPrecisionAddition = 4 * STORAGE_SIZE; //2 limbs
	static inline const float additionalPrecision = 4 * STORAGE_SIZE; //4 limbs
	
	static inline const double logBase_10 = std::log(10.0) / std::log(static_cast<double>(base));
	static inline const double log10_base = std::log(static_cast<double>(base)) / std::log(10.0);

	static inline const size_t repeatingDigitCutoffLimit = 10; //how many repeating digits needs to happen for a cutoff
	static inline const size_t decimalFormatPrescision = 20; //display 20 digits of significant

private:
	BigInt m_mantissa;
	int64_t m_exponent = 0;

	//BigFloat(std::vector<StorageType>&& mantissa, bool sign, StorageType exponent) : 
	//	m_mantissa(std::move(mantissa), sign), m_exponent(exponent) {
	//	clearTailingZeros();
	//};

	BigFloat(BigInt&& mantissa, StorageType exponent) : m_mantissa(std::move(mantissa)), m_exponent(exponent) {
		clearTailingZeros();
	};

public:
	BigFloat() : m_mantissa(0), m_exponent(0) {};

	BigFloat(const std::string& str) {
		initFromString(str);
	};

	BigFloat(BigFloat&&) = default;
	BigFloat(const BigFloat&) = default;

	BigFloat& operator=(BigFloat&&) = default;
	BigFloat& operator=(const BigFloat&) = default;

	BigFloat(BigInt&& other) {
		m_mantissa = std::move(other);
		m_exponent = m_mantissa.bitLength() - 1;
		clearTailingZeros();
	}
	BigFloat(const BigInt& other)
	{
		m_mantissa = other;		
		m_exponent = m_mantissa.bitLength() - 1;
		clearTailingZeros();
	}

	BigFloat& operator=(BigInt&& other)
	{
		m_mantissa = std::move(other);
		m_exponent = m_mantissa.bitLength() - 1;
		clearTailingZeros();
		return *this;
	}

	BigFloat& operator=(const BigInt& other)
	{
		m_mantissa = other;
		m_exponent = m_mantissa.bitLength() - 1;
		clearTailingZeros();
		return *this;
	}
	
	BigFloat operator+(const BigFloat& other) const
	{
		if (m_mantissa.isZero()) return other;
		if (other.m_mantissa.isZero()) return *this;

		size_t bitLengthThis = m_mantissa.bitLength();
		size_t bitLengthOther = other.m_mantissa.bitLength();

		if(bitLengthThis > bitLengthOther)
		{
			BigInt shiftedOther = other.m_mantissa << bitLengthThis - bitLengthOther;
			if (m_exponent >= other.m_exponent)
			{
				size_t shift = m_exponent - other.m_exponent;
				BigInt shiftedThis = m_mantissa << shift;
				bitLengthThis += shift;
				shiftedOther = shiftedThis + shiftedOther;
				return BigFloat(std::move(shiftedOther), m_exponent + (int64_t)shiftedOther.bitLength() - (int64_t)bitLengthThis);
			}
			else
			{
				size_t shift = other.m_exponent - m_exponent;
				shiftedOther = shiftedOther << shift;
				bitLengthOther = bitLengthThis + shift;
				shiftedOther = m_mantissa + shiftedOther;
				return BigFloat(std::move(shiftedOther), other.m_exponent + (int64_t)shiftedOther.bitLength() - (int64_t)bitLengthOther);
			}
		}
		else
		{
			BigInt shiftedThis = m_mantissa << bitLengthOther - bitLengthThis;
			if (m_exponent >= other.m_exponent)
			{
				size_t shift = m_exponent - other.m_exponent;
				shiftedThis = shiftedThis << shift;
				bitLengthThis = bitLengthOther + shift;
				shiftedThis = shiftedThis + other.m_mantissa;
				return BigFloat(std::move(shiftedThis), m_exponent + (int64_t)shiftedThis.bitLength() - (int64_t)bitLengthThis);
			}
			else
			{
				size_t shift = other.m_exponent - m_exponent;
				BigInt shiftedOther = other.m_mantissa << shift;
				bitLengthOther += shift;
				shiftedThis = shiftedThis + shiftedOther;
				return BigFloat(std::move(shiftedThis), other.m_exponent + (int64_t)shiftedThis.bitLength() - (int64_t)bitLengthOther);
			}
		}
	}

	BigFloat operator-() const {
		BigFloat result(*this);
		result.m_mantissa.m_sign = !result.m_mantissa.m_sign;
		return result;
	}

	BigFloat operator-(const BigFloat& other) const
	{
		if (m_mantissa.isZero()) return other;
		if (other.m_mantissa.isZero()) return *this;

		size_t bitLengthThis = m_mantissa.bitLength();
		size_t bitLengthOther = other.m_mantissa.bitLength();

		if (bitLengthThis > bitLengthOther)
		{
			BigInt shiftedOther = other.m_mantissa << bitLengthThis - bitLengthOther;
			if (m_exponent >= other.m_exponent)
			{
				size_t shift = m_exponent - other.m_exponent;
				BigInt shiftedThis = m_mantissa << shift;
				bitLengthThis += shift;
				shiftedOther = shiftedThis - shiftedOther;
				return BigFloat(std::move(shiftedOther), m_exponent + (int64_t)shiftedOther.bitLength() - (int64_t)bitLengthThis);
			}
			else
			{
				size_t shift = other.m_exponent - m_exponent;
				shiftedOther = shiftedOther << shift;
				bitLengthOther = bitLengthThis + shift;
				shiftedOther = m_mantissa - shiftedOther;
				return BigFloat(std::move(shiftedOther), other.m_exponent + (int64_t)shiftedOther.bitLength() - (int64_t)bitLengthOther);
			}
		}
		else
		{
			BigInt shiftedThis = m_mantissa << bitLengthOther - bitLengthThis;
			if (m_exponent >= other.m_exponent)
			{
				size_t shift = m_exponent - other.m_exponent;
				shiftedThis = shiftedThis << shift;
				bitLengthThis = bitLengthOther + shift;
				shiftedThis = shiftedThis - other.m_mantissa;
				return BigFloat(std::move(shiftedThis), m_exponent + (int64_t)shiftedThis.bitLength() - (int64_t)bitLengthThis);
			}
			else
			{
				size_t shift = other.m_exponent - m_exponent;
				BigInt shiftedOther = other.m_mantissa << shift;
				bitLengthOther += shift;
				shiftedThis = shiftedThis - shiftedOther;
				return BigFloat(std::move(shiftedThis), other.m_exponent + (int64_t)shiftedThis.bitLength() - (int64_t)bitLengthOther);
			}
		}
	}

	BigFloat operator*(const BigFloat& other) const
	{
		if (m_mantissa.isZero() || other.m_mantissa.isZero()) return BigFloat();
		size_t bitLengthThis = m_mantissa.bitLength();
		size_t bitLengthOther = other.m_mantissa.bitLength();
		if (bitLengthThis >= bitLengthOther)
		{
			BigInt shiftedOther = other.m_mantissa << bitLengthThis - bitLengthOther;
			BigInt result = m_mantissa * shiftedOther;
			if (result.bitLength() == bitLengthThis + shiftedOther.bitLength())
				return BigFloat(std::move(result), m_exponent + other.m_exponent + 1);
			return BigFloat(std::move(result), m_exponent + other.m_exponent);
		}
		else
		{
			BigInt shiftedThis = m_mantissa << bitLengthOther - bitLengthThis;
			BigInt result = shiftedThis * other.m_mantissa;
			if (result.bitLength() == shiftedThis.bitLength() + bitLengthOther)
				return BigFloat(std::move(result), m_exponent + other.m_exponent + 1);
			return BigFloat(std::move(result), m_exponent + other.m_exponent);
		}
	}

	BigFloat operator/(const BigFloat& other) const
	{
		if (other.m_mantissa.isZero()) throw std::invalid_argument("Division by zero");
		if (m_mantissa.isZero()) return BigFloat();

		size_t bitLengthThis = m_mantissa.bitLength();
		size_t bitLengthOther = other.m_mantissa.bitLength();
		BigInt shiftedThis;
		if (bitLengthThis >= bitLengthOther)
			shiftedThis = m_mantissa << divisionPrecisionAddition;
		else shiftedThis = m_mantissa << bitLengthOther - bitLengthThis + divisionPrecisionAddition;

		BigInt result = shiftedThis / other.m_mantissa;
		if (result.bitLength() == shiftedThis.bitLength() - bitLengthOther)
			return BigFloat(std::move(result), m_exponent - other.m_exponent - 1);
		return BigFloat(std::move(result), m_exponent - other.m_exponent);
	}

	bool isRound()
	{		
		if (m_exponent < -1)
			return m_mantissa.raw().size() == 0;

		size_t fractionLength = m_mantissa.bitLength() - 1 - m_exponent;
		for (size_t i = 0; i < fractionLength; ++i)
		{
			if (m_mantissa.raw()[i / STORAGE_SIZE] & (StorageType(1) << i % STORAGE_SIZE))
				return false;
		}
		return true;
	}

	void floor()
	{
		if (isRound())
			return;

		if (m_exponent >= 0)
		{
			size_t bitLength = m_mantissa.bitLength();
			m_mantissa >>= bitLength - 1 - m_exponent;
			m_exponent = m_mantissa.bitLength() - 1;
			if (m_mantissa.m_sign == BigInt::NEGATIVE)
				*this = *this - BigFloat(1);
		}
		else m_mantissa = 0;
	}

	void ceiling()
	{
		if (isRound())
			return;

		if (m_exponent >= 0)
		{
			size_t bitLength = m_mantissa.bitLength();
			m_mantissa >>= bitLength - 1 - m_exponent;
			m_exponent = m_mantissa.bitLength() - 1;
			if (m_mantissa.m_sign == BigInt::POSITIVE)
				*this = *this + BigFloat(1);
		}
		else m_mantissa = 0;
	}

	BigFloat operator%(const BigFloat& other) const {
		if (other.m_mantissa.isZero()) throw std::invalid_argument("Modulo by zero");
		if (m_mantissa.isZero()) return BigFloat();

		BigFloat quotient = *this / other;
		quotient.floor();

		return *this - (other * quotient);
	}

	friend inline std::ostream& operator<<(std::ostream& os, const BigFloat& num);
	friend inline std::istream& operator>>(std::istream& is, BigFloat& num);

	friend BigFloat operator""_bf(const char* str);

	void initFromString(const std::string& str);
	std::string toString() const;
	std::string toBinary() const;

private:
	static void growMantissa(BigInt& mantissa, size_t growthAmount) {
		mantissa.m_num.raw().insert(mantissa.m_num.raw().begin(), growthAmount, 0);
	}

	void clearTailingZeros() {
		auto& num = m_mantissa.raw();
		for (size_t i = 0; i < num.size(); ++i)
		{
			if (num[i] != 0)
			{
				num.erase(num.begin(), num.begin() + i);
				return;
			}
		}
	}

	void fillFraction(std::string& str, BigInt& fraction) const;

	static void formatStringOutput(std::string& str,
		size_t decimalPrecision, size_t cutoffLimit, size_t decimalPosition);

	static void cutoffRepeatingPatterns(std::string& strRaw,
		size_t decimalPrecision, size_t cutoffLimit, int64_t& exponent);
};

inline BigFloat operator""_bf(const char* str) {
	return BigFloat(str);
}

std::ostream& operator<<(std::ostream& os, const BigFloat& num) {

	if (num.m_mantissa.m_num.size() == 0) {
		os << "0";
		return os;
	}

	std::string base10 = num.toString();
	
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