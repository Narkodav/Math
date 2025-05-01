#pragma once
#include <array>
#include <span>
#include <initializer_list>

//this is nice looking, but too impractical
template<typename T, size_t first, size_t... rest>
class Tensor
{
public:
	static inline constexpr size_t rang = sizeof...(rest) + 1;
	static inline constexpr size_t capacity = first * (rest * ...);
	static inline constexpr std::array<size_t, rang> dimensions = { first, rest... };
    using LowerTensor = Tensor<T, rest...>;

private:

	std::array<LowerTensor, first> m_data;

public:
    LowerTensor& operator[](size_t index) {
        return m_data[index];
    }

    const LowerTensor& operator[](size_t index) const {
        return m_data[index];
    }

    T& at(std::span<const size_t> indices) {
        return m_data[indices[0]].at(indices.subspan(1));
    }

    const T& at(std::span<const size_t> indices) const {
        return m_data[indices[0]].at(indices.subspan(1));
    }

    template<size_t... newSizes>
    Tensor<T, newSizes...> reshape() {
        // Verify total size remains the same
        static_assert(capacity == (newSizes * ...) >,
            "New shape must have same total number of elements");

        Tensor<T, newSizes...> result;
        for(size_t i = 0; i < capacity; ++i)
            result.at(linearToIndices(i)) = at(linearToIndices(i))
        return result;
    }

    std::array<size_t, capacity> toFlatArray() const {
        std::array<size_t, capacity> result;

        for (size_t i = 0; i < capacity; ++i)
            result[i] = at(linearToIndices(i));

        return result;
    }

    void fromFlatArray(const std::array<size_t, capacity>& array) {

        for (size_t i = 0; i < capacity; ++i)
            at(linearToIndices(i)) = array[i];
    }

private:
    std::array<size_t, rang> linearToIndices(size_t linearIndex) const {
        std::array<size_t, rang> indices;

        for (size_t i = rang - 1; i >= 0; i--) {
            indices[i] = linearIndex % dimensions[i];
            linearIndex /= dimensions[i];
        }

        return indices;
    }
};

//template<typename T, size_t sizeI, size_t sizeJ>
//class Tensor<T, sizeI, sizeJ> {
//public:
//    static inline constexpr size_t rang = 2;
//    static inline constexpr size_t capacity = sizeI * sizeJ;
//    static inline constexpr std::array<size_t, rang> dimensions = { sizeI, sizeJ };
//    using LowerTensor = Tensor<T, sizeI>;
//
//private:
//    std::array<LowerTensor, sizeJ> m_data;
//
//public:
//
//    LowerTensor& operator[](size_t index) {
//        return m_data[index];
//    }
//
//    const LowerTensor& operator[](size_t index) const {
//        return m_data[index];
//    }
//};

template<typename T, size_t size>
class Tensor<T, size> {
public:
    static inline constexpr size_t rang = 1;
    static inline constexpr size_t capacity = size;
    static inline constexpr std::array<size_t, rang> dimensions = { size };

private:
    std::array<T, size> m_data;

public:

    T& operator[](size_t index) {
        return m_data[index];
    }

    const T& operator[](size_t index) const {
        return m_data[index];
    }

    T& at(const size_t& index) {
        return m_data[index];
    }

    const T& at(const size_t& index) const {
        return m_data[index];
    }
};