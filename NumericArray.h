#pragma once
#include <iostream>
#include <cstdlib>
#include <cstdint>
#include <type_traits>
#include <vector>
#include <string>
#include <iostream>

//an array specifically for numeric types
template <typename T, size_t initialSize = 0, size_t initialCapacity = 4, size_t growthRate = 2>
requires std::is_arithmetic_v<T> && std::is_unsigned_v<T>
class NumericArray
{
private:
	T* m_array;
	size_t m_size;
	size_t m_capacity;

	NumericArray(T* array, size_t size, size_t capacity) :
		m_array(array), m_size(size), m_capacity(capacity) {};

public:

	NumericArray() : m_array(new T[initialCapacity](0)), m_size(initialSize), m_capacity(initialCapacity) {};
	~NumericArray() { delete[] m_array; };

	NumericArray(const NumericArray& other) : 
	m_array(new T[other.m_capacity]), m_size(other.m_size), m_capacity(other.m_capacity)
	{
		for (size_t i = 0; i < m_size; ++i)
			m_array[i] = other.m_array[i];
	}

	NumericArray(NumericArray&& other) :
		m_array(other.m_array), m_size(other.m_size), m_capacity(other.m_capacity)
	{
		other.m_array = nullptr;
	}

	NumericArray& operator=(const NumericArray& other)
	{
		if (this == &other)
			return *this;
		m_size = other.m_size;
		m_capacity = other.m_capacity;
		delete[] m_array;
		m_array = new T[m_capacity];
		for (size_t i = 0; i < m_size; ++i)
			m_array[i] = other.m_array[i];
		return *this;
	}

	NumericArray& operator=(NumericArray&& other)
	{
		if (this == &other)
			return *this;
		m_size = other.m_size;
		m_capacity = other.m_capacity;
		m_array = other.m_array;
		other.m_array = nullptr;
		return *this;
	}

	// grows by growth rate times the mult, passing 0 is undefined behavior
	void grow(size_t mult = 1)
	{
		size_t newCapacity = m_capacity * growthRate * mult;
		T* newArray = new T[m_capacity * growthRate * mult];

		for(int i = 0; i < m_capacity; ++i)
			newArray[i] = m_array[i];

		for (int i = m_capacity; i < newCapacity; ++i)
			newArray[i] = 0;

		delete[] m_array;
		m_array = newArray;
		m_capacity = newCapacity;
	}

	template<typename U>
		requires std::is_arithmetic_v<U> && std::is_unsigned_v<U>
	NumericArray& operator=(U&& value)
	{
		if constexpr (sizeof(T) < sizeof(U))
		{
			U temp = std::forward<U>(value);
			m_size = (sizeof(U) - 1) / sizeof(T) + 1;
			if (m_size > m_capacity)
				grow(m_size);
			for (size_t i = 0; i < m_size; ++i)
			{
				m_array[i] = static_cast<T>(temp);
				temp >>= sizeof(T) * 8;
				if(temp == 0)
				{
					m_size = i + 1;
					break;
				}
			}
		}
		else
		{
			m_size = 1;
			if (m_size > m_capacity)
				grow(m_size); //multiplier for growth
			m_array[0] = std::forward<U>(value);
		}	
		return *this;
	}

	NumericArray operator+(const NumericArray& other)
	{
		size_t newCapacity = 0;
		MagnitudeSpan<T> span;

		if (other.m_size > m_size)
		{
			span.size = other.m_size;
			if ((other.m_array[other.m_size - 1] >> (sizeof(T) * 8 - 1)) == 1 && other.m_capacity == other.m_size)
			{
				newCapacity = other.m_capacity * growthRate;
				span.size = other.m_size + 1;
			}
			else
			{
				newCapacity = other.m_capacity;
				span.size = other.m_size;
			}

			span.array = new T[newCapacity];
			addArraysToNew(MagnitudeSpan(m_array, m_size),
				MagnitudeSpan(other.m_array, other.m_size),
				span);
		}
		else if (other.m_size < m_size)
		{
			span.size = m_size;
			if ((m_array[m_size - 1] >> (sizeof(T) * 8 - 1)) == 1 && m_capacity == m_size)
			{
				newCapacity = m_capacity * growthRate;
				span.size = m_size + 1;
			}
			else
			{
				newCapacity = m_capacity;
				span.size = m_size;
			}

			span.array = new T[newCapacity];
			addArraysToNew(MagnitudeSpan(other.m_array, other.m_size),
				MagnitudeSpan(m_array, m_size),
				span);
		}
		else
		{

			if ((other.m_array[other.m_size - 1] >> (sizeof(T) * 8 - 1)) == 1 && other.m_capacity == other.m_size ||
				(m_array[m_size - 1] >> (sizeof(T) * 8 - 1)) == 1 && m_capacity == m_size)
			{
				newCapacity = m_capacity * growthRate;
				span.size = m_size + 1;
			}
			else
			{
				newCapacity = m_capacity;
				span.size = m_size;
			}
			span.array = new T[newCapacity];
			addArraysToNew(MagnitudeSpan(m_array, m_size),
				MagnitudeSpan(other.m_array, other.m_size),
				span);
		}

		for (size_t i = span.size - 1; i != std::numeric_limits<size_t>::max(); --i)
			if (span.array[i] != 0)
			{
				span.size = i + 1;
				break;
			}

		return NumericArray(span.array, span.size, newCapacity);
	}

	NumericArray operator-(const NumericArray& other) //assumes other is smaller
	{
		size_t newCapacity = 0;
		MagnitudeSpan<T> span;

		span.size = m_size;
		newCapacity = span.size;
		span.array = new T[newCapacity](0);
		substractArraysToNew(MagnitudeSpan(m_array, m_size),
			MagnitudeSpan(other.m_array, other.m_size),
			span);

		for (size_t i = span.size - 1; i != std::numeric_limits<size_t>::max(); --i)
			if (span.array[i] != 0)
			{
				span.size = i + 1;
				break;
			}

		return NumericArray(span.array, span.size, newCapacity);
	}

	NumericArray operator*(const NumericArray& other) //assumes other is smaller
	{
		size_t newCapacity = 0;
		MagnitudeSpan<T> span;

		span.size = m_size + other.m_size + 1;
		newCapacity = span.size;
		span.array = new T[newCapacity](0);
		multiplyArraysToNew(MagnitudeSpan(m_array, m_size),
			MagnitudeSpan(other.m_array, other.m_size),
			span);

		for (size_t i = span.size - 1; i != std::numeric_limits<size_t>::max(); --i)
			if (span.array[i] != 0)
			{
				span.size = i + 1;
				break;
			}

		return NumericArray(span.array, span.size, newCapacity);
	}

	NumericArray operator/(const NumericArray& other) //assumes other is smaller
	{
		if (other.m_size == 0)
			throw std::invalid_argument("Division by zero");
		if (other.m_size > m_size)
			return NumericArray(0);

		size_t newCapacity = 0;
		MagnitudeSpan<T> span;

		span.size = m_size - other.m_size + 1;
		newCapacity = span.size;
		span.array = new T[newCapacity](0);
		divideArraysToNew(MagnitudeSpan(m_array, m_size),
			MagnitudeSpan(other.m_array, other.m_size),
			span);

		for (size_t i = span.size - 1; i != std::numeric_limits<size_t>::max(); --i)
			if (span.array[i] != 0)
			{
				span.size = i + 1;
				break;
			}

		return NumericArray(span.array, span.size, newCapacity);
	}

	template<typename U>
		requires std::is_arithmetic_v<U> && std::is_unsigned_v<U>
	NumericArray operator+(const U& other)
	{
		size_t newSize = 0;
		size_t newCapacity = 0;
		T* newArray;
		
		return NumericArray(newArray, newSize, newCapacity);
	}

	void print()
	{
		for (size_t i = m_size - 1; i != std::numeric_limits<size_t>::max(); --i)
			std::cout << (uint64_t)m_array[i] << " ";
	}
};

