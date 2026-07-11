#pragma once
#include <stdexcept>
#include <cassert>
#include <concepts>
#include <array>

#include "Math/Tensors/TensorFunctions.h"
#include "Math/Tensors/Transform.h"
#include "Math/Tensors/MiscTensorFunctions.h"
#include "Math/Tensors/TensorBase.h"

namespace Math {

    template<typename Derived, typename Base, typename ShapeT>
    class TensorFunctions;

    template<typename ShapeT>
    class TensorViewBase : public TensorFunctions<TensorViewBase<ShapeT>, TensorWritableExpression<TensorViewBase<ShapeT>>, ShapeT> {
    public:
        using NotIncludedInScalarArithmetic = void;
        using Base = TensorFunctions<TensorViewBase<ShapeT>, TensorWritableExpression<TensorViewBase<ShapeT>>, ShapeT>;
        using BaseExpr = TensorWritableExpression<TensorViewBase<ShapeT>>;
        using ShapeType = ShapeT;
        using ValueType = typename ShapeType::ValueType;

    private:
        ValueType* m_data;

    public:
        template<typename U>
        constexpr explicit TensorViewBase(U* ptr) requires std::same_as<std::remove_cv_t<ValueType>, U> : m_data(ptr) {}

        template<typename U>
        constexpr TensorViewBase(TensorViewBase<typename ShapeType::template ConvertedSelf<U>::ViewType> view) 
            requires std::same_as<std::remove_cv_t<ValueType>, U> : m_data(view.data()) {}

        using Base::size;
        using Base::operator[];
        using Base::operator=;
        using Base::Base;
        
        constexpr const ValueType* data() const { return m_data; }
        constexpr ValueType* data() { return m_data; }
    };

    template<typename ShapeT, size_t Alignment>
    class alignas(Alignment) TensorBase : public TensorFunctions<TensorBase<ShapeT, Alignment>, 
        TensorStorageExpression<TensorBase<ShapeT, Alignment>>, ShapeT> {
    public:
        using NotIncludedInScalarArithmetic = void;
        using Self = TensorBase<ShapeT, Alignment>;
        using BaseExpr = TensorStorageExpression<TensorBase<ShapeT, Alignment>>;
        using Base = TensorFunctions<Self, BaseExpr, ShapeT>;
        using ShapeType = ShapeT;
        using ValueType = typename ShapeType::ValueType;

    private:
        alignas(Alignment) ValueType m_storage[Base::size()];

    public:
        constexpr TensorBase() = default;

        using Base::size;
        using Base::operator[];
        using Base::operator=;
        using Base::Base;

        constexpr TensorBase(ShapeType::ViewType view) : Base(view.data(), view.size()) {};
        constexpr TensorBase& operator=(ShapeType::ConstViewType view) { 
            Base::operator=(view);
            return *this; 
        };

        constexpr operator typename ShapeType::ViewType() { return Base::view(); }
        constexpr operator typename ShapeType::ConstViewType() const { return Base::view(); }

        constexpr const ValueType* data() const { return m_storage; }
        constexpr ValueType* data() { return m_storage; }
        static constexpr size_t alignment() { return Alignment; }
    };

    template<typename Derived, typename Base, typename ShapeT>
    class TensorFunctions : public Base {
    public:
        using NotIncludedInScalarArithmetic = void;
        using ShapeType = ShapeT;
        using ValueType = typename ShapeType::ValueType;

        using Base::operator=;
        using Base::Base;

    public:
        static constexpr size_t size() { return ShapeType::size(); }
        static constexpr size_t stride() { return ShapeType::stride(); }
        static constexpr const auto& dims() { return ShapeType::dims(); }
        static constexpr size_t dimCount() { return ShapeType::dimCount(); }
        static constexpr size_t outerSize() { return ShapeType::outerSize(); }

        constexpr ShapeType::SubViewType operator[](size_t i) { return ShapeType::getView(derived().data(), i); }
        constexpr ShapeType::ConstSubViewType operator[](size_t i) const { return ShapeType::getViewConst(derived().data(), i); }

        constexpr auto view() { return typename ShapeType::ViewType(derived().data()); }
        constexpr auto view() const { return typename ShapeType::ConstViewType(derived().data()); }

        // Vector utilities
        template<typename Result = float, VecExpr R>
        requires (ShapeType::dims() == R::ShapeType::dims() && VecExpr<Derived>)
        constexpr Result dot(const TensorExpression<R>& other) const {
            return Math::dot<Result>(*this, other);
        }

        template<typename Result = float> requires VecExpr<Derived>
        constexpr Result length() const {
            return Math::length<Result>(*this);
        }

        constexpr decltype(auto) normalize() const requires VecExpr<Derived> {
            return Math::normalize(*this);
        }

        template<typename Result = float, VecExpr B>
        requires (ShapeType::dims() == B::ShapeType::dims() && VecExpr<Derived>)
        constexpr Result distance(const TensorExpression<B>& b) const {
            return Math::distance<Result>(*this, b);
        }

        template<Vec3Expr R> requires (Vec3Expr<Derived>)
        constexpr decltype(auto) cross(const TensorExpression<R>& rhs) const {
            return Math::cross(*this, rhs);
        }

        template<VecExpr N>
        requires (ShapeType::dims() == N::ShapeType::dims() && VecExpr<Derived>)
        constexpr decltype(auto) reflect(const TensorExpression<N>& n) const {
            return Math::reflect(*this, n);
        }

        template<VecExpr N>
        requires (ShapeType::dims() == N::ShapeType::dims() && VecExpr<Derived>)
        constexpr decltype(auto) refract(const TensorExpression<N>& n, float eta) const {
            return Math::refract(*this, n, eta);
        }

        // Project this on B
        template<VecExpr B>
        requires (ShapeType::dims() == B::ShapeType::dims() && VecExpr<Derived>)
        constexpr decltype(auto) project(const TensorExpression<B>& b) const {
            return Math::project(*this, b);
        }

        template<VecExpr B>
        requires (ShapeType::dims() == B::ShapeType::dims() && VecExpr<Derived>)
        constexpr decltype(auto) angle(const TensorExpression<B>& b) const {
            return Math::angle(*this, b);
        }

        // General utilities
        template<typename T>
        constexpr decltype(auto) sum() const {
            return Math::sum(*this);
        }

        template<typename T>
        constexpr decltype(auto) product() const {
            return Math::product(*this);
        }

        template<typename T>
        constexpr decltype(auto) min() const {
            return Math::min(*this);
        }

        template<typename T>
        constexpr decltype(auto) max() const {
            return Math::max(*this);
        }

        // Matrix utilities
        template<typename R>
        constexpr decltype(auto) mul(const TensorExpression<R>& rhs) const {
            return Math::mul(*this, rhs);
        }

        template<SquareMatExpr T> requires (SquareMatExpr<Derived>)
        constexpr decltype(auto) transpose() const {
            return Math::transpose(*this);
        }

    protected:
        constexpr Derived& derived() { return static_cast<Derived&>(*this); }
        constexpr const Derived& derived() const { return static_cast<const Derived&>(*this); }
    };

    template<typename T, size_t N, size_t... rest>
    struct Shape<T, N, rest...> {
        using ValueType = T;
        using InternalShapeType = Shape<T, rest...>;
        using ConstInternalShapeType = Shape<const T, rest...>;

        using Self = Shape<T, N, rest...>;
        using ConstSelf = Shape<const T, N, rest...>;

        using SubViewType = TensorViewBase<InternalShapeType>;
        using ConstSubViewType = TensorViewBase<ConstInternalShapeType>;
        using ViewType = TensorViewBase<Self>;
        using ConstViewType = TensorViewBase<ConstSelf>;

        template<typename U>
        using ConvertedSelf = Shape<U, N, rest...>;

        static constexpr size_t s_stride = (rest * ...);
        static constexpr size_t s_size = N * s_stride;
        static constexpr size_t s_dimCount = 1 + sizeof...(rest);
        static constexpr std::array<size_t, s_dimCount> s_dims = { N, rest... };

        static constexpr std::array<size_t, s_dimCount> dims() { return s_dims; }
        static constexpr size_t dimCount() { return s_dimCount; }
        static constexpr size_t size() { return s_size; }
        static constexpr size_t stride() { return s_stride; }
        static constexpr size_t outerSize() { return N; }

        static constexpr SubViewType getView(T* data, size_t index) {
            return SubViewType(data + index * stride());
        }
        static constexpr ConstSubViewType getViewConst(const T* data, size_t index) {
            return ConstSubViewType(data + index * stride());
        }

        template<typename I, typename...Rest>
        static constexpr size_t index(I&& index, Rest&&... restIs) {
            return index * s_stride + InternalShapeType::index(std::forward<Rest>(restIs)...);
        }

    };

    template<typename T, size_t N>
    struct Shape<T, N> {
        using ValueType = T;
        using InternalShapeType = void;
        using ConstInternalShapeType =void;

        using Self = Shape<T, N>;
        using ConstSelf = Shape<const T, N>;

        using SubViewType = T&;
        using ConstSubViewType = const T&;
        using ViewType = TensorViewBase<Self>;
        using ConstViewType = TensorViewBase<ConstSelf>;

        template<typename U>
        using ConvertedSelf = Shape<U, N>;

        static constexpr size_t s_stride = 1;
        static constexpr size_t s_size = N;
        static constexpr size_t s_dimCount = 1;
        static constexpr std::array<size_t, 1> s_dims = { N };

        static constexpr std::array<size_t, 1> dims() { return s_dims; }
        static constexpr size_t dimCount() { return s_dimCount; }
        static constexpr size_t size() { return s_size; }
        static constexpr size_t stride() { return s_stride; }
        static constexpr size_t outerSize() { return N; }

        static constexpr SubViewType getView(T* data, size_t index) { return data[index]; }
        static constexpr ConstSubViewType getViewConst(const T* data, size_t index) { return data[index]; }

        static constexpr size_t index(size_t index) { return index; }
    };

    template<typename T, size_t... dims>
    using Tensor = TensorBase<Shape<T, dims...>>;

    template<typename T, size_t... dims>
    using TensorView = TensorViewBase<Shape<T, dims...>>;

    static_assert(std::is_standard_layout_v<Tensor<float, 4>>);
    static_assert(std::is_trivially_copyable_v<Tensor<float, 4>>);

    static_assert(std::is_standard_layout_v<Tensor<float, 4, 4>>);
    static_assert(std::is_trivially_copyable_v<Tensor<float, 4, 4>>);

    static_assert(sizeof(Tensor<float, 4>) == 4 * sizeof(float));
    static_assert(alignof(Tensor<float, 4>) == alignof(float));

    static_assert(Tensor<int, 4>::size() == 4);
    static_assert(Tensor<int, 2, 3>::size() == 6);
    static_assert(Tensor<int, 2, 3>::stride() == 3);
    static_assert(Tensor<int, 2, 3, 4>::stride() == 12);

    static_assert(std::same_as<
        Tensor<int, 2, 3>::ShapeType,
        Shape<int, 2, 3>
    >);

    static_assert(sizeof(Tensor<int, 4>) == sizeof(int) * 4);
    static_assert(sizeof(Tensor<float, 4, 4>) == sizeof(float) * 16);

    template<typename T, size_t size, size_t alignment = alignof(T)>
    using Vec = TensorBase<Shape<T, size>, alignment>;
    template<typename T, size_t alignment = alignof(T)>
    using Vec1 = Vec<T, 1, alignment>;
    template<typename T, size_t alignment = alignof(T)>
    using Vec2 = Vec<T, 2, alignment>;
    template<typename T, size_t alignment = alignof(T)>
    using Vec3 = Vec<T, 3, alignment>;
    template<typename T, size_t alignment = alignof(T)>
    using Vec4 = Vec<T, 4, alignment>;

    using FVec1 = Vec1<float>;
    using FVec2 = Vec3<float>;
    using FVec3 = Vec2<float>;
    using FVec4 = Vec4<float>;

    using DVec1 = Vec1<double>;
    using DVec2 = Vec3<double>;
    using DVec3 = Vec2<double>;
    using DVec4 = Vec4<double>;

    using IVec1 = Vec1<int32_t>;
    using IVec2 = Vec3<int32_t>;
    using IVec4 = Vec2<int32_t>;
    using IVec3 = Vec4<int32_t>;

    using UVec1 = Vec1<uint32_t>;
    using UVec2 = Vec2<uint32_t>;
    using UVec3 = Vec3<uint32_t>;
    using UVec4 = Vec4<uint32_t>;

    template<typename T, size_t col, size_t row, size_t alignment = alignof(T)>
    using Mat = TensorBase<Shape<T, col, row>, alignment>;
    template<typename T, size_t alignment = alignof(T)>
    using Mat1 = Mat<T, 1, 1, alignment>;
    template<typename T, size_t alignment = alignof(T)>
    using Mat2 = Mat<T, 2, 2, alignment>;
    template<typename T, size_t alignment = alignof(T)>
    using Mat3 = Mat<T, 3, 3, alignment>;
    template<typename T, size_t alignment = alignof(T)>
    using Mat4 = Mat<T, 4, 4, alignment>;

    using FMat1 = Mat1<float>;
    using FMat2 = Mat3<float>;
    using FMat3 = Mat2<float>;
    using FMat4 = Mat4<float>;

    using DMat1 = Mat1<double>;
    using DMat2 = Mat3<double>;
    using DMat3 = Mat2<double>;
    using DMat4 = Mat4<double>;

    using IMat1 = Mat1<int32_t>;
    using IMat2 = Mat2<int32_t>;
    using IMat3 = Mat3<int32_t>;
    using IMat4 = Mat4<int32_t>;

    using UMat1 = Mat1<uint32_t>;
    using UMat2 = Mat2<uint32_t>;
    using UMat3 = Mat3<uint32_t>;
    using UMat4 = Mat4<uint32_t>;

    static_assert(alignof(TensorBase<Shape<int, 4, 3, 2>, 4>) == 4);
    static_assert(alignof(TensorBase<Shape<int, 4, 3, 2>, 8>) == 8);
    static_assert(alignof(TensorBase<Shape<int, 4, 3, 2>, 16>) == 16);

    static_assert(std::same_as<
        decltype(std::declval<IVec4>() + std::declval<IVec4>()),
        TensorBinaryExpression<
            IVec4, IVec4,
            BinaryOpType::Add
        >
    >);

    static_assert(std::same_as<
        decltype(-std::declval<IVec4>()),
        TensorUnaryExpression<
            IVec4,
            UnaryOpType::Negate
        >
    >);

    using Expr = decltype(
        std::declval<IVec4>() +
        std::declval<IVec4>() *
        std::declval<IVec4>()
    );

    static_assert(
        std::derived_from<
            Expr,
            TensorArithmeticExpression<Expr>
        >
    );
}