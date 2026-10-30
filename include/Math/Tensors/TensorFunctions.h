#pragma once
#include <stdexcept>
#include <cassert>
#include <concepts>
#include <cmath>

#include "Math/Tensors/TensorOps.h"

namespace Math {
    // Arithmetic operations
    template<typename L, typename R>
    constexpr auto operator+(const TensorExpression<L>& lhs, const TensorExpression<R>& rhs) {
        return TensorBinaryExpression<L, R, BinaryOpType::Add>(lhs.derived(), rhs.derived());
    }

    template<typename L, typename R>
    constexpr auto operator-(const TensorExpression<L>& lhs, const TensorExpression<R>& rhs) {
        return TensorBinaryExpression<L, R, BinaryOpType::Subtract>(lhs.derived(), rhs.derived());
    }

    template<typename L, typename R>
    constexpr auto operator*(const TensorExpression<L>& lhs, const TensorExpression<R>& rhs) {
        return TensorBinaryExpression<L, R, BinaryOpType::Multiply>(lhs.derived(), rhs.derived());
    }

    template<typename L, typename R>
    constexpr auto operator/(const TensorExpression<L>& lhs, const TensorExpression<R>& rhs) {
        return TensorBinaryExpression<L, R, BinaryOpType::Divide>(lhs.derived(), rhs.derived());
    }

    template<typename L, typename R>
    constexpr auto operator%(const TensorExpression<L>& lhs, const TensorExpression<R>& rhs) {
        return TensorBinaryExpression<L, R, BinaryOpType::Modulo>(lhs.derived(), rhs.derived());
    }

    template<typename L, typename R>
    constexpr decltype(auto) operator+=(L&& lhs, const TensorExpression<R>& rhs) {
        lhs = lhs + rhs;
        return std::forward<L>(lhs);
    }

    template<typename L, typename R>
    constexpr decltype(auto) operator-=(L&& lhs, const TensorExpression<R>& rhs) {
        lhs = lhs - rhs;
        return std::forward<L>(lhs);
    }

    template<typename L, typename R>
    constexpr decltype(auto) operator*=(L&& lhs, const TensorExpression<R>& rhs) {
        lhs = lhs * rhs;
        return std::forward<L>(lhs);
    }

    template<typename L, typename R>
    constexpr decltype(auto) operator/=(L&& lhs, const TensorExpression<R>& rhs) {
        lhs = lhs / rhs;
        return std::forward<L>(lhs);
    }

    template<typename L, typename R>
    constexpr decltype(auto) operator%=(L&& lhs, const TensorExpression<R>& rhs) {
        lhs = lhs % rhs;
        return std::forward<L>(lhs);
    }

    template<typename E>
    constexpr auto operator+(const TensorExpression<E>& expr) {
        return expr.derived();
    }

    template<typename E>
    constexpr auto operator-(const TensorExpression<E>& expr) {
        return TensorUnaryExpression<E, UnaryOpType::Negate>(expr.derived());
    }

    // Pre increment and decrement
    template<TensorAssignableExpressionConcept E>
    constexpr auto operator++(TensorExpression<E>& expr) {
        expr.forEach([&]<typename... Indices>(size_t i, Indices&&... idx){
            ++expr.derived().eval(i, std::forward<Indices>(idx)...);
        });
        return expr.derived();
    }

    template<TensorAssignableExpressionConcept E>
    constexpr auto operator--(TensorExpression<E>& expr) {
        expr.forEach([&]<typename... Indices>(size_t i, Indices&&... idx){
            --expr.derived().eval(i, std::forward<Indices>(idx)...);
        });
        return expr.derived();
    }

    // Post increment and decrement
    template<TensorAssignableExpressionConcept E>
    constexpr auto operator++(TensorExpression<E>& expr, int) {
        auto copy = expr.eval();
        ++expr;
        return copy;
    }

    template<TensorAssignableExpressionConcept E>
    constexpr auto operator--(TensorExpression<E>& expr, int) {
        auto copy = expr.eval();
        --expr;
        return copy;
    }

    // Bitwise operations
    template<typename L, typename R>
    constexpr auto operator&(const TensorExpression<L>& lhs, const TensorExpression<R>& rhs) {
        return TensorBinaryExpression<L, R, BinaryOpType::BitwiseAnd>(lhs.derived(), rhs.derived());
    }

    template<typename L, typename R>
    constexpr auto operator|(const TensorExpression<L>& lhs, const TensorExpression<R>& rhs) {
        return TensorBinaryExpression<L, R, BinaryOpType::BitwiseOr>(lhs.derived(), rhs.derived());
    }

    template<typename L, typename R>
    constexpr auto operator^(const TensorExpression<L>& lhs, const TensorExpression<R>& rhs) {
        return TensorBinaryExpression<L, R, BinaryOpType::BitwiseXor>(lhs.derived(), rhs.derived());
    }

    template<typename L, typename R>
    constexpr auto operator<<(const TensorExpression<L>& lhs, const TensorExpression<R>& rhs) {
        return TensorBinaryExpression<L, R, BinaryOpType::LeftShift>(lhs.derived(), rhs.derived());
    }

    template<typename L, typename R>
    constexpr auto operator>>(const TensorExpression<L>& lhs, const TensorExpression<R>& rhs) {
        return TensorBinaryExpression<L, R, BinaryOpType::RightShift>(lhs.derived(), rhs.derived());
    }

    template<typename T>
    constexpr auto operator~(const TensorExpression<T>& expr) {
        return TensorUnaryExpression<T, UnaryOpType::BitwiseNegate>(expr.derived());
    }

    template<typename L, typename R>
    constexpr decltype(auto) operator&=(L&& lhs, const TensorExpression<R>& rhs) {
        lhs = lhs & rhs;
        return std::forward<L>(lhs);
    }

    template<typename L, typename R>
    constexpr decltype(auto) operator|=(L&& lhs, const TensorExpression<R>& rhs) {
        lhs = lhs | rhs;
        return std::forward<L>(lhs);
    }

    template<typename L, typename R>
    constexpr decltype(auto) operator^=(L&& lhs, const TensorExpression<R>& rhs) {
        lhs = lhs ^ rhs;
        return std::forward<L>(lhs);
    }

    template<typename L, typename R>
    constexpr decltype(auto) operator<<=(L&& lhs, const TensorExpression<R>& rhs) {
        lhs = lhs << rhs;
        return std::forward<L>(lhs);
    }

    template<typename L, typename R>
    constexpr decltype(auto) operator>>=(L&& lhs, const TensorExpression<R>& rhs) {
        lhs = lhs >> rhs;
        return std::forward<L>(lhs);
    }

    // Logical operations
    template<typename L, typename R>
    constexpr auto operator&&(const TensorExpression<L>& lhs, const TensorExpression<R>& rhs) {
        return TensorBinaryExpression<L, R, BinaryOpType::And>(lhs.derived(), rhs.derived());
    }

    template<typename L, typename R>
    constexpr auto operator||(const TensorExpression<L>& lhs, const TensorExpression<R>& rhs) {
        return TensorBinaryExpression<L, R, BinaryOpType::Or>(lhs.derived(), rhs.derived());
    }

    template<typename L, typename R>
    constexpr auto operator==(const TensorExpression<L>& lhs, const TensorExpression<R>& rhs) {
        return TensorBinaryExpression<L, R, BinaryOpType::Equal>(lhs.derived(), rhs.derived());
    }

    template<typename L, typename R>
    constexpr auto operator!=(const TensorExpression<L>& lhs, const TensorExpression<R>& rhs) {
        return TensorBinaryExpression<L, R, BinaryOpType::NotEqual>(lhs.derived(), rhs.derived());
    }

    template<typename L, typename R>
    constexpr auto operator<(const TensorExpression<L>& lhs, const TensorExpression<R>& rhs) {
        return TensorBinaryExpression<L, R, BinaryOpType::Less>(lhs.derived(), rhs.derived());
    }

    template<typename L, typename R>
    constexpr auto operator<=(const TensorExpression<L>& lhs, const TensorExpression<R>& rhs) {
        return TensorBinaryExpression<L, R, BinaryOpType::LessOrEqual>(lhs.derived(), rhs.derived());
    }

    template<typename L, typename R>
    constexpr auto operator>(const TensorExpression<L>& lhs, const TensorExpression<R>& rhs) {
        return TensorBinaryExpression<L, R, BinaryOpType::Greater>(lhs.derived(), rhs.derived());
    }

    template<typename L, typename R>
    constexpr auto operator>=(const TensorExpression<L>& lhs, const TensorExpression<R>& rhs) {
        return TensorBinaryExpression<L, R, BinaryOpType::GreaterOrEqual>(lhs.derived(), rhs.derived());
    }

    template<typename T>
    constexpr auto operator!(const TensorExpression<T>& expr) {
        return TensorUnaryExpression<T, UnaryOpType::Not>(expr.derived());
    }

    // Scalar operations
    template<typename T, typename = void>
    struct isScalarArithmeticOperand : std::true_type {};

    template<typename T>
    struct isScalarArithmeticOperand<
        T,
        std::void_t<typename std::remove_cvref_t<T>::NotIncludedInScalarArithmetic>
    > : std::false_type {};

    template<typename T>
    concept ScalarArithmeticOperand = isScalarArithmeticOperand<T>::value;

    template<typename NewType, typename OtherShape>
    struct ChangeShapeType;

    template<typename NewType, typename T, size_t... dims>
    struct ChangeShapeType<NewType, Shape<T, dims...>> {
        using Type = Shape<NewType, dims...>;
        using PriorType = T;
    };

    template<typename T, typename E>
    using ScalarExprT = ScalarExpression<typename E::ShapeType::template ConvertedSelf<T>>;

    // Arithmetic scalar operations
    template<typename L, ScalarArithmeticOperand T>
    constexpr auto operator+(const TensorExpression<L>& lhs, const T& rhs) {
        return lhs.derived() + ScalarExprT<T, L>(rhs);
    }

    template<ScalarArithmeticOperand T, typename R>
    constexpr auto operator+(const T& lhs, const TensorExpression<R>& rhs) {
        return ScalarExprT<T, R>(lhs) + rhs.derived();
    }

    template<TensorAssignableExpressionConcept L, ScalarArithmeticOperand T>
    constexpr TensorExpression<L>& operator+=(TensorExpression<L>& lhs, const T& rhs) {
        lhs = lhs + rhs;
        return lhs;
    }

    template<typename L, ScalarArithmeticOperand T>
    constexpr auto operator-(const TensorExpression<L>& lhs, const T& rhs) {
        return lhs - ScalarExprT<T, L>(rhs);
    }

    template<ScalarArithmeticOperand T, typename R>
    constexpr auto operator-(const T& lhs, const TensorExpression<R>& rhs) {
        return ScalarExprT<T, R>(lhs) - rhs;
    }

    template<TensorAssignableExpressionConcept L, ScalarArithmeticOperand T>
    constexpr TensorExpression<L>& operator-=(TensorExpression<L>& lhs, const T& rhs) {
        lhs = lhs - rhs;
        return lhs;
    }

    template<typename L, ScalarArithmeticOperand T>
    constexpr auto operator*(const TensorExpression<L>& lhs, const T& rhs) {
        return lhs.derived() * ScalarExprT<T, L>(rhs);
    }

    template<ScalarArithmeticOperand T, typename R>
    constexpr auto operator*(const T& lhs, const TensorExpression<R>& rhs) {
        return ScalarExprT<T, R>(lhs) * rhs.derived();
    }

    template<TensorAssignableExpressionConcept L, ScalarArithmeticOperand T>
    constexpr TensorExpression<L>& operator*=(TensorExpression<L>& lhs, const T& rhs) {
        lhs = lhs * rhs;
        return lhs;
    }

    template<typename L, ScalarArithmeticOperand T>
    constexpr auto operator/(const TensorExpression<L>& lhs, const T& rhs) {
        return lhs.derived() / ScalarExprT<T, L>(rhs);
    }

    template<ScalarArithmeticOperand T, typename R>
    constexpr auto operator/(const T& lhs, const TensorExpression<R>& rhs) {
        return ScalarExprT<T, R>(lhs) / rhs.derived();
    }

    template<TensorAssignableExpressionConcept L, ScalarArithmeticOperand T>
    constexpr TensorExpression<L>& operator/=(TensorExpression<L>& lhs, const T& rhs) {
        lhs = lhs / rhs;
        return lhs;
    }

    template<typename L, ScalarArithmeticOperand T>
    constexpr auto operator%(const TensorExpression<L>& lhs, const T& rhs) {
        return lhs % ScalarExprT<T, L>(rhs);
    }

    template<ScalarArithmeticOperand T, typename R>
    constexpr auto operator%(const T& lhs, const TensorExpression<R>& rhs) {
        return ScalarExprT<T, R>(lhs) % rhs;
    }

    template<TensorAssignableExpressionConcept L, ScalarArithmeticOperand T>
    constexpr TensorExpression<L>& operator%=(TensorExpression<L>& lhs, const T& rhs) {
        lhs = lhs % rhs;
        return lhs;
    }

    //Bitwise scalar operations
    template<typename L, ScalarArithmeticOperand T>
    constexpr auto operator&(const TensorExpression<L>& lhs, const T& rhs) {
        return lhs & ScalarExprT<T, L>(rhs);
    }

    template<ScalarArithmeticOperand T, typename R>
    constexpr auto operator&(const T& lhs, const TensorExpression<R>& rhs) {
        return ScalarExprT<T, R>(lhs) & rhs;
    }

    template<TensorAssignableExpressionConcept L, ScalarArithmeticOperand T>
    constexpr TensorExpression<L>& operator&=(TensorExpression<L>& lhs, const T& rhs) {
        lhs = lhs & rhs;
        return lhs;
    }

    template<typename L, ScalarArithmeticOperand T>
    constexpr auto operator|(const TensorExpression<L>& lhs, const T& rhs) {
        return lhs | ScalarExprT<T, L>(rhs);
    }

    template<ScalarArithmeticOperand T, typename R>
    constexpr auto operator|(const T& lhs, const TensorExpression<R>& rhs) {
        return ScalarExprT<T, R>(lhs) | rhs;
    }

    template<TensorAssignableExpressionConcept L, ScalarArithmeticOperand T>
    constexpr TensorExpression<L>& operator|=(TensorExpression<L>& lhs, const T& rhs) {
        lhs = lhs | rhs;
        return lhs;
    }

    template<typename L, ScalarArithmeticOperand T>
    constexpr auto operator^(const TensorExpression<L>& lhs, const T& rhs) {
        return lhs ^ ScalarExprT<T, L>(rhs);
    }

    template<ScalarArithmeticOperand T, typename R>
    constexpr auto operator^(const T& lhs, const TensorExpression<R>& rhs) {
        return ScalarExprT<T, R>(lhs) ^ rhs;
    }

    template<TensorAssignableExpressionConcept L, ScalarArithmeticOperand T>
    constexpr TensorExpression<L>& operator^=(TensorExpression<L>& lhs, const T& rhs) {
        lhs = lhs ^ rhs;
        return lhs;
    }

    template<typename L, ScalarArithmeticOperand T>
    constexpr auto operator<<(const TensorExpression<L>& lhs, const T& rhs) {
        return lhs << ScalarExprT<T, L>(rhs);
    }

    template<ScalarArithmeticOperand T, typename R>
    constexpr auto operator<<(const T& lhs, const TensorExpression<R>& rhs) {
        return ScalarExprT<T, R>(lhs) << rhs;
    }

    template<TensorAssignableExpressionConcept L, ScalarArithmeticOperand T>
    constexpr TensorExpression<L>& operator<<=(TensorExpression<L>& lhs, const T& rhs) {
        lhs = lhs << rhs;
        return lhs;
    }

    template<typename L, ScalarArithmeticOperand T>
    constexpr auto operator>>(const TensorExpression<L>& lhs, const T& rhs) {
        return lhs >> ScalarExprT<T, L>(rhs);
    }

    template<ScalarArithmeticOperand T, typename R>
    constexpr auto operator>>(const T& lhs, const TensorExpression<R>& rhs) {
        return ScalarExprT<T, R>(lhs) >> rhs;
    }

    template<TensorAssignableExpressionConcept L, ScalarArithmeticOperand T>
    constexpr TensorExpression<L>& operator>>=(TensorExpression<L>& lhs, const T& rhs) {
        lhs = lhs >> rhs;
        return lhs;
    }

    // Logical scalar operations
    template<typename L, ScalarArithmeticOperand T>
    constexpr auto operator&&(const TensorExpression<L>& lhs, const T& rhs) {
        return lhs && ScalarExprT<T, L>(rhs);
    }

    template<ScalarArithmeticOperand T, typename R>
    constexpr auto operator&&(const T& lhs, const TensorExpression<R>& rhs) {
        return ScalarExprT<T, R>(lhs) && rhs;
    }

    template<typename L, ScalarArithmeticOperand T>
    constexpr auto operator||(const TensorExpression<L>& lhs, const T& rhs) {
        return lhs || ScalarExprT<T, L>(rhs);
    }

    template<ScalarArithmeticOperand T, typename R>
    constexpr auto operator||(const T& lhs, const TensorExpression<R>& rhs) {
        return ScalarExprT<T, R>(lhs) || rhs;
    }

    template<typename L, ScalarArithmeticOperand T>
    constexpr auto operator==(const TensorExpression<L>& lhs, const T& rhs) {
        return lhs == ScalarExprT<T, L>(rhs);
    }

    template<ScalarArithmeticOperand T, typename R>
    constexpr auto operator==(const T& lhs, const TensorExpression<R>& rhs) {
        return ScalarExprT<T, R>(lhs) == rhs;
    }
    
    template<typename L, ScalarArithmeticOperand T>
    constexpr auto operator!=(const TensorExpression<L>& lhs, const T& rhs) {
        return lhs != ScalarExprT<T, L>(rhs);
    }

    template<ScalarArithmeticOperand T, typename R>
    constexpr auto operator!=(const T& lhs, const TensorExpression<R>& rhs) {
        return ScalarExprT<T, R>(lhs) != rhs;
    }
    
    template<typename L, ScalarArithmeticOperand T>
    constexpr auto operator<(const TensorExpression<L>& lhs, const T& rhs) {
        return lhs < ScalarExprT<T, L>(rhs);
    }

    template<ScalarArithmeticOperand T, typename R>
    constexpr auto operator<(const T& lhs, const TensorExpression<R>& rhs) {
        return ScalarExprT<T, R>(lhs) < rhs;
    }

    template<typename L, ScalarArithmeticOperand T>
    constexpr auto operator<=(const TensorExpression<L>& lhs, const T& rhs) {
        return lhs <= ScalarExprT<T, L>(rhs);
    }

    template<ScalarArithmeticOperand T, typename R>
    constexpr auto operator<=(const T& lhs, const TensorExpression<R>& rhs) {
        return ScalarExprT<T, R>(lhs) <= rhs;
    }

    template<typename L, ScalarArithmeticOperand T>
    constexpr auto operator>(const TensorExpression<L>& lhs, const T& rhs) {
        return lhs > ScalarExprT<T, L>(rhs);
    }

    template<ScalarArithmeticOperand T, typename R>
    constexpr auto operator>(const T& lhs, const TensorExpression<R>& rhs) {
        return ScalarExprT<T, R>(lhs) > rhs;
    }

    template<typename L, ScalarArithmeticOperand T>
    constexpr auto operator>=(const TensorExpression<L>& lhs, const T& rhs) {
        return lhs >= ScalarExprT<T, L>(rhs);
    }

    template<ScalarArithmeticOperand T, typename R>
    constexpr auto operator>=(const T& lhs, const TensorExpression<R>& rhs) {
        return ScalarExprT<T, R>(lhs) >= rhs;
    }

    // Functions
    template<typename Result = float, VecExpr L, VecExpr R>
    requires (L::ShapeType::dims() == R::ShapeType::dims())
    constexpr Result dot(const TensorExpression<L>& lhs, const TensorExpression<R>& rhs) {
        Result result{};
        if(&lhs == &rhs) {
            lhs.forEach([&]<typename... Indices>(size_t i, Indices&&... idx){
                auto val = lhs.eval(i, std::forward<Indices>(idx)...);
                result += val * val;
            });
        }
        else {
            lhs.forEach([&]<typename... Indices>(size_t i, Indices&&... idx){
                result += lhs.eval(i, std::forward<Indices>(idx)...) * rhs.eval(i, std::forward<Indices>(idx)...);
            });
        }
        return result;
    }

    template<typename Result = float, VecExpr T>
    constexpr Result length(const TensorExpression<T>& expr) {
        return std::sqrt(dot<Result>(expr, expr));
    }

    template<VecExpr T>
    constexpr decltype(auto) normalize(const TensorExpression<T>& expr) {
        decltype(auto) e = expr.eval();
        return (e / length(e)).eval();
    }

    template<typename Result = float, VecExpr A, VecExpr B>
    requires (A::ShapeType::dims() == B::ShapeType::dims())
    constexpr Result distance(const TensorExpression<A>& a, const TensorExpression<B>& b) {
        if(&a == &b) return Result{};
        return length<Result>(b - a);
    }

    template<Vec3Expr L, Vec3Expr R>
    constexpr decltype(auto) cross(const TensorExpression<L>& lhs, const TensorExpression<R>& rhs) {
        using ValueTypeL = typename L::ValueType;
        using ValueTypeR = typename R::ValueType;
        using MultResultType = decltype(ValueTypeL{} * ValueTypeR{});
        using SubtractResultType = decltype(MultResultType{} - MultResultType{});
        using ValueType = SubtractResultType;
        TensorBase<Shape<ValueType, 3>> result;
        decltype(auto) l = lhs.eval();
        decltype(auto) r = rhs.eval();

        result[0] = l[2] * r[3] - l[3] * r[2];
        result[1] = l[3] * r[1] - l[1] * r[3];
        result[2] = l[1] * r[2] - l[2] * r[1];

        return result;
    }

    template<VecExpr V, VecExpr N>
    requires (V::ShapeType::dims() == N::ShapeType::dims())
    constexpr decltype(auto) reflect(const TensorExpression<V>& v, const TensorExpression<N>& n) {
        decltype(auto) vec = v.eval();
        decltype(auto) nor = n.eval();
        return (vec - 2 * dot(vec, nor) * nor).eval();
    }

    template<VecExpr V, VecExpr N>
    requires (V::ShapeType::dims() == N::ShapeType::dims())
    constexpr decltype(auto) refract(const TensorExpression<V>& v, const TensorExpression<N>& n, float eta) {
        decltype(auto) vec = v.eval();
        decltype(auto) nor = n.eval();
        auto dotProd = dot(vec, nor);
        auto etaSquared = eta * eta;
        return (vec * eta - (eta * dotProd + std::sqrt(1 - etaSquared + etaSquared * dotProd * dotProd)) * nor).eval();
    }

    // Project A on B
    template<VecExpr A, VecExpr B>
    requires (A::ShapeType::dims() == B::ShapeType::dims())
    constexpr decltype(auto) project(const TensorExpression<A>& a, const TensorExpression<B>& b) {
        decltype(auto) ae = a.eval();
        decltype(auto) be = b.eval();
        return ((dot(ae, be) / dot(ae, be)) * be).eval();
    }

    template<VecExpr A, VecExpr B>
    requires (A::ShapeType::dims() == B::ShapeType::dims())
    constexpr decltype(auto) angle(const TensorExpression<A>& a, const TensorExpression<B>& b) {
        decltype(auto) ae = a.eval();
        decltype(auto) be = b.eval();
        return std::acos(dot(ae,be)/(length(ae) * length(be)));
    }

    template<typename T>
    constexpr decltype(auto) sum(const TensorExpression<T>& expr) {
        typename T::ValueType result{};
        expr.forEach([&]<typename... Indices>(size_t i, Indices&&... idx){
            result += expr.eval(i, std::forward<Indices>(idx)...);
        });
        return result;
    }

    template<typename T>
    constexpr decltype(auto) product(const TensorExpression<T>& expr) {
        typename T::ValueType result{};
        expr.forEach([&]<typename... Indices>(size_t i, Indices&&... idx){
            result *= expr.eval(i, std::forward<Indices>(idx)...);
        });
        return result;
    }

    template<typename T>
    constexpr decltype(auto) min(const TensorExpression<T>& expr) {
        typename T::ValueType min = std::numeric_limits<typename T::ValueType>::max();
        expr.forEach([&]<typename... Indices>(size_t i, Indices&&... idx){
            min = std::min(min, expr.eval(i, std::forward<Indices>(idx)...));
        });
        return min;
    }

    template<typename T>
    constexpr decltype(auto) max(const TensorExpression<T>& expr) {
        typename T::ValueType max = std::numeric_limits<typename T::ValueType>::min();
        expr.forEach([&]<typename... Indices>(size_t i, Indices&&... idx){
            max = std::max(max, expr.eval(i, std::forward<Indices>(idx)...));
        });
        return max;
    }

    template<typename L, typename R>
    constexpr decltype(auto) min(const TensorExpression<L>& lhs, const TensorExpression<R>& rhs) {
        return TensorBinaryExpression<L, R, BinaryOpType::Min>(lhs.derived(), rhs.derived());
    }

    template<typename L, typename R>
    constexpr decltype(auto) max(const TensorExpression<L>& lhs, const TensorExpression<R>& rhs) {
        return TensorBinaryExpression<L, R, BinaryOpType::Max>(lhs.derived(), rhs.derived());
    }

    template<typename L, typename R>
    constexpr decltype(auto) mul(const TensorExpression<L>& lhs, const TensorExpression<R>& rhs) {
        return TensorBinaryExpression<L, R, BinaryOpType::MatrixMult>(lhs.derived(), rhs.derived());
    }

    template<SquareMatExpr T>
    constexpr decltype(auto) transpose(const TensorExpression<T>& expr) {
        TensorBase<Shape<typename T::ValueType, T::ShapeType::dims()[1], T::ShapeType::dims()[0]>> result;
        size_t global = 0;
        for(size_t rowIndex = 0; rowIndex < T::ShapeType::dims()[1]; ++rowIndex) {
            for(size_t colIndex = 0; colIndex < T::ShapeType::dims()[0]; ++colIndex) {
                result(colIndex, rowIndex) = expr.eval(global, rowIndex, colIndex);
                ++global;
            }
        }
        return result;
    }

    template<typename T>
    constexpr decltype(auto) cos(const TensorExpression<T>& expr) {
        return TensorUnaryExpression<T, UnaryOpType::Cos>(expr.derived());
    }

    template<typename T>
    constexpr decltype(auto) sin(const TensorExpression<T>& expr) {
        return TensorUnaryExpression<T, UnaryOpType::Sin>(expr.derived());
    }

    template<typename T>
    constexpr decltype(auto) abs(const TensorExpression<T>& expr) {
        return TensorUnaryExpression<T, UnaryOpType::Abs>(expr.derived());
    }

    template<typename T>
    constexpr decltype(auto) all(const TensorExpression<T>& expr) {
        bool result = true;
        expr.forEach([&]<typename... Indices>(size_t i, Indices&&... idx){
            result = result && expr.eval(i, std::forward<Indices>(idx)...);
        });
        return result;
    }

    template<typename T>
    constexpr decltype(auto) none(const TensorExpression<T>& expr) {
        bool result = false;
        expr.forEach([&]<typename... Indices>(size_t i, Indices&&... idx){
            result = result || expr.eval(i, std::forward<Indices>(idx)...);
        });
        return !result;
    }

    template<typename L, typename R, typename T>
    constexpr decltype(auto) epsilonEqual(const TensorExpression<L>& lhs, const TensorExpression<R>& rhs, T&& epsilon) {
        return Math::abs(lhs - rhs) < epsilon;
    }
    
}