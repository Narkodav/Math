#include <iostream>
#include <iomanip>
#include <bitset>

#include "Math/BigNum/BigInt.h"
#include "Math/BigNum/StaticBigUint.h"

#include "Math/Tensors/Tensors.h"

std::string floatToBinary(float f) {
	// Get the bit representation of the float
	uint32_t bits = *reinterpret_cast<uint32_t*>(&f);

	// Extract components
	bool sign = (bits >> 31) & 1;
	uint32_t exponent = (bits >> 23) & 0xFF;
	uint32_t mantissa = bits & 0x7FFFFF;

	// Create the binary string
	std::string result;

	// Add sign bit
	result += (sign ? "1" : "0");
	result += " | ";

	// Add exponent (8 bits)
	result += std::bitset<8>(exponent).to_string();
	result += " | ";

	// Add mantissa (23 bits)
	result += std::bitset<23>(mantissa).to_string();

	return result;
}

void printExactFloat(float f) {
	// Get the bit representation
	uint32_t bits = *reinterpret_cast<uint32_t*>(&f);

	// Extract components
	bool sign = (bits >> 31) & 1;
	int32_t exponent = ((bits >> 23) & 0xFF) - 127; // Remove bias
	uint32_t mantissa = bits & 0x7FFFFF;

	// Calculate the exact decimal value
	double exact = 0.0;

	// Add the implicit 1
	exact = 1.0;

	// Add the mantissa bits
	for (int i = 0; i < 23; i++) {
		if (mantissa & (1 << (22 - i))) {
			exact += std::pow(2.0, -(i + 1));
		}
	}

	// Apply the exponent
	exact *= std::pow(2.0, exponent);

	// Apply sign
	if (sign) exact = -exact;

	// Print with maximum precision
	std::cout << std::setprecision(std::numeric_limits<double>::digits10 + 1)
		<< std::fixed
		<< exact
		<< std::endl;
}

template<typename T, size_t... dims>
void foo(Math::TensorView<T, dims...> view) {

}

void fooDef(Math::TensorView<int, 2,3,4> view) {

}

int main() {
	using namespace Math;

    //--------------------------------------------------------------------------
    // Construction
    //--------------------------------------------------------------------------

    Tensor<int, 2,3,4> t1;
    Tensor<int, 2,3,4> t2;

    TensorView<int, 2,3,4> v1 = t1;
    TensorView<int, 2,3,4> v2 = t2;

	foo(t1.view());
	fooDef(t1);

    auto aa = t1[0];

    TensorView<int, 3,4> v11 = t1[0];
    TensorView<int, 3,4> v12 = t2[0];

    TensorView<int, 4> v21 = v11[0];
    TensorView<int, 4> v22 = v12[0];

    int& x1 = v21[0];
    int& x2 = v22[0];

    int scalar;

    //--------------------------------------------------------------------------
    // Copy construction
    //--------------------------------------------------------------------------

    Tensor<int, 2,3,4> ct1 = t1;
    Tensor<int, 2,3,4> ct2(v1);

    Tensor<int, 3,4> ct3 = v11;
    Tensor<int, 3,4> ct4(v12);

    Tensor<int, 4> ct5 = v21;
    Tensor<int, 4> ct6(v22);

    Tensor<int, 2,3,4> cccc = t1 + ct1 - ct1.view();

    //--------------------------------------------------------------------------
    // Assignment : tensor <- tensor
    //--------------------------------------------------------------------------

    ct1 = t2;
    ct1 = ct2;

    //--------------------------------------------------------------------------
    // Assignment : tensor <- view
    //--------------------------------------------------------------------------

    ct1 = v1;
    ct3 = v11;
    ct5 = v21;

    //--------------------------------------------------------------------------
    // Assignment : view <- tensor
    //--------------------------------------------------------------------------

    v1 = t1;
    v11 = ct3;
    v21 = ct5;

    //--------------------------------------------------------------------------
    // Assignment : view <- view
    //--------------------------------------------------------------------------

    v1 = v2;
    v11 = v12;
    v21 = v22;

    //--------------------------------------------------------------------------
    // Simple expressions
    //--------------------------------------------------------------------------

    auto e1 = v21 + v22;
    auto e2 = v21 - v22;
    auto e3 = v21 * v22;
    auto e4 = v21 / v22;

    //--------------------------------------------------------------------------
    // Tensor construction from expression
    //--------------------------------------------------------------------------

    Tensor<int, 4> r1 = v21 + v22;
    Tensor<int, 4> r2 = v21 - v22;
    Tensor<int, 4> r3 = v21 * v22;
    //Tensor<int, 4> r4 = v21 / v22;

    //--------------------------------------------------------------------------
    // Nested expressions
    //--------------------------------------------------------------------------

    Tensor<int, 4> r5 =
        v21 + v22 + v21;

    Tensor<int, 4> r6 =
        v21 + v22 - v21;

    Tensor<int, 4> r7 =
        (v21 + v22) * v21;

    // Tensor<int, 4> r8 =
    //     (v21 + v22) / v21;

    //--------------------------------------------------------------------------
    // Full tensor expressions
    //--------------------------------------------------------------------------

    Tensor<int, 2,3,4> r9 =
        t1 + t2;

    Tensor<int, 2,3,4> r10 =
        t1 + v2;

    Tensor<int, 2,3,4> r11 =
        v1 + t2;

    Tensor<int, 2,3,4> r12 =
        v1 + v2;

    //--------------------------------------------------------------------------
    // Long expression chains
    //--------------------------------------------------------------------------

    Tensor<int, 2,3,4> r13 =
        t1 + t2.view() + t2 + t2.view();

    Tensor<int, 2,3,4> r14 =
        t1 + v2 + t2 + v1;

    Tensor<int, 2,3,4> r15 =
        ((t1 + t2) - v1) + (v2 + t1);

    //--------------------------------------------------------------------------
    // Compound assignment
    //--------------------------------------------------------------------------

    r9 += v1;
    r9 -= v1;
    r9 *= v1;
    //r9 /= v1;

    //--------------------------------------------------------------------------
    // Compound assignment chains
    //--------------------------------------------------------------------------

    r10 += (v1 -= t1);

    r11 -= (v1 += t1);

    r12 *= (v1 -= v2);

    //r13 /= (v1 += v2);

    //--------------------------------------------------------------------------
    // Result of compound assignment
    //--------------------------------------------------------------------------

    Tensor<int, 2,3,4> r16 =
        (r14 -= r15);

    Tensor<int, 2,3,4> r17 =
        (r14 += r15);

    Tensor<int, 2,3,4> r18 =
        (r14 *= r15);

    // Tensor<int, 2,3,4> r19 =
    //     (r14 /= r15);

    //--------------------------------------------------------------------------
    // Deep indexing
    //--------------------------------------------------------------------------

    auto a = t1[0];
    auto b = t1[0][0];
    auto c = t1[0][0][0];

    auto d = v1[0];
    auto e = v1[0][0];
    auto f = v1[0][0][0];

    //--------------------------------------------------------------------------
    // Explicit view conversions
    //--------------------------------------------------------------------------

    TensorView<int, 2,3,4> vv1 = t1.view();
    //TensorView<const int, 2,3,4> vv2 = t1.view();

    TensorView<int, 3,4> vv3 = t1[0];
    TensorView<int, 4> vv4 = t1[0][0];

    //--------------------------------------------------------------------------
    // Scalar operations
    //--------------------------------------------------------------------------

    v1 = v1 + scalar;
    v1 = scalar + v1;
    

    static_assert(std::is_assignable_v<
        Tensor<int,2,3,4>&,
        TensorView<int,2,3,4>>);

    static_assert(std::is_assignable_v<
        TensorView<int,2,3,4>&,
        Tensor<int,2,3,4>>);

    static_assert(std::is_constructible_v<
        Tensor<int,2,3,4>,
        TensorView<int,2,3,4>>);


    auto expr = t1 + t2;
    Tensor<int,2,3,4> r = expr;
    r = expr;

    auto fff = (!t1[0]).eval();

    Tensor<int, 3,4> vv5 = !t1[0];
    
    TensorView<int, 3,4> vv6 = vv5++;
    TensorView<int, 3,4> vv7 = ++vv5;
    auto dfgh = vv7++;

    Tensor<int, 3> cross1;
    Tensor<int, 3> cross2;
    Tensor<int, 3> cross3 = cross(cross1, cross2);

    Tensor<int, 2, 4> aaa;
    Tensor<int, 3, 2> bbb;

    Tensor<int, 2> ccc;

    auto res1 = Math::mul(aaa, bbb);

    auto res2 = Math::mul(aaa, ccc);

    Tensor<int, 2> ccc1({0, 1});
    Tensor<int, 2> ccc2 = {0, 1};
    Tensor<int, 2> ccc3(0, 1);

    auto ccc4 = (ccc2 * 1.f).eval();
    static_assert(std::same_as<typename decltype(ccc4)::ValueType, float>);

    Tensor<int, 4, 4> tt1(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);
    Tensor<int, 3, 3> tt2 = tt1;

    for(size_t c = 0; c < tt1.outerSize(); ++c) {
        auto col = tt1[c];
        for(size_t r = 0; r < tt1.outerSize(); ++r) {
            auto& val = col[r];
            std::cout << val << " ";
        }
        std::cout << "\n";
    }
    std::cout << "\n";

    for(size_t c = 0; c < tt2.outerSize(); ++c) {
        auto col = tt2[c];
        for(size_t r = 0; r < tt2.outerSize(); ++r) {
            auto& val = col[r];
            std::cout << val << " ";
        }
        std::cout << "\n";
    }

    tt1 = tt2;
    
    for(size_t c = 0; c < tt1.outerSize(); ++c) {
        auto col = tt1[c];
        for(size_t r = 0; r < tt1.outerSize(); ++r) {
            auto& val = col[r];
            std::cout << val << " ";
        }
        std::cout << "\n";
    }
    std::cout << "\n";
    
    tt1 = tt2;

    for(size_t c = 0; c < tt2.outerSize(); ++c) {
        auto col = tt2[c];
        for(size_t r = 0; r < tt2.outerSize(); ++r) {
            auto& val = col[r];
            std::cout << val << " ";
        }
        std::cout << "\n";
    }

    Tensor<int, 3> h1;
    Tensor<int, 3, 3> h2;
    h1.dot(h1);
    h1.length();
    h1.normalize();
    h1.distance(h1);
    h1.cross(h1);
    // h2.length();
    // h2.normalize();

    std::cout << "Done" << std::endl;
    return 0;
}