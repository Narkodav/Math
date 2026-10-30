#pragma once
#include <concepts>
#include <cmath>

namespace Math {
        template<typename T, size_t... dims>
    struct Shape;

    template<typename From, typename To>
    struct ConvertibleShapeExactT;

    template<typename FromValue, typename ToValue, size_t... FromDims, size_t... ToDims>
    requires(sizeof...(FromDims) == sizeof...(ToDims))
    struct ConvertibleShapeExactT<Shape<FromValue, FromDims...>, Shape<ToValue, ToDims...>> {
        static constexpr bool s_value = std::convertible_to<FromValue, ToValue> && 
            sizeof...(FromDims) == sizeof...(ToDims) && ((FromDims == ToDims) && ...);
    };

    template<typename From, typename To>
    concept ConvertibleShapeExact = ConvertibleShapeExactT<From, To>::s_value;

    template<typename From, typename To>
    struct ConvertibleShapeT;

    template<typename FromValue, typename ToValue, size_t... FromDims, size_t... ToDims>
    struct ConvertibleShapeT<Shape<FromValue, FromDims...>, Shape<ToValue, ToDims...>> {
        static constexpr bool s_value = std::convertible_to<FromValue, ToValue> && sizeof...(FromDims) == sizeof...(ToDims);
    };

    template<typename From, typename To>
    concept ConvertibleShape = ConvertibleShapeT<From, To>::s_value;

    template<typename DerivedT, typename BaseT, typename ShapeT, auto... Params>
    struct TensorCRTPTraits {
        using Derived = DerivedT;
        using Base = BaseT;
        using ShapeType = ShapeT;
        using ValueType = typename ShapeType::ValueType;
        static constexpr std::tuple<decltype(Params)...> s_params = { Params... };
    };

    template<typename ShapeT, size_t Alignment = alignof(typename ShapeT::ValueType)>
    class alignas(Alignment) TensorBase;

    template<typename D, typename Storage>
    class TensorAssignableExpression;

    template<typename D>
    class TensorExpression;

    template<typename Shape>
    class ScalarExpression;

    template<typename T>
    concept TensorAssignableExpressionConcept = requires() {
        std::same_as<typename T::TensorAssignableExpression, void>;
    };

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
}