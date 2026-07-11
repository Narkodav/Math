#pragma once
#include <stdexcept>
#include <cassert>
#include <concepts>
#include <array>

namespace Math {
    template<typename T, size_t... dims>
    struct Shape;

    template<typename ShapeT, size_t Alignment = alignof(typename ShapeT::ValueType)>
    class alignas(Alignment) TensorBase;

    template<typename D>
    class TensorExpression {
    public:
        using Derived = D;
        using NotIncludedInScalarArithmetic = void;

        constexpr const Derived& derived() const {
            return static_cast<const Derived&>(*this);
        }

        static constexpr size_t size() {
            return Derived::size();
        }

        template<typename... Indices>
        constexpr auto eval(size_t global, Indices&&... Idx) const {
            return derived().eval(global, std::forward<Indices>(Idx)...);
        }

        constexpr auto eval() const {
            return derived().eval();
        }

        template<typename... Indices>
        constexpr auto evalCoord(Indices&&... Idx) const {
            auto index = D::ShapeType::index(std::forward<Indices>(Idx)...);
            return derived().eval(index, std::forward<Indices>(Idx)...);
        }
    };

    namespace Detail {
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
    }

    template<typename Derived>
    class TensorArithmeticExpression : public TensorExpression<Derived> {
    public:
        using NotIncludedInScalarArithmetic = void;
    };

    template<typename Shape>
    class ScalarExpression : public TensorArithmeticExpression<ScalarExpression<Shape>> {
    public:
        using NotIncludedInScalarArithmetic = void;
        using ShapeType = Shape;
        using ValueType = typename ShapeType::ValueType;

    private:
        ValueType m_value;

    public:
        constexpr operator ValueType() const { return m_value; }
        static constexpr size_t size() { return ShapeType::size(); }
        constexpr ScalarExpression(ValueType value) : m_value(value) {}
        template<typename... Indices>
        constexpr auto eval(size_t, Indices&&...) const { return m_value; }
        constexpr auto eval() const { return m_value; }
    };

    template<typename D>
    class TensorWritableExpression : public TensorExpression<D> {
    public:
        using NotIncludedInScalarArithmetic = void;
        using Derived = D;

        template<typename Der>
        requires(Der::ShapeType::dims().size() == Derived::ShapeType::dims().size() &&
        Der::ShapeType::dims() != Derived::ShapeType::dims())
        constexpr Derived& operator=(const TensorExpression<Der>& e) {
            if constexpr (std::same_as<Der, Derived>) {
                if(&e == this) return derived();
            }
            using ValueType = typename Derived::ValueType;
            decltype(auto) expr = e.eval();
            size_t i = 0;
            Detail::clampedRecursiveLoop<0, Derived::ShapeType::dims().size() - 1, 
                typename Derived::ShapeType, typename Der::ShapeType>(
                [&]<typename...Indices>(Indices&&... idx){
                    auto* src = expr.data() + Der::ShapeType::index(std::forward<Indices>(idx)..., 0);
                    auto* dst = data() + i;
                    if constexpr(Der::ShapeType::dims().back() < Derived::ShapeType::dims().back()) {
                        std::copy(src, src + Der::ShapeType::dims().back(), dst);
                        std::fill(dst + Der::ShapeType::dims().back(), 
                            dst + Derived::ShapeType::dims().back(), 
                            ValueType{});
                    }else {
                        std::copy(src, src + Derived::ShapeType::dims().back(), dst);
                    }
                    i += Derived::ShapeType::dims().back();
                },
                [&]<typename...Indices>(Indices&&...){
                    auto* dst = data() + i;
                    std::fill(dst, dst + Derived::ShapeType::dims().back(), ValueType{});
                    i += Derived::ShapeType::dims().back();
                }
            );
            return derived();
        }

        template<typename Der>
        requires(Der::ShapeType::dims() == Derived::ShapeType::dims())
        constexpr Derived& operator=(const TensorExpression<Der>& e) {
            if constexpr (std::same_as<Der, Derived>) {
                if(&e == this) return derived();
            }
            decltype(auto) expr = e.eval();
            std::copy(expr.data(), expr.data() + size(), data());
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

        template<std::convertible_to<typename Derived::ValueType> Arg> 
        constexpr Derived& operator=(Arg&& arg) {
            std::fill(data(), data() + size(), std::forward<Arg>(arg));
            return derived();
        }

        // Non const access to derived
        constexpr Derived& derived() { return static_cast<Derived&>(*this); }
        constexpr const Derived& derived() const { return static_cast<const Derived&>(*this); }
        static constexpr size_t size() { return Derived::size(); }
        constexpr decltype(auto) data() { return this->derived().data(); }
        constexpr decltype(auto) data() const { return this->derived().data(); }

        constexpr const auto& eval() const { return derived(); }
        constexpr auto& eval() { return derived(); }

        template<typename... Indices>
        constexpr auto eval(size_t global, Indices&&...) const {
            return derived().data()[global];
        }

        template<typename... Indices>
        constexpr auto eval(size_t global, Indices&&...) {
            return derived().data()[global];
        }

        template<typename... I>
        constexpr auto operator()(I&&... coords) const {
            return derived().data()[D::ShapeType::index(std::forward<I>(coords)...)];
        }

        template<typename... I>
        constexpr auto operator()(I&&... coords) {
            return derived().data()[D::ShapeType::index(std::forward<I>(coords)...)];
        }
    };

    template<typename D>
    class TensorStorageExpression : public TensorWritableExpression<D> {
    public:
        using NotIncludedInScalarArithmetic = void;
        using Derived = D;

        constexpr TensorStorageExpression() = default;
        using TensorWritableExpression<D>::operator=;
        using TensorWritableExpression<D>::data;
        using TensorWritableExpression<D>::size;

        template<typename Der>
        requires(Der::ShapeType::dims().size() == Derived::ShapeType::dims().size() &&
        Der::ShapeType::dims() != Derived::ShapeType::dims())
        constexpr TensorStorageExpression(const TensorExpression<Der>& e) {
            using ValueType = typename Derived::ValueType;
            decltype(auto) expr = e.eval();
            size_t i = 0;
            Detail::clampedRecursiveLoop<0, Derived::ShapeType::dims().size() - 1, 
                typename Derived::ShapeType, typename Der::ShapeType>(
                [&]<typename...Indices>(Indices&&... idx){
                    
                    auto* src = expr.data() + Der::ShapeType::index(std::forward<Indices>(idx)..., 0);
                    auto* dst = data() + i;
                    if constexpr(Der::ShapeType::dims().back() < Derived::ShapeType::dims().back()) {
                        std::copy(src, src + Der::ShapeType::dims().back(), dst);
                        std::fill(dst + Der::ShapeType::dims().back(), 
                            dst + Derived::ShapeType::dims().back(), 
                            ValueType{});
                    }else {
                        std::copy(src, src + Derived::ShapeType::dims().back(), dst);
                    }
                    i += Derived::ShapeType::dims().back();
                },
                [&]<typename...Indices>(Indices&&...){
                    auto* dst = data() + i;
                    std::fill(dst, dst + Derived::ShapeType::dims().back(), ValueType{});
                    i += Derived::ShapeType::dims().back();
                }
            );
        }

        template<typename Der>
        requires(Der::ShapeType::dims() == Derived::ShapeType::dims())
        constexpr TensorStorageExpression(const TensorExpression<Der>& e) {
            decltype(auto) expr = e.eval();
            std::copy(expr.data(), expr.data() + size(), data());
        }

        template<typename Ptr>
        explicit constexpr TensorStorageExpression(const Ptr* ptr, size_t size) {
            auto& derived = this->derived();
            std::copy(ptr, ptr + size, derived.data());
        }

        template<typename T>
        constexpr TensorStorageExpression(std::initializer_list<T> list) {
            if(list.size() < size()) {
                std::copy(list.begin(), list.end(), data());
            }
            else {
                std::copy(list.begin(), list.begin() + size(), data());
            }
        }

        template<std::convertible_to<typename Derived::ValueType>... Args> 
        requires(sizeof...(Args) == Derived::size())
        constexpr TensorStorageExpression(Args&&... args) {
            size_t i = 0; ((data()[i++] = args), ...);
        }

        template<std::convertible_to<typename Derived::ValueType> Arg> 
        constexpr TensorStorageExpression(Arg&& arg) {
            std::fill(data(), data() + size(), std::forward<Arg>(arg));
        }
    };
}