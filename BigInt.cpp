#include "BigInt.h"
#include "BigUint.h"

BigInt::BigInt(const BigUint& other) noexcept : m_num(other.m_num.size()), m_sign(POSITIVE)
{
	for (size_t i = 0; i < m_num.size(); i++)
		m_num[i] = other.m_num[i];
}

BigInt::BigInt(BigUint&& other) noexcept :m_num(std::exchange(other.m_num, std::vector<StorageType>())), m_sign(POSITIVE) {};

BigInt& BigInt::operator=(const BigUint& other) noexcept
{
	m_sign = POSITIVE;
	m_num.resize(other.m_num.size());
	for (size_t i = 0; i < m_num.size(); i++)
		m_num[i] = other.m_num[i];
	return *this;
}

BigInt& BigInt::operator=(BigUint&& other) noexcept
{
	m_sign = POSITIVE;
	m_num = std::exchange(other.m_num, std::vector<StorageType>());
	return *this;
}