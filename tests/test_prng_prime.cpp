// ============================================================================
// test_prng_prime.cpp - Unit test cho PRNG, random_bits, random_range & Miller-Rabin
// Biên dịch: g++ -std=c++14 -O2 -Wall -Wextra -o test_prng_prime test_prng_prime.cpp
// ============================================================================
#define _CRT_RAND_S
#include <iostream>
#include <cassert>
#include <cstring>
#include <vector>
#include "../project_01_source/fips202.cpp"
#include "../project_01_source/prng.cpp"
#include "../project_01_source/bigint.cpp"
#include "../project_01_source/dh.cpp"
#include "../project_01_source/prime.cpp"

void test_prng_determinism_and_entropy()
{
    std::cout << "[Test 1] PRNG Determinism & OS Entropy (randombytes)...\n";
    uint8_t seed1[32], seed2[32];
    randombytes(seed1, sizeof(seed1));
    randombytes(seed2, sizeof(seed2));

    // Hai lần lấy 32 byte entropy từ OS không được trùng nhau
    assert(std::memcmp(seed1, seed2, sizeof(seed1)) != 0);

    // Cùng một seed cố định phải sinh ra đúng một dãy số giả ngẫu nhiên
    prng_ctx ctx1, ctx2;
    prng_seed(&ctx1, seed1, sizeof(seed1));
    prng_seed(&ctx2, seed1, sizeof(seed1));

    for (int i = 0; i < 100; i++) {
        assert(prng_next_u16(&ctx1) == prng_next_u16(&ctx2));
        assert(prng_next_u32(&ctx1) == prng_next_u32(&ctx2));
        assert(prng_next_u64(&ctx1) == prng_next_u64(&ctx2));
    }

    // Kiểm tra lấy > 255 byte qua tham số size_t outlen
    uint8_t big_buf[300];
    std::memset(big_buf, 0, sizeof(big_buf));
    randombytes(big_buf, sizeof(big_buf));
    bool non_zero_tail = false;
    for (size_t i = 256; i < 300; i++) {
        if (big_buf[i] != 0) non_zero_tail = true;
    }
    assert(non_zero_tail);
    std::cout << "  => PASS: PRNG & randombytes(size_t)\n";
}

void test_random_bits_and_range()
{
    std::cout << "[Test 2] random_bits & random_range (Rejection Sampling)...\n";
    prng_ctx ctx;
    uint8_t seed[32] = {1, 2, 3, 4, 5, 6, 7, 8};
    prng_seed(&ctx, seed, sizeof(seed));

    size_t test_sizes[] = {1, 2, 7, 63, 64, 65, 127, 128, 256, 511, 512, 1024};
    for (size_t b : test_sizes) {
        for (int trial = 0; trial < 20; trial++) {
            big_t x = random_bits(&ctx, b);
            assert(x.bit_length() == b);
            assert(x.test_bit(b - 1));
        }
    }

    // Kiểm tra random_range trên khoảng nhỏ [2, 12] (đảm bảo cả nửa dưới và nửa trên đều xuất hiện)
    big_t lo = big_t::from_u64(2);
    big_t hi = big_t::from_u64(12);
    bool seen_low = false, seen_high = false;
    for (int i = 0; i < 200; i++) {
        big_t v = random_range(&ctx, lo, hi);
        assert(v >= lo && v <= hi);
        if (v <= big_t::from_u64(5)) seen_low = true;
        if (v >= big_t::from_u64(9)) seen_high = true;
    }
    assert(seen_low && seen_high);

    // Kiểm tra random_range trên số lớn 512-bit (sinh khóa riêng trong [2, p - 2])
    big_t p512 = random_bits(&ctx, 512);
    for (int i = 0; i < 20; i++) {
        big_t priv = generate_private_key(&ctx, p512);
        assert(priv >= big_t::from_u64(2));
        assert(priv <= p512 - big_t::from_u64(2));
    }
    std::cout << "  => PASS: random_bits & random_range\n";
}

void test_miller_rabin_u64_and_big()
{
    std::cout << "[Test 3] Miller-Rabin (uint64_t & big_t, including Carmichael numbers)...\n";
    prng_ctx ctx;
    uint8_t seed[32] = {0x42, 0x13, 0x37};
    prng_seed(&ctx, seed, sizeof(seed));

    // Các số nguyên tố nhỏ và vừa
    uint64_t small_primes[] = {2, 3, 5, 7, 11, 13, 17, 19, 23, 97, 1009, 65537, 1000000007ULL, 2305843009213693951ULL};
    for (uint64_t p : small_primes) {
        assert(is_probable_prime_u64(&ctx, p, 40));
        assert(is_probable_prime(&ctx, big_t::from_u64(p), 40));
    }

    // Các hợp số và số Carmichael nổi tiếng (561, 1105, 1729, 41041, 825265, 321197185)
    uint64_t composites[] = {0, 1, 4, 6, 9, 15, 21, 91, 561, 1105, 1729, 2465, 2821, 6601, 41041, 825265, 321197185ULL};
    for (uint64_t c : composites) {
        assert(!is_probable_prime_u64(&ctx, c, 40));
        assert(!is_probable_prime(&ctx, big_t::from_u64(c), 40));
    }

    // Số nguyên tố Mersenne M_127 = 2^127 - 1 (127 bit)
    big_t m127 = (big_t::from_u64(1) << 127) - big_t::from_u64(1);
    assert(is_probable_prime(&ctx, m127, 25));

    // Hợp số 128 bit: (2^61 - 1) * (2^67 - 1)
    big_t c128 = ((big_t::from_u64(1) << 61) - big_t::from_u64(1)) *
                 ((big_t::from_u64(1) << 67) - big_t::from_u64(1));
    assert(!is_probable_prime(&ctx, c128, 25));

    // Kiểm tra một số nguyên tố an toàn 256-bit đã biết:
    // p = 0xFFFFFFFFFFFFFFFFC90FDAA22168C234C4C6628B80DC1CD129024E088A67CC74... (RFC 3526 prefix test)
    // Dùng số nguyên tố 192-bit chuẩn NIST P-192: p = 2^192 - 2^64 - 1
    big_t p192 = (big_t::from_u64(1) << 192) - (big_t::from_u64(1) << 64) - big_t::from_u64(1);
    assert(is_probable_prime(&ctx, p192, 15));

    std::cout << "  => PASS: Miller-Rabin (uint64_t & big_t)\n";
}

void test_generate_safe_prime_and_dh()
{
    std::cout << "[Test 4] generate_safe_prime (sieve + base-2 + Miller-Rabin) & DH...\n";
    prng_ctx ctx;
    uint8_t seed[32] = {0x18, 0x24, 0x12, 0x00, 0x91};
    prng_seed(&ctx, seed, sizeof(seed));

    // Kiểm tra sinh safe prime 64-bit, 128-bit và 512-bit
    size_t sizes[] = {64, 128, 512};
    for (size_t bits : sizes) {
        big_t p = generate_safe_prime(&ctx, bits, 40);
        assert(p.bit_length() == bits);
        assert(p.is_odd());

        big_t q = (p - big_t::from_u64(1)) >> 1;
        assert(q.bit_length() == bits - 1);
        assert(is_probable_prime(&ctx, q, 40));
        assert(is_probable_prime(&ctx, p, 40));

        // Kiểm tra toàn trình Diffie-Hellman trên safe prime vừa sinh
        big_t g = choose_generator(p);
        big_t a = generate_private_key(&ctx, p);
        big_t b = generate_private_key(&ctx, p);
        big_t A = modular_exponentiation(g, a, p);
        big_t B = modular_exponentiation(g, b, p);
        assert(is_valid_public_value(A, p) && is_valid_public_value(B, p));
        big_t s_a = modular_exponentiation(B, a, p);
        big_t s_b = modular_exponentiation(A, b, p);
        assert(s_a == s_b && s_a > big_t::from_u64(1));

        std::cout << "  - Safe prime " << bits << "-bit OK (candidates tried: "
                  << g_last_safe_prime_stats.total_candidates
                  << ", passed sieve: " << g_last_safe_prime_stats.passed_sieve
                  << ", passed q base-2: " << g_last_safe_prime_stats.passed_q_base2 << ")\n";
    }
    std::cout << "  => PASS: generate_safe_prime & end-to-end DH\n";
}

int main()
{
    std::cout << "========================================================\n";
    std::cout << "  RUNNING UNIT TESTS FOR PRNG & PRIME (MINH - 24120091) \n";
    std::cout << "========================================================\n";

    test_prng_determinism_and_entropy();
    test_random_bits_and_range();
    test_miller_rabin_u64_and_big();
    test_generate_safe_prime_and_dh();

    std::cout << "========================================================\n";
    std::cout << "  ALL PRNG & PRIME TESTS PASSED SUCCESSFULLY!           \n";
    std::cout << "========================================================\n";
    return 0;
}

