// ============================================================================
// prime.cpp - Số ngẫu nhiên, kiểm tra nguyên tố & sinh số nguyên tố an toàn
// Phụ trách: Lâm Duy Minh (24120091)
//
// Nhiệm vụ B & C:
//   - random_bits: sinh số ngẫu nhiên có độ dài đúng bits bit
//   - random_range: sinh số ngẫu nhiên trong đoạn [lo, hi] dùng rejection sampling
//   - is_probable_prime: kiểm tra số nguyên tố bằng Miller-Rabin
//   - generate_safe_prime: sinh số nguyên tố an toàn p (p = 2q + 1, p và q đều nguyên tố)
//   - generate_private_key: sinh khóa riêng trong đoạn [2, p - 2]
// ============================================================================
#ifndef PRIME_CPP
#define PRIME_CPP

#include "prng.cpp"
#include "bigint.cpp"
#include "dh.cpp"

// Sinh số ngẫu nhiên có đúng `bits` bit (bit cao nhất luôn bật = 1)
// TODO(Minh):
//   1. Dùng prng_next_u64(&ctx) để lấp đầy các limb cần thiết.
//   2. Mask bỏ các bit thừa ở limb cao nhất vượt quá bits.
//   3. Đảm bảo bit (bits - 1) được bật = 1.
big_t random_bits(prng_ctx *ctx, size_t bits)
{
    // TODO(Minh)
    (void)ctx;
    (void)bits;
    return big_t();
}

// Sinh số ngẫu nhiên đều trong đoạn [lo, hi] bằng kỹ thuật Rejection Sampling
// Không dùng phép chia lấy dư % (modulo bias).
// TODO(Minh):
//   1. Tính range = hi - lo.
//   2. Tìm số bit k cần thiết sao cho 2^k > range.
//   3. Lặp: r = random_bits(ctx, k); nếu r <= range thì return lo + r.
big_t random_range(prng_ctx *ctx, const big_t &lo, const big_t &hi)
{
    // TODO(Minh)
    (void)ctx;
    (void)lo;
    (void)hi;
    return lo;
}

// Kiểm tra số nguyên tố Miller-Rabin với `rounds` vòng lặp
// TODO(Minh):
//   1. Xử lý các số nhỏ: n < 2 -> false; n == 2, 3 -> true; n chẵn -> false.
//   2. Phân tích n - 1 = 2^s * d (với d lẻ).
//   3. Lặp `rounds` lần:
//      - Chọn cơ sở a ngẫu nhiên trong [2, n - 2] bằng random_range.
//      - x = modular_exponentiation(a, d, n).
//      - Nếu x == 1 hoặc x == n - 1 -> sang vòng tiếp theo.
//      - Lặp s - 1 lần: x = (x * x) % n. Nếu x == n - 1 -> break sang vòng tiếp.
//      - Nếu không qua -> return false (hợp số).
//   4. Trả về true (khả năng rất cao là số nguyên tố).
bool is_probable_prime(prng_ctx *ctx, const big_t &n, int rounds = 40)
{
    // TODO(Minh)
    (void)ctx;
    (void)n;
    (void)rounds;
    return false;
}

// Sinh số nguyên tố an toàn p có độ dài bitsize (mặc định 512 bit)
// p = 2q + 1, sao cho cả q và p đều là số nguyên tố (q là Sophie Germain prime).
// TODO(Minh):
//   1. Sinh ứng viên q ngẫu nhiên có (bitsize - 1) bit (đảm bảo q lẻ).
//   2. Sàng sơ bộ q và p = 2q + 1 bằng danh sách các số nguyên tố nhỏ (mod_small) để loại nhanh.
//   3. Kiểm tra Miller-Rabin cho q (ví dụ 1 vòng thử nghiệm nhanh).
//   4. Nếu q vượt qua, tính p = 2*q + 1 và kiểm tra Miller-Rabin cho p.
//   5. Nếu cả hai vượt qua kiểm tra đầy đủ số vòng `rounds` -> return p.
big_t generate_safe_prime(prng_ctx *ctx, size_t bitsize = 512)
{
    // TODO(Minh)
    (void)ctx;
    (void)bitsize;
    return big_t();
}

// Sinh khóa riêng cho Alice/Bob ngẫu nhiên trong khoảng [2, p - 2]
big_t generate_private_key(prng_ctx *ctx, const big_t &p)
{
    big_t two = big_t::from_u64(2);
    big_t p_minus_two = p - two;
    return random_range(ctx, two, p_minus_two);
}

#endif // PRIME_CPP
