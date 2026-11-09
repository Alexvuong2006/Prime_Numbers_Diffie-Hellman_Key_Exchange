// bench_modexp.cpp - Do toc do mulmod va modular_exponentiation theo do lon modulus.
// Bien dich: g++ -std=c++14 -O2 -o bench_modexp bench_modexp.cpp && ./bench_modexp
#include <iostream>
#include <iomanip>
#include <chrono>
#include "../project_01_source/dh.cpp"

int main()
{
    const size_t sizes[] = {512, 768, 1024};
    std::cout << std::fixed << std::setprecision(3);
    for (size_t bits : sizes) {
        // modulus le co do dai dung `bits` bit (khong can nguyen to de do toc do)
        big_t m = (big_t::from_u64(1) << (bits - 1)) + big_t::from_hex("123456789abcdef1");
        big_t x = m - big_t::from_u64(12345), e = m - big_t::from_u64(777);

        int n = 20000;
        big_t t = x;
        auto t0 = std::chrono::steady_clock::now();
        for (int i = 0; i < n; i++) t = (t * x) % m;
        double mul_us = std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count() / n * 1e6;

        int k = 200;
        big_t r;
        t0 = std::chrono::steady_clock::now();
        for (int i = 0; i < k; i++) r = modular_exponentiation(x, e + big_t::from_u64(i), m);
        double exp_ms = std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count() / k * 1e3;

        std::cout << bits << "-bit: mulmod " << mul_us << " us, modexp(" << bits << "-bit exp) " << exp_ms << " ms\n";
        if (r.is_zero()) std::cout << "";
    }
    return 0;
}
