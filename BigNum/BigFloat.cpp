#include "BigFloat.h"

void BigFloat::initFromString(const std::string& str)
{
	bool sign = BigInt::POSITIVE;
	size_t start = 0;
	if (str[0] == '-')
	{
		sign = BigInt::NEGATIVE;		
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
	BigInt nextBaseTen = power(10, str.size() - decimal - 1);

	if(fraction == 0)
	{
		clearTailingZeros();
		return;
	}

	size_t i = 0;
	if (m_mantissa.raw().size() == 0)
	{
		m_mantissa.raw().push_back(0);
		for (; i < log10_base * (double)(str.size() - decimal) + additionalPrecision; ++i, --m_exponent)
		{
			m_mantissa <<= 1;
			fraction <<= 1;
			if (fraction >= nextBaseTen)
			{
				m_mantissa.raw()[0] |= 1;
				fraction -= nextBaseTen;
				++m_exponent;
				break;
			}
		}

		if (fraction == 0)
		{
			clearTailingZeros();
			return;
		}
	}

	for (; i < log10_base * (double)(str.size() - decimal) + additionalPrecision; ++i)
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

	if (m_mantissa.m_sign == BigInt::NEGATIVE)
		result = "-";

	if (m_exponent >= 0)
	{
		size_t coefficient = m_mantissa.bitLength() - m_exponent - 1;
		BigInt integer = m_mantissa >> coefficient;
		BigInt fraction = m_mantissa - (integer << coefficient);

		// Convert integer part to string
		result += integer.toString();
		// Handle fraction part if non-zero
		if (!fraction.isZero()) {
			result += ".";
			size_t decimalPoint = m_mantissa.bitLength() - m_exponent - 1;
			size_t decimalPointIndex = decimalPoint / STORAGE_SIZE;
			size_t decimalPointBit = decimalPoint % STORAGE_SIZE;

			while (!fraction.isZero())
			{
				//std::cout << fraction << std::endl;
				fraction *= 10;
				//std::cout << fraction << std::endl;
				size_t bitLegth = fraction.bitLength();
				//std::cout << bitLegth << std::endl;
				if (bitLegth > decimalPoint)
				{
					uint64_t integer = fraction.raw()[decimalPointIndex];
					integer >>= decimalPointBit;
					if (fraction.raw().size() > decimalPointIndex + 1)
					{
						uint64_t nextInteger = fraction.raw()[decimalPointIndex + 1];
						nextInteger <<= (STORAGE_SIZE - decimalPointBit);
						integer |= nextInteger;
					}
					//if (integer > 9)
					//	__debugbreak();
					result += std::to_string(integer);
					//std::cout << fraction.toBinary() << std::endl;
					fraction.eraseMSBs(bitLegth - decimalPoint);
					//std::cout << fraction.toBinary() << std::endl;
				}
				else result += '0';
/*				std::cout << result << std::endl;	*/			
			}
		}
	}
	else //if exponent is negative there is no integer part
	{
		BigInt fraction = m_mantissa;

		result += "0.";
		size_t decimalPoint = m_mantissa.bitLength() - m_exponent;
		size_t decimalPointIndex = decimalPoint / STORAGE_SIZE;
		size_t decimalPointBit = decimalPoint % STORAGE_SIZE;

		while (!fraction.isZero())
		{
			//std::cout << fraction << std::endl;
			fraction *= 10;
			//std::cout << fraction << std::endl;
			size_t bitLegth = fraction.bitLength();
			//std::cout << bitLegth << std::endl;
			if (bitLegth > decimalPoint)
			{
				uint64_t integer = fraction.raw()[decimalPointIndex];
				integer >>= decimalPointBit;
				if (fraction.raw().size() > decimalPointIndex + 1)
				{
					uint64_t nextInteger = fraction.raw()[decimalPointIndex + 1];
					nextInteger <<= (STORAGE_SIZE - decimalPointBit);
					integer |= nextInteger;
				}
				//if (integer > 9)
				//	__debugbreak();
				result += std::to_string(integer);
				//std::cout << fraction.toBinary() << std::endl;
				fraction.eraseMSBs(bitLegth - decimalPoint);
				//std::cout << fraction.toBinary() << std::endl;
			}
			else result += '0';
			/*				std::cout << result << std::endl;	*/
		}		
	}
	return result;
}

std::string BigFloat::toBinary() const
{
	std::string binary = m_mantissa.toBinary();
	if (m_exponent >= 0 && m_exponent < binary.size() - 1)
	{
		binary.insert(binary.begin() + m_exponent + 1, '.');
		return binary;
	}
	binary.insert(binary.begin() + 1, '.');
	binary += "e" + std::to_string(m_exponent);
	return binary;
}