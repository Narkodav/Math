#pragma once
#include <stdexcept>
#include <cassert>
#include <concepts>
#include <cmath>

#include "Math/Tensors/TensorBinaryOps.h"
#include "Math/Tensors/TensorUnaryOps.h"

namespace Math {

	template<typename V>
	concept VecExpr = (V::ShapeType::dims().size() == 1);

	template<typename V, size_t size>
	concept SizedVecExpr = (VecExpr<V> && V::ShapeType::dims()[0] == size);

	template<typename V>
	concept Vec1Expr = SizedVecExpr<V, 1>;

	template<typename V>
	concept Vec2Expr = SizedVecExpr<V, 2>;

	template<typename V>
	concept Vec3Expr = SizedVecExpr<V, 3>;

	template<typename V>
	concept Vec4Expr = SizedVecExpr<V, 4>;

	template<typename M>
	concept MatExpr = (M::ShapeType::dims().size() == 2);

	template<typename M>
	concept SquareMatExpr = (MatExpr<M> * M::ShapeType::dims()[0] == M::ShapeType::dims()[1]);

	template<typename V, size_t size>
	concept SizedSquareMatExpr = (SquareMatExpr<V> && V::ShapeType::dims()[0] == size);

	template<typename M>
	concept Mat1Expr = SizedSquareMatExpr<M, 1>;

	template<typename M>
	concept Mat2Expr = SizedSquareMatExpr<M, 2>;

	template<typename M>
	concept Mat3Expr = SizedSquareMatExpr<M, 3>;

	template<typename M>
	concept Mat4Expr = SizedSquareMatExpr<M, 4>;


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
    constexpr TensorWritableExpression<L>& operator+=(TensorWritableExpression<L>& lhs, const TensorExpression<R>& rhs) {
        lhs = lhs + rhs;
        return lhs;
    }

    template<typename L, typename R>
    constexpr TensorWritableExpression<L>& operator-=(TensorWritableExpression<L>& lhs, const TensorExpression<R>& rhs) {
        lhs = lhs - rhs;
        return lhs;
    }

    template<typename L, typename R>
    constexpr TensorWritableExpression<L>& operator*=(TensorWritableExpression<L>& lhs, const TensorExpression<R>& rhs) {
        lhs = lhs * rhs;
        return lhs;
    }

    template<typename L, typename R>
    constexpr TensorWritableExpression<L>& operator/=(TensorWritableExpression<L>& lhs, const TensorExpression<R>& rhs) {
        lhs = lhs / rhs;
        return lhs;
    }

    template<typename L, typename R>
    constexpr TensorWritableExpression<L>& operator%=(TensorWritableExpression<L>& lhs, const TensorExpression<R>& rhs) {
        lhs = lhs % rhs;
        return lhs;
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
    template<typename E>
    constexpr auto operator++(TensorWritableExpression<E>& expr) {
        for(size_t i = 0; i < expr.size(); ++i) ++expr.derived().data()[i];
        return expr.derived();
    }

    template<typename E>
    constexpr auto operator--(TensorWritableExpression<E>& expr) {
        for(size_t i = 0; i < expr.size(); ++i) --expr.derived().data()[i];
        return expr.derived();
    }

    // Post increment and decrement
    template<typename E>
    constexpr auto operator++(TensorWritableExpression<E>& expr, int) {
        auto copy = expr.eval();
        ++expr;
        return copy;
    }

    template<typename E>
    constexpr auto operator--(TensorWritableExpression<E>& expr, int) {
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
    constexpr TensorWritableExpression<L>& operator&=(TensorWritableExpression<L>& lhs, const TensorExpression<R>& rhs) {
        lhs = lhs & rhs;
        return lhs;
    }

    template<typename L, typename R>
    constexpr TensorWritableExpression<L>& operator|=(TensorWritableExpression<L>& lhs, const TensorExpression<R>& rhs) {
        lhs = lhs | rhs;
        return lhs;
    }

    template<typename L, typename R>
    constexpr TensorWritableExpression<L>& operator^=(TensorWritableExpression<L>& lhs, const TensorExpression<R>& rhs) {
        lhs = lhs ^ rhs;
        return lhs;
    }

    template<typename L, typename R>
    constexpr TensorWritableExpression<L>& operator<<=(TensorWritableExpression<L>& lhs, const TensorExpression<R>& rhs) {
        lhs = lhs << rhs;
        return lhs;
    }

    template<typename L, typename R>
    constexpr TensorWritableExpression<L>& operator>>=(TensorWritableExpression<L>& lhs, const TensorExpression<R>& rhs) {
        lhs = lhs >> rhs;
        return lhs;
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
    constexpr auto operator+(const TensorExpression<L>& lhs, T rhs) {
        return lhs + ScalarExprT<T, L>(rhs);
    }

    template<ScalarArithmeticOperand T, typename R>
    constexpr auto operator+(T lhs, const TensorExpression<R>& rhs) {
        return ScalarExprT<T, R>(lhs) + rhs;
    }

    template<typename L, ScalarArithmeticOperand T>
    constexpr TensorWritableExpression<L>& operator+=(TensorWritableExpression<L>& lhs, T rhs) {
        lhs = lhs + rhs;
        return lhs;
    }

    template<typename L, ScalarArithmeticOperand T>
    constexpr auto operator-(const TensorExpression<L>& lhs, T rhs) {
        return lhs - ScalarExprT<T, L>(rhs);
    }

    template<ScalarArithmeticOperand T, typename R>
    constexpr auto operator-(T lhs, const TensorExpression<R>& rhs) {
        return ScalarExprT<T, R>(lhs) - rhs;
    }

    template<typename L, ScalarArithmeticOperand T>
    constexpr TensorWritableExpression<L>& operator-=(TensorWritableExpression<L>& lhs, T rhs) {
        lhs = lhs - rhs;
        return lhs;
    }

    template<typename L, ScalarArithmeticOperand T>
    constexpr auto operator*(const TensorExpression<L>& lhs, T rhs) {
        return lhs * ScalarExprT<T, L>(rhs);
    }

    template<ScalarArithmeticOperand T, typename R>
    constexpr auto operator*(T lhs, const TensorExpression<R>& rhs) {
        return ScalarExprT<T, R>(lhs) * rhs;
    }

    template<typename L, ScalarArithmeticOperand T>
    constexpr TensorWritableExpression<L>& operator*=(TensorWritableExpression<L>& lhs, T rhs) {
        lhs = lhs * rhs;
        return lhs;
    }

    template<typename L, ScalarArithmeticOperand T>
    constexpr auto operator/(const TensorExpression<L>& lhs, T rhs) {
        return lhs / ScalarExprT<T, L>(rhs);
    }

    template<ScalarArithmeticOperand T, typename R>
    constexpr auto operator/(T lhs, const TensorExpression<R>& rhs) {
        return ScalarExprT<T, R>(lhs) / rhs;
    }

    template<typename L, ScalarArithmeticOperand T>
    constexpr TensorWritableExpression<L>& operator/=(TensorWritableExpression<L>& lhs, T rhs) {
        lhs = lhs / rhs;
        return lhs;
    }

    template<typename L, ScalarArithmeticOperand T>
    constexpr auto operator%(const TensorExpression<L>& lhs, T rhs) {
        return lhs % ScalarExprT<T, L>(rhs);
    }

    template<ScalarArithmeticOperand T, typename R>
    constexpr auto operator%(T lhs, const TensorExpression<R>& rhs) {
        return ScalarExprT<T, R>(lhs) % rhs;
    }

    template<typename L, ScalarArithmeticOperand T>
    constexpr TensorWritableExpression<L>& operator%=(TensorWritableExpression<L>& lhs, T rhs) {
        lhs = lhs % rhs;
        return lhs;
    }

    //Bitwise scalar operations
    template<typename L, ScalarArithmeticOperand T>
    constexpr auto operator&(const TensorExpression<L>& lhs, T rhs) {
        return lhs & ScalarExprT<T, L>(rhs);
    }

    template<ScalarArithmeticOperand T, typename R>
    constexpr auto operator&(T lhs, const TensorExpression<R>& rhs) {
        return ScalarExprT<T, R>(lhs) & rhs;
    }

    template<typename L, ScalarArithmeticOperand T>
    constexpr TensorWritableExpression<L>& operator&=(TensorWritableExpression<L>& lhs, T rhs) {
        lhs = lhs & rhs;
        return lhs;
    }

    template<typename L, ScalarArithmeticOperand T>
    constexpr auto operator|(const TensorExpression<L>& lhs, T rhs) {
        return lhs | ScalarExprT<T, L>(rhs);
    }

    template<ScalarArithmeticOperand T, typename R>
    constexpr auto operator|(T lhs, const TensorExpression<R>& rhs) {
        return ScalarExprT<T, R>(lhs) | rhs;
    }

    template<typename L, ScalarArithmeticOperand T>
    constexpr TensorWritableExpression<L>& operator|=(TensorWritableExpression<L>& lhs, T rhs) {
        lhs = lhs | rhs;
        return lhs;
    }

    template<typename L, ScalarArithmeticOperand T>
    constexpr auto operator^(const TensorExpression<L>& lhs, T rhs) {
        return lhs ^ ScalarExprT<T, L>(rhs);
    }

    template<ScalarArithmeticOperand T, typename R>
    constexpr auto operator^(T lhs, const TensorExpression<R>& rhs) {
        return ScalarExprT<T, R>(lhs) ^ rhs;
    }

    template<typename L, ScalarArithmeticOperand T>
    constexpr TensorWritableExpression<L>& operator^=(TensorWritableExpression<L>& lhs, T rhs) {
        lhs = lhs ^ rhs;
        return lhs;
    }

    template<typename L, ScalarArithmeticOperand T>
    constexpr auto operator<<(const TensorExpression<L>& lhs, T rhs) {
        return lhs << ScalarExprT<T, L>(rhs);
    }

    template<ScalarArithmeticOperand T, typename R>
    constexpr auto operator<<(T lhs, const TensorExpression<R>& rhs) {
        return ScalarExprT<T, R>(lhs) << rhs;
    }

    template<typename L, ScalarArithmeticOperand T>
    constexpr TensorWritableExpression<L>& operator<<=(TensorWritableExpression<L>& lhs, T rhs) {
        lhs = lhs << rhs;
        return lhs;
    }

    template<typename L, ScalarArithmeticOperand T>
    constexpr auto operator>>(const TensorExpression<L>& lhs, T rhs) {
        return lhs >> ScalarExprT<T, L>(rhs);
    }

    template<ScalarArithmeticOperand T, typename R>
    constexpr auto operator>>(T lhs, const TensorExpression<R>& rhs) {
        return ScalarExprT<T, R>(lhs) >> rhs;
    }

    template<typename L, ScalarArithmeticOperand T>
    constexpr TensorWritableExpression<L>& operator>>=(TensorWritableExpression<L>& lhs, T rhs) {
        lhs = lhs >> rhs;
        return lhs;
    }

    // Logical scalar operations
    template<typename L, ScalarArithmeticOperand T>
    constexpr auto operator&&(const TensorExpression<L>& lhs, T rhs) {
        return lhs && ScalarExprT<T, L>(rhs);
    }

    template<ScalarArithmeticOperand T, typename R>
    constexpr auto operator&&(T lhs, const TensorExpression<R>& rhs) {
        return ScalarExprT<T, R>(lhs) && rhs;
    }

    template<typename L, ScalarArithmeticOperand T>
    constexpr auto operator||(const TensorExpression<L>& lhs, T rhs) {
        return lhs || ScalarExprT<T, L>(rhs);
    }

    template<ScalarArithmeticOperand T, typename R>
    constexpr auto operator||(T lhs, const TensorExpression<R>& rhs) {
        return ScalarExprT<T, R>(lhs) || rhs;
    }

    template<typename L, ScalarArithmeticOperand T>
    constexpr auto operator==(const TensorExpression<L>& lhs, T rhs) {
        return lhs == ScalarExprT<T, L>(rhs);
    }

    template<ScalarArithmeticOperand T, typename R>
    constexpr auto operator==(T lhs, const TensorExpression<R>& rhs) {
        return ScalarExprT<T, R>(lhs) == rhs;
    }
    
    template<typename L, ScalarArithmeticOperand T>
    constexpr auto operator!=(const TensorExpression<L>& lhs, T rhs) {
        return lhs != ScalarExprT<T, L>(rhs);
    }

    template<ScalarArithmeticOperand T, typename R>
    constexpr auto operator!=(T lhs, const TensorExpression<R>& rhs) {
        return ScalarExprT<T, R>(lhs) != rhs;
    }
    
    template<typename L, ScalarArithmeticOperand T>
    constexpr auto operator<(const TensorExpression<L>& lhs, T rhs) {
        return lhs < ScalarExprT<T, L>(rhs);
    }

    template<ScalarArithmeticOperand T, typename R>
    constexpr auto operator<(T lhs, const TensorExpression<R>& rhs) {
        return ScalarExprT<T, R>(lhs) < rhs;
    }

    template<typename L, ScalarArithmeticOperand T>
    constexpr auto operator<=(const TensorExpression<L>& lhs, T rhs) {
        return lhs <= ScalarExprT<T, L>(rhs);
    }

    template<ScalarArithmeticOperand T, typename R>
    constexpr auto operator<=(T lhs, const TensorExpression<R>& rhs) {
        return ScalarExprT<T, R>(lhs) <= rhs;
    }

    template<typename L, ScalarArithmeticOperand T>
    constexpr auto operator>(const TensorExpression<L>& lhs, T rhs) {
        return lhs > ScalarExprT<T, L>(rhs);
    }

    template<ScalarArithmeticOperand T, typename R>
    constexpr auto operator>(T lhs, const TensorExpression<R>& rhs) {
        return ScalarExprT<T, R>(lhs) > rhs;
    }

    template<typename L, ScalarArithmeticOperand T>
    constexpr auto operator>=(const TensorExpression<L>& lhs, T rhs) {
        return lhs >= ScalarExprT<T, L>(rhs);
    }

    template<ScalarArithmeticOperand T, typename R>
    constexpr auto operator>=(T lhs, const TensorExpression<R>& rhs) {
        return ScalarExprT<T, R>(lhs) >= rhs;
    }

    // Functions
    template<typename Result = float, VecExpr L, VecExpr R>
    requires (L::ShapeType::dims() == R::ShapeType::dims())
    constexpr Result dot(const TensorExpression<L>& lhs, const TensorExpression<R>& rhs) {
        Result result{};
        if(&lhs == &rhs) {
            for(size_t i = 0; i < lhs.size(); ++i) {
                auto val = lhs.eval(i, i);
                result += val * val;
            }
        }
        else {
            for(size_t i = 0; i < lhs.size(); ++i) {
                result += lhs.eval(i, i) * rhs.eval(i, i);
            }
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
        for(size_t i = 0; i < expr.derived().size(); ++i) {
            result += expr.eval(i, i);
        }
        return result;
    }

    template<typename T>
    constexpr decltype(auto) product(const TensorExpression<T>& expr) {
        typename T::ValueType result = 1;
        for(size_t i = 0; i < expr.derived().size(); ++i) {
            result *= expr.eval(i, i);
        }
        return result;
    }

    template<typename T>
    constexpr decltype(auto) min(const TensorExpression<T>& expr) {
        auto min = std::numeric_limits<typename T::ValueType>::max();
        for(size_t i = 0; i < expr.derived().size(); ++i) {
            min = std::min(min, expr.eval(i, i));
        }
        return min;
    }

    template<typename T>
    constexpr decltype(auto) max(const TensorExpression<T>& expr) {
        auto max = std::numeric_limits<typename T::ValueType>::min();
        for(size_t i = 0; i < expr.derived().size(); ++i) {
            max = std::max(max, expr.eval(i, i));
        }
        return max;
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
        return TensorUnaryExpression<T, UnaryOpType::Cos>(expr);
    }

    template<typename T>
    constexpr decltype(auto) sin(const TensorExpression<T>& expr) {
        return TensorUnaryExpression<T, UnaryOpType::Sin>(expr);
    }
}