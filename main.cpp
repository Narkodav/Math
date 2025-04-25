#include <iostream>

//#include "BigInt.h"
//#include "bigIntFunctions/bigIntAddAndSub.h"
//#include "bigIntFunctions/bigIntMult.h"
//#include "bigIntFunctions/bigIntComparison.h"

#include "VectorArithmetic.h"

//
//template <typename T>
//    requires std::is_unsigned_v<T>
//T hi(T x) {
//    static const size_t halfBits = sizeof(T) * 4;
//    return x >> halfBits;
//}
//
//template <typename T>
//    requires std::is_unsigned_v<T>
//T lo(T x) {
//    static const size_t halfBits = sizeof(T) * 4;
//    return ((T(1) << halfBits) - 1) & x;
//}
//
//template <typename T>
//    requires std::is_unsigned_v<T>
//std::pair<T, T> multiply_with_overflow(T a, T b) {
//    T s0, s1, s2, s3;
//
//    T x = lo(a) * lo(b);
//    s0 = lo(x);
//
//    x = hi(a) * lo(b) + hi(x);
//    s1 = lo(x);
//    s2 = hi(x);
//
//    x = s1 + lo(a) * hi(b);
//    s1 = lo(x);
//
//    x = s2 + hi(a) * hi(b) + hi(x);
//    s2 = lo(x);
//    s3 = hi(x);
//
//    T result = s1 << (sizeof(T) * 4) | s0;
//    T carry = s3 << (sizeof(T) * 4) | s2;
//
//    return { carry, result };
//}

int main()
{
	//BigInt b = std::numeric_limits<int64_t>::min();
	//BigInt c = -6;
	//BigInt d = b + c;
	//BigInt b = -12341111111111111111111111111111111111111111_bi;
	//std::cout << b << std::endl;

	//std::vector<uint64_t> array1 = { 0x123456789ABCDEF0, 0xFEDCBA9876543210 };
	//std::vector<uint64_t> array2 = { 0xFEDCBA9876543210, 0x1 };

	//std::vector<uint64_t> result = multiplyMagnitudes(array1, array2);
	//std::cout << std::hex;

	//for (int i = result.size() - 1; i >= 0; i--)
	//	std::cout << result[i] << " ";

	std::string array1 = "12143125231654236346436537347437476354";
	std::string array2 = "12345678901234567890263643574574753746";
	std::string array3 = "0";

	std::string result = substract(array2, array1);
	std::cout << array2 << " - " << array1 << " = ";
	for (int i = 0; i < result.size(); i++)
		std::cout << result[i];
}