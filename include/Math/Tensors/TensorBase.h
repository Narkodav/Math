#pragma once
#include <stdexcept>
#include <cassert>
#include <concepts>
#include <array>
#include <span>

#include "Math/Tensors/TensorFunctions.h"

namespace Math::Detail {
    template<size_t Level, typename Shape, typename Fn, typename... Indices>
    void recursiveLoop(Fn&& f, Indices&&... idx) {
        if constexpr(Level != Shape::dims().size()) {
            for (size_t i = 0; i < Shape::dims()[Level]; ++i)
                recursiveLoop<Level + 1, Shape>(
                    std::forward<Fn>(f), std::forward<Indices>(idx)..., i);
        }
        else {
            f(idx...);
        }
    }

    template<size_t Level, size_t StopLevel, typename Shape1, typename Shape2, bool isOverflow = false, typename Fn, typename OverflowFn, typename... Indices>
    requires (Shape1::dims().size() == Shape2::dims().size())
    void clampedRecursiveLoop(Fn&& f, OverflowFn&& overflow, Indices&&... idx) {
        if constexpr(Level != StopLevel) {
            if constexpr (Shape1::dims()[Level] > Shape2::dims()[Level]) {
                size_t i = 0;
                for (; i < Shape2::dims()[Level]; ++i)
                    clampedRecursiveLoop<Level + 1, StopLevel, Shape1, Shape2, isOverflow>(
                        std::forward<Fn>(f), std::forward<OverflowFn>(overflow), 
                        std::forward<Indices>(idx)..., i);
                for (; i < Shape1::dims()[Level]; ++i)
                    clampedRecursiveLoop<Level + 1, StopLevel, Shape1, Shape2, true>(
                        std::forward<Fn>(f), std::forward<OverflowFn>(overflow), 
                        std::forward<Indices>(idx)..., i);
            }
            else {
                for (size_t i = 0; i < Shape1::dims()[Level]; ++i)
                    clampedRecursiveLoop<Level + 1, StopLevel, Shape1, Shape2, isOverflow>(
                        std::forward<Fn>(f), std::forward<OverflowFn>(overflow), 
                        std::forward<Indices>(idx)..., i);
            }
        }
        else if constexpr (isOverflow) {
            overflow(idx...);
        }
        else {
            f(idx...);
        }
    }

    template<typename T, typename V>
    concept Vec1Like = requires(T v) {
        { v.x } -> std::convertible_to<V>;
    };
    template<typename T, typename V>
    concept Vec2Like = Vec1Like<T, V> && requires(T v) {
        { v.y } -> std::convertible_to<V>;
    };
    template<typename T, typename V>
    concept Vec3Like = Vec2Like<T, V> && requires(T v) {
        { v.z } -> std::convertible_to<V>;
    };
    template<typename T, typename V>
    concept Vec4Like = Vec3Like<T, V> && requires(T v) {
        { v.w } -> std::convertible_to<V>;
    };

    template<typename T, typename V>
    concept Vec1LikeReverse = requires(T v) {
        std::convertible_to<V, decltype(v.x)>;
    };
    template<typename T, typename V>
    concept Vec2LikeReverse = Vec1Like<T, V> && requires(T v) {
        std::convertible_to<V, decltype(v.y)>;
    };
    template<typename T, typename V>
    concept Vec3LikeReverse = Vec2Like<T, V> && requires(T v) {
        std::convertible_to<V, decltype(v.z)>;
    };
    template<typename T, typename V>
    concept Vec4LikeReverse = Vec3Like<T, V> && requires(T v) {
        std::convertible_to<V, decltype(v.w)>;
    };

    template<typename T, typename V>
    concept VecLike = Vec4Like<T, V> || Vec3Like<T, V> || Vec2Like<T, V> || Vec1Like<T, V>;
    template<typename T, typename V>
    concept VecLikeReverse = Vec4LikeReverse<T, V> || Vec3LikeReverse<T, V> || Vec2LikeReverse<T, V> || Vec1LikeReverse<T, V>;

    template<typename T, typename V>
    consteval size_t vecLikeSize() {
        if constexpr (Vec4Like<T, V>)
            return 4;
        else if constexpr (Vec3Like<T, V>)
            return 3;
        else if constexpr (Vec2Like<T, V>)
            return 2;
        else if constexpr (Vec1Like<T, V>)
            return 1;
        else
            return 0;
    }

    template<typename T, typename V>
    consteval size_t vecLikeSizeReverse() {
        if constexpr (Vec4LikeReverse<T, V>)
            return 4;
        else if constexpr (Vec3LikeReverse<T, V>)
            return 3;
        else if constexpr (Vec2LikeReverse<T, V>)
            return 2;
        else if constexpr (Vec1LikeReverse<T, V>)
            return 1;
        else
            return 0;
    }

    template<size_t Size, size_t Limit = 0, class T, class Vec>
    constexpr void assignFromVecLike(T* data, const Vec& vec) {
        if constexpr (Size > Limit) assignFromVecLike<Size - 1>(data, vec);
        if constexpr (Size == 0) data[0] = vec.x;
        if constexpr (Size == 1) data[1] = vec.y;
        if constexpr (Size == 2) data[2] = vec.z;
        if constexpr (Size == 3) data[3] = vec.w;
    }

    template<size_t Size, size_t Limit = 0, class T, class Vec>
    constexpr void assignToVecLike(const T* data, Vec& vec) {
        if constexpr (Size > Limit) assignToVecLike<Size - 1>(data, vec);
        if constexpr (Size == 0) vec.x = data[0];
        if constexpr (Size == 1) vec.y = data[1];
        if constexpr (Size == 2) vec.z = data[2];
        if constexpr (Size == 3) vec.w = data[3];
    }

    template<size_t Size, size_t Limit = 0, class T, class Vec>
    constexpr void assignToVecLike(T&& arg, Vec& vec) {
        if constexpr (Size > Limit) assignToVecLike<Size - 1>(arg, vec);
        if constexpr (Size == 0) vec.x = arg;
        if constexpr (Size == 1) vec.y = arg;
        if constexpr (Size == 2) vec.z = arg;
        if constexpr (Size == 3) vec.w = arg;
    }
}

namespace Math {
    template<typename D>
    class TensorExpression {
    public:
        using Derived = D;
        using NotIncludedInScalarArithmetic = void;
        using IsTensorExpression = void;

        constexpr const Derived& derived() const {
            return static_cast<const Derived&>(*this);
        }
        constexpr Derived& derived() {
            return static_cast<Derived&>(*this);
        }

        static constexpr size_t size() { return Derived::ShapeType::size(); }
        static constexpr size_t stride() { return Derived::ShapeType::stride(); }
        static constexpr const auto& dims() { return Derived::ShapeType::dims(); }
        static constexpr size_t dimCount() { return Derived::ShapeType::dimCount(); }
        static constexpr size_t outerSize() { return Derived::ShapeType::outerSize(); }

        template<typename... Indices>
        constexpr decltype(auto) eval(size_t global, Indices&&... Idx) const {
            return derived().eval(global, std::forward<Indices>(Idx)...);
        }

        constexpr decltype(auto) eval() const {
            return derived().eval();
        }

        template<typename... Indices>
        constexpr decltype(auto) evalCoord(Indices&&... Idx) const {
            auto index = D::ShapeType::index(std::forward<Indices>(Idx)...);
            return derived().eval(index, std::forward<Indices>(Idx)...);
        }

        template<typename T, typename S, typename U = typename T::ValueType>
        requires(T::ShapeType::dims().size() == Derived::ShapeType::dims().size())
        constexpr void eval(TensorAssignableExpression<T, S>& dest, U&& def = typename T::ValueType{}) const {
            derived().eval(dest, std::forward<U>(def));
        }

        template<typename Fn>
        void forEach(Fn&& fn) const {
            size_t i = 0;
            Math::Detail::recursiveLoop<0, typename Derived::ShapeType>(
                [&]<typename... Indices>(Indices&&... idx){
                fn(i, std::forward<Indices>(idx)...); ++i;
            });
        }

        operator bool() const { return Math::all(*this); }
    };

    template<typename T>
    concept TensorExpr = std::derived_from<std::remove_cvref_t<T>, TensorExpression<std::remove_cvref_t<T>>>;

    template<typename Derived>
    class TensorArithmeticExpression : public TensorExpression<Derived> {
    public:
        using NotIncludedInScalarArithmetic = void;
        using IsTensorExpression = void;
        using Base = TensorExpression<Derived>;

        using Base::derived;

        constexpr auto eval() const {
            TensorBase<typename Derived::ShapeType> result;
            eval(result);
            return result;
        }

        template<typename T, typename S, typename U = typename T::ValueType>
        requires(T::ShapeType::dims() == Derived::ShapeType::dims())
        constexpr void eval(TensorAssignableExpression<T, S>& dest, U&& = typename T::ValueType{}) const {
            size_t i = 0;
            Math::Detail::recursiveLoop<0, typename T::ShapeType>(
                [&]<typename... Indices>(Indices&&... idx){
                    dest.data()[i] = derived().eval(i, std::forward<Indices>(idx)...);
                    ++i;
            });
        }
        
        template<typename T, typename S, typename U = typename T::ValueType>
        requires(T::ShapeType::dims() != Derived::ShapeType::dims() &&
            T::ShapeType::dims().size() == Derived::ShapeType::dims().size())
        constexpr void eval(TensorAssignableExpression<T, S>& dest) const {
            using DestShape = typename T::ShapeType;
            using SrcShape = typename Derived::ShapeType;
            size_t i = 0;
            Math::Detail::clampedRecursiveLoop<0, DestShape::dims().size(), DestShape, SrcShape>(
                [&]<typename...Indices>(Indices&&... idx){
                    dest.data()[i] = derived().eval(SrcShape::index(std::forward<Indices>(idx)...), std::forward<Indices>(idx)...);
                    ++i;
                },
                [&]<typename...Indices>(Indices&&...){
                    // Leave dest as is
                    ++i;
                }
            );
        }

        template<typename T, typename S, typename U>
        requires(T::ShapeType::dims() != Derived::ShapeType::dims() &&
            T::ShapeType::dims().size() == Derived::ShapeType::dims().size())
        constexpr void eval(TensorAssignableExpression<T, S>& dest, U&& def) const {
            using DestShape = typename T::ShapeType;
            using SrcShape = typename Derived::ShapeType;
            size_t i = 0;
            Math::Detail::clampedRecursiveLoop<0, DestShape::dims().size(), DestShape, SrcShape>(
                [&]<typename...Indices>(Indices&&... idx){
                    dest.data()[i] = derived().eval(SrcShape::index(std::forward<Indices>(idx)...), std::forward<Indices>(idx)...);
                    ++i;
                },
                [&]<typename...Indices>(Indices&&...){
                    dest.data()[i] = std::forward<U>(def);
                    ++i;
                }
            );
        }
    };

    template<typename Shape>
    class ScalarExpression : public TensorArithmeticExpression<ScalarExpression<Shape>> {
    public:
        using NotIncludedInScalarArithmetic = void;
        using IsTensorExpression = void;
        using ShapeType = Shape;
        using ValueType = typename ShapeType::ValueType;

    private:
        const ValueType& m_value;

    public:
        constexpr operator const ValueType&() const { return m_value; }
        constexpr ScalarExpression(const ValueType& value) : m_value(value) {}
        template<typename... Indices>
        constexpr const ValueType& eval(size_t, Indices&&...) const { return m_value; }
        constexpr const ValueType& eval() const { return m_value; }

        template<typename T, typename S>
        constexpr void eval(TensorAssignableExpression<T, S>& dest) const {
            std::fill(dest.data(), dest.data() + dest.size(), m_value);
        }
    };

    template<typename D, typename ShapeT, size_t Alignment>
    class TensorDataExpression : public TensorExpression<D> {
    public:
        using NotIncludedInScalarArithmetic = void;
        using IsTensorExpression = void;
        using ShapeType = ShapeT;
        using ValueType = typename ShapeType::ValueType;

    private:
        alignas(Alignment) ValueType m_storage[ShapeType::size()];

    public:
        constexpr const ValueType* data() const { return m_storage; }
        constexpr ValueType* data() { return m_storage; }
        static constexpr size_t alignment() { return Alignment; }
    };

    template<typename D, typename ShapeT>
    class TensorViewExpression : public TensorExpression<D> {
    public:
        using NotIncludedInScalarArithmetic = void;
        using IsTensorExpression = void;
        using ShapeType = ShapeT;
        using ValueType = typename ShapeType::ValueType;

    private:
        ValueType* m_data;

    public:
        constexpr explicit TensorViewExpression(ValueType* ptr) : m_data(ptr) {}
        constexpr const ValueType* data() const { return m_data; }
        constexpr ValueType* data() { return m_data; }
    };

    template<typename D, typename Storage>
    class TensorAssignableExpression : public Storage {
    public:
        using NotIncludedInScalarArithmetic = void;
        using IsTensorExpression = void;
        using IsAssignableExpression = void;
        using Derived = D;
        using Base = Storage;

        using Base::Base;

        using Base::data;
        using Base::size;
        using Base::derived;

        template<typename Der>
        constexpr Derived& operator=(const TensorExpression<Der>& e) {
            if constexpr (std::same_as<Der, Derived>) {
                if(&e == this) return derived();
            }
            e.eval(*this);
            return derived();
        }

        template<typename T>
        constexpr Derived& operator=(std::initializer_list<T> list) {
            if(list.size() < size()) {
                std::copy(list.begin(), list.end(), data());
            }
            else {
                std::copy(list.begin(), list.begin() + size(), data());
            }
            return derived();
        }

        template<typename Arg>
        requires (!TensorExpr<Arg> && std::convertible_to<Arg, typename Derived::ValueType>)
        constexpr Derived& operator=(Arg&& arg) {
            std::fill(data(), data() + size(), std::forward<Arg>(arg));
            return derived();
        }

        template<typename T>
        constexpr Derived& operator=(std::span<T> arr) {
            if(arr.size() < size()) {
                std::copy(arr.begin(), arr.end(), data());
            }
            else {
                std::copy(arr.begin(), arr.begin() + size(), data());
            }
            return derived();
        }

        template<typename T>
        requires(!TensorExpr<T> && Detail::VecLike<T, typename Derived::ValueType> && Derived::ShapeType::dims().size() == 1)
        constexpr Derived& operator=(const T& vec) {
            constexpr auto limit = std::min(Detail::vecLikeSize<T, typename Derived::ValueType>(), Derived::ShapeType::dims()[0]);
            Detail::assignFromVecLike<limit - 1>(data(), vec);
            return derived();
        }

        // Non const access to derived
        constexpr Derived& derived() { return static_cast<Derived&>(*this); }
        constexpr const Derived& derived() const { return static_cast<const Derived&>(*this); }

        constexpr const auto& eval() const { return derived(); }
        constexpr auto& eval() { return derived(); }

        template<typename... Indices>
        constexpr decltype(auto) eval(size_t global, Indices&&...) const {
            return data()[global];
        }

        template<typename... Indices>
        constexpr decltype(auto) eval(size_t global, Indices&&...) {
            return data()[global];
        }

        template<typename... I>
        constexpr decltype(auto) operator()(I&&... coords) const {
            return data()[D::ShapeType::index(std::forward<I>(coords)...)];
        }

        template<typename... I>
        constexpr decltype(auto) operator()(I&&... coords) {
            return data()[D::ShapeType::index(std::forward<I>(coords)...)];
        }

        template<typename T, typename S, typename U = typename T::ValueType>
        requires(T::ShapeType::dims() == Derived::ShapeType::dims())
        constexpr void eval(TensorAssignableExpression<T, S>& dest, U&& = typename T::ValueType{}) const {
            std::copy(data(), data() + size(), dest.data());
        }

        template<typename T, typename S>
        requires(T::ShapeType::dims() != Derived::ShapeType::dims() &&
            T::ShapeType::dims().size() == Derived::ShapeType::dims().size())
        constexpr void eval(TensorAssignableExpression<T, S>& dest) const {
            using DestShape = typename T::ShapeType;
            using SrcShape = typename Derived::ShapeType;
            size_t i = 0;
            Math::Detail::clampedRecursiveLoop<0, DestShape::dims().size() - 1, 
                DestShape, SrcShape>(
                [&]<typename...Indices>(Indices&&... idx){
                    auto* src = data() + SrcShape::index(std::forward<Indices>(idx)..., 0);
                    auto* dst = dest.data() + i;
                    if constexpr(SrcShape::dims().back() < DestShape::dims().back()) {
                        std::copy(src, src + SrcShape::dims().back(), dst);
                    }else {
                        std::copy(src, src + DestShape::dims().back(), dst);
                    }
                    i += DestShape::dims().back();
                },
                [&]<typename...Indices>(Indices&&...){
                    // Leave dest as is
                    i += DestShape::dims().back();
                }
            );
        }

        template<typename T, typename S, typename U>
        requires(T::ShapeType::dims() != Derived::ShapeType::dims() &&
            T::ShapeType::dims().size() == Derived::ShapeType::dims().size())
        constexpr void eval(TensorAssignableExpression<T, S>& dest, U&& def) const {
            using DestShape = typename T::ShapeType;
            using SrcShape = typename Derived::ShapeType;
            size_t i = 0;
            Math::Detail::clampedRecursiveLoop<0, DestShape::dims().size() - 1, 
                DestShape, SrcShape>(
                [&]<typename...Indices>(Indices&&... idx){
                    auto* src = data() + SrcShape::index(std::forward<Indices>(idx)..., 0);
                    auto* dst = dest.data() + i;
                    if constexpr(SrcShape::dims().back() < DestShape::dims().back()) {
                        std::copy(src, src + SrcShape::dims().back(), dst);
                        std::fill(dst + SrcShape::dims().back(), dst + DestShape::dims().back(), std::forward<U>(def));
                    }else {
                        std::copy(src, src + DestShape::dims().back(), dst);
                    }
                    i += DestShape::dims().back();
                },
                [&]<typename...Indices>(Indices&&...){
                    auto* dst = dest.data() + i;
                    std::fill(dst, dst + DestShape::dims().back(), std::forward<U>(def));
                    i += DestShape::dims().back();
                }
            );
        }
    };

    template<typename D, typename Storage>
    class TensorConstructibleExpression : public TensorAssignableExpression<D, Storage> {
    public:
        using NotIncludedInScalarArithmetic = void;
        using IsTensorExpression = void;
        using IsAssignableExpression = void;
        using Derived = D;
        using Base = TensorAssignableExpression<D, Storage>;

        using Base::operator=;
        using Base::Base;
        using Base::data;
        using Base::size;

        template<typename Der>
        requires(Der::ShapeType::dims().size() == Derived::ShapeType::dims().size())
        constexpr TensorConstructibleExpression(const TensorExpression<Der>& e) {
            e.eval(*this, typename Der::ValueType{});
        }

        template<typename Der, typename U>
        requires(Der::ShapeType::dims().size() == Derived::ShapeType::dims().size())
        constexpr TensorConstructibleExpression(const TensorExpression<Der>& e, U&& def) {
            e.eval(*this, std::forward<U>(def));
        }

        template<typename Ptr>
        explicit constexpr TensorConstructibleExpression(const Ptr* ptr, size_t size) {
            auto& derived = this->derived();
            std::copy(ptr, ptr + size, derived.data());
        }

        template<typename T>
        constexpr TensorConstructibleExpression(std::initializer_list<T> list) {
            if(list.size() < size()) {
                std::copy(list.begin(), list.end(), data());
            }
            else {
                std::copy(list.begin(), list.begin() + size(), data());
            }
        }

        template<typename T>
        constexpr TensorConstructibleExpression(std::span<T> arr) {
            if(arr.size() < size()) {
                std::copy(arr.begin(), arr.end(), data());
            }
            else {
                std::copy(arr.begin(), arr.begin() + size(), data());
            }
        }

        template<std::convertible_to<typename Derived::ValueType>... Args> 
        requires(sizeof...(Args) != 1 && sizeof...(Args) <= Derived::size() && ((!TensorExpr<Args>) && ...))
        constexpr TensorConstructibleExpression(Args&&... args) {
            size_t i = 0; ((data()[i++] = args), ...);
        }

        template<typename Arg>
        requires (!TensorExpr<Arg> && std::convertible_to<Arg, typename Derived::ValueType>)
        constexpr TensorConstructibleExpression(Arg&& arg) {
            std::fill(data(), data() + size(), std::forward<Arg>(arg));
        }

        template<typename T>
        requires(!TensorExpr<T> && Detail::VecLike<T, typename Derived::ValueType> && Derived::ShapeType::dims().size() == 1)
        explicit constexpr TensorConstructibleExpression(const T& vec) {
            static constexpr auto limit = std::min(Detail::vecLikeSize<T, typename Derived::ValueType>(), Derived::ShapeType::dims()[0]);
            Detail::assignFromVecLike<limit - 1>(data(), vec);
        }
    };
}