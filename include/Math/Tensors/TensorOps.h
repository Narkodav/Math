#pragma once

#include "Math/Tensors/TensorDefines.h"

namespace Math {
    template<typename Derived>
    class TensorArithmeticExpression;

    enum class BinaryOpType {
        // Arithmetic operations
        Add,
        Subtract,
        Divide,
        Multiply,
        Modulo,

        // Shape transforming operation
        MatrixMult,
        // CrossProd,

        // Bitwise operations
        BitwiseAnd,
        BitwiseOr,
        BitwiseXor,
        LeftShift,
        RightShift,

        // Logical operations
        And,
        Or,
        Equal,
        NotEqual,
        Less,
        LessOrEqual,
        Greater,
        GreaterOrEqual,

        // Operators
        Min,
        Max,
    };

    template<typename L, typename R, typename ValueType, size_t... resultDims>
    requires (L::dims().size() == R::dims().size())
    struct DeduceShape;

    template<typename VL, typename VR, typename ValueType, size_t dimL, size_t... restL, 
        size_t dimR, size_t... restR, size_t... resultDims>
    struct DeduceShape<Shape<VL, dimL, restL...>, Shape<VR, dimR, restR...>, ValueType, resultDims...> {
        using ShapeType = typename DeduceShape<Shape<VL, restL...>, Shape<VR, restR...>, 
            ValueType, resultDims..., std::min(dimL, dimR)>::ShapeType;
    };

    template<typename VL, typename VR, typename ValueType, size_t dimL, size_t dimR, size_t... resultDims>
    struct DeduceShape<Shape<VL, dimL>, Shape<VR, dimR>, ValueType, resultDims...> {
        using ShapeType = Shape<ValueType, resultDims..., std::min(dimL, dimR)>;
    };

    template<BinaryOpType opType>
    struct SimpleBinaryOpTrait;

    template<BinaryOpType op, typename L, typename R>
    struct SimpleBinaryOp {
        using ShapeTypeL = typename L::ShapeType;
        using ShapeTypeR = typename R::ShapeType;
        using ValueTypeL = typename L::ValueType;
        using ValueTypeR = typename R::ValueType;

        using Traits = SimpleBinaryOpTrait<op>;
    
        using ValueType = std::remove_cvref_t<decltype(Traits{}(ValueTypeL{}, ValueTypeR{}))>;
        using ShapeType = typename DeduceShape<ShapeTypeL, ShapeTypeR, ValueType>::ShapeType;

        template<typename... Indices>
        constexpr decltype(auto) operator()(const L& lhs, const R& rhs, size_t global, Indices&&... Idx) const {
            if constexpr (L::ShapeType::dims() == R::ShapeType::dims()) {
                return Traits{}(lhs.eval(global, std::forward<Indices>(Idx)...), rhs.eval(global, std::forward<Indices>(Idx)...));
            }
            else if constexpr (L::ShapeType::dims().size() == R::ShapeType::dims().size()) {
                return Traits{}(lhs.eval(ShapeTypeL::index(std::forward<Indices>(Idx)...), std::forward<Indices>(Idx)...), 
                    rhs.eval(ShapeTypeR::index(std::forward<Indices>(Idx)...), std::forward<Indices>(Idx)...));
            }
        }
    };

    // Default spec for simple ops
    template<BinaryOpType opType, typename L, typename R>
    struct BinaryOpTraits : public SimpleBinaryOp<opType, L, R> {
        using Base = SimpleBinaryOp<opType, L, R>;
        using ValueType = typename Base::ValueType;
        using ShapeType = typename Base::ShapeType;
    };

    // Complex binary ops
    template<typename L, typename R>
    requires (L::ShapeType::dims().size() == 2 && R::ShapeType::dims().size() == 2 &&
        L::ShapeType::dims()[0] == R::ShapeType::dims()[1])
    struct BinaryOpTraits<BinaryOpType::MatrixMult, L, R> {
        using ShapeTypeL = typename L::ShapeType;
        using ShapeTypeR = typename R::ShapeType;
        using ValueTypeL = typename L::ValueType;
        using ValueTypeR = typename R::ValueType;
        static constexpr size_t s_common = L::ShapeType::dims()[0];
        static constexpr size_t s_col = R::ShapeType::dims()[0];
        static constexpr size_t s_row = L::ShapeType::dims()[1];

        using ValueTypeMult = decltype(ValueTypeL{} * ValueTypeR{});

        template<size_t... Idx>
        struct SumResultTrait {
            using ValueType = decltype(((Idx, ValueTypeMult{}) + ...));
        };

        template<size_t... Idx>
        static constexpr auto makeSumResult(std::index_sequence<Idx...>) {
            return typename SumResultTrait<Idx...>::ValueType{};
        }

        using ValueType = decltype(makeSumResult(std::make_index_sequence<s_common>{}));
        using ShapeType = Shape<ValueType, s_col, s_row>;

        template<typename Idx1, typename Idx2>
        constexpr decltype(auto) operator()(const L& lhs, const R& rhs, size_t, Idx1&& col, Idx2&& row) {
            ValueType val{};
            for(size_t i = 0; i < s_common; ++i) {
                val += lhs.eval(ShapeTypeL::index(i, std::forward<Idx2>(row)), i, std::forward<Idx2>(row)) * 
                    rhs.eval(ShapeTypeR::index(std::forward<Idx1>(col), i), std::forward<Idx1>(col), i);
            }
            return val;
        }
    };

    // Overload for Matrix * Vector mult
    template<typename L, typename R>
    requires (L::ShapeType::dims().size() == 2 && R::ShapeType::dims().size() == 1 &&
        L::ShapeType::dims()[0] == R::ShapeType::dims()[0])
    struct BinaryOpTraits<BinaryOpType::MatrixMult, L, R> {
        using ShapeTypeL = typename L::ShapeType;
        using ShapeTypeR = typename R::ShapeType;
        using ValueTypeL = typename L::ValueType;
        using ValueTypeR = typename R::ValueType;
        static constexpr size_t s_common = L::ShapeType::dims()[0];
        static constexpr size_t s_row = R::ShapeType::dims()[0];

        using ValueTypeMult = decltype(ValueTypeL{} * ValueTypeR{});

        template<size_t... Idx>
        struct SumResultTrait {
            using ValueType = decltype(((Idx, ValueTypeMult{}) + ...));
        };

        template<size_t... Idx>
        static constexpr auto makeSumResult(std::index_sequence<Idx...>) {
            return typename SumResultTrait<Idx...>::ValueType{};
        }

        using ValueType = decltype(makeSumResult(std::make_index_sequence<s_common>{}));
        using ShapeType = Shape<ValueType, s_row>;
        
        template<typename Idx>
        constexpr decltype(auto) operator()(const L& lhs, const R& rhs, size_t, Idx&& row) {
            ValueType val{};
            for(size_t i = 0; i < s_common; ++i) {
                val += lhs.eval(ShapeTypeL::index(i, std::forward<Idx>(row)), i, std::forward<Idx>(row)) * 
                    rhs.eval(ShapeTypeR::index(i), i);
            }
            return val;
        }
    };

    // Not implemented since it has duplicate evaluations, cross product makes a copy
    // template<typename L, typename R>
    // requires (L::ShapeType::dims() == 1 && R::ShapeType::dims() == 1 &&
    //     L::ShapeType::dims()[0] == 3 && R::ShapeType::dims()[0] == 3)
    // struct BinaryOpTraits<BinaryOpType::CrossProd, L, R> {
    //     using ShapeTypeL = typename L::ShapeType;
    //     using ShapeTypeR = typename R::ShapeType;
    //     using ValueTypeL = typename L::ValueType;
    //     using ValueTypeR = typename R::ValueType;

    //     using MultResultType = decltype(ValueTypeL{} * ValueTypeR{});
    //     using SubtractResultType = decltype(MultResultType{} - MultResultType{});
    //     using ValueType = SubtractResultType;

    //     static constexpr std::array<std::pair<size_t, size_t>, 3> s_indices = {
    //         std::make_pair<size_t, size_t>(2, 3),
    //         std::make_pair<size_t, size_t>(3, 1),
    //         std::make_pair<size_t, size_t>(1, 2),
    //     };

    //     constexpr decltype(auto) operator()(size_t idx, const L& lhs, const R& rhs) {
    //         return lhs.eval(s_indices[idx].first) * rhs.eval(s_indices[idx].second) - 
    //             lhs.eval(s_indices[idx].second) * rhs.eval(s_indices[idx].first);
    //     }

    //     constexpr auto operator()(size_t i, const L& lhs, const R& rhs) { 
    //         return lhs.eval(s_indices[i].first) * rhs.eval(s_indices[i].second) - 
    //             lhs.eval(s_indices[i].second) * rhs.eval(s_indices[i].first);
    //     }
    // };

    // Arithmetic operations
    template<>
    struct SimpleBinaryOpTrait<BinaryOpType::Add> {
        template <typename L, typename R>
        constexpr decltype(auto) operator()(L&& lhs, R&& rhs) {
            return std::forward<L>(lhs) + std::forward<R>(rhs); 
        }
    };
    template<>
    struct SimpleBinaryOpTrait<BinaryOpType::Subtract> {
        template <typename L, typename R>
        constexpr decltype(auto) operator()(L&& lhs, R&& rhs) {
            return std::forward<L>(lhs) - std::forward<R>(rhs); 
        }
    };
    template<>
    struct SimpleBinaryOpTrait<BinaryOpType::Divide> {
        template <typename L, typename R>
        constexpr decltype(auto) operator()(L&& lhs, R&& rhs) {
            return std::forward<L>(lhs) / std::forward<R>(rhs); 
        }
    };
    template<>
    struct SimpleBinaryOpTrait<BinaryOpType::Multiply> {
        template <typename L, typename R>
        constexpr decltype(auto) operator()(L&& lhs, R&& rhs) {
            return std::forward<L>(lhs) * std::forward<R>(rhs); 
        }
    };
    template<>
    struct SimpleBinaryOpTrait<BinaryOpType::Modulo> {
        template <typename L, typename R>
        constexpr decltype(auto) operator()(L&& lhs, R&& rhs) {
            return std::forward<L>(lhs) % std::forward<R>(rhs); 
        }
    };

    // Bitwise operations
    template<>
    struct SimpleBinaryOpTrait<BinaryOpType::BitwiseAnd> {
        template <typename L, typename R>
        constexpr decltype(auto) operator()(L&& lhs, R&& rhs) {
            return std::forward<L>(lhs) & std::forward<R>(rhs); 
        }
    };
    template<>
    struct SimpleBinaryOpTrait<BinaryOpType::BitwiseOr> {
        template <typename L, typename R>
        constexpr decltype(auto) operator()(L&& lhs, R&& rhs) {
            return std::forward<L>(lhs) | std::forward<R>(rhs); 
        }
    };
    template<>
    struct SimpleBinaryOpTrait<BinaryOpType::BitwiseXor> {
        template <typename L, typename R>
        constexpr decltype(auto) operator()(L&& lhs, R&& rhs) {
            return std::forward<L>(lhs) ^ std::forward<R>(rhs); 
        }
    };
    template<>
    struct SimpleBinaryOpTrait<BinaryOpType::LeftShift> {
        template <typename L, typename R>
        constexpr decltype(auto) operator()(L&& lhs, R&& rhs) {
            return std::forward<L>(lhs) << std::forward<R>(rhs); 
        }
    };
    template<>
    struct SimpleBinaryOpTrait<BinaryOpType::RightShift> {
        template <typename L, typename R>
        constexpr decltype(auto) operator()(L&& lhs, R&& rhs) {
            return std::forward<L>(lhs) >> std::forward<R>(rhs); 
        }
    };

    // Logical operations
    template<>
    struct SimpleBinaryOpTrait<BinaryOpType::And> {
        template <typename L, typename R>
        constexpr decltype(auto) operator()(L&& lhs, R&& rhs) {
            return std::forward<L>(lhs) && std::forward<R>(rhs); 
        }
    };
    template<>
    struct SimpleBinaryOpTrait<BinaryOpType::Or> {
        template <typename L, typename R>
        constexpr decltype(auto) operator()(L&& lhs, R&& rhs) {
            return std::forward<L>(lhs) || std::forward<R>(rhs); 
        }
    };
    template<>
    struct SimpleBinaryOpTrait<BinaryOpType::Equal> {
        template <typename L, typename R>
        constexpr decltype(auto) operator()(L&& lhs, R&& rhs) {
            return std::forward<L>(lhs) == std::forward<R>(rhs); 
        }
    };
    template<>
    struct SimpleBinaryOpTrait<BinaryOpType::NotEqual> {
        template <typename L, typename R>
        constexpr decltype(auto) operator()(L&& lhs, R&& rhs) {
            return std::forward<L>(lhs) != std::forward<R>(rhs); 
        }
    };
    template<>
    struct SimpleBinaryOpTrait<BinaryOpType::Less> {
        template <typename L, typename R>
        constexpr decltype(auto) operator()(L&& lhs, R&& rhs) {
            return std::forward<L>(lhs) < std::forward<R>(rhs); 
        }
    };
    template<>
    struct SimpleBinaryOpTrait<BinaryOpType::LessOrEqual> {
        template <typename L, typename R>
        constexpr decltype(auto) operator()(L&& lhs, R&& rhs) {
            return std::forward<L>(lhs) <= std::forward<R>(rhs); 
        }
    };
    template<>
    struct SimpleBinaryOpTrait<BinaryOpType::Greater> {
        template <typename L, typename R>
        constexpr decltype(auto) operator()(L&& lhs, R&& rhs) {
            return std::forward<L>(lhs) > std::forward<R>(rhs); 
        }
    };
    template<>
    struct SimpleBinaryOpTrait<BinaryOpType::GreaterOrEqual> {
        template <typename L, typename R>
        constexpr decltype(auto) operator()(L&& lhs, R&& rhs) {
            return std::forward<L>(lhs) >= std::forward<R>(rhs); 
        }
    };

    // Operators
    template<>
    struct SimpleBinaryOpTrait<BinaryOpType::Min> {
        template <typename L, typename R>
        constexpr decltype(auto) operator()(L&& lhs, R&& rhs) {
            return std::min(std::forward<L>(lhs), std::forward<R>(rhs)); 
        }
    };
    template<>
    struct SimpleBinaryOpTrait<BinaryOpType::Max> {
        template <typename L, typename R>
        constexpr decltype(auto) operator()(L&& lhs, R&& rhs) {
            return std::max(std::forward<L>(lhs), std::forward<R>(rhs)); 
        }
    };

    template<typename T>
    struct StorageTypeTrait {
        using StorageType = const T&;
    };

    template<typename T>
    struct StorageTypeTrait<ScalarExpression<T>> {
        using StorageType = ScalarExpression<T>;
    };

    template<typename L, typename R, BinaryOpType operation>
    class TensorBinaryExpression : public TensorArithmeticExpression<TensorBinaryExpression<L, R, operation>> {
    public:
        using NotIncludedInScalarArithmetic = void;
        using IsTensorExpression = void;
        using Self = TensorBinaryExpression<L, R, operation>;
        using Base = TensorArithmeticExpression<Self>;
        using Traits = BinaryOpTraits<operation, L, R>;
        using ValueType = typename Traits::ValueType;
        using ShapeType = typename Traits::ShapeType;
        using TensorType = TensorBase<ShapeType>;

    private:
        typename StorageTypeTrait<L>::StorageType m_lhs;
        typename StorageTypeTrait<R>::StorageType m_rhs;

    public:
        constexpr explicit TensorBinaryExpression(const L& lhs, const R& rhs) : m_lhs(lhs), m_rhs(rhs) {}

        template<typename... Indices>
        constexpr ValueType eval(size_t i, Indices&&... coords) const {
            return Traits{}(m_lhs, m_rhs, i, std::forward<Indices>(coords)...); 
        }

        using Base::eval;
    };

    enum class UnaryOpType {
        // Arithmetic operations
        Negate,
        Cos,
        Sin,
        Abs,

        // Bitwise operations
        BitwiseNegate,

        // Logical operations
        Not,
    };

    template<UnaryOpType opType>
    struct SimpleUnaryOpTrait;

    template<UnaryOpType op, typename T>
    struct SimpleUnaryOp {
        using ShapeTypeT = typename T::ShapeType;
        using ValueTypeT = typename T::ValueType;

        using Traits = SimpleUnaryOpTrait<op>;

        using ValueType = std::remove_cvref_t<decltype(Traits{}(ValueTypeT{}))>;
        using ShapeType = ShapeTypeT::template ConvertedSelf<ValueType>;
        
        template<typename... Indices>
        constexpr decltype(auto) operator()(const T& expr, size_t global, Indices&&... Idx) { 
            return Traits{}(expr.eval(global, std::forward<Indices>(Idx)...)); 
        }
    };

    template<UnaryOpType opType, typename T>
    struct UnaryOpTraits : SimpleUnaryOp<opType, T> {
        using Base = SimpleUnaryOp<opType, T>;
        using ValueType = typename Base::ValueType;
        using ShapeType = typename Base::ShapeType;
    };

    // Arithmetic operations
    template<>
    struct SimpleUnaryOpTrait<UnaryOpType::Negate> {
        template <typename T>
        constexpr decltype(auto) operator()(const T& val) { return -val; }
    };
    template<>
    struct SimpleUnaryOpTrait<UnaryOpType::Cos> {
        template <typename T>
        constexpr decltype(auto) operator()(const T& val) { return std::cos(val); }
    };
    template<>
    struct SimpleUnaryOpTrait<UnaryOpType::Sin> {
        template <typename T>
        constexpr decltype(auto) operator()(const T& val) { return std::sin(val); }
    };
    template<>
    struct SimpleUnaryOpTrait<UnaryOpType::Abs> {
        template <typename T>
        constexpr decltype(auto) operator()(const T& val) { return std::abs(val); }
    };

    // Bitwise operations
    template<>
    struct SimpleUnaryOpTrait<UnaryOpType::BitwiseNegate> {
        template <typename T>
        constexpr decltype(auto) operator()(const T& val) { return ~val; }
    };

    // Logical operations
    template<>
    struct SimpleUnaryOpTrait<UnaryOpType::Not> {
        template <typename T>
        constexpr decltype(auto) operator()(const T& val) { return !val; }
    };

    template<typename T, UnaryOpType operation>
    class TensorUnaryExpression : public TensorArithmeticExpression<TensorUnaryExpression<T, operation>> {
    public:
        using NotIncludedInScalarArithmetic = void;
        using IsTensorExpression = void;
        using Self = TensorUnaryExpression<T, operation>;
        using Base = TensorArithmeticExpression<Self>;
        using Traits = UnaryOpTraits<operation, T>;
        using ValueType = typename Traits::ValueType;
        using ShapeType = typename Traits::ShapeType;
        using TensorType = TensorBase<ShapeType>;

    private:
        const T& m_expr;

    public:
        constexpr explicit TensorUnaryExpression(const T& expr) : m_expr(expr){}

        template<typename... Indices>
        constexpr ValueType eval(size_t i, Indices&&... coords) const { 
            return Traits{}(m_expr, i, std::forward<Indices>(coords)...); 
        }

        using Base::eval;
    };
}