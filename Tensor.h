//#pragma once
//#include <array>
//#include <span>
//#include <initializer_list>
//
////this is nice looking, but too impractical
//template<typename T, size_t... sizes>
//class Tensor
//{
//public:
//    static inline constexpr size_t rang = sizeof...(sizes);
//    static inline constexpr size_t capacity = (sizes * ...);
//    static inline constexpr std::array<size_t, rang> dimensions = { sizes... };
//    static inline constexpr std::array<size_t, rang> indexMults = [&]() {
//        std::array<size_t, rang> mults;
//        mults[rang - 1] = 1;
//        for (size_t i = rang - 1; i > 0; --i) {
//            mults[i - 1] = mults[i] * dimensions[i];
//        }
//        return mults;
//        }();
//
//private:
//
//    std::array<T, capacity> m_data;
//
//public:
//
//    template<typename... Args>
//    T& operator()(Args... indices) {
//        return const_cast<T&>(std::as_const(*this).operator()(std::forward<Args>(indices)...));
//    }
//
//    template<typename... Args>
//    const T& operator()(Args... indices) const {
//        static_assert(sizeof...(indices) == rang, "Incorrect number of indices");
//        size_t pos = 0;
//        return m_data[((indices * indexMults[pos++]) + ...)];
//    }
//
//    template<size_t... newSizes>
//    Tensor<T, newSizes...> reshape() {
//        using NewTensor = Tensor<T, newSizes...>;
//        // Verify total size remains the same
//        static_assert(NewTensor::capacity == capacity, "New shape must have same total number of elements");
//
//        NewTensor result;
//        result.m_data = m_data;
//        return result;
//    }
//
//    std::array<T, capacity> toFlatArray() const {
//        return m_data;
//    }
//
//    void fromFlatArray(const std::array<T, capacity>& array) {
//        m_data = array;
//    }
//
//    Tensor operator+(const Tensor& other) const {
//        Tensor result;
//        for (size_t i = 0; i < capacity; ++i) {
//            result.m_data[i] = m_data[i] + other.m_data[i];
//        }
//        return result;
//    }
//
//    Tensor operator-(const Tensor& other) const {
//        Tensor result;
//        for (size_t i = 0; i < capacity; ++i) {
//            result.m_data[i] = m_data[i] - other.m_data[i];
//        }
//        return result;
//    }
//
//    Tensor operator*(const Tensor& other) const {
//        Tensor result;
//        for (size_t i = 0; i < capacity; ++i) {
//            result.m_data[i] = m_data[i] * other.m_data[i];
//        }
//        return result;
//    }
//
//    template<size_t size>
//    Tensor operator*(const Tensor<T, size>& other) const requires (rank == 2 && Tensor<T, size>::rank == 1) {
//        Tensor result;
//        for (size_t i = 0; i < capacity; ++i) {
//            result.m_data[i] = m_data[i] * other.m_data[i];
//        }
//        return result;
//    }
//};