#pragma once
#include "LinearAlgebra/ArrayBase.h"
#include "LinearAlgebra/SpanBase.h"
#include <cmath>

namespace Math::LinearAlgebra {

    template<typename T, typename Derived>
    class MatrixOperations {
    public:
        using ValueType = T;

        //constexpr Derived& operator*=(const Derived& other) noexcept { getStorage() *= other.getStorage(); return getDerived(); }
        constexpr Derived& operator/=(const Derived& other) noexcept { getStorage() /= other.getStorage(); return getDerived(); }
        constexpr Derived& operator+=(const Derived& other) noexcept { getStorage() += other.getStorage(); return getDerived(); }
        constexpr Derived& operator-=(const Derived& other) noexcept { getStorage() -= other.getStorage(); return getDerived(); }

        //constexpr Derived operator*(const Derived& other) const noexcept { return { getStorage() * other.getStorage() }; }
        constexpr Derived operator/(const Derived& other) const noexcept { return { getStorage() / other.getStorage() }; }
        constexpr Derived operator+(const Derived& other) const noexcept { return { getStorage() + other.getStorage() }; }
        constexpr Derived operator-(const Derived& other) const noexcept { return { getStorage() - other.getStorage() }; }

        //constexpr Derived& operator*=(const ValueType& other) noexcept { getStorage() *= other; return getStorage(); }
        constexpr Derived& operator/=(const ValueType& other) noexcept { getStorage() /= other; return getStorage(); }
        constexpr Derived& operator+=(const ValueType& other) noexcept { getStorage() += other; return getStorage(); }
        constexpr Derived& operator-=(const ValueType& other) noexcept { getStorage() -= other; return getStorage(); }

        //constexpr Derived operator*(const ValueType& other) const noexcept { return { getStorage() * other }; }
        constexpr Derived operator/(const ValueType& other) const noexcept { return { getStorage() / other }; }
        constexpr Derived operator+(const ValueType& other) const noexcept { return { getStorage() + other }; }
        constexpr Derived operator-(const ValueType& other) const noexcept { return { getStorage() - other }; }

        constexpr auto& getStorage() noexcept { return getDerived().m_storage; }
        constexpr const auto& getStorage() const noexcept { return getDerived().m_storage; }

        constexpr ValueType* data() noexcept { return getStorage().data(); }
        constexpr const ValueType* data() const noexcept { return getStorage().data(); }

        constexpr ValueType length() const noexcept requires std::is_floating_point_v<ValueType> { return lengthImpl(getDerived()); }
        constexpr auto normalize() const noexcept requires std::is_floating_point_v<ValueType> { return normalizeImpl(getDerived()); }

    private:
        inline constexpr auto& getDerived() noexcept { return *static_cast<Derived*>(this); }
        inline constexpr const auto& getDerived() const noexcept { return *static_cast<const Derived*>(this); }
    };

    template<typename T, size_t s_columns, size_t s_rows>
    class MatrixViewBase : public MatrixOperations<T, MatrixViewBase<T, s_columns, s_rows>> {
        friend class MatrixOperations<T, MatrixViewBase<T, s_columns, s_rows>>;
    public:
        using ValueType = T;
    private:
        static inline const size_t s_size = s_columns * s_rows;
        SpanBase<ValueType, s_size> m_storage;
        
    public:

        constexpr MatrixViewBase(ValueType* data) noexcept : m_storage(data) {}
        constexpr MatrixViewBase(SpanBase<ValueType, s_size> data) noexcept : m_storage(data) {}
        constexpr MatrixViewBase(const ValueType& val) noexcept : m_storage(val) {}

        constexpr MatrixViewBase() noexcept = default;
        constexpr ~MatrixViewBase() noexcept = default;
        constexpr MatrixViewBase(const MatrixViewBase&) noexcept = default;
        constexpr MatrixViewBase(MatrixViewBase&&) noexcept = default;
        constexpr MatrixViewBase& operator=(const MatrixViewBase&) noexcept = default;
        constexpr MatrixViewBase& operator=(MatrixViewBase&&) noexcept = default;

        static inline constexpr size_t rows() noexcept { return s_rows; }
        static inline constexpr size_t columns() noexcept { return s_columns; }

        inline constexpr MatrixViewBase<ValueType, 1, s_rows> operator[](size_t N) requires (s_columns > 1)
            { return { this->data() + N * s_rows }; }
        inline constexpr MatrixViewBase<const ValueType, 1, s_rows> operator[](size_t N) const requires (s_columns > 1)
            { return { this->data() + N * s_rows }; }

        inline constexpr ValueType& operator[](size_t N) requires (s_columns == 1)
            { return *(this->data() + N * s_rows); }
        inline constexpr const ValueType& operator[](size_t N) const requires (s_columns == 1)
            { return *(this->data() + N * s_rows); }

        inline constexpr MatrixViewBase<ValueType, s_columns, s_rows> view() noexcept { return { this->data() }; }
        inline constexpr MatrixViewBase<const ValueType, s_columns, s_rows> view() const noexcept { return { this->data() }; }
    };

    template<typename T, size_t s_columns, size_t s_rows>
    class MatrixBase : public MatrixOperations<T, MatrixBase<T, s_columns, s_rows>> {
        friend class MatrixOperations<T, MatrixBase<T, s_columns, s_rows>>;
    public:
        using ValueType = T;
    private:
        static inline const size_t s_size = s_columns * s_rows;
        ArrayBase<ValueType, s_size> m_storage;
        
    public:

        constexpr MatrixBase(ArrayBase<ValueType, s_size> data) noexcept : m_storage(data) {}
        constexpr MatrixBase(const ValueType& val) noexcept : m_storage(val) {}

        constexpr MatrixBase() noexcept = default;
        constexpr ~MatrixBase() noexcept = default;
        constexpr MatrixBase(const MatrixBase&) noexcept = default;
        constexpr MatrixBase(MatrixBase&&) noexcept = default;
        constexpr MatrixBase& operator=(const MatrixBase&) noexcept = default;
        constexpr MatrixBase& operator=(MatrixBase&&) noexcept = default;

        static inline constexpr size_t rows() noexcept { return s_rows; }
        static inline constexpr size_t columns() noexcept { return s_columns; }

        inline constexpr MatrixViewBase<ValueType, 1, s_rows> operator[](size_t N) requires (s_columns > 1)
            { return { this->data() + N * s_rows }; }
        inline constexpr MatrixViewBase<const ValueType, 1, s_rows> operator[](size_t N) const requires (s_columns > 1)
            { return { this->data() + N * s_rows }; }

        inline constexpr ValueType& operator[](size_t N) requires (s_columns == 1)
            { return *(this->data() + N * s_rows); }
        inline constexpr const ValueType& operator[](size_t N) const requires (s_columns == 1)
            { return *(this->data() + N * s_rows); }

        inline constexpr MatrixViewBase<ValueType, s_columns, s_rows> view() noexcept { return { this->data() }; }
        inline constexpr MatrixViewBase<const ValueType, s_columns, s_rows> view() const noexcept { return { this->data() }; }
    };

    template<typename T, size_t s_columns, size_t s_rows>
    using Mat = MatrixBase<T, s_columns, s_rows>;

    template<typename T>
    using Mat2 = MatrixBase<T, 2, 2>;

    template<typename T>
    using Mat3 = MatrixBase<T, 3, 3>;

    template<typename T>
    using Mat4 = MatrixBase<T, 4, 4>;


    using Mat2f = Mat2<float>;
    static_assert(sizeof(Mat2f) == sizeof(float) * 2 * 2);
    static_assert(alignof(Mat2f) == alignof(float));

    using Mat2d = Mat2<double>;
    static_assert(sizeof(Mat2d) == sizeof(double) * 2 * 2);
    static_assert(alignof(Mat2d) == alignof(double));

    using Mat2i32 = Mat2<int32_t>;
    static_assert(sizeof(Mat2i32) == sizeof(int32_t) * 2 * 2);
    static_assert(alignof(Mat2i32) == alignof(int32_t));
    
    using Mat2u32 = Mat2<uint32_t>;
    static_assert(sizeof(Mat2u32) == sizeof(uint32_t) * 2 * 2);
    static_assert(alignof(Mat2u32) == alignof(uint32_t));


    using Mat3f = Mat3<float>;
    static_assert(sizeof(Mat3f) == sizeof(float) * 3 * 3);
    static_assert(alignof(Mat3f) == alignof(float));

    using Mat3d = Mat3<double>;
    static_assert(sizeof(Mat3d) == sizeof(double) * 3 * 3);
    static_assert(alignof(Mat3d) == alignof(double));

    using Mat3i32 = Mat3<int32_t>;
    static_assert(sizeof(Mat3i32) == sizeof(int32_t) * 3 * 3);
    static_assert(alignof(Mat3i32) == alignof(int32_t));
    
    using Mat3u32 = Mat3<uint32_t>;
    static_assert(sizeof(Mat3u32) == sizeof(uint32_t) * 3 * 3);
    static_assert(alignof(Mat3u32) == alignof(uint32_t));


    using Mat4f = Mat4<float>;
    static_assert(sizeof(Mat4f) == sizeof(float) * 4 * 4);
    static_assert(alignof(Mat4f) == alignof(float));

    using Mat4d = Mat4<double>;
    static_assert(sizeof(Mat4d) == sizeof(double) * 4 * 4);
    static_assert(alignof(Mat4d) == alignof(double));

    using Mat4i32 = Mat4<int32_t>;
    static_assert(sizeof(Mat4i32) == sizeof(int32_t) * 4 * 4);
    static_assert(alignof(Mat4i32) == alignof(int32_t));
    
    using Mat4u32 = Mat4<uint32_t>;
    static_assert(sizeof(Mat4u32) == sizeof(uint32_t) * 4 * 4);
    static_assert(alignof(Mat4u32) == alignof(uint32_t));


    template<typename T, size_t s_rows>
    using Vec = MatrixBase<T, 1, s_rows>;

    template<typename T>
    using Vec2 = Vec<T, 2>;

    template<typename T>
    using Vec3 = Vec<T, 3>;

    template<typename T>
    using Vec4 = Vec<T, 4>;


    using Vec2f = Vec2<float>;
    static_assert(sizeof(Vec2f) == sizeof(float) * 2);
    static_assert(alignof(Vec2f) == alignof(float));

    using Vec2d = Vec2<double>;
    static_assert(sizeof(Vec2d) == sizeof(double) * 2);
    static_assert(alignof(Vec2d) == alignof(double));

    using Vec2i32 = Vec2<int32_t>;
    static_assert(sizeof(Vec2i32) == sizeof(int32_t) * 2);
    static_assert(alignof(Vec2i32) == alignof(int32_t));
    
    using Vec2u32 = Vec2<uint32_t>;
    static_assert(sizeof(Vec2u32) == sizeof(uint32_t) * 2);
    static_assert(alignof(Vec2u32) == alignof(uint32_t));


    using Vec3f = Vec3<float>;
    static_assert(sizeof(Vec3f) == sizeof(float) * 3);
    static_assert(alignof(Vec3f) == alignof(float));

    using Vec3d = Vec3<double>;
    static_assert(sizeof(Vec3d) == sizeof(double) * 3);
    static_assert(alignof(Vec3d) == alignof(double));

    using Vec3i32 = Vec3<int32_t>;
    static_assert(sizeof(Vec3i32) == sizeof(int32_t) * 3);
    static_assert(alignof(Vec3i32) == alignof(int32_t));
    
    using Vec3u32 = Vec3<uint32_t>;
    static_assert(sizeof(Vec3u32) == sizeof(uint32_t) * 3);
    static_assert(alignof(Vec3u32) == alignof(uint32_t));


    using Vec4f = Vec4<float>;
    static_assert(sizeof(Vec4f) == sizeof(float) * 4);
    static_assert(alignof(Vec4f) == alignof(float));

    using Vec4d = Vec4<double>;
    static_assert(sizeof(Vec4d) == sizeof(double) * 4);
    static_assert(alignof(Vec4d) == alignof(double));

    using Vec4i32 = Vec4<int32_t>;
    static_assert(sizeof(Vec4i32) == sizeof(int32_t) * 4);
    static_assert(alignof(Vec4i32) == alignof(int32_t));
    
    using Vec4u32 = Vec4<uint32_t>;
    static_assert(sizeof(Vec4u32) == sizeof(uint32_t) * 4);
    static_assert(alignof(Vec4u32) == alignof(uint32_t));

    // Euclidean norm of a vector or Frobenius norm of a matrix
    template<typename MatrixType>
    constexpr typename MatrixType::ValueType lengthImpl(const MatrixType& mat) 
        requires std::is_floating_point_v<typename MatrixType::ValueType> {
        using ValueType = typename MatrixType::ValueType;
        auto& storage = mat.getStorage();

        ValueType maxComp = 0.0;
        
        for(size_t i = 0; i < storage.size(); ++i) {
            ValueType absVal = abs(storage.at(i));
            if(maxComp < absVal) maxComp = absVal;
        }

        if (maxComp == 0.0)
            return 0.0;
            
        ValueType result = 0.0;
        
        for(size_t i = 0; i < storage.size(); ++i) {
            ValueType value = storage.at(i) / maxComp;
            result += value * value;
        }
        
        return maxComp * std::sqrt(result);
    }

    // Normalises vectors. For general matrices divides the components by the result of length
    template<typename MatrixType>
    constexpr auto normalizeImpl(const MatrixType& mat) 
        requires std::is_floating_point_v<typename MatrixType::ValueType> {
        using ValueType = typename MatrixType::ValueType;
        ValueType length = mat.length();
        if (length <= std::numeric_limits<ValueType>::epsilon()) 
        return Mat<ValueType, MatrixType::columns(), MatrixType::rows()>(0);
        return mat / length;
    }

    template<typename MatLeft, typename MatRight>
    constexpr auto operator*(const MatLeft& left, const MatRight& right)
        requires (MatRight::rows() == MatLeft::columns()) {
        Mat<typename MatLeft::ValueType, MatRight::columns(), MatLeft::rows()> result;
        for(size_t i = 0; i < MatRight::columns(); ++i)
            for(size_t j = 0; j < MatLeft::rows(); ++j)
                for(size_t k = 0; k < MatRight::rows(); ++k)
                    {
                        *(result.data() + i * result.rows() + j) = 
                        *(left.data() + k * left.rows() + j) * 
                        *(right.data() + i * right.rows() + k);
                    }
        return result;
    }

    template<typename MatLeft, typename MatRight>
    constexpr auto& operator*=(MatLeft& left, const MatRight& right) requires (MatRight::rows() == MatLeft::columns()) {
        left = left * right;
        return left;
    }
}