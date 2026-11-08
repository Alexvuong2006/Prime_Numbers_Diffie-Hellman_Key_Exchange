// ============================================================================
// test_bigint.cpp - Unit test cho lõi số lớn BigInt (uint_t<N>) & Phép toán mô-đun
// Biên dịch: g++ -std=c++14 -O2 -I../project_01_source -o test_bigint test_bigint.cpp
// ============================================================================
#include <iostream>
#include <cassert>
#include <string>
#include "../project_01_source/bigint.cpp"
#include "../project_01_source/dh.cpp"

void test_basic_and_conversions()
{
    std::cout << "[Test] Basic & Conversions...\n";
    uint512_t zero;
    assert(zero.is_zero());
    assert(!zero.is_odd());
    assert(zero.bit_length() == 0);
    assert(zero.to_hex() == "0");
    assert(zero.to_dec() == "0");

    uint512_t one = uint512_t::from_u64(1);
    assert(!one.is_zero());
    assert(one.is_odd());
    assert(one.bit_length() == 1);
    assert(one.to_hex() == "1");
    assert(one.to_dec() == "1");

    uint512_t val = uint512_t::from_hex("0x123456789abcdef0");
    assert(val.to_hex() == "123456789abcdef0");
    assert(val.bit_length() == 61);

    // Large hex conversion
    std::string hex_str = "ffffffffffffffffffffffffffffffff"; // 128 bit = 2 limbs all 1s
    uint512_t val128 = uint512_t::from_hex(hex_str);
    assert(val128.bit_length() == 128);
    assert(val128.to_hex() == hex_str);

    // Decimal conversion check: 2^64 = 18446744073709551616
    uint512_t two_64;
    two_64.digits[1] = 1;
    assert(two_64.to_dec() == "18446744073709551616");

    std::cout << "  => PASS: Basic & Conversions\n";
}

void test_comparisons()
{
    std::cout << "[Test] Comparisons...\n";
    uint512_t a = uint512_t::from_u64(100);
    uint512_t b = uint512_t::from_u64(200);
    uint512_t c = uint512_t::from_u64(100);

    assert(a == c);
    assert(a != b);
    assert(a < b);
    assert(a <= b);
    assert(a <= c);
    assert(b > a);
    assert(b >= a);
    assert(c >= a);

    uint512_t big1; big1.digits[2] = 5;
    uint512_t big2; big2.digits[1] = 999999999;
    assert(big1 > big2);

    std::cout << "  => PASS: Comparisons\n";
}

void test_arithmetic()
{
    std::cout << "[Test] Arithmetic (+, -, *)...\n";
    uint512_t max_u64 = uint512_t::from_u64(0xFFFFFFFFFFFFFFFFULL);
    uint512_t one = uint512_t::from_u64(1);

    // Addition with carry into limb 1
    uint512_t sum = max_u64 + one;
    assert(sum.digits[0] == 0);
    assert(sum.digits[1] == 1);
    assert(sum.bit_length() == 65);

    // Subtraction with borrow
    uint512_t diff = sum - one;
    assert(diff == max_u64);

    // Multiplication
    uint512_t p = max_u64 * max_u64;
    // (2^64 - 1)^2 = 2^128 - 2*2^64 + 1
    // digits[0] = 1, digits[1] = 2^64 - 2 = 0xFFFFFFFFFFFFFFFE
    assert(p.digits[0] == 1);
    assert(p.digits[1] == 0xFFFFFFFFFFFFFFFEULL);
    assert(p.digits[2] == 0);

    std::cout << "  => PASS: Arithmetic (+, -, *)\n";
}

void test_shifts_and_mod()
{
    std::cout << "[Test] Shifts & Modulo...\n";
    uint512_t x = uint512_t::from_u64(1);
    uint512_t x_shl = x << 100;
    assert(x_shl.bit_length() == 101);
    assert(x_shl.test_bit(100));
    assert(!x_shl.test_bit(99));

    uint512_t x_shr = x_shl >> 100;
    assert(x_shr == x);

    // mod_small
    uint512_t val = uint512_t::from_u64(1007);
    assert(mod_small(val, 10) == 7);
    assert(mod_small(val, 100) == 7);

    // Division / Modulo
    uint512_t a = uint512_t::from_hex("123456789abcdef0123456789abcdef0");
    uint512_t b = uint512_t::from_hex("fedcba9876543210");
    uint512_t q = a / b;
    uint512_t r = a % b;

    assert(r < b);
    assert(b * q + r == a);

    std::cout << "  => PASS: Shifts & Modulo\n";
}

void test_pseudocode_modular()
{
    std::cout << "[Test] Pseudocode Modular Operations (AddMod, MultMod, ExpMod)...\n";
    big_t p = big_t::from_hex("ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
    big_t a = big_t::from_hex("123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef0");
    big_t b = big_t::from_hex("fedcba9876543210fedcba9876543210fedcba9876543210fedcba9876543210");

    // AddMod check
    big_t add_expected = (a + b) % p;
    big_t add_actual = AddMod(a, b, p);
    assert(add_expected == add_actual);

    // MultMod check
    big_t mul_expected = (a * b) % p;
    big_t mul_actual = MultMod(a, b, p);
    assert(mul_expected == mul_actual);

    // ExpMod check vs modular_exponentiation
    big_t exp = big_t::from_u64(65537);
    big_t exp_expected = modular_exponentiation(a, exp, p);
    big_t exp_actual = ExpMod(a, exp, p);
    assert(exp_expected == exp_actual);

    std::cout << "  => PASS: Pseudocode Modular Operations (AddMod, MultMod, ExpMod)\n";
}

int main()
{
    std::cout << "========================================\n";
    std::cout << "  RUNNING UNIT TESTS FOR BIGINT\n";
    std::cout << "========================================\n";

    test_basic_and_conversions();
    test_comparisons();
    test_arithmetic();
    test_shifts_and_mod();
    test_pseudocode_modular();

    std::cout << "========================================\n";
    std::cout << "  ALL UNIT TESTS PASSED SUCCESSFULLY!   \n";
    std::cout << "========================================\n";
    return 0;
}
