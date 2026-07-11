#pragma once
#include <iostream>
#include <cstdlib>
#include <cstdint>
#include <type_traits>
#include <vector>
#include <string>
#include <utility>
#include <limits>

#include "Math/BigNum/StaticBigNumArithmetic.h"
#include "Math/Utility/ArrayBase.h"

namespace Math::StaticBigNum {

    template<size_t s_byteCount>
	class BigUint
	{
		template<size_t byteCount>
		friend class BigUint;
	public:
		using LimbType = uint64_t;
		using StaticBigUintTag = void;
		static inline const size_t s_storageSize = sizeof(LimbType) * 8;
		static inline const size_t s_limbCount = (s_byteCount - 1) / sizeof(LimbType) + 1;
		static inline const size_t s_byteSize = s_limbCount * sizeof(LimbType);
		static inline const size_t s_bitSize = s_byteSize * 8;

	private:
		Math::Utility::ArrayBase<LimbType, s_limbCount> m_storage;

	public:

		constexpr BigUint() noexcept = default;
		constexpr ~BigUint() noexcept = default;

		constexpr BigUint(const BigUint&) noexcept = default;
		constexpr BigUint(BigUint&&) noexcept = default;

		constexpr BigUint& operator=(const BigUint&) noexcept = default;
		constexpr BigUint& operator=(BigUint&&) noexcept = default;

		template<size_t byteCount>
		constexpr BigUint& operator=(const BigUint<byteCount>& other) noexcept {
			m_storage = other.m_storage;
			return *this;
		}
		template<size_t byteCount>
		constexpr BigUint& operator=(BigUint<byteCount>&& other) noexcept {
			m_storage = std::move(other.m_storage);
			return *this;
		}

		template<size_t s_otherLimbCount>
		constexpr BigUint(const Math::Utility::ArrayBase<LimbType, s_otherLimbCount>& other) noexcept : m_storage(other) {
		}

		template<size_t s_otherLimbCount>
		constexpr BigUint(Math::Utility::ArrayBase<LimbType, s_otherLimbCount>&& other) noexcept : m_storage(std::move(other)) {
		}

		template<std::integral T>
		constexpr BigUint(const T& other) noexcept : m_storage{} {
			m_storage[0] = other;
		}

		template<size_t digits>
		constexpr BigUint(std::span<const char, digits> str) noexcept : m_storage{} {
			m_storage = stringToBigInt<LimbType, digits>(str);
		}

		constexpr BigUint(const std::string& str)
		{
			initFromString(str);
		}

		bool operator<(const BigUint& other) const noexcept
		{
			return compare(m_storage, other.m_storage) == CompareResult::Less;
		}

		bool operator>(const BigUint& other) const noexcept
		{
			return compare(m_storage, other.m_storage) == CompareResult::Greater;
		}

		bool operator<=(const BigUint& other) const noexcept
		{
			return compare(m_storage, other.m_storage) != CompareResult::Greater;
		}

		bool operator>=(const BigUint& other) const noexcept
		{
			return compare(m_storage, other.m_storage) != CompareResult::Less;
		}

		bool operator!=(const BigUint& other) const noexcept
		{
			return compare(m_storage, other.m_storage) != CompareResult::Equal;
		}

		bool operator==(const BigUint& other) const noexcept
		{
			return compare(m_storage, other.m_storage) == CompareResult::Equal;
		}

		inline constexpr LimbType& operator[](size_t index) noexcept { return m_storage[index]; };
		inline constexpr const LimbType& operator[](size_t index) const noexcept { return m_storage[index]; };

		static inline constexpr size_t size() noexcept { return s_limbCount; };

		friend std::ostream& operator<<(std::ostream& os, const BigUint& num) {
			std::string base10 = bigIntToString(num.m_storage);
			os << base10;
			return os;
		}

		friend std::istream& operator>>(std::istream& is, BigUint& num) {

			std::string base10;
			if (is >> base10) {
				num.initFromString(base10.c_str());
			}

			return is;
		}

		void initFromString(const std::string& str)
		{
			if (str[0] < '0' || str[0] > '9')
				throw std::invalid_argument("Init string for BigUint must only contain digits");
			if (str[0] == '0')
			{
				m_storage.clear();
				return;
			}
			m_storage = stringToBigInt<LimbType, s_limbCount>(std::span<const char, s_limbCount>(str.data()));
		}

		inline std::string toString() const {
			std::string str = bigIntToString(m_storage);
			return str;
		}

		inline constexpr bool isOdd() const noexcept
		{
			return !(m_storage[0] & 1);
		}

		inline constexpr bool isEven() const noexcept
		{
			return m_storage[0] & 1;
		}

		inline constexpr auto& data() noexcept { return m_storage; };

		inline constexpr const auto& data() const noexcept { return m_storage; };

		std::string toBinary() const;

		template<char... Cs>
		friend auto operator""_sbui();
	};

	//BigUint power(BigUint base, BigUint exponent);

	template<typename LeftNum, typename RightNum>
	requires (std::same_as<typename LeftNum::StaticBigUintTag, void> && std::same_as<typename RightNum::StaticBigUintTag, void>)
	inline constexpr auto operator+(const LeftNum& left, const RightNum& right) noexcept {
		auto result = add(left.data(), right.data());
		BigUint<std::max(LeftNum::size(), RightNum::size()) * sizeof(typename LeftNum::LimbType)> num = result;
		return num;
	}
}


template<char... Cs>
constexpr auto operator""_sbui()
{
    constexpr std::size_t digits = sizeof...(Cs);

    constexpr std::size_t bytes = Math::StaticBigNum::estimateByteCount<digits>();

    return Math::StaticBigNum::BigUint<bytes>(
        std::array<char, digits>{Cs...}
    );
}