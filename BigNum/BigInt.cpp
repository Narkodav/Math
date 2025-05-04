#include "BigInt.h"
#include "BigUint.h"

BigInt::BigInt(const BigUint& other, bool sign /*= POSITIVE*/) noexcept : m_num(other), m_sign(sign) {};

BigInt::BigInt(BigUint&& other, bool sign /*= POSITIVE*/) noexcept :m_num(std::exchange(other, BigUint())), m_sign(sign) {};

BigInt& BigInt::operator=(const BigUint& other) noexcept
{
	m_sign = POSITIVE;
	m_num = other;
	return *this;
}

BigInt& BigInt::operator=(BigUint&& other) noexcept
{
	m_sign = POSITIVE;
	m_num = std::exchange(other, BigUint());
	return *this;
}

BigInt power(BigInt base, BigInt exponent)
{	
	if (base == BigInt(-1))
		if (exponent.isEven())
			return 1;
		else return -1;
	BigInt result = power(base.m_num, exponent.m_num);
	if (result != 0 && base.m_sign == BigInt::NEGATIVE && exponent.isOdd())
		result.m_sign = BigInt::NEGATIVE;
	return result;
}

std::string BigInt::toBinary() const
{	
	return m_num.toBinary();
}

void BigInt::eraseMSBs(size_t bitAmount) noexcept
{
	m_num.eraseMSBs(bitAmount);
}