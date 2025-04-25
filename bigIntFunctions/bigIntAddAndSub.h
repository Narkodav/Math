#pragma once
#include <cstdlib>
#include <cstdint>
#include <type_traits>
#include <vector>

template<typename T>
	requires std::is_arithmetic_v<T>
std::vector<T> addMagnitudes(const std::vector<T>& thisArray, const std::vector<T>& otherArray) //assumes thisSize >= otherSize
{
	std::vector<T> newArray(thisArray.size() + 1);
	T leftovers = 0;

	for (size_t i = 0; i < otherArray.size(); i++)
	{
		newArray[i] = thisArray[i] + otherArray[i] + leftovers;
		leftovers = std::numeric_limits<T>::max() - thisArray[i] < otherArray[i];
	}

	for (size_t i = otherArray.size(); i < thisArray.size(); i++)
	{
		newArray[i] = thisArray[i] + leftovers;
		leftovers = std::numeric_limits<T>::max() - thisArray[i] == 0;
	}
	if (leftovers)
		newArray[thisArray.size()] = 1;
	else newArray.pop_back();
	return newArray;
}

template<typename T>
	requires std::is_arithmetic_v<T>
std::vector<T> substractMagnitudes(const std::vector<T>& thisArray, const std::vector<T>& otherArray) //assumes thisSize >= otherSize
{
	std::vector<T> newArray(thisArray.size());
	T leftovers = 0;

	for (size_t i = 0; i < otherArray.size(); i++)
	{
		newArray[i] = thisArray[i] - otherArray[i] - leftovers;
		if (leftovers)
			leftovers = otherArray[i] >= thisArray[i];
		else leftovers = otherArray[i] > thisArray[i];
	}

	for (size_t i = otherArray.size(); i < thisArray.size(); i++)
	{
		newArray[i] = thisArray[i] - leftovers;
		leftovers = thisArray[i] < leftovers;
	}

	for (size_t i = thisArray.size() - 1; i != std::numeric_limits<size_t>::max(); i--)
	{
		if (newArray[i] != 0)
			break;
		newArray.pop_back();
	}

	return newArray;
}