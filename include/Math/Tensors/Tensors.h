#pragma once
#include <stdexcept>
#include <cassert>
#include <concepts>
#include <array>

#include "Math/Tensors/TensorFunctions.h"
#include "Math/Tensors/Transform.h"
#include "Math/Tensors/MiscTensorFunctions.h"
#include "Math/Tensors/TensorBase.h"
#include "Math/Common.h"

namespace Math {

    template<typename Derived, typename Base, typename ShapeT>
    class TensorFunctions;

    template<typename ShapeT>
    class TensorViewBase : public TensorFunctions<TensorViewBase<ShapeT>, 
    TensorAssignableExpression<TensorViewBase<ShapeT>,
    TensorViewExpression<TensorViewBase<ShapeT>, ShapeT>>,  
    ShapeT> {
    public:
        using NotIncludedInScalarArithmetic = void;
        using IsTensorExpression = void;
        using IsAssignableExpression = void;
        using Self = TensorViewBase<ShapeT>;
        using BaseExpr = TensorAssignableExpression<Self, TensorViewExpression<Self, ShapeT>>;
        using Base = TensorFunctions<Self, BaseExpr, ShapeT>;
        using ShapeType = ShapeT;
        using ValueType = typename ShapeType::ValueType;

        template<typename U>
        constexpr explicit TensorViewBase(U* ptr) requires std::same_as<std::remove_cv_t<ValueType>, U> : Base(ptr) {}

        template<typename U>
        constexpr TensorViewBase(TensorViewBase<typename ShapeType::template ConvertedSelf<U>::ViewType> view) 
            requires std::same_as<std::remove_cv_t<ValueType>, U> : Base(view.data()) {}

        using Base::size;
        using Base::operator[];
        using Base::operator=;
        using Base::Base;
    };

    template<typename ShapeT, size_t Alignment>
    class alignas(Alignment) TensorBase : public TensorFunctions<TensorBase<ShapeT, Alignment>,
        TensorConstructibleExpression<TensorBase<ShapeT, Alignment>,
        TensorDataExpression<TensorBase<ShapeT, Alignment>, ShapeT, Alignment>>, 
        ShapeT> {
    public:
        using NotIncludedInScalarArithmetic = void;
        using IsTensorExpression = void;
        using IsAssignableExpression = void;
        using Self = TensorBase<ShapeT, Alignment>;
        using BaseExpr = TensorConstructibleExpression<Self, TensorDataExpression<Self, ShapeT, Alignment>>;
        using Base = TensorFunctions<Self, BaseExpr, ShapeT>;
        using ShapeType = ShapeT;
        using ValueType = typename ShapeType::ValueType;

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

        static constexpr auto identity() requires (ShapeType::dims().size() == 2 && ShapeType::dims()[0] == ShapeType::dims()[1]) {
            return diagonal(1.0f);
        }

        static constexpr auto diagonal(ValueType value) requires (ShapeType::dims().size() == 2 && ShapeType::dims()[0] == ShapeType::dims()[1]) {
            Self mat(0.f);
            for(size_t i = 0; i < ShapeType::dims()[0]; ++i) mat[i][i] = value;
            return mat;
        }

        template<typename T>
        static constexpr auto diagonal(const TensorExpression<T>& expr) requires (ShapeType::dims().size() == 2 && 
        ShapeType::dims()[0] == ShapeType::dims()[1] && T::ShapeType::dims().size() == 1 && 
        T::ShapeType::dims()[0] == ShapeType::dims()[0]) {
            Self mat(0.f);
            expr.forEach([&]<typename... Indices>(size_t i, Indices&&... idx){
                mat[i][i] = expr.eval(i, std::forward<Indices>(idx)...);
            });
            return mat;
        }
    };

    template<typename Derived, typename Base, typename ShapeT>
    class TensorFunctions : public Base {
    public:
        using NotIncludedInScalarArithmetic = void;
        using IsTensorExpression = void;
        using IsAssignableExpression = void;
        using ShapeType = ShapeT;
        using ValueType = typename ShapeType::ValueType;

        using Base::operator=;
        using Base::Base;
        using Base::data;
        using Base::size;

        // Conversions
        template<typename T>
        requires(Detail::VecLikeReverse<T, typename Derived::ValueType> && Derived::ShapeType::dims().size() == 1)
        T as() const {
            static constexpr auto limit = std::min(Detail::vecLikeSizeReverse<T, typename Derived::ValueType>(), Derived::ShapeType::dims()[0]);
            T vec;
            Detail::assignToVecLike<limit - 1>(data(), vec);
            return vec;
        }

        template<typename T, typename U>
        requires(Detail::VecLikeReverse<T, typename Derived::ValueType> && Derived::ShapeType::dims().size() == 1 && 
            Detail::vecLikeSizeReverse<T, typename Derived::ValueType>() > Derived::ShapeType::dims()[0])
        T as(U&& arg) const {
            T vec;
            static constexpr auto limit = Detail::vecLikeSizeReverse<T, typename Derived::ValueType>();
            Detail::assignToVecLike<Derived::ShapeType::dims()[0] - 1>(data(), vec);
            Detail::assignToVecLike<limit - 1, Derived::ShapeType::dims()[0]>(std::forward<U>(arg), vec);
            return vec;
        }

        // Iterator support
        class Iterator {
            friend class TensorFunctions;
            typename Derived::ValueType* m_data;

            constexpr Iterator(typename Derived::ValueType* data) : m_data(data) {}
        public:
            constexpr Iterator() = default;

            constexpr decltype(auto) operator*() { return (Derived::ShapeType::getView(m_data, 0)); }
            constexpr decltype(auto) operator*() const { return (Derived::ShapeType::getViewConst(m_data, 0)); }

            constexpr Iterator& operator++() {
                m_data += Derived::ShapeType::stride();
                return *this;
            }

            constexpr Iterator& operator--() {
                m_data -= Derived::ShapeType::stride();
                return *this;
            }

            constexpr std::strong_ordering operator<=>(const Iterator&) const = default;
        };

        class ConstIterator {
            friend class TensorFunctions;
            typename Derived::ValueType* m_data;

            constexpr ConstIterator(typename Derived::ValueType* data) : m_data(data) {}
        public:
            constexpr ConstIterator() = default;

            constexpr decltype(auto) operator*() const { return (Derived::ShapeType::getViewConst(m_data, 0)); }

            constexpr ConstIterator& operator++() {
                m_data += Derived::ShapeType::stride();
                return *this;
            }

            constexpr ConstIterator& operator--() {
                m_data -= Derived::ShapeType::stride();
                return *this;
            }

            constexpr std::strong_ordering operator<=>(const ConstIterator&) const = default;
        };

        constexpr decltype(auto) operator[](size_t i) { return (Derived::ShapeType::getView(data(), i)); }
        constexpr decltype(auto) operator[](size_t i) const { return (Derived::ShapeType::getViewConst(data(), i)); }

        constexpr decltype(auto) view() { return typename Derived::ShapeType::ViewType(data()); }
        constexpr decltype(auto) view() const { return typename Derived::ShapeType::ConstViewType(data()); }

        constexpr Iterator begin() { return Iterator(data()); }
        constexpr Iterator end() { return Iterator(data() + size()); }
        constexpr ConstIterator begin() const { return ConstIterator(data()); }
        constexpr ConstIterator end() const { return ConstIterator(data() + size()); }
        constexpr ConstIterator cbegin() const { return ConstIterator(data()); }
        constexpr ConstIterator cend() const { return ConstIterator(data() + size()); }

        constexpr decltype(auto) front() { return (Derived::ShapeType::getView(data(), 0)); }
        constexpr decltype(auto) back() { return (Derived::ShapeType::getView(data(), size() - 1)); }

        constexpr decltype(auto) front() const { return (Derived::ShapeType::getViewConst(data(), 0)); }
        constexpr decltype(auto) back() const { return (Derived::ShapeType::getViewConst(data(), size() - 1)); }


        // Vector utilities
        template<typename Result = float, VecExpr R>
        requires (Derived::ShapeType::dims() == R::ShapeType::dims() && VecExpr<Derived>)
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
        requires (Derived::ShapeType::dims() == B::ShapeType::dims() && VecExpr<Derived>)
        constexpr Result distance(const TensorExpression<B>& b) const {
            return Math::distance<Result>(*this, b);
        }

        template<Vec3Expr R> requires (Vec3Expr<Derived>)
        constexpr decltype(auto) cross(const TensorExpression<R>& rhs) const {
            return Math::cross(*this, rhs);
        }

        template<VecExpr N>
        requires (Derived::ShapeType::dims() == N::ShapeType::dims() && VecExpr<Derived>)
        constexpr decltype(auto) reflect(const TensorExpression<N>& n) const {
            return Math::reflect(*this, n);
        }

        template<VecExpr N>
        requires (Derived::ShapeType::dims() == N::ShapeType::dims() && VecExpr<Derived>)
        constexpr decltype(auto) refract(const TensorExpression<N>& n, float eta) const {
            return Math::refract(*this, n, eta);
        }

        // Project this on B
        template<VecExpr B>
        requires (Derived::ShapeType::dims() == B::ShapeType::dims() && VecExpr<Derived>)
        constexpr decltype(auto) project(const TensorExpression<B>& b) const {
            return Math::project(*this, b);
        }

        template<VecExpr B>
        requires (Derived::ShapeType::dims() == B::ShapeType::dims() && VecExpr<Derived>)
        constexpr decltype(auto) angle(const TensorExpression<B>& b) const {
            return Math::angle(*this, b);
        }

        // General utilities
        constexpr decltype(auto) sum() const {
            return Math::sum(*this);
        }

        constexpr decltype(auto) product() const {
            return Math::product(*this);
        }

        constexpr decltype(auto) min() const {
            return Math::min(*this);
        }

        constexpr decltype(auto) max() const {
            return Math::max(*this);
        }

        // Matrix utilities
        template<typename R>
        constexpr decltype(auto) mul(const TensorExpression<R>& rhs) const {
            return Math::mul(*this, rhs);
        }

        constexpr decltype(auto) transpose() const requires (SquareMatExpr<Derived>) {
            return Math::transpose(*this);
        }


        template<Vec3Expr V> requires (Mat4Expr<Derived>)
        constexpr decltype(auto) translate(const TensorExpression<V>& v) const {
            return Math::translate(*this, v);
        }

        template<Vec3Expr V, typename T> requires (Mat4Expr<Derived>)
        constexpr decltype(auto) rotate(const TensorExpression<V>& v, T&& angle) const {
            return Math::rotate(*this, v, std::forward<T>(angle));
        }

        template<Vec3Expr V> requires (Mat4Expr<Derived>)
        constexpr decltype(auto) scale(const TensorExpression<V>& v) const {
            return Math::scale(*this, v);
        }

        template<Vec3Expr V, Vec2Expr VX, Vec2Expr VY, Vec2Expr VZ> requires (Mat4Expr<Derived>)
        constexpr decltype(auto) shear(const TensorExpression<V>& v, const TensorExpression<VX>& vx, 
            const TensorExpression<VY>& vy, const TensorExpression<VZ>& vz) const {
            return Math::shear(*this, v, vx, vy, vz);
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
   //static_assert(std::is_trivially_constructible_v<Tensor<float, 4>>); // This does not work on MSVC
    //static_assert(std::is_aggregate_v<Tensor<float, 4>>);

    static_assert(std::is_standard_layout_v<Tensor<float, 4, 4>>);
    static_assert(std::is_trivially_copyable_v<Tensor<float, 4, 4>>);
    //static_assert(std::is_trivially_constructible_v<Tensor<float, 4, 4>>); // This does not work on MSVC
    //static_assert(std::is_aggregate_v<Tensor<float, 4, 4>>);

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
    using FVec2 = Vec2<float>;
    using FVec3 = Vec3<float>;
    using FVec4 = Vec4<float>;

    using DVec1 = Vec1<double>;
    using DVec2 = Vec2<double>;
    using DVec3 = Vec3<double>;
    using DVec4 = Vec4<double>;

    using IVec1 = Vec1<int32_t>;
    using IVec2 = Vec2<int32_t>;
    using IVec3 = Vec3<int32_t>;
    using IVec4 = Vec4<int32_t>;

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
    using FMat2 = Mat2<float>;
    using FMat3 = Mat3<float>;
    using FMat4 = Mat4<float>;

    using DMat1 = Mat1<double>;
    using DMat2 = Mat2<double>;
    using DMat3 = Mat3<double>;
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

namespace std {
    template<typename T>
    struct hash<Math::TensorExpression<T>> {
        uint64_t operator()(const Math::TensorExpression<T>& expr) const {
            uint64_t h = 0;
            expr.forEach([&]<typename... Indices>(size_t i, Indices&&... idx) {
                size_t hx = std::hash(expr.eval(i, std::forward<Indices>(idx)...));
                h ^= hx + 0x9e3779b97f4a7c15ULL + (h << 6) + (h >> 2);
            });
            return h;
        }
    };
}