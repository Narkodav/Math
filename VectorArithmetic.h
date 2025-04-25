#pragma once
#include <iostream>
#include <cstdlib>
#include <cstdint>
#include <type_traits>
#include <vector>
#include <string>

//test naive implementation, makes the result human readable for algorithm testing

std::string add(const std::string& x, const std::string& y)
{
	if (y.size() > x.size())
		return add(y, x);

	std::string result(x.size() + 1, '0');
	char carry = 0;

	for (size_t i = 0; i < y.size(); ++i)
	{
		char sum = x[x.size() - i - 1] - '0' + y[y.size() - i - 1] - '0' + carry;
		result[result.size() - i - 1] = (sum % 10) + '0';
		carry = sum / 10;
	}

	for (size_t i = y.size(); i < x.size(); ++i)
	{
		char sum = x[x.size() - i - 1] - '0' + carry;
		result[result.size() - i - 1] = (sum % 10) + '0';
		carry = sum / 10;
	}

	if (carry)
		result.front() = carry + '0';
	else result.erase(0, 1);

	return result;
}

std::string substract(const std::string& x, const std::string& y) //assumes x is bigger than y
{
	std::string result(x.size(), '0');
	char borrow = 0;

	for (size_t i = 0; i < y.size(); ++i)
	{
		//situations when x[x.size() - i - 1] - y[y.size() - i - 1] - borrow is less than 0 cannot happen because x > y
		char res;
		if(x[x.size() - i - 1] < y[y.size() - i - 1] + borrow)
		{
			res = 10 + x[x.size() - i - 1] - y[y.size() - i - 1] - borrow;
			borrow = 1;
		}
		else
		{
			res = x[x.size() - i - 1] - y[y.size() - i - 1] - borrow;
			borrow = 0;
		}
		result[result.size() - i - 1] = res + '0';
	}

	for (size_t i = y.size(); i < x.size(); ++i)
	{
		char res;
		if (x[x.size() - i - 1] < borrow)
		{
			res = 10 + x[x.size() - i - 1] - borrow;
			borrow = 1;
		}
		else
		{
			res = x[x.size() - i - 1] - borrow;
			borrow = 0;
		}
		result[result.size() - i - 1] = res + '0';
	}

	size_t toErase = 0;

	for (int i = 0; i < result.size() - 1; i++)
	{
		if (result[i] != '0')
			break;
		toErase++;
	}
	result.erase(0, toErase);

	return result;
}