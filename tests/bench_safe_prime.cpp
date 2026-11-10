// ============================================================================
// bench_safe_prime.cpp - Benchmark sinh số nguyên tố an toàn 512 / 768 / 1024 bit
// Phụ trách: Lâm Duy Minh (24120091)
// Biên dịch: g++ -std=c++14 -O2 -Wall -Wextra -o bench_safe_prime bench_safe_prime.cpp
// ============================================================================
#define _CRT_RAND_S
#include <iostream>
#include <iomanip>
#include <chrono>
#include "../project_01_source/fips202.cpp"
#include "../project_01_source/prng.cpp"
#include "../project_01_source/bigint.cpp"
#include "../project_01_source/dh.cpp"
#include "../project_01_source/prime.cpp"

struct BenchConfig
{
    size_t bits;
    int trials;
};

int main()
{
    std::cout << "==============================================================================\n";
    std::cout << "  BENCHMARK SINH SO NGUYEN TO AN TOAN (SAFE PRIME) 512 / 768 / 1024 BIT\n";
    std::cout << "==============================================================================\n";

    prng_ctx ctx;
    uint8_t seed[32] = {
        0x24, 0x12, 0x00, 0x70, 0x24, 0x12, 0x00, 0x89,
        0x24, 0x12, 0x00, 0x91, 0x18, 0xaa, 0xbb, 0xcc,
        0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef,
        0xfe, 0xdc, 0xba, 0x98, 0x76, 0x54, 0x32, 0x10
    };
    prng_seed(&ctx, seed, sizeof(seed));

    const BenchConfig configs[] = {
        {512, 10},
        {768, 5},
        {1024, 3}
    };

    std::cout << std::fixed << std::setprecision(4);
    for (const auto &cfg : configs)
    {
        double total_time = 0.0;
        size_t sum_candidates = 0;
        size_t sum_passed_sieve = 0;
        size_t sum_passed_q_base2 = 0;

        std::cout << "\n--- Kich thuoc: " << cfg.bits << " bit (" << cfg.trials << " lan chay, rounds = 40) ---\n";
        for (int t = 1; t <= cfg.trials; t++)
        {
            auto t0 = std::chrono::steady_clock::now();
            big_t p = generate_safe_prime(&ctx, cfg.bits, 40);
            auto t1 = std::chrono::steady_clock::now();
            double dt = std::chrono::duration<double>(t1 - t0).count();

            total_time += dt;
            sum_candidates += g_last_safe_prime_stats.total_candidates;
            sum_passed_sieve += g_last_safe_prime_stats.passed_sieve;
            sum_passed_q_base2 += g_last_safe_prime_stats.passed_q_base2;

            std::cout << "  [Lan " << std::setw(2) << t << "] Thoi gian: " << std::setw(8) << dt << " s"
                      << " | Ung vien q: " << std::setw(7) << g_last_safe_prime_stats.total_candidates
                      << " | Qua sang (<2000): " << std::setw(4) << g_last_safe_prime_stats.passed_sieve
                      << " | Qua MR-base2(q): " << std::setw(3) << g_last_safe_prime_stats.passed_q_base2
                      << " | bit_length(p): " << p.bit_length() << "\n";
        }

        double avg_time = total_time / cfg.trials;
        double avg_candidates = static_cast<double>(sum_candidates) / cfg.trials;
        double avg_sieve = static_cast<double>(sum_passed_sieve) / cfg.trials;
        double avg_q_base2 = static_cast<double>(sum_passed_q_base2) / cfg.trials;
        double sieve_reject_rate = 100.0 * (1.0 - avg_sieve / avg_candidates);

        std::cout << "  => TRUNG BINH (" << cfg.bits << " bit):\n"
                  << "     - Thoi gian trung binh       : " << avg_time << " s (" << (avg_time * 1000.0) << " ms)\n"
                  << "     - So ung vien q da thu       : " << std::setprecision(1) << avg_candidates << "\n"
                  << "     - So ung vien vuot qua sang  : " << avg_sieve
                  << " (ty le loai cua sang: " << std::setprecision(2) << sieve_reject_rate << "%)\n"
                  << "     - So q nguyen to (MR base 2) : " << std::setprecision(1) << avg_q_base2 << "\n"
                  << std::setprecision(4);
    }

    std::cout << "==============================================================================\n";
    return 0;
}

