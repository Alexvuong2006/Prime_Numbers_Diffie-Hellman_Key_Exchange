// ============================================================================
// main.cpp - Chương trình chính trao đổi khóa Diffie-Hellman
// Tích hợp và hoàn thiện: Vương Hữu Khang (24120070) & Tạ Quang Mạnh (24120089)
//
// Cách biên dịch (chỉ 1 file main.cpp theo mô hình unity-build):
//   g++ -std=c++14 -O2 -Wall -Wextra -o dh main.cpp
//
// Cách chạy:
//   ./dh                         # chạy mặc định (512 bit, seed ngẫu nhiên)
//   ./dh --bits 512 --rounds 40  # tùy chỉnh số bit và số vòng Miller-Rabin
//   ./dh --seed <32-byte-hex>    # chạy với seed cố định để kiểm thử/debug
// ============================================================================

#define _CRT_RAND_S // Bắt buộc ở đầu file để dùng rand_s trên Windows

#include <iostream>
#include <iomanip>
#include <chrono>
#include <string>
#include <cstring>
#include <cstdlib>
#include <cctype>

// Các thành phần của chương trình (theo thứ tự phụ thuộc)
#include "fips202.cpp"
#include "prng.cpp"
#include "bigint.cpp"
#include "dh.cpp"
#include "prime.cpp"

void print_usage(const char *prog_name)
{
    std::cout << "Su dung: " << prog_name << " [tuy chon]\n"
              << "Tuy chon:\n"
              << "  --bits <N>      Do dai bit cua so nguyen to p (mac dinh: 512, toi thieu: 512)\n"
              << "  --rounds <K>    So vong kiem tra Miller-Rabin (mac dinh: 40)\n"
              << "  --seed <HEX>    Seed co dinh (chuoi hex 64 ky tu = 32 byte) de debug\n"
              << "  -h, --help      Hien thi huong dan nay\n";
}

int main(int argc, char *argv[])
{
    size_t bit_size = 512;
    int rounds = 40;
    std::string fixed_seed_hex = "";
    bool use_fixed_seed = false;

    // ---------------------- 1. Xu ly tham so dong lenh ----------------------
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "-h" || arg == "--help") {
            print_usage(argv[0]);
            return 0;
        } else if (arg == "--bits" && i + 1 < argc) {
            char *end = nullptr;
            unsigned long v = std::strtoul(argv[++i], &end, 10);
            if (end == argv[i] || *end != '\0' || v == 0) {
                std::cerr << "Loi: --bits phai la so nguyen duong.\n";
                return 1;
            }
            bit_size = static_cast<size_t>(v);
        } else if (arg == "--rounds" && i + 1 < argc) {
            char *end = nullptr;
            long v = std::strtol(argv[++i], &end, 10);
            if (end == argv[i] || *end != '\0' || v < 1 || v > 1000) {
                std::cerr << "Loi: --rounds phai nam trong khoang [1, 1000].\n";
                return 1;
            }
            rounds = static_cast<int>(v);
        } else if (arg == "--seed" && i + 1 < argc) {
            fixed_seed_hex = argv[++i];
            use_fixed_seed = true;
            bool ok = (fixed_seed_hex.length() == 64);
            for (size_t k = 0; ok && k < fixed_seed_hex.length(); k++)
                ok = std::isxdigit(static_cast<unsigned char>(fixed_seed_hex[k])) != 0;
            if (!ok) {
                std::cerr << "Loi: --seed phai la chuoi hex dung 64 ky tu (32 byte).\n";
                return 1;
            }
        } else {
            std::cerr << "Loi: Tham so khong hop le: " << arg << "\n";
            print_usage(argv[0]);
            return 1;
        }
    }

    if (bit_size < 512) {
        std::cerr << "Canh bao: De bai yeu cau do lon toi thieu cua p la 512 bit.\n";
    }
    if (bit_size > MAX_PRIME_BITS) {
        std::cerr << "Loi: bit_size vuot qua MAX_PRIME_BITS (" << MAX_PRIME_BITS
                  << "). Hay tang MAX_PRIME_BITS trong bigint.cpp.\n";
        return 1;
    }

    std::cout << "===============================================================\n"
              << "  DO AN 1: SO NGUYEN TO & TRAO DOI KHOA DIFFIE-HELLMAN\n"
              << "  Nhom 18 - Mon: Nhap mon ma hoa mat ma\n"
              << "===============================================================\n"
              << "Cau hinh:\n"
              << "  - Do dai bit p: " << bit_size << " bit\n"
              << "  - So vong Miller-Rabin: " << rounds << "\n";

    // -------------------------- 2. Khoi tao PRNG ----------------------------
    prng_ctx ctx;
    uint8_t seed[32];

    if (use_fixed_seed) {
        std::cout << "  - PRNG Seed: Co dinh tu tham so --seed\n";
        // Parse hex seed
        std::memset(seed, 0, sizeof(seed));
        for (size_t i = 0; i < 32 && (i * 2 + 1) < fixed_seed_hex.length(); i++) {
            std::string byte_str = fixed_seed_hex.substr(i * 2, 2);
            seed[i] = static_cast<uint8_t>(std::strtoul(byte_str.c_str(), nullptr, 16));
        }
    } else {
        std::cout << "  - PRNG Seed: Ngau nhien tu he dieu hanh (randombytes)\n";
        // Lay du 32 byte entropy tu OS (Windows: rand_s, Linux/macOS: getrandom)
        randombytes(seed, sizeof(seed));
    }

    prng_seed(&ctx, seed, sizeof(seed));
    std::cout << "===============================================================\n\n";

    // ------------------- 3. Sinh so nguyen to an toan p va g -----------------
    std::cout << "[1] Dang sinh so nguyen to an toan p (" << bit_size << " bit)...\n";
    auto t0 = std::chrono::high_resolution_clock::now();
    big_t p = generate_safe_prime(&ctx, bit_size);
    auto t1 = std::chrono::high_resolution_clock::now();
    double time_prime = std::chrono::duration<double>(t1 - t0).count();
    std::cout << "    => Hoan tat trong: " << time_prime << " giay\n";
    std::cout << "    p = " << p.to_hex() << "\n\n";

    std::cout << "[2] Chon phan tu sinh g...\n";
    big_t g = choose_generator(p);
    std::cout << "    g = " << g.to_hex() << "\n\n";

    // ------------------- 4. Sinh khoa rieng cua Alice & Bob ------------------
    std::cout << "[3] Sinh khoa rieng cho Alice va Bob trong khoang [2, p - 2]...\n";
    big_t a = generate_private_key(&ctx, p); // Khoa rieng Alice
    big_t b = generate_private_key(&ctx, p); // Khoa rieng Bob
    std::cout << "    Khoa rieng Alice a = " << a.to_hex() << "\n";
    std::cout << "    Khoa rieng Bob   b = " << b.to_hex() << "\n\n";

    // ------------------- 5. Tinh gia tri cong khai A va B -------------------
    std::cout << "[4] Tinh gia tri cong khai:\n";
    big_t A = modular_exponentiation(g, a, p); // A = g^a mod p
    big_t B = modular_exponentiation(g, b, p); // B = g^b mod p
    std::cout << "    Alice gui A = g^a mod p = " << A.to_hex() << "\n";
    std::cout << "    Bob   gui B = g^b mod p = " << B.to_hex() << "\n\n";

    // ----------------------- 6. Tinh bi mat chung ---------------------------
    std::cout << "[5] Tinh bi mat chung:\n";
    auto t_dh0 = std::chrono::high_resolution_clock::now();
    big_t s_alice = modular_exponentiation(B, a, p); // s_A = B^a mod p
    big_t s_bob   = modular_exponentiation(A, b, p); // s_B = A^b mod p
    auto t_dh1 = std::chrono::high_resolution_clock::now();
    double time_dh = std::chrono::duration<double>(t_dh1 - t_dh0).count();

    std::cout << "    Bi mat chung Alice tinh duoc (s_A) = " << s_alice.to_hex() << "\n";
    std::cout << "    Bi mat chung Bob   tinh duoc (s_B) = " << s_bob.to_hex() << "\n";
    std::cout << "    Thoi gian tinh toan DH: " << time_dh << " giay\n\n";

    // ------------------------- 7. Xac minh ket qua --------------------------
    std::cout << "===============================================================\n";
    bool matched = (s_alice == s_bob) && (!s_alice.is_zero());
    if (matched) {
        std::cout << "  [XAC MINH THANH CONG] Bi mat chung hai ben hoan toan TRUNG KHOP!\n";
    } else {
        std::cout << "  [XAC MINH THAT BAI] Bi mat chung khong khop hoac chua duoc cai dat day du.\n";
    }
    std::cout << "===============================================================\n";

    return 0;
}
