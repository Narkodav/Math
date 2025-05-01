#include "BigUint.h"
#include "BigInt.h"

BigUint::BigUint(const BigInt& other) noexcept : m_num(other.raw()) {};

BigUint::BigUint(BigInt&& other) noexcept :m_num(std::exchange(other.m_num.m_num, std::vector<StorageType>())) {};

BigUint& BigUint::operator=(const BigInt& other) noexcept
{
	m_num = other.raw();
	return *this;
}

BigUint& BigUint::operator=(BigInt&& other) noexcept
{
	m_num = std::exchange(other.m_num.m_num, std::vector<StorageType>());
	return *this;
}

BigUint power(BigUint base, BigUint exponent)
{
	if (exponent < 0)
		throw std::invalid_argument("Negative exponent is not supported on BigUints, use BigFloats instead");
	if (exponent == 0) {
		return BigUint(1);
	}
	if (exponent == 0) return BigUint(1);
	if (base == 0 || base == 1) return std::move(base);

	BigUint result = 1;

	while (exponent.raw().size() != 0) {
		if (exponent.raw().front() & 1) {
			result = result * base;  // Multiply if bit is set
		}
		base = base * base;  // Square the base
		exponent >>= 1;       // Move to next bit
	}

	return result;
}

std::string BigUint::toBinary() const
{
	std::string binary;
	StorageType msbMask = StorageType(1) << (STORAGE_SIZE - 1);
	for (size_t i = m_num.size() - 1; i != std::numeric_limits<size_t>::max(); i--)
	{
		for (size_t j = 0; j < STORAGE_SIZE; j++)
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

void BigUint::eraseMSBs(size_t bitAmount) noexcept
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
		size_t shift = STORAGE_SIZE - lastAmount + bitAmount;
		m_num.back() <<= shift;
		m_num.back() >>= shift;
	}
	else if(bitAmount = lastAmount)
		m_num.pop_back();
	else
	{
		size_t limbsToErase = (bitAmount - lastAmount) / STORAGE_SIZE;
		size_t rightBitsToErase = (bitAmount - lastAmount) % STORAGE_SIZE;

		m_num.erase(m_num.end() - limbsToErase - 1, m_num.end());
		m_num.back() <<= rightBitsToErase;
		m_num.back() >>= rightBitsToErase;
	}	
}