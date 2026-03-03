#include <iostream>
#include <iomanip>
#include "BigNum/BigInt.h"
#include "LinearAlgebra/Matrix.h"

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

int main() {
    Math::BigNum::BigInt num = 13231542352314837689345076983754980267398547693285476985376893547698457968548966521461346134763473147_bi;
    std::cout << num << std::endl;

    Math::LinearAlgebra::Mat<float, 2, 2> mat;

    Math::LinearAlgebra::Vec<float, 2> vec;

    auto col = mat[1];

    auto& val = col[1];

    auto res = mat * vec;

    vec.length();
    vec.normalize().view();

    vec.view().view();

}