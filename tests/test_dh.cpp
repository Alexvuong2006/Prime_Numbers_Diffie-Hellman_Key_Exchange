// ============================================================================
// test_dh.cpp - Unit test modular_exponentiation, choose_generator, trao doi khoa DH
// Khong phu thuoc prime.cpp (Minh): dung 2 so nguyen to an toan da xac thuc bang Python:
//   - P512: safe prime 512 bit sinh bang Python (Miller-Rabin 40 vong cho ca p va q)
//   - OAKLEY2: nhom MODP 1024 bit cua RFC 2409 (Oakley Group 2), la safe prime
//     (RFC 3526 nho nhat la 1536 bit > MAX_PRIME_BITS nen khong dung duoc o day)
// Bien dich: g++ -std=c++14 -O2 -Wall -Wextra -o test_dh test_dh.cpp && ./test_dh
// ============================================================================
#include <iostream>
#include <cassert>
#include <string>
#include "../project_01_source/fips202.cpp"
#include "../project_01_source/prng.cpp"
#include "../project_01_source/bigint.cpp"
#include "../project_01_source/dh.cpp"

static const char *P512 =
    "b7c1ee3587d75cc9c5fc038596a743a8e48995bdb0a7e80b5bec6089af192477"
    "594ecf34067bd1dcb1423e0861ee0af6d1be2102dd05904afd708387c6d57a5b";
static const char *OAKLEY2 =
    "ffffffffffffffffc90fdaa22168c234c4c6628b80dc1cd129024e088a67cc74"
    "020bbea63b139b22514a08798e3404ddef9519b3cd3a431b302b0a6df25f1437"
    "4fe1356d6d51c245e485b576625e7ec6f44c42e9a637ed6b0bff5cb6f406b7ed"
    "ee386bfb5a899fa5ae9f24117c4b1fe649286651ece65381ffffffffffffffff";

static big_t H(const char *s) { return big_t::from_hex(s); }
static big_t U(uint64_t v) { return big_t::from_u64(v); }

// Khoa ngau nhien trong [2, p-2] chi de test (khong dung random_range cua Minh).
static big_t test_key(prng_ctx *ctx, const big_t &p)
{
    big_t r;
    for (size_t i = 0; i < BIG_LIMBS / 2; i++) r.digits[i] = prng_next_u64(ctx);
    return (r % (p - U(3))) + U(2); // lech nhe khong anh huong test
}

static void test_modexp_small()
{
    std::cout << "[Test] modular_exponentiation: gia tri nho & bien...\n";
    assert(modular_exponentiation(U(3), U(4), U(5)) == U(1));        // 81 mod 5
    assert(modular_exponentiation(U(2), U(10), U(1000)) == U(24));   // 1024 mod 1000
    assert(modular_exponentiation(U(7), U(0), U(13)) == U(1));       // exp = 0
    assert(modular_exponentiation(U(7), U(0), U(1)) == U(0));        // mod = 1
    assert(modular_exponentiation(U(0), U(5), U(13)) == U(0));       // base = 0
    assert(modular_exponentiation(U(0), U(0), U(13)) == U(1));       // 0^0 quy uoc 1
    assert(modular_exponentiation(U(20), U(3), U(7)) == U(6));       // base >= mod: 20 = 6 mod 7, 6^3 = 216 = 6 mod 7
    assert(modular_exponentiation(U(5), U(1), U(1000003)) == U(5));  // exp = 1
    assert(modular_exponentiation(U(5), U(3), big_t()) == big_t());  // mod = 0 -> 0, khong crash
    // Fermat nho: a^(p-1) = 1 mod p voi p = 2^61 - 1 (nguyen to Mersenne)
    uint64_t m61 = (1ULL << 61) - 1;
    assert(modular_exponentiation(U(123456789), U(m61 - 1), U(m61)) == U(1));
    std::cout << "  => PASS\n";
}

static void test_modexp_big()
{
    std::cout << "[Test] modular_exponentiation: so lon...\n";
    big_t p = H(P512), pm1 = p - U(1);
    // Fermat nho tren p 512 bit va Oakley2 1024 bit
    assert(modular_exponentiation(U(2), pm1, p) == U(1));
    assert(modular_exponentiation(U(0xdeadbeef), pm1, p) == U(1));
    big_t o = H(OAKLEY2);
    assert(modular_exponentiation(U(3), o - U(1), o) == U(1));
    // (-1)^2 = 1 va (-1)^3 = -1 mod p
    assert(modular_exponentiation(pm1, U(2), p) == U(1));
    assert(modular_exponentiation(pm1, U(3), p) == pm1);
    // a^(x+y) = a^x * a^y
    big_t a = H("123456789abcdef0fedcba9876543210"), x = H("abcdef1234567890abcdef"), y = H("fedcba0987654321fedcba");
    assert(modular_exponentiation(a, x + y, p) ==
           (modular_exponentiation(a, x, p) * modular_exponentiation(a, y, p)) % p);
    std::cout << "  => PASS\n";
}

static void test_generator(const char *name, const big_t &p)
{
    std::cout << "[Test] choose_generator tren " << name << " (" << p.bit_length() << " bit)...\n";
    big_t g = choose_generator(p);
    big_t q = (p - U(1)) >> 1;
    assert(g != U(1) && g != p - U(1));
    assert(g < p);
    assert(modular_exponentiation(g, q, p) == U(1)); // g thuoc nhom con bac q
    // q nguyen to va g != 1 => bac cua g dung bang q: g^2 != 1
    assert(modular_exponentiation(g, U(2), p) != U(1));
    std::cout << "    g = " << g.to_hex() << "\n  => PASS\n";
}

static void test_public_value_validation()
{
    std::cout << "[Test] is_valid_public_value...\n";
    big_t p = H(P512), g = choose_generator(p);
    assert(!is_valid_public_value(U(0), p));
    assert(!is_valid_public_value(U(1), p));
    assert(!is_valid_public_value(p - U(1), p));
    assert(!is_valid_public_value(p, p));
    assert(is_valid_public_value(g, p));
    assert(is_valid_public_value(modular_exponentiation(g, U(123456), p), p));
    // phan tu bac 2q (phi-thang du bac hai) phai bi tu choi: 2^q = -1 neu p = 3 hoac 5 mod 8
    big_t q = (p - U(1)) >> 1;
    for (uint64_t h = 2; h < 50; h++) {
        big_t v = U(h);
        bool in_subgroup = (modular_exponentiation(v, q, p) == U(1));
        assert(is_valid_public_value(v, p) == in_subgroup);
    }
    std::cout << "  => PASS\n";
}

static int run_dh(const big_t &p, prng_ctx *ctx, int rounds)
{
    big_t g = choose_generator(p);
    for (int i = 0; i < rounds; i++) {
        big_t a = test_key(ctx, p), b = test_key(ctx, p);
        assert(a >= U(2) && a <= p - U(2) && b >= U(2) && b <= p - U(2));
        big_t A = modular_exponentiation(g, a, p), B = modular_exponentiation(g, b, p);
        assert(is_valid_public_value(A, p) && is_valid_public_value(B, p));
        big_t sa = modular_exponentiation(B, a, p), sb = modular_exponentiation(A, b, p);
        if (!(sa == sb) || sa.is_zero() || sa == U(1)) return 0;
    }
    return 1;
}

static void test_dh()
{
    std::cout << "[Test] Diffie-Hellman: bi mat chung Alice == Bob...\n";
    prng_ctx ctx;
    uint8_t seed[32];
    // seed co dinh: tai lap duoc
    for (int i = 0; i < 32; i++) seed[i] = (uint8_t)(i * 7 + 1);
    prng_seed(&ctx, seed, sizeof(seed));
    assert(run_dh(H(P512), &ctx, 100));
    assert(run_dh(H(OAKLEY2), &ctx, 30));
    // seed ngau nhien tu he dieu hanh (chi chay duoc tren Linux/macOS)
#ifndef _WIN32
    for (int t = 0; t < 5; t++) {
        randombytes(seed, sizeof(seed));
        prng_seed(&ctx, seed, sizeof(seed));
        assert(run_dh(H(P512), &ctx, 20));
    }
#endif
    std::cout << "  => PASS (100 + 30 + 5x20 lan, p = 512 / 1024 bit)\n";
}

int main()
{
    std::cout << "========================================\n  UNIT TESTS: modexp + generator + DH\n========================================\n";
    test_modexp_small();
    test_modexp_big();
    test_generator("P512", H(P512));
    test_generator("Oakley Group 2", H(OAKLEY2));
    test_public_value_validation();
    test_dh();
    std::cout << "========================================\n  ALL DH TESTS PASSED\n========================================\n";
    return 0;
}
