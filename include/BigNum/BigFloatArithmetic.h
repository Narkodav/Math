#pragma once
#include <cstdlib>
#include <cstdint>
#include <type_traits>
#include <vector>
#include <span>

namespace Math::BigNum {
	template<typename T>
		requires std::is_unsigned_v<T>
	void addFloat(const std::vector<T>& x, int64_t xExponent,
		const std::vector<T>& y, int64_t yExponent,
		std::vector<T>& result, int64_t& resultExponent) //assumes y has lesser exponent
	{
		resultExponent = xExponent;
		int64_t offset = xExponent - yExponent;
		int64_t overlap = x.size() - offset;
		if (overlap <= 0) //trivial case when y ends before x begins
		{
			result.reserve(y.size() + x.size() - overlap);
			size_t i = 0;
			for (; i < y.size(); ++i)
				result.push_back(y[i]);
			for (; i < y.size() + offset; ++i)
				result.push_back(0);
			for (; i < y.size() + offset + x.size(); ++i)
				result.push_back(y[i]);
		}
		else if (overlap <= y.size()) //case when y starts before x and overlaps with it
		{
			result.reserve(y.size() + x.size() - overlap + 1);
			size_t i = 0;
			for (; i < y.size() - overlap; ++i)
				result.push_back(y[i]);

			T carry = 0;
			size_t j = 0;
			for (; i < y.size(); ++i, ++j)
			{
				result.push_back(x[j] + y[i] + carry);
				if (carry)
					carry = std::numeric_limits<T>::max() - x[i] <= y[j];
				else carry = std::numeric_limits<T>::max() - x[i] < y[j];
			}

			for (; j < x.size(); ++j)
			{
				result.push_back(x[j] + carry);
				carry = std::numeric_limits<T>::max() - x[j] == 0;
			}
			if (carry)
			{
				resultExponent++;
				result.push_back(1);
			}
		}
		else //case when x starts before y and overlaps with it
		{
			result.reserve(x.size() + 1);
			size_t i = 0;
			for (; i < overlap - offset; ++i)
				result.push_back(x[i]);

			T carry = 0;
			size_t j = 0;
			for (; j < y.size(); ++i, ++j)
			{
				result.push_back(x[i] + y[j] + carry);
				if (carry)
					carry = std::numeric_limits<T>::max() - x[j] <= y[i];
				else carry = std::numeric_limits<T>::max() - x[j] < y[i];
			}

			for (; i < x.size(); ++i)
			{
				result.push_back(x[i] + carry);
				carry = std::numeric_limits<T>::max() - x[i] == 0;
			}
			if (carry)
			{
				resultExponent++;
				result.push_back(1);
			}
		}
	}

	template<typename T>
		requires std::is_unsigned_v<T>
	void substractFloat(const std::vector<T>& x, int64_t xExponent,
		const std::vector<T>& y, int64_t yExponent,
		std::vector<T>& result, int64_t& resultExponent) //assumes y has lesser exponent
	{
		resultExponent = xExponent;
		int64_t offset = xExponent - yExponent;
		int64_t overlap = x.size() - offset;
		if (overlap <= 0) //trivial case when y ends before x begins
		{
			result.reserve(y.size() + x.size() - overlap);
			size_t i = 0;
			for (; i < y.size(); ++i)
				result.push_back(y[i]);
			for (; i < y.size() + offset; ++i)
				result.push_back(0);
			for (; i < y.size() + offset + x.size(); ++i)
				result.push_back(y[i]);
		}
		else if (overlap <= y.size()) //case when y starts before x and overlaps with it
		{
			result.reserve(y.size() + x.size() - overlap + 1);
			size_t i = 0;
			for (; i < y.size() - overlap; ++i)
				result.push_back(y[i]);

			T carry = 0;
			size_t j = 0;
			for (; i < y.size(); ++i, ++j)
			{
				result.push_back(x[j] + y[i] + carry);
				if (carry)
					carry = std::numeric_limits<T>::max() - x[i] <= y[j];
				else carry = std::numeric_limits<T>::max() - x[i] < y[j];
			}

			for (; j < x.size(); ++j)
			{
				result.push_back(x[j] + carry);
				carry = std::numeric_limits<T>::max() - x[j] == 0;
			}
			if (carry)
			{
				resultExponent++;
				result.push_back(1);
			}
		}
		else //case when x starts before y and overlaps with it
		{
			result.reserve(x.size() + 1);
			size_t i = 0;
			for (; i < overlap - offset; ++i)
				result.push_back(x[i]);

			T carry = 0;
			size_t j = 0;
			for (; j < y.size(); ++i, ++j)
			{
				result.push_back(x[i] + y[j] + carry);
				if (carry)
					carry = std::numeric_limits<T>::max() - x[j] <= y[i];
				else carry = std::numeric_limits<T>::max() - x[j] < y[i];
			}

			for (; i < x.size(); ++i)
			{
				result.push_back(x[i] + carry);
				carry = std::numeric_limits<T>::max() - x[i] == 0;
			}
			if (carry)
			{
				resultExponent++;
				result.push_back(1);
			}
		}
	}

	//expects floats to be normalized and x >= y
	template<typename T>
		requires std::is_unsigned_v<T>
	std::vector<T> divideFloat(const std::vector<T>& x, const std::vector<T>& y, size_t& limbOffset) {
		if (y.size() == 0)
			throw std::invalid_argument("Division by zero");
		if (x.size() == 0)
			return std::vector<T>();
		CompareResult res = compare(x, y);
		if (res != CompareResult::GREATER)
			return res == CompareResult::EQUAL ? std::vector<T>() : std::vector<T>{ 1 };

		std::vector<T> quotient(x.size() + 1, 0);
		std::vector<T> remainder;
		limbOffset = 0;
		for (size_t i = x.size() - 1; i != std::numeric_limits<size_t>::max(); --i) {
			// Shift R left by one limb (multiply by base 2^64)
			remainder.insert(remainder.begin(), x[i]);

			if (compare(remainder, y) != CompareResult::LESS)
			{
				// Binary search for the quotient digit q
				T q_low = 0, q_high = std::numeric_limits<T>::max();
				T q = 0;

				while (q_low < q_high) {
					T q_mid = q_low + (q_high - q_low) / 2;
					std::vector<T> product = multiply(y, { q_mid });

					if (compare(remainder, product) != CompareResult::LESS) {
						q_low = q_mid + 1;
					}
					else {
						q_high = q_mid - 1;
					}
				}
				if (q_low == q_high && compare(remainder, multiply(y, { q_high })) == CompareResult::LESS)
					q_high = q_high - 1;

				q = q_high;

				if (q > 0) {
					remainder = subtract(remainder, multiply(y, { q }));
					quotient[i] = q;
				}
			}
			else limbOffset++;
		}

		for (size_t i = quotient.size() - 1; i != std::numeric_limits<size_t>::max(); i--)
		{
			if (quotient[i] != 0)
				break;
			quotient.pop_back();
		}

		return quotient;
	}
}