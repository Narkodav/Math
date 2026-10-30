#include <iostream>
#include <iomanip>
#include <bitset>
#include <cstdlib>
#include <iostream>

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

using Coordinate3D = Math::Tensor<uint32_t, 3>;
using CoordinateSigned3D = Math::Tensor<int32_t, 3>;
using Position3D = Math::Tensor<float, 3>;
using Direction3D = Math::Tensor<int32_t, 3>;
using Size3D = Math::Tensor<uint32_t, 3>;

using Coordinate2D = Math::Tensor<uint32_t, 2>;
using CoordinateSigned2D = Math::Tensor<int32_t, 2>;
using Position2D = Math::Tensor<float, 2>;
using Direction2D = Math::Tensor<int32_t, 2>;
using Size2D = Math::Tensor<uint32_t, 2>;

using ChunkRelativeCoord3D = Math::Tensor<int32_t, 3>;

constexpr std::array<Direction2D, 4> s_directions2D = {
    Direction2D(0, -1), Direction2D(1, 0), Direction2D(0, 1), Direction2D(-1, 0),
};

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
    Tensor<int, 5, 5> tt3(tt1, 3);

    for(auto col : tt1) {
        for(auto& val : col) {
            std::cout << val << " ";
        }
        std::cout << "\n";
    }
    std::cout << "\n";

    for(auto col : tt2) {
        for(auto& val : col) {
            std::cout << val << " ";
        }
        std::cout << "\n";
    }
    std::cout << "\n";

    tt1 = tt2;
    
    for(auto col : tt1) {
        for(auto& val : col) {
            std::cout << val << " ";
        }
        std::cout << "\n";
    }
    std::cout << "\n";
    
    tt1 = tt2;

    for(auto col : tt2) {
        for(auto& val : col) {
            std::cout << val << " ";
        }
        std::cout << "\n";
    }
    std::cout << "\n";

    for(auto col : tt3) {
        for(auto& val : col) {
            std::cout << val << " ";
        }
        std::cout << "\n";
    }
    std::cout << "\n";


    Tensor<int, 3> h1;
    Tensor<int, 3, 3> h2;
    Tensor<int, 4> h3;
    Tensor<int, 4, 4> h4(h2, 2);
    h1.dot(h1);
    h1.length();
    h1.normalize();
    h1.distance(h1);
    h1.cross(h1);
    // h2.length();
    // h2.normalize();

    auto expr1 = Math::FVec3() + Math::FVec3();

    Math::lookAt(Math::FVec3(), Math::FVec3() + Math::FVec3(), Math::FVec3());

    static_assert(decltype(expr1)::ShapeType::dims().size() == 1);
    static_assert(decltype(expr1)::ShapeType::dims()[0] == 3);
    static_assert(Vec3Expr<decltype(expr1)>);
    
    h3 = h1 + h3;

    h4.translate(h1);
    h4.transpose();

    static constexpr Tensor<int, 3> cten{1, 2, 3};

    auto ress1 = Math::all(h4);
    auto ress2 = Math::none(h4);

    auto ress3 = Math::epsilonEqual(h1, h3, 0);

    Tensor<bool, 3> jj = cten == h3; 

    // automatically converts to bool using Math::all
    if(cten == h3) {

    } else {

    }

    const Tensor<int, 4> ct{};
    auto cv = ct.view();

    Math::IVec3 coord1(ct * jj, 1);

    struct UserVec4 {
        float x, y, z, w;
    };

    UserVec4 userVec4{1, 2, 3, 4};

    coord1 = userVec4;
    
    Math::IVec3 coord2(userVec4);

    UserVec4 userVec4Res = coord1.as<UserVec4>(5);

    std::cout << userVec4Res.x << userVec4Res.y << userVec4Res.z << userVec4Res.w << std::endl;

    Math::IVec3 init(1, 2);

    Math::FVec4 vec(0, 0, 4.65251535E-13, 4);

    for(auto& val : vec) {
        std::cout << val << " ";
    }
    std::cout << "\n";

    auto vecRed = Math::FVec3(vec);

    static_assert(std::convertible_to<Math::FVec4, float>);

    for(auto& val : vecRed) {
        std::cout << val << " ";
    }
    std::cout << "\n";

    vecRed = vecRed.normalize();

    std::cout << vecRed[0] << " " << vecRed[1] << " " << vecRed[2] << std::endl;

    FVec3 test = (Math::FVec3(1.0f) / Math::FVec3(2.0f) * 4).eval();

    test = test.normalize();

    std::cout << sizeof(decltype(Math::FVec4() + Math::FVec4() - Math::FVec4() + vec + 4)) << std::endl;

    std::cout << test[0] << " " << test[1] << " " << test[2] << std::endl;

    auto res = Math::lookAt(Math::FVec3(0.0f), Math::FVec3(0.0f, 0.0f, -1.0f), Math::FVec3(0.0f, 1.0f, 0.0f));
    std::cout << '\n';
    for(auto col : res) {
        for(auto val : col) {
            std::cout << val << " ";
        }
        std::cout << '\n';
    }
    std::cout << std::endl;

	const std::array<Math::FMat4, 6> conversionMatrices = {
		// Front face - no rotation needed (identity matrix)
		Math::FMat4::identity(),
		// Right face - 90 degrees around Y axis
		Math::rotate(Math::FMat4::identity(), Math::FVec3(0.0f, 1.0f, 0.0f), Math::radians(90.0f)),
		// Top face - -90 degrees around X axis
		Math::rotate(Math::FMat4::identity(), Math::FVec3(1.0f, 0.0f, 0.0f), Math::radians(90.0f)),
		// Back face - 180 degrees around Y axis
		Math::rotate(Math::FMat4::identity(), Math::FVec3(0.0f, 1.0f, 0.0f), Math::radians(180.0f)),
		// Left face - -90 degrees around Y axis
		Math::rotate(Math::FMat4::identity(), Math::FVec3(0.0f, 1.0f, 0.0f), Math::radians(-90.0f)),
		// Bottom face - 90 degrees around X axis
		Math::rotate(Math::FMat4::identity(), Math::FVec3(1.0f, 0.0f, 0.0f), Math::radians(-90.0f))
	};

    std::cout << "Done" << std::endl;
    return 0;
}