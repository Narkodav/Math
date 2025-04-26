#include <iostream>

#include "BigInt.h"
#include "BigUint.h"
#include "BigFloat.h"

int main()
{
	//BigInt a = -12341111111111111111111111111111111111111111_bi;
	//auto b =  21513252346432623472347547254374457454768_bui;
	//BigInt c = a % b;

	//std::cout << a << " % " << std::endl;
	//std::cout << b << " = " << std::endl;
	//std::cout << c << std::endl;

	BigFloat f = 1243523.23246432614_bf;
	BigFloat g = 12245443523.214554_bf;

	BigFloat res = f + g;

	std::cout << res << std::endl;
}