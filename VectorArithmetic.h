#pragma once
#include <iostream>
#include <cstdlib>
#include <cstdint>
#include <type_traits>
#include <vector>
#include <string>

//test naive implementation, makes the result human readable for algorithm testing

//-1 if x < y 1 if x > y 0 if x == y
char compare(const std::string& x, const std::string& y)
{
	if (y.size() > x.size())
		return -1;
	else if(x.size() > y.size())
		return 1;

	for (int i = 0; i < x.size(); ++i)
		if (x[i] > y[i])
			return 1;
		else if (y[i] > x[i])
			return -1;
	return 0;
}


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
		result[result.size() - i - 1] = res;
	}

	size_t toErase = 0;

	for (int i = 0; i < result.size() - 1; ++i)
	{
		if (result[i] != '0')
			break;
		toErase++;
	}
	result.erase(0, toErase);

	return result;
}

std::string multiply(const std::string& x, const std::string& y)
{
	if (y.size() > x.size())
		return multiply(y, x);
	std::string result(x.size() * y.size() + 1, '0');

	for (size_t i = 0; i < x.size(); ++i)
	{
		char carry = 0;
		for (size_t j = 0; j < y.size(); ++j)
		{
			char res = (x[x.size() - i - 1] - '0') * (y[y.size() - j - 1] - '0') + carry + result[result.size() - i - j - 1] - '0';
			carry = res / 10;
			result[result.size() - i - j - 1] = res % 10 + '0';
		}
		result[result.size() - i - y.size() - 1] = carry + '0';
	}

	size_t toErase = 0;

	for (int i = 0; i < result.size() - 1; ++i)
	{
		if (result[i] != '0')
			break;
		toErase++;
	}
	result.erase(0, toErase);

	return result;
}

std::string divide(const std::string& x, const std::string& y)
{
	char comparison = compare(x, y);
	std::string result;
	if (comparison <= 0)
	{
		result = "0";
		result[0] += comparison + 1;
		return result;
	}

	std::string part(x.begin(), x.begin() + y.size() - 1);
	for (int i = y.size() - 1; i < x.size(); ++i)
	{
		part.push_back(x[i]);
		char comparison = compare(part, y);
		if (comparison == 1)
		{
			char count = '0';
			do
			{
				part = substract(part, y);
				comparison = compare(part, y);
				count++;
			} while (comparison >= 0);
			result.push_back(count);
			if (part[0] == '0') part.clear();
			i++;
			while (i < x.size() && x[i] == '0')
			{
				i++;
				result.push_back('0');
			}
			i--;
		}
		else result.push_back('0');
	}
	return result;
}

std::string modulo(const std::string& x, const std::string& y)
{
	char comparison = compare(x, y);
	if (comparison <= 0)
		return x;

	std::string part(x.begin(), x.begin() + y.size() - 1);
	for (int i = y.size() - 1; i < x.size(); ++i)
	{
		part.push_back(x[i]);
		char comparison = compare(part, y);
		if (comparison == 1)
		{
			do
			{
				part = substract(part, y);
				comparison = compare(part, y);
			} while (comparison >= 0);
			if (part[0] == '0') part = '0';
		}
	}
	return part;
}