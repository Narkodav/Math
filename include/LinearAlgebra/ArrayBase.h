#pragma once
#include <tuple>
#include <type_traits>
#include <cstdint>

namespace Math::LinearAlgebra {
    template<typename T, size_t s_size>
    class ArrayBase {
    public:
        using ValueType = T;
    private:
        T m_data[s_size];

    public:
        constexpr ArrayBase() noexcept = default;
        constexpr ~ArrayBase() noexcept = default;
        constexpr ArrayBase(const ArrayBase&) noexcept = default;
        constexpr ArrayBase(ArrayBase&&) noexcept = default;
        constexpr ArrayBase& operator=(const ArrayBase&) noexcept = default;
        constexpr ArrayBase& operator=(ArrayBase&&) noexcept = default;

        constexpr ArrayBase(std::initializer_list<T> list) noexcept {
            for(size_t i = 0; i < s_size; ++i) m_data[i] = list.begin()[i];
        };

        constexpr ArrayBase(const T& value) noexcept {
            for(size_t i = 0; i < s_size; ++i) m_data[i] = value;
        };

        template<size_t N>
        constexpr T& at() noexcept { return m_data[N]; }
        template<size_t N>
        constexpr const T& at() const noexcept { return m_data[N]; }        
        constexpr T& at(size_t N) noexcept { return m_data[N]; }
        constexpr const T& at(size_t N) const noexcept { return m_data[N]; }

        constexpr T* data() noexcept { return m_data; }
        constexpr const T* data() const noexcept { return m_data; }

        constexpr T max() const noexcept {
            T max = std::numeric_limits<T>::min();
            for(size_t i = 0; i < s_size; ++i) if(max < m_data[i]) max = m_data[i];
            return max;
        }

        static inline constexpr size_t size() noexcept { return s_size; }

        constexpr ArrayBase& operator*=(const ArrayBase& other) noexcept {
            for(size_t i = 0; i < s_size; ++i) m_data[i] *= other.m_data[i];
            return *this;
        }

        constexpr ArrayBase& operator/=(const ArrayBase& other) noexcept {
            for(size_t i = 0; i < s_size; ++i) m_data[i] /= other.m_data[i];
            return *this;
        }

        constexpr ArrayBase& operator+=(const ArrayBase& other) noexcept {
            for(size_t i = 0; i < s_size; ++i) m_data[i] += other.m_data[i];
            return *this;
        }

        constexpr ArrayBase& operator-=(const ArrayBase& other) noexcept {
            for(size_t i = 0; i < s_size; ++i) m_data[i] -= other.m_data[i];
            return *this;
        }


        constexpr ArrayBase operator*(const ArrayBase& other) const noexcept {
            ArrayBase result;
            for(size_t i = 0; i < s_size; ++i) result.m_data[i] = m_data[i] * other.m_data[i];
            return result;
        }

        constexpr ArrayBase operator/(const ArrayBase& other) const noexcept {
            ArrayBase result;
            for(size_t i = 0; i < s_size; ++i) result.m_data[i] = m_data[i] / other.m_data[i];
            return result;
        }

        constexpr ArrayBase operator+(const ArrayBase& other) const noexcept {
            ArrayBase result;
            for(size_t i = 0; i < s_size; ++i) result.m_data[i] = m_data[i] + other.m_data[i];
            return result;
        }

        constexpr ArrayBase operator-(const ArrayBase& other) const noexcept {
            ArrayBase result;
            for(size_t i = 0; i < s_size; ++i) result.m_data[i] = m_data[i] - other.m_data[i];
            return result;
        }


        constexpr ArrayBase& operator*=(const T& other) noexcept {
            for(size_t i = 0; i < s_size; ++i) m_data[i] *= other;
            return *this;
        }

        constexpr ArrayBase& operator/=(const T& other) noexcept {
            for(size_t i = 0; i < s_size; ++i) m_data[i] /= other;
            return *this;
        }

        constexpr ArrayBase& operator+=(const T& other) noexcept {
            for(size_t i = 0; i < s_size; ++i) m_data[i] += other;
            return *this;
        }

        constexpr ArrayBase& operator-=(const T& other) noexcept {
            for(size_t i = 0; i < s_size; ++i) m_data[i] -= other;
            return *this;
        }


        constexpr ArrayBase operator*(const T& other) const noexcept {
            ArrayBase result;
            for(size_t i = 0; i < s_size; ++i) result.m_data[i] = m_data[i] * other;
            return result;
        }

        constexpr ArrayBase operator/(const T& other) const noexcept {
            ArrayBase result;
            for(size_t i = 0; i < s_size; ++i) result.m_data[i] = m_data[i] / other;
            return result;
        }

        constexpr ArrayBase operator+(const T& other) const noexcept {
            ArrayBase result;
            for(size_t i = 0; i < s_size; ++i) result.m_data[i] = m_data[i] + other;
            return result;
        }

        constexpr ArrayBase operator-(const T& other) const noexcept {
            ArrayBase result;
            for(size_t i = 0; i < s_size; ++i) result.m_data[i] = m_data[i] - other;
            return result;
        }
    };

}