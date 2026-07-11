#include "Math/BigNum/BigFloat.h"

namespace Math::BigNum {
	void BigFloat::initFromString(const std::string& str)
	{
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

	std::string BigFloat::toString() const {
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

	void BigFloat::fillFraction(std::string& str, BigInt& fraction) const
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

	std::string BigFloat::toBinary() const
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

	void BigFloat::formatStringOutput(std::string& str, size_t decimalPrecision, size_t cutoffLimit, size_t decimalPosition)
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

	void BigFloat::cutoffRepeatingPatterns(std::string& strRaw,
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

	//void BigFloat::formatStringOutput(std::string& str, size_t decimalPrecision)
	//{
	//	size_t decimal = str.find('.');
	//	if (decimal == std::string::npos)
	//		return;
	//
	//	size_t i = 0;
	//	for (; i < decimalPrecision && i < str.size(); ++i)
	//	{
	//		if (str[i + decimal + 1] != '0')
	//			break;
	//	}
	//
	//	if (i == decimalPrecision)
	//	{
	//		str = str.substr(0, decimal + 1);
	//		return;
	//	}
	//
	//	size_t zeroCounter = 0;
	//	size_t nineCounter = 0;
	//
	//	for (; i < str.size() - decimal - 1 && i < decimalPrecision; ++i)
	//	{
	//		if (str[i + decimal + 1] == '0')
	//		{
	//			for (; i < str.size() - decimal - 1; ++i, ++zeroCounter)
	//			{
	//				if (str[i + decimal + 1] != '0')
	//					break;
	//				else if (zeroCounter > 10)
	//				{
	//					str = str.substr(0, i + decimal + 1 - zeroCounter);
	//					return;
	//				}
	//			}
	//			zeroCounter = 0;
	//		}
	//
	//		if (str[i + decimal + 1] == '9')
	//		{
	//			for (; i < str.size() - decimal - 1; ++i, ++nineCounter)
	//			{
	//				if (str[i + decimal + 1] != '9')
	//					break;
	//				else if (nineCounter > 10)
	//				{
	//					str = str.substr(0, i + decimal + 1 - nineCounter);
	//					char carry = 1;
	//					char back = (str.back() - '0' + carry) % 10 + '0';
	//					carry = (str.back() - '0' + carry) / 10;
	//					str.back() = back;
	//					while (carry != 0 && str.size() - 1 != decimal)
	//					{
	//						str.pop_back();
	//						back = (str.back() - '0' + carry) % 10 + '0';
	//						carry = (str.back() - '0' + carry) / 10;
	//						str.back() = back;
	//					}
	//					if (carry && str.size() - 1 == decimal)
	//					{
	//						str.pop_back();
	//						size_t i = str.size() - 1;
	//						while (carry != 0 && i != 0)
	//						{
	//							back = (str.back() - '0' + carry) % 10 + '0';
	//							carry = (str.back() - '0' + carry) / 10;
	//							str.back() = back;
	//							i--;
	//						}
	//						if (carry && i == 0)
	//							str.insert(0, "1");
	//					}
	//					return;
	//				}
	//			}
	//			nineCounter = 0;
	//		}
	//	}
	//	--i;
	//	if (i == decimalPrecision - 1)
	//	{
	//		if(str.size() - 1 <= i + decimal + 1)
	//			return;
	//		if (str[i + decimal + 2] < '5')
	//		{
	//			str = str.substr(0, i + decimal + 2);
	//			return;
	//		}
	//		str = str.substr(0, i + decimal + 2);
	//		char carry = 1;
	//		char back = (str.back() - '0' + carry) % 10 + '0';
	//		carry = (str.back() - '0' + carry) / 10;
	//		str.back() = back;
	//		while (carry != 0 && str.size() - 1 != decimal)
	//		{
	//			str.pop_back();
	//			back = (str.back() - '0' + carry) % 10 + '0';
	//			carry = (str.back() - '0' + carry) / 10;
	//			str.back() = back;
	//		}
	//		if (carry && str.size() - 1 == decimal)
	//		{
	//			str.pop_back();
	//			size_t i = str.size() - 1;
	//			while (carry != 0 && i != 0)
	//			{
	//				back = (str.back() - '0' + carry) % 10 + '0';
	//				carry = (str.back() - '0' + carry) / 10;
	//				str.back() = back;
	//				i--;
	//			}
	//			if (carry && i == 0)
	//				str.insert(0, "1");
	//		}
	//		return;
	//	}
	//}
}