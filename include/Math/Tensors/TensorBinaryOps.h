#pragma once
#include "Math/Tensors/TensorBase.h"

namespace Math {
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
        GreaterOrEqual
    };

    template<BinaryOpType opType, typename L, typename R>
    struct BinaryOpTraits;

    // Arithmetic operations
    template<typename L, typename R>
    requires (L::ShapeType::dims() == R::ShapeType::dims())
    struct BinaryOpTraits<BinaryOpType::Add, L, R> {
        using ShapeTypeL = typename L::ShapeType;
        using ShapeTypeR = typename R::ShapeType;
        using ValueTypeL = typename L::ValueType;
        using ValueTypeR = typename R::ValueType;

        static constexpr decltype(auto) apply(const ValueTypeL& lhs, const ValueTypeR& rhs) { 
            return lhs + rhs; 
        }

        using ValueType = decltype(apply(ValueTypeL{}, ValueTypeR{}));
        using ShapeType = ShapeTypeL::template ConvertedSelf<ValueType>;

        template<typename... Indices>
        constexpr decltype(auto) operator()(const L& lhs, const R& rhs, size_t global, Indices&&... Idx) { 
            return apply(lhs.eval(global, std::forward<Indices>(Idx)...), rhs.eval(global, std::forward<Indices>(Idx)...)); 
        }
    };

    template<typename L, typename R>
    requires (L::ShapeType::dims() == R::ShapeType::dims())
    struct BinaryOpTraits<BinaryOpType::Subtract, L, R> {
        using ShapeTypeL = typename L::ShapeType;
        using ShapeTypeR = typename R::ShapeType;
        using ValueTypeL = typename L::ValueType;
        using ValueTypeR = typename R::ValueType;

        static constexpr decltype(auto) apply(const ValueTypeL& lhs, const ValueTypeR& rhs) { 
            return lhs - rhs; 
        }

        using ValueType = decltype(apply(ValueTypeL{}, ValueTypeR{}));
        using ShapeType = ShapeTypeL::template ConvertedSelf<ValueType>;
        
        template<typename... Indices>
        constexpr decltype(auto) operator()(const L& lhs, const R& rhs, size_t global, Indices&&... Idx) { 
            return apply(lhs.eval(global, std::forward<Indices>(Idx)...), rhs.eval(global, std::forward<Indices>(Idx)...)); 
        }
    };

    template<typename L, typename R>
    requires (L::ShapeType::dims() == R::ShapeType::dims())
    struct BinaryOpTraits<BinaryOpType::Divide, L, R> {
        using ShapeTypeL = typename L::ShapeType;
        using ShapeTypeR = typename R::ShapeType;
        using ValueTypeL = typename L::ValueType;
        using ValueTypeR = typename R::ValueType;

        static constexpr decltype(auto) apply(const ValueTypeL& lhs, const ValueTypeR& rhs) { 
            return lhs / rhs; 
        }

        using ValueType = decltype(apply(ValueTypeL{}, ValueTypeR{}));
        using ShapeType = ShapeTypeL::template ConvertedSelf<ValueType>;
        
        template<typename... Indices>
        constexpr decltype(auto) operator()(const L& lhs, const R& rhs, size_t global, Indices&&... Idx) { 
            return apply(lhs.eval(global, std::forward<Indices>(Idx)...), rhs.eval(global, std::forward<Indices>(Idx)...)); 
        }
    };

    template<typename L, typename R>
    requires (L::ShapeType::dims() == R::ShapeType::dims())
    struct BinaryOpTraits<BinaryOpType::Multiply, L, R> {
        using ShapeTypeL = typename L::ShapeType;
        using ShapeTypeR = typename R::ShapeType;
        using ValueTypeL = typename L::ValueType;
        using ValueTypeR = typename R::ValueType;

        static constexpr decltype(auto) apply(const ValueTypeL& lhs, const ValueTypeR& rhs) { 
            return lhs * rhs; 
        }

        using ValueType = decltype(apply(ValueTypeL{}, ValueTypeR{}));
        using ShapeType = ShapeTypeL::template ConvertedSelf<ValueType>;
        
        template<typename... Indices>
        constexpr decltype(auto) operator()(const L& lhs, const R& rhs, size_t global, Indices&&... Idx) { 
            return apply(lhs.eval(global, std::forward<Indices>(Idx)...), rhs.eval(global, std::forward<Indices>(Idx)...)); 
        }
    };

    template<typename L, typename R>
    requires (L::ShapeType::dims() == R::ShapeType::dims())
    struct BinaryOpTraits<BinaryOpType::Modulo, L, R> {
        using ShapeTypeL = typename L::ShapeType;
        using ShapeTypeR = typename R::ShapeType;
        using ValueTypeL = typename L::ValueType;
        using ValueTypeR = typename R::ValueType;

        static constexpr decltype(auto) apply(const ValueTypeL& lhs, const ValueTypeR& rhs) { 
            return lhs % rhs; 
        }

        using ValueType = decltype(apply(ValueTypeL{}, ValueTypeR{}));
        using ShapeType = ShapeTypeL::template ConvertedSelf<ValueType>;
        
        template<typename... Indices>
        constexpr decltype(auto) operator()(const L& lhs, const R& rhs, size_t global, Indices&&... Idx) { 
            return apply(lhs.eval(global, std::forward<Indices>(Idx)...), rhs.eval(global, std::forward<Indices>(Idx)...)); 
        }
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

    // Bitwise operations
    template<typename L, typename R>
    requires (L::ShapeType::dims() == R::ShapeType::dims())
    struct BinaryOpTraits<BinaryOpType::BitwiseAnd, L, R> {
        using ShapeTypeL = typename L::ShapeType;
        using ShapeTypeR = typename R::ShapeType;
        using ValueTypeL = typename L::ValueType;
        using ValueTypeR = typename R::ValueType;

        static constexpr decltype(auto) apply(const ValueTypeL& lhs, const ValueTypeR& rhs) { 
            return lhs & rhs; 
        }

        using ValueType = decltype(apply(ValueTypeL{}, ValueTypeR{}));
        using ShapeType = ShapeTypeL::template ConvertedSelf<ValueType>;
        
        template<typename... Indices>
        constexpr decltype(auto) operator()(const L& lhs, const R& rhs, size_t global, Indices&&... Idx) { 
            return apply(lhs.eval(global, std::forward<Indices>(Idx)...), rhs.eval(global, std::forward<Indices>(Idx)...)); 
        }
    };

    template<typename L, typename R>
    requires (L::ShapeType::dims() == R::ShapeType::dims())
    struct BinaryOpTraits<BinaryOpType::BitwiseOr, L, R> {
        using ShapeTypeL = typename L::ShapeType;
        using ShapeTypeR = typename R::ShapeType;
        using ValueTypeL = typename L::ValueType;
        using ValueTypeR = typename R::ValueType;

        static constexpr decltype(auto) apply(const ValueTypeL& lhs, const ValueTypeR& rhs) { 
            return lhs | rhs; 
        }

        using ValueType = decltype(apply(ValueTypeL{}, ValueTypeR{}));
        using ShapeType = ShapeTypeL::template ConvertedSelf<ValueType>;
        
        template<typename... Indices>
        constexpr decltype(auto) operator()(const L& lhs, const R& rhs, size_t global, Indices&&... Idx) { 
            return apply(lhs.eval(global, std::forward<Indices>(Idx)...), rhs.eval(global, std::forward<Indices>(Idx)...)); 
        }
    };

    template<typename L, typename R>
    requires (L::ShapeType::dims() == R::ShapeType::dims())
    struct BinaryOpTraits<BinaryOpType::BitwiseXor, L, R> {
        using ShapeTypeL = typename L::ShapeType;
        using ShapeTypeR = typename R::ShapeType;
        using ValueTypeL = typename L::ValueType;
        using ValueTypeR = typename R::ValueType;

        static constexpr decltype(auto) apply(const ValueTypeL& lhs, const ValueTypeR& rhs) { 
            return lhs ^ rhs; 
        }

        using ValueType = decltype(apply(ValueTypeL{}, ValueTypeR{}));
        using ShapeType = ShapeTypeL::template ConvertedSelf<ValueType>;
        
        template<typename... Indices>
        constexpr decltype(auto) operator()(const L& lhs, const R& rhs, size_t global, Indices&&... Idx) { 
            return apply(lhs.eval(global, std::forward<Indices>(Idx)...), rhs.eval(global, std::forward<Indices>(Idx)...)); 
        }
    };

    template<typename L, typename R>
    requires (L::ShapeType::dims() == R::ShapeType::dims())
    struct BinaryOpTraits<BinaryOpType::LeftShift, L, R> {
        using ShapeTypeL = typename L::ShapeType;
        using ShapeTypeR = typename R::ShapeType;
        using ValueTypeL = typename L::ValueType;
        using ValueTypeR = typename R::ValueType;

        static constexpr decltype(auto) apply(const ValueTypeL& lhs, const ValueTypeR& rhs) { 
            return lhs << rhs; 
        }

        using ValueType = decltype(apply(ValueTypeL{}, ValueTypeR{}));
        using ShapeType = ShapeTypeL::template ConvertedSelf<ValueType>;
        
        template<typename... Indices>
        constexpr decltype(auto) operator()(const L& lhs, const R& rhs, size_t global, Indices&&... Idx) { 
            return apply(lhs.eval(global, std::forward<Indices>(Idx)...), rhs.eval(global, std::forward<Indices>(Idx)...)); 
        }
    };

    template<typename L, typename R>
    requires (L::ShapeType::dims() == R::ShapeType::dims())
    struct BinaryOpTraits<BinaryOpType::RightShift, L, R> {
        using ShapeTypeL = typename L::ShapeType;
        using ShapeTypeR = typename R::ShapeType;
        using ValueTypeL = typename L::ValueType;
        using ValueTypeR = typename R::ValueType;

        static constexpr decltype(auto) apply(const ValueTypeL& lhs, const ValueTypeR& rhs) { 
            return lhs >> rhs; 
        }

        using ValueType = decltype(apply(ValueTypeL{}, ValueTypeR{}));
        using ShapeType = ShapeTypeL::template ConvertedSelf<ValueType>;
        
        template<typename... Indices>
        constexpr decltype(auto) operator()(const L& lhs, const R& rhs, size_t global, Indices&&... Idx) { 
            return apply(lhs.eval(global, std::forward<Indices>(Idx)...), rhs.eval(global, std::forward<Indices>(Idx)...)); 
        }
    };

    // Logical operations
    template<typename L, typename R>
    requires (L::ShapeType::dims() == R::ShapeType::dims())
    struct BinaryOpTraits<BinaryOpType::And, L, R> {
        using ShapeTypeL = typename L::ShapeType;
        using ShapeTypeR = typename R::ShapeType;
        using ValueTypeL = typename L::ValueType;
        using ValueTypeR = typename R::ValueType;

        static constexpr decltype(auto) apply(const ValueTypeL& lhs, const ValueTypeR& rhs) { 
            return lhs && rhs; 
        }

        using ValueType = decltype(apply(ValueTypeL{}, ValueTypeR{}));
        using ShapeType = ShapeTypeL::template ConvertedSelf<ValueType>;
        
        template<typename... Indices>
        constexpr decltype(auto) operator()(const L& lhs, const R& rhs, size_t global, Indices&&... Idx) { 
            return apply(lhs.eval(global, std::forward<Indices>(Idx)...), rhs.eval(global, std::forward<Indices>(Idx)...)); 
        }
    };

    template<typename L, typename R>
    requires (L::ShapeType::dims() == R::ShapeType::dims())
    struct BinaryOpTraits<BinaryOpType::Or, L, R> {
        using ShapeTypeL = typename L::ShapeType;
        using ShapeTypeR = typename R::ShapeType;
        using ValueTypeL = typename L::ValueType;
        using ValueTypeR = typename R::ValueType;

        static constexpr decltype(auto) apply(const ValueTypeL& lhs, const ValueTypeR& rhs) { 
            return lhs || rhs; 
        }

        using ValueType = decltype(apply(ValueTypeL{}, ValueTypeR{}));
        using ShapeType = ShapeTypeL::template ConvertedSelf<ValueType>;
        
        template<typename... Indices>
        constexpr decltype(auto) operator()(const L& lhs, const R& rhs, size_t global, Indices&&... Idx) { 
            return apply(lhs.eval(global, std::forward<Indices>(Idx)...), rhs.eval(global, std::forward<Indices>(Idx)...)); 
        }
    };

    template<typename L, typename R>
    requires (L::ShapeType::dims() == R::ShapeType::dims())
    struct BinaryOpTraits<BinaryOpType::Equal, L, R> {
        using ShapeTypeL = typename L::ShapeType;
        using ShapeTypeR = typename R::ShapeType;
        using ValueTypeL = typename L::ValueType;
        using ValueTypeR = typename R::ValueType;

        static constexpr decltype(auto) apply(const ValueTypeL& lhs, const ValueTypeR& rhs) { 
            return lhs == rhs; 
        }

        using ValueType = decltype(apply(ValueTypeL{}, ValueTypeR{}));
        using ShapeType = ShapeTypeL::template ConvertedSelf<ValueType>;
        
        template<typename... Indices>
        constexpr decltype(auto) operator()(const L& lhs, const R& rhs, size_t global, Indices&&... Idx) { 
            return apply(lhs.eval(global, std::forward<Indices>(Idx)...), rhs.eval(global, std::forward<Indices>(Idx)...)); 
        }
    };

    template<typename L, typename R>
    requires (L::ShapeType::dims() == R::ShapeType::dims())
    struct BinaryOpTraits<BinaryOpType::NotEqual, L, R> {
        using ShapeTypeL = typename L::ShapeType;
        using ShapeTypeR = typename R::ShapeType;
        using ValueTypeL = typename L::ValueType;
        using ValueTypeR = typename R::ValueType;

        static constexpr decltype(auto) apply(const ValueTypeL& lhs, const ValueTypeR& rhs) { 
            return lhs != rhs; 
        }

        using ValueType = decltype(apply(ValueTypeL{}, ValueTypeR{}));
        using ShapeType = ShapeTypeL::template ConvertedSelf<ValueType>;
        
        template<typename... Indices>
        constexpr decltype(auto) operator()(const L& lhs, const R& rhs, size_t global, Indices&&... Idx) { 
            return apply(lhs.eval(global, std::forward<Indices>(Idx)...), rhs.eval(global, std::forward<Indices>(Idx)...)); 
        }
    };

    template<typename L, typename R>
    requires (L::ShapeType::dims() == R::ShapeType::dims())
    struct BinaryOpTraits<BinaryOpType::Less, L, R> {
        using ShapeTypeL = typename L::ShapeType;
        using ShapeTypeR = typename R::ShapeType;
        using ValueTypeL = typename L::ValueType;
        using ValueTypeR = typename R::ValueType;

        static constexpr decltype(auto) apply(const ValueTypeL& lhs, const ValueTypeR& rhs) { 
            return lhs < rhs; 
        }

        using ValueType = decltype(apply(ValueTypeL{}, ValueTypeR{}));
        using ShapeType = ShapeTypeL::template ConvertedSelf<ValueType>;
        
        template<typename... Indices>
        constexpr decltype(auto) operator()(const L& lhs, const R& rhs, size_t global, Indices&&... Idx) { 
            return apply(lhs.eval(global, std::forward<Indices>(Idx)...), rhs.eval(global, std::forward<Indices>(Idx)...)); 
        }
    };

    template<typename L, typename R>
    requires (L::ShapeType::dims() == R::ShapeType::dims())
    struct BinaryOpTraits<BinaryOpType::LessOrEqual, L, R> {
        using ShapeTypeL = typename L::ShapeType;
        using ShapeTypeR = typename R::ShapeType;
        using ValueTypeL = typename L::ValueType;
        using ValueTypeR = typename R::ValueType;

        static constexpr decltype(auto) apply(const ValueTypeL& lhs, const ValueTypeR& rhs) { 
            return lhs <= rhs; 
        }

        using ValueType = decltype(apply(ValueTypeL{}, ValueTypeR{}));
        using ShapeType = ShapeTypeL::template ConvertedSelf<ValueType>;
        
        template<typename... Indices>
        constexpr decltype(auto) operator()(const L& lhs, const R& rhs, size_t global, Indices&&... Idx) { 
            return apply(lhs.eval(global, std::forward<Indices>(Idx)...), rhs.eval(global, std::forward<Indices>(Idx)...)); 
        }
    };

    template<typename L, typename R>
    requires (L::ShapeType::dims() == R::ShapeType::dims())
    struct BinaryOpTraits<BinaryOpType::Greater, L, R> {
        using ShapeTypeL = typename L::ShapeType;
        using ShapeTypeR = typename R::ShapeType;
        using ValueTypeL = typename L::ValueType;
        using ValueTypeR = typename R::ValueType;

        static constexpr decltype(auto) apply(const ValueTypeL& lhs, const ValueTypeR& rhs) { 
            return lhs > rhs; 
        }

        using ValueType = decltype(apply(ValueTypeL{}, ValueTypeR{}));
        using ShapeType = ShapeTypeL::template ConvertedSelf<ValueType>;
        
        template<typename... Indices>
        constexpr decltype(auto) operator()(const L& lhs, const R& rhs, size_t global, Indices&&... Idx) { 
            return apply(lhs.eval(global, std::forward<Indices>(Idx)...), rhs.eval(global, std::forward<Indices>(Idx)...)); 
        }
    };

    template<typename L, typename R>
    requires (L::ShapeType::dims() == R::ShapeType::dims())
    struct BinaryOpTraits<BinaryOpType::GreaterOrEqual, L, R> {
        using ShapeTypeL = typename L::ShapeType;
        using ShapeTypeR = typename R::ShapeType;
        using ValueTypeL = typename L::ValueType;
        using ValueTypeR = typename R::ValueType;

        static constexpr decltype(auto) apply(const ValueTypeL& lhs, const ValueTypeR& rhs) { 
            return lhs >= rhs; 
        }

        using ValueType = decltype(apply(ValueTypeL{}, ValueTypeR{}));
        using ShapeType = ShapeTypeL::template ConvertedSelf<ValueType>;
        
        template<typename... Indices>
        constexpr decltype(auto) operator()(const L& lhs, const R& rhs, size_t global, Indices&&... Idx) { 
            return apply(lhs.eval(global, std::forward<Indices>(Idx)...), rhs.eval(global, std::forward<Indices>(Idx)...)); 
        }
    };

    template<typename L, typename R, BinaryOpType operation>
    class TensorBinaryExpression : public TensorArithmeticExpression<TensorBinaryExpression<L, R, operation>> {
    public:
        using NotIncludedInScalarArithmetic = void;
        using Traits = BinaryOpTraits<operation, L, R>;
        using ValueType = typename Traits::ValueType;
        using ShapeType = typename Traits::ShapeType;
        using TensorType = TensorBase<ShapeType>;

    private:
        const L& m_lhs;
        const R& m_rhs;

    public:
        constexpr explicit TensorBinaryExpression(const L& lhs, const R& rhs) : m_lhs(lhs), m_rhs(rhs) {}

        static constexpr size_t size() { return ShapeType::size(); }

        template<typename... Indices>
        constexpr ValueType eval(size_t i, Indices&&... coords) const { 
            return Traits{}(m_lhs, m_rhs, i, std::forward<Indices>(coords)...); 
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