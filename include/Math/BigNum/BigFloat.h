#pragma once
#include <cmath>
#include <cassert>

#include "Math/BigNum/BigInt.h"
#include "Math/BigNum/BigFloatArithmetic.h"

namespace Math::BigNum {
	class BigFloat
	{
	public:
		using StorageType = BigInt::StorageType;
		static inline const size_t s_storageSize = BigInt::s_storageSize; //exponents are stored in s_base 2
		static inline const size_t s_base = 2; //exponents are stored in s_base 2
		// the scale by which the mantissa is expanded during division
		// can be adjusted depending on desired precision or performance
		static inline const float s_divisionPrecisionScale = 1.618;
		static inline const size_t s_divisionPrecisionAddition = 4 * s_storageSize; //2 limbs
		static inline const size_t s_additionalPrecision = 4 * s_storageSize; //4 limbs
		
		static inline const double s_logBase_10 = std::log(10.0) / std::log(static_cast<double>(s_base));
		static inline const double s_log10_base = std::log(static_cast<double>(s_base)) / std::log(10.0);

		static inline const size_t s_repeatingDigitCutoffLimit = 10; //how many repeating digits needs to happen for a cutoff
		static inline const size_t s_decimalFormatPrescision = 20; //display 20 digits of significant

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
				shiftedThis = m_mantissa << s_divisionPrecisionAddition;
			else shiftedThis = m_mantissa << bitLengthOther - bitLengthThis + s_divisionPrecisionAddition;

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
				if (m_mantissa.raw()[i / s_storageSize] & (StorageType(1) << i % s_storageSize))
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
				if (m_mantissa.m_sign == BigInt::s_positive)
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
				if (m_mantissa.m_sign == BigInt::s_positive)
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

		void initFromString(const std::string& str) {
			bool sign = BigInt::s_positive;
			size_t start = 0;
			if (str[0] == '-')
			{
				sign = BigInt::s_negative;		
				start = 1;
			}
			else if (str[0] == '+')
				start = 1;

			size_t decimal = str.find('.');
			if (decimal == std::string::npos)
			{
				m_mantissa.initFromString(str);
				m_exponent = m_mantissa.bitLength() - 1;
				return;
			}

			m_mantissa.m_sign = sign;
			m_mantissa.initFromString(str.substr(start, decimal));
			m_exponent = m_mantissa.bitLength() - 1;

			for (start = decimal + 1; start < str.size(); ++start)
			{
				if (str[start] != '0')
					break;
			}

			BigInt fraction(str.substr(start, str.size() - start));
			BigInt nextBaseTen = power(BigInt(10), str.size() - decimal - 1);

			if(fraction == 0)
				return;

			size_t i = 0;
			if (m_mantissa.raw().size() == 0)
			{
				m_mantissa.raw().push_back(0);
				for (; i < s_log10_base * (double)(str.size() - decimal) + s_additionalPrecision; ++i, --m_exponent)
				{
					m_mantissa <<= 1;
					fraction <<= 1;
					if (fraction >= nextBaseTen)
					{
						m_mantissa.raw()[0] |= 1;
						fraction -= nextBaseTen;
						//++m_exponent;
						break;
					}
				}

				if (fraction == 0)
				{
					clearTailingZeros();
					return;
				}
			}

			for (; i < s_log10_base * (double)(str.size() - decimal) + s_additionalPrecision; ++i)
			{
				m_mantissa <<= 1;
				fraction <<= 1;
				if(fraction >= nextBaseTen)
				{
					m_mantissa.raw()[0] |= 1;
					fraction -= nextBaseTen;
					if (fraction == 0)
						break;
				}		
			}

			clearTailingZeros();
		}

		std::string toString() const {
			std::string result;
			size_t decimalPosition;
			if (m_exponent >= 0)
			{
				size_t exponent = m_exponent;
				BigInt mantissa = m_mantissa;
				if (m_mantissa.bitLength() < static_cast<size_t>(m_exponent + 1))
				{
					mantissa = mantissa << (exponent + 1 - m_mantissa.bitLength());
					exponent = mantissa.bitLength() - 1;
				}

				size_t coefficient = mantissa.bitLength() - exponent - 1;
				BigInt integer = mantissa >> coefficient;
				BigInt fraction = mantissa - (integer << coefficient);

				result += integer.toString();
				decimalPosition = result.size();
				if (!fraction.isZero())
					fillFraction(result, fraction);
			}
			else //if exponent is negative there is no integer part
			{
				if (m_mantissa.m_sign == BigInt::s_negative)
					result += "-";
				BigInt fraction = m_mantissa;

				result += "0";
				decimalPosition = 1;
				fillFraction(result, fraction);
			}
			formatStringOutput(result, s_decimalFormatPrescision, s_repeatingDigitCutoffLimit, decimalPosition);
			return result;
		}
		std::string toBinary() const
		{
			std::string binary = m_mantissa.toBinary();
			if (m_exponent >= 0 && m_exponent < static_cast<int64_t>(binary.size()) - 1)
			{
				binary.insert(binary.begin() + m_exponent + 1, '.');
				return binary;
			}
			binary.insert(binary.begin() + 1, '.');
			binary += "e" + std::to_string(m_exponent);
			return binary;
		}

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

		void fillFraction(std::string& str, BigInt& fraction) const
		{
			str += ".";
			size_t decimalPoint = m_mantissa.bitLength() - m_exponent - 1;
			size_t decimalPointIndex = decimalPoint / s_storageSize;
			size_t decimalPointBit = decimalPoint % s_storageSize;
			do
			{
				fraction *= 10;
				size_t bitLegth = fraction.bitLength();
				if (bitLegth > decimalPoint)
				{
					uint64_t integer = fraction.raw()[decimalPointIndex];
					integer >>= decimalPointBit;
					if (fraction.raw().size() > decimalPointIndex + 1)
					{
						uint64_t nextInteger = fraction.raw()[decimalPointIndex + 1];
						nextInteger <<= (s_storageSize - decimalPointBit);
						integer |= nextInteger;
					}
					str += std::to_string(integer);
					fraction.eraseMSBs(bitLegth - decimalPoint);
				}
				else str += '0';
			} while (!fraction.isZero());
		}

		static void formatStringOutput(std::string& str, size_t decimalPrecision, size_t cutoffLimit, size_t decimalPosition)
		{
			std::string numberRaw = str;
			numberRaw.erase(decimalPosition, 1);
			int64_t decimalExponent = std::numeric_limits<int64_t>::max();
			if (str[decimalPosition - 1] != '0')
			{
				decimalExponent = decimalPosition - 1;		
				cutoffRepeatingPatterns(numberRaw, decimalPrecision, cutoffLimit, decimalExponent);

				
				str = numberRaw;
				if (decimalExponent < static_cast<int64_t>(str.size()))
					str.insert(decimalExponent + 1, ".");
				else
				{
					str.insert(1, ".");
					str += "e" + std::to_string(decimalExponent);
				}
			}
			else
			{
				for (size_t i = 1; i < str.size(); ++i)
					if (numberRaw[i] != '0')
					{
						decimalExponent = -(int64_t)i + 1;
						numberRaw = numberRaw.substr(i);
						cutoffRepeatingPatterns(numberRaw, decimalPrecision, cutoffLimit, decimalExponent);
						str = numberRaw;

						if (-decimalExponent < static_cast<int64_t>(decimalPrecision))
						{
							str.insert(0, -decimalExponent, '0');
							str = "0." + str;
						}
						else
						{					
							str.insert(1, ".");
							str += "e" + std::to_string(decimalExponent - 1);
						}
						return;
					}
				if (decimalExponent == std::numeric_limits<int64_t>::max())
				{
					str = '0';
					return;
				}
			}
		}

		static void cutoffRepeatingPatterns(std::string& strRaw,
		size_t decimalPrecision, size_t cutoffLimit, int64_t& exponent)
		{
			size_t nineCounter = 0;
			size_t zeroCounter = 0;

			for (size_t i = 0; i < decimalPrecision; ++i)
			{
				if (strRaw[i] == '0')
				{
					for (; i < strRaw.size(); ++i, ++zeroCounter)
					{
						if (strRaw[i] != '0')
							break;
						else if (zeroCounter > cutoffLimit)
						{
							strRaw = strRaw.substr(0, i - zeroCounter);
							if (strRaw.size() == 0) //probably cannot happen
							{
								strRaw = "0";
								exponent = 0;
							}
							return;
						}
					}
				}
				else if (strRaw[i] == '9')
				{
					for (; i < strRaw.size(); ++i, ++nineCounter)
					{
						if (strRaw[i] != '9')
							break;
						else if (nineCounter > cutoffLimit)
						{
							strRaw = strRaw.substr(0, i - nineCounter);
							if (strRaw.size() == 0) //can happen in very rare cases
							{
								strRaw = "1"; //the number will be added with an exponent
								exponent += 1;
							}
							else strRaw.back() = strRaw.back() + 1; //back is at most 8 s no carry
							return;
						}
					}
				}
			}

			if (strRaw.size() <= decimalPrecision && strRaw[decimalPrecision] < '5')
				return;
			strRaw = strRaw.substr(0, decimalPrecision);

			//propagate the rounding carry
			while(strRaw.size() > 0)
			{
				if (strRaw.back() != '9')
				{
					strRaw.back() += 1;
					return;
				}
				strRaw.pop_back();
			}

			//if we are here the string is empty
			strRaw = "1"; //the number will be added with an exponent
			exponent += 1;
		}
	};

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
}

inline Math::BigNum::BigFloat operator""_bf(const char* str) {
	return Math::BigNum::BigFloat(str);
}