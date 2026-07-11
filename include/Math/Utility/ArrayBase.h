#pragma once
#include <tuple>
#include <type_traits>
#include <cstdint>
#include <concepts>

namespace Math::Utility {
    template<typename T, size_t s_size>
    class ArrayBase {
    public:
        using ValueType = T;
        static inline const size_t s_valueByteSize = sizeof(ValueType);
        static inline const size_t s_valueBitSize = s_valueByteSize * 8;
    private:
        T m_data[s_size];

    public:
        constexpr ArrayBase() noexcept = default;
        constexpr ~ArrayBase() noexcept = default;
        constexpr ArrayBase(const ArrayBase&) noexcept = default;
        constexpr ArrayBase(ArrayBase&&) noexcept = default;
        constexpr ArrayBase& operator=(const ArrayBase&) noexcept = default;
        constexpr ArrayBase& operator=(ArrayBase&&) noexcept = default;

        template<size_t size>
        constexpr ArrayBase(const ArrayBase<T, size>& other) noexcept {
            if constexpr (s_size > ArrayBase<T, size>::size()) {                
                for(size_t i = 0; i < ArrayBase<T, size>::size(); ++i) {
                    m_data[i] = other[i];
                }
            }
            else {
                for(size_t i = 0; i < s_size; ++i) {
                    m_data[i] = other[i];
                }
            }
        }
        template<size_t size>
        constexpr ArrayBase(ArrayBase<T, size>&& other) noexcept {
            if constexpr (s_size > ArrayBase<T, size>::size()) {                
                for(size_t i = 0; i < ArrayBase<T, size>::size(); ++i) {
                    m_data[i] = other[i];
                }
            }
            else {
                for(size_t i = 0; i < s_size; ++i) {
                    m_data[i] = other[i];
                }
            }
        }
        template<size_t size>
        constexpr ArrayBase& operator=(const ArrayBase<T, size>& other) noexcept {
            if constexpr (s_size > ArrayBase<T, size>::size()) {                
                for(size_t i = 0; i < ArrayBase<T, size>::size(); ++i) {
                    m_data[i] = other[i];
                }
            }
            else {
                for(size_t i = 0; i < s_size; ++i) {
                    m_data[i] = other[i];
                }
            }
            return *this;
        }
        template<size_t size>
        constexpr ArrayBase& operator=(ArrayBase<T, size>&& other) noexcept {
            if constexpr (s_size > ArrayBase<T, size>::size()) {                
                for(size_t i = 0; i < ArrayBase<T, size>::size(); ++i) {
                    m_data[i] = other[i];
                }
            }
            else {
                for(size_t i = 0; i < s_size; ++i) {
                    m_data[i] = other[i];
                }
            }
            return *this;
        }

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

        inline constexpr ValueType& operator[](size_t N) noexcept { return m_data[N]; }
        inline constexpr const ValueType& operator[](size_t N) const noexcept { return m_data[N]; }

        inline constexpr bool getBit(size_t N) const noexcept {
            const std::byte* bytes = reinterpret_cast<const std::byte*>(m_data);
            return static_cast<bool>((bytes[N / 8] >> N % 8) & std::byte(1));
        }

        inline constexpr void setBit(size_t N, bool val) noexcept {
            size_t byteIndex = N / 8;
            size_t bitIndex = N % 8;
            std::byte* bytes = reinterpret_cast<std::byte*>(m_data);
            std::byte mask = std::byte(1) << (bitIndex);
            std::byte stateMask = std::byte(val) << (bitIndex);
            bytes[byteIndex] = stateMask | (~mask & bytes[byteIndex]);
        }

        constexpr T* data() noexcept { return m_data; }
        constexpr const T* data() const noexcept { return m_data; }

        static inline constexpr size_t size() noexcept { return s_size; }
    };

}