#pragma once
#include <cstdlib>
#include <cstdint>
#include <type_traits>
#include <vector>

template<typename T>
	requires std::is_unsigned_v<T>
void add(const std::vector<T>& x, int64_t xExponent,
	const std::vector<T>& y, int64_t yExponent,
	std::vector<T>& result, int64_t resultExponent)
{	
	if (xExponent >= 0 && yExponent >= 0)
	{
		size_t xRightSize = x.size() - xExponent - 1;
		size_t yRightSize = y.size() - yExponent - 1;

		size_t xLeftSize = x.size() - xRightSize;
		size_t yLeftSize = y.size() - yRightSize;

		if (xRightSize >= yRightSize)
		{
			if(xLeftSize >= yLeftSize)
			{
				result.reserve(x.size() + 1);
				size_t i = 0;
				for (; i < xRightSize - yRightSize; ++i)
					result.push_back(x[i]);

				for (size_t j = 0; i < xRightSize; ++i, ++j)
				{
					T carry = 0;

				}
				result.push_back(x[i] + y[j]);
			}
		}

	}
	







	std::vector<T> result(x.size() + 1);
	T carry = 0;

	for (size_t i = 0; i < y.size(); i++)
	{
		result[i] = x[i] + y[i] + carry;
		if (carry)
			carry = std::numeric_limits<T>::max() - x[i] <= y[i];
		else carry = std::numeric_limits<T>::max() - x[i] < y[i];
	}

	for (size_t i = y.size(); i < x.size(); i++)
	{
		result[i] = x[i] + carry;
		carry = std::numeric_limits<T>::max() - x[i] == 0;
	}
	if (carry)
		result[x.size()] = 1;
	else result.pop_back();
	return result;
}