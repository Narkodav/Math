#pragma once
#include <cstdlib>
#include <cstdint>
#include <type_traits>
#include <vector>

template<typename T>
	requires std::is_arithmetic_v<T>
bool compareMagnitutedsLess(const std::vector<T>& thisArray, const std::vector<T>& otherArray)
{
	if (thisArray.size() < otherArray.size())
		return true;
	else if (thisArray.size() > otherArray.size())
		return false;
	for (size_t i = thisArray.size() - 1; i != std::numeric_limits<size_t>::max(); i--)
	{
		if (thisArray[i] > otherArray[i])
			return false;
		else if (thisArray[i] < otherArray[i])
			return true;
	}
	return false;
}

template<typename T>
	requires std::is_arithmetic_v<T>
bool compareMagnitutedsMore(const std::vector<T>& thisArray, const std::vector<T>& otherArray)
{
	if (thisArray.size() < otherArray.size())
		return false;
	else if (thisArray.size() > otherArray.size())
		return true;
	for (size_t i = thisArray.size() - 1; i != std::numeric_limits<size_t>::max(); i--)
	{
		if (thisArray[i] > otherArray[i])
			return true;
		else if (thisArray[i] < otherArray[i])
			return false;
	}
	return false;
}

template<typename T>
	requires std::is_arithmetic_v<T>
bool compareMagnitutedsEqual(const std::vector<T>& thisArray, const std::vector<T>& otherArray)
{
	if (thisArray.size() < otherArray.size())
		return false;
	else if (thisArray.size() > otherArray.size())
		return false;
	for (size_t i = thisArray.size() - 1; i != std::numeric_limits<size_t>::max(); i--)
	{
		if (thisArray[i] != otherArray[i])
			return false;
	}
	return true;
}