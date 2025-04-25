#pragma once
#include <cstdlib>
#include <cstdint>
#include <type_traits>
#include <vector>

#include "bigIntMult.h"
#include "bigIntComparison.h"

// Returns (carry, a - b) where carry = 1 if a < b
template<typename T>
    requires std::is_arithmetic_v<T>
std::pair<bool, std::vector<T>> subWithBorrow(
    const std::vector<T>& a,
    const std::vector<T>& b
) {
    std::vector<T> res;
    T borrow = 0;
    bool underflow = false;

    for (size_t i = 0; i < a.size(); ++i) {
        T bi = (i < b.size()) ? b[i] : 0;
        T diff = a[i] - bi - borrow;
        borrow = (bi + borrow > a[i]) ? 1 : 0;
        res.push_back(diff);
    }

    underflow = (borrow != 0);
    return { underflow, res };
}

// Left-shifts a number by `shift` bits (equivalent to *= 2^shift)
template<typename T>
    requires std::is_arithmetic_v<T>
std::vector<uint64_t> shiftLeft(const std::vector<T>& num, uint64_t shift) {
    if (num.empty() || shift == 0) return num;
    static const size_t bitSize = sizeof(T) * 8;
    size_t limb_shift = shift / bitSize;
    size_t bit_shift = shift % bitSize;

    std::vector<T> res(num.size() + limb_shift + 1, 0);

    if (bit_shift == 0) {
        for (size_t i = 0; i < num.size(); ++i)
            res[i + limb_shift] = num[i];
    }
    else {
        T carry = 0;
        for (size_t i = 0; i < num.size(); ++i) {
            T val = (num[i] << bit_shift) | carry;
            carry = num[i] >> (bitSize - bit_shift);
            res[i + limb_shift] = val;
        }
        res[num.size() + limb_shift] = carry;
    }

    // Remove leading zeros
    while (res.size() > 1 && res.back() == 0)
        res.pop_back();
    return res;
}

std::vector<uint64_t> naiveDivide(
    std::vector<uint64_t> dividend,
    std::vector<uint64_t> divisor
) {
    //// Handle edge cases
    //if (compare(dividend, divisor) < 0) return { 0 };  // dividend < divisor → return 0
    //if (divisor.empty()) throw std::runtime_error("Division by zero");

    // Normalize divisor (shift until MSB is 1)
    int shift = 0;
    uint64_t divisor_msb = divisor.back();
    while ((divisor_msb >> 63) == 0) {
        divisor_msb <<= 1;
        shift++;
    }
    divisor = shiftLeft(divisor, shift);
    dividend = shiftLeft(dividend, shift);

    size_t n = divisor.size();
    size_t m = dividend.size() - n;
    std::vector<uint64_t> quotient(m + 1, 0);

    // Main loop
    for (int i = m; i >= 0; --i) {
        // Estimate q̂ = min((dividend[i+n] * 2^64 + dividend[i+n-1]) / divisor.back(), 2^64 - 1)
        uint64_t q_hat;
        if (i + n >= dividend.size()) {
            q_hat = 0;
        }
        else {
            uint64_t d1 = (i + n < dividend.size()) ? dividend[i + n] : 0;
            uint64_t d0 = (i + n - 1 < dividend.size()) ? dividend[i + n - 1] : 0;

            if (d1 == divisor.back()) {
                q_hat = UINT64_MAX;
            }
            else {
                __uint128_t num = (static_cast<__uint128_t>(d1) << 64) | d0;
                q_hat = num / divisor.back();
            }
        }

        // Adjust q̂ if too large
        auto product = shiftLeft(divisor, i);
        product = multiplyMagnitudes(product, q_hat);
        while (compareMagnitutedsLess(dividend, product)) {
            q_hat--;
            product = substractMagnitudes(product, shiftLeft(divisor, i));
        }

        // Subtract and set quotient limb
        dividend = substractMagnitudes(dividend, product);
        quotient[i] = q_hat;
    }

    // Remove leading zeros
    while (quotient.size() > 1 && quotient.back() == 0) quotient.pop_back();
    return quotient;
}

// Recursive division (divides a 2n-limb number by an n-limb number)
template<typename T>
    requires std::is_arithmetic_v<T>
std::vector<T> burnikelZiegler(
    const std::vector<T>& dividend,
    const std::vector<T>& divisor
) {
    size_t n = divisor.size();

    // Base case: If divisor is small, use naive division
    if (n <= 2) {
        // Implement naive division here (e.g., Knuth's Algorithm D)
        // ...
    }

    // Split dividend into A1, A2, A3, A4 (each of size n/2)
    size_t half = n / 2;
    auto A1 = std::vector<T>(dividend.begin(), dividend.begin() + half);
    auto A2 = std::vector<T>(dividend.begin() + half, dividend.begin() + 2 * half);
    auto A3 = std::vector<T>(dividend.begin() + 2 * half, dividend.begin() + 3 * half);
    auto A4 = std::vector<T>(dividend.begin() + 3 * half, dividend.end());

    // Step 1: Compute Q1 = (A1 * B^2 + A2 * B + A3) / divisor
    auto numerator = addMagnitudes(shiftLeft(A1, 128), addMagnitudes(shiftLeft(A2, 64), A3));
    auto Q1 = burnikelZiegler(numerator, divisor);

    // Step 2: Compute R1 = (A1 * B^2 + A2 * B + A3) - Q1 * divisor
    auto product = multiplyMagnitudes(Q1, divisor);
    auto R1 = subWithBorrow(numerator, product).second;

    // Step 3: Compute Q2 = (R1 * B + A4) / divisor
    numerator = addMagnitudes(shiftLeft(R1, 64), A4);
    auto Q2 = burnikelZiegler(numerator, divisor);

    // Final quotient: Q1 * B^64 + Q2
    auto quotient = addMagnitudes(shiftLeft(Q1, 64), Q2);
    return quotient;
}