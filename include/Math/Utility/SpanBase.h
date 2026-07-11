#pragma once
#include <tuple>
#include <type_traits>
#include <cstdint>

namespace Math::Utility {
    template<typename T, size_t s_size>
    class SpanBase {
    public:
        using ValueType = T;
    private:
        T* m_data;

    public:
        constexpr SpanBase() noexcept = default;
        constexpr ~SpanBase() noexcept = default;
        constexpr SpanBase(const SpanBase&) noexcept = default;
        constexpr SpanBase(SpanBase&&) noexcept = default;
        constexpr SpanBase& operator=(const SpanBase&) noexcept = default;
        constexpr SpanBase& operator=(SpanBase&&) noexcept = default;

        constexpr SpanBase(T* data) noexcept : m_data(data) {};

        template<size_t N>
        constexpr T& at() noexcept { return m_data[N]; }
        template<size_t N>
        constexpr const T& at() const noexcept { return m_data[N]; }        
        constexpr T& at(size_t N) noexcept { return m_data[N]; }
        constexpr const T& at(size_t N) const noexcept { return m_data[N]; }

        inline constexpr ValueType& operator[](size_t N) noexcept { return m_data[N]; }
        inline constexpr const ValueType& operator[](size_t N) const noexcept { return m_data[N]; }

        constexpr T* data() noexcept { return m_data; }
        constexpr const T* data() const noexcept { return m_data; }

        static inline constexpr size_t size() noexcept { return s_size; }
    };

}