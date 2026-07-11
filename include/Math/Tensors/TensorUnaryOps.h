#pragma once
#include "Math/Tensors/TensorBase.h"

#include<cmath>

namespace Math {
    enum class UnaryOpType {
        // Arithmetic operations
        Negate,
        Cos,
        Sin,

        // Bitwise operations
        BitwiseNegate,

        // Logical operations
        Not,
    };

    template<UnaryOpType opType, typename T>
    struct UnaryOpTraits;

    // Arithmetic operations
    template<typename T>
    struct UnaryOpTraits<UnaryOpType::Negate, T> {
        using ShapeTypeT = typename T::ShapeType;
        using ValueTypeT = typename T::ValueType;

        static constexpr decltype(auto) apply(const ValueTypeT& val) { return -val; }

        using ValueType = decltype(apply(ValueTypeT{}));
        using ShapeType = ShapeTypeT::template ConvertedSelf<ValueType>;
        
        template<typename... Indices>
        constexpr decltype(auto) operator()(const T& expr, size_t global, Indices&&... Idx) { 
            return apply(expr.eval(global, std::forward<Indices>(Idx)...)); 
        }
    };

    template<typename T>
    struct UnaryOpTraits<UnaryOpType::Cos, T> {
        using ShapeTypeT = typename T::ShapeType;
        using ValueTypeT = typename T::ValueType;

        static constexpr decltype(auto) apply(const ValueTypeT& val) { return std::cos(val); }

        using ValueType = decltype(apply(ValueTypeT{}));
        using ShapeType = ShapeTypeT::template ConvertedSelf<ValueType>;
        
        template<typename... Indices>
        constexpr decltype(auto) operator()(const T& expr, size_t global, Indices&&... Idx) { 
            return apply(expr.eval(global, std::forward<Indices>(Idx)...)); 
        }
    };

    template<typename T>
    struct UnaryOpTraits<UnaryOpType::Sin, T> {
        using ShapeTypeT = typename T::ShapeType;
        using ValueTypeT = typename T::ValueType;

        static constexpr decltype(auto) apply(const ValueTypeT& val) { return std::sin(val); }

        using ValueType = decltype(apply(ValueTypeT{}));
        using ShapeType = ShapeTypeT::template ConvertedSelf<ValueType>;
        
        template<typename... Indices>
        constexpr decltype(auto) operator()(const T& expr, size_t global, Indices&&... Idx) { 
            return apply(expr.eval(global, std::forward<Indices>(Idx)...)); 
        }
    };

    // Bitwise operations
    template<typename T>
    struct UnaryOpTraits<UnaryOpType::BitwiseNegate, T> {
        using ShapeTypeT = typename T::ShapeType;
        using ValueTypeT = typename T::ValueType;

        static constexpr decltype(auto) apply(const ValueTypeT& val) { return ~val; }

        using ValueType = decltype(apply(ValueTypeT{}));
        using ShapeType = ShapeTypeT::template ConvertedSelf<ValueType>;

        template<typename... Indices>
        constexpr decltype(auto) operator()(const T& expr, size_t global, Indices&&... Idx) { 
            return apply(expr.eval(global, std::forward<Indices>(Idx)...)); 
        }
    };

    // Logical operations
    template<typename T>
    struct UnaryOpTraits<UnaryOpType::Not, T> {
        using ShapeTypeT = typename T::ShapeType;
        using ValueTypeT = typename T::ValueType;

        static constexpr decltype(auto) apply(const ValueTypeT& val) { return !val; }

        using ValueType = decltype(apply(ValueTypeT{}));
        using ShapeType = ShapeTypeT::template ConvertedSelf<ValueType>;
        
        template<typename... Indices>
        constexpr decltype(auto) operator()(const T& expr, size_t global, Indices&&... Idx) { 
            return apply(expr.eval(global, std::forward<Indices>(Idx)...)); 
        }
    };

    template<typename T, UnaryOpType operation>
    class TensorUnaryExpression : public TensorArithmeticExpression<TensorUnaryExpression<T, operation>> {
    public:
        using NotIncludedInScalarArithmetic = void;
        using Traits = UnaryOpTraits<operation, T>;
        using ValueType = typename Traits::ValueType;
        using ShapeType = typename Traits::ShapeType;
        using TensorType = TensorBase<ShapeType>;

    private:
        const T& m_expr;

    public:
        constexpr explicit TensorUnaryExpression(const T& expr) : m_expr(expr){}

        static constexpr size_t size() { return ShapeType::size(); }

        template<typename... Indices>
        constexpr ValueType eval(size_t i, Indices&&... coords) const { 
            return Traits{}(m_expr, i, std::forward<Indices>(coords)...); 
        }

        constexpr decltype(auto) eval() const {
            TensorType result;
            size_t i = 0;
            Detail::recursiveLoop<0, ShapeType>(
                [&]<typename... Indices>(Indices... idx){
                    result.data()[i] = eval(i, idx...);
                    ++i;
            });
            return result;
        }
    };
}