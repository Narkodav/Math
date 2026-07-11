#pragma once
#include <cstdlib>
#include <cstdint>
#include <type_traits>
#include <vector>
#include <string>
#include <limits>

#include "Math/BigNum/StaticBigNumArithmetic.h"

namespace Math::BigNum {
	template<typename T>
		requires std::is_unsigned_v<T>
	std::string bigIntToString(const std::vector<T>& num) {
		std::vector<bool> bits;
		bits.reserve(num.size() * sizeof(T) * 8);

		for (size_t i = num.size() - 1; i != std::numeric_limits<size_t>::max(); i--) {
			for (size_t j = sizeof(T) * 8 - 1; j != std::numeric_limits<size_t>::max(); j--) {  // MSB first
				bits.push_back((num[i] >> j) & 1);
			}
		}

		if (bits.empty()) return "0";

		std::vector<uint8_t> decimal{ 0 }; // Store decimal digits

		// Process each bit from MSB to LSB
		for (size_t i = 0; i < bits.size(); i++) {
			// Multiply current number by 2
			uint8_t carry = 0;
			for (size_t j = 0; j < decimal.size(); j++) {
				uint16_t current = decimal[j] * 2 + carry;
				decimal[j] = current % 10;
				carry = static_cast<uint8_t>(current / 10);
			}
			if (carry) {
				decimal.push_back(carry);
			}

			// Add current bit
			if (bits[i]) {
				// Add 1 to the result
				carry = 1;
				for (size_t j = 0; j < decimal.size() && carry; j++) {
					uint8_t sum = decimal[j] + carry;
					decimal[j] = sum % 10;
					carry = sum / 10;
				}
				if (carry) {
					decimal.push_back(carry);
				}
			}
		}

		// Convert to string
		std::string result;
		result.reserve(decimal.size());
		for (auto it = decimal.rbegin(); it != decimal.rend(); ++it) {
			result.push_back('0' + *it);
		}

		return result;
	}

	enum class CompareResult
	{
		Less = -1,
		Equal = 0,
		Greater = 1
	};

	//-1 if x < y 1 if x > y 0 if x == y
	template<typename T>
		requires std::is_unsigned_v<T>
	CompareResult compare(const std::vector<T>& x, const std::vector<T>& y)
	{
		if (x.size() > y.size())
			return CompareResult::Greater;
		else if (y.size() > x.size())
			return CompareResult::Less;

		for (size_t i = x.size() - 1; i != std::numeric_limits<size_t>::max(); --i)
			if (x[i] > y[i])
				return CompareResult::Greater;
			else if (y[i] > x[i])
				return CompareResult::Less;
		return CompareResult::Equal;
	}

	template<typename T>
		requires std::is_unsigned_v<T>
	std::vector<T> add(const std::vector<T>& x, const std::vector<T>& y) //assumes thisSize >= otherSize
	{
		//if (x.size() < y.size())
		//	return add(y, x);
		std::vector<T> result(x.size() + 1);
		bool carry = 0;
		for (size_t i = 0; i < y.size(); i++)
			std::tie(result[i], carry) = Math::Utility::addWithCarry(x[i], y[i], carry);
		for (size_t i = y.size(); i < x.size(); i++)
			std::tie(result[i], carry) = Math::Utility::addWithCarry(x[i], static_cast<T>(0), carry);
		if (carry)
			result[x.size()] = 1;
		else result.pop_back();
		return result;
	}

	template<typename T>
		requires std::is_unsigned_v<T>
	std::vector<T> subtract(const std::vector<T>& x, const std::vector<T>& y) //assumes x >= y
	{
		//if (compare(x, y) != CompareResult::GREATER)
		//	return std::vector<T>();

		std::vector<T> result(x.size());
		bool borrow = 0;

		for (size_t i = 0; i < y.size(); i++)
			std::tie(result[i], borrow) = Math::Utility::subWithBorrow(x[i], y[i], borrow);

		for (size_t i = y.size(); i < x.size(); i++)
			std::tie(result[i], borrow) = Math::Utility::subWithBorrow(x[i], static_cast<T>(0), borrow);

		for (size_t i = result.size() - 1; i != std::numeric_limits<size_t>::max(); i--)
		{
			if (result[i] != 0)
				break;
			result.pop_back();
		}

		return result;
	}

	template<typename T>
		requires std::is_unsigned_v<T>
	std::vector<T> multiply(const std::vector<T>& x, const std::vector<T>& y)
	{
		//if (y.size() > x.size())
		//	return multiply(y, x);
		std::vector<T> result(x.size() * y.size() + 1, 0);

		for (size_t i = 0; i < x.size(); ++i)
		{
			T carry = 0;
			for (size_t j = 0; j < y.size(); ++j)
			{
				auto [res, carryInternal] = Math::Utility::mult(x[i], y[j]);
				std::tie(result[i + j], carry) = Math::Utility::addWithBigCarry(result[i + j], res, carry);
				carry += carryInternal;
			}
			result[i + y.size()] = carry;
		}

		for (size_t i = result.size() - 1; i != std::numeric_limits<size_t>::max(); i--)
		{
			if (result[i] != 0)
				break;
			result.pop_back();
		}

		return result;
	}

	template<typename T>
		requires std::is_unsigned_v<T>
	std::vector<T> divide(const std::vector<T>& x, const std::vector<T>& y) {
		if (y.size() == 0)
			throw std::invalid_argument("Division by zero");
		if (x.size() == 0)
			return std::vector<T>();
		CompareResult res = compare(x, y);
		if (res != CompareResult::Greater)
			return res == CompareResult::Equal ? std::vector<T>() : std::vector<T>{ 1 };

		std::vector<T> quotient(x.size() + 1, 0);
		std::vector<T> remainder;

		for (size_t i = x.size() - 1; i != std::numeric_limits<size_t>::max(); --i) {
			// Shift R left by one limb (multiply by base 2^64)
			remainder.insert(remainder.begin(), x[i]);

			if (compare(remainder, y) != CompareResult::Less)
			{
				// Binary search for the quotient digit q
				T q_low = 0, q_high = std::numeric_limits<T>::max();
				T q = 0;

				while (q_low < q_high) {
					T q_mid = q_low + (q_high - q_low) / 2;
					std::vector<T> product = multiply(y, { q_mid });

					if (compare(remainder, product) != CompareResult::Less) {
						q_low = q_mid + 1;
					}
					else {
						q_high = q_mid - 1;
					}
				}
				if (q_low == q_high && compare(remainder, multiply(y, { q_high })) == CompareResult::Less)
					q_high = q_high - 1;

				q = q_high;

				if (q > 0) {
					remainder = subtract(remainder, multiply(y, { q }));
					quotient[i] = q;
				}
			}
		}

		for (size_t i = quotient.size() - 1; i != std::numeric_limits<size_t>::max(); i--)
		{
			if (quotient[i] != 0)
				break;
			quotient.pop_back();
		}

		return quotient;
	}

	template<typename T>
		requires std::is_unsigned_v<T>
	std::vector<T> modulo(const std::vector<T>& x, const std::vector<T>& y) {
		if (y.size() == 0)
			throw std::invalid_argument("Division by zero");
		if (x.size() == 0)
			return std::vector<T>();
		CompareResult res = compare(x, y);
		if (res == CompareResult::Less)
			return x;
		else if (res == CompareResult::Equal)
			return std::vector<T>();

		std::vector<T> remainder;

		for (size_t i = x.size() - 1; i != std::numeric_limits<size_t>::max(); --i) {
			// Shift R left by one limb (multiply by base 2^64)
			remainder.insert(remainder.begin(), x[i]);

			if (compare(remainder, y) != CompareResult::Less)
			{
				// Binary search for the quotient digit q
				T q_low = 0, q_high = std::numeric_limits<T>::max();
				T q = 0;

				while (q_low <= q_high) {
					T q_mid = q_low + (q_high - q_low) / 2;
					std::vector<T> product = multiply(y, { q_mid });

					if (compare(remainder, product) != CompareResult::Less) {
						q_low = q_mid + 1;
					}
					else {
						q_high = q_mid - 1;
					}
				}
				q = q_high;

				if (q > 0) {
					remainder = subtract(remainder, multiply(y, { q }));
				}
			}
		}

		return remainder;
	}

	template<typename T>
		requires std::is_unsigned_v<T>
	std::vector<T> stringToBigInt(const std::string& str) {
		std::vector<T> num{ 0 };  // Start with 0

		{
			T digit = str[0] - '0';
			num[0] = digit;
		}

		for (size_t i = 1; i < str.size(); ++i) {
			if (!std::isdigit(str[i])) {
				throw std::invalid_argument("Invalid character in string");
			}

			// Multiply current number by 10
			std::vector<T> temp = multiply(num, { 10 });

			// Add current digit
			T digit = str[i] - '0';
			if (digit != 0) {
				temp = add(temp, { digit });
			}

			num = std::move(temp);
		}

		return num;
	}
}