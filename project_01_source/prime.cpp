// ============================================================================
// prime.cpp - Số ngẫu nhiên, kiểm tra nguyên tố & sinh số nguyên tố an toàn
// Phụ trách: Lâm Duy Minh (24120091)
//
// Nhiệm vụ B & C (Đã hoàn thiện các hạng mục đến Mốc 09/11):
//   - random_bits: sinh số ngẫu nhiên có độ dài đúng bits bit (bit cao nhất = 1)
//   - random_range: sinh số ngẫu nhiên đều trong đoạn [lo, hi] bằng Rejection Sampling
//   - is_probable_prime_u64: kiểm tra số nguyên tố Miller-Rabin trên uint64_t
//   - is_probable_prime: kiểm tra số nguyên tố Miller-Rabin trên big_t
//   - generate_private_key: sinh khóa riêng trong đoạn [2, p - 2]
// ============================================================================
#ifndef PRIME_CPP
#define PRIME_CPP

#include "prng.cpp"
#include "bigint.cpp"
#include "dh.cpp"

// ----------------------------------------------------------------------------
// 1. SINH SỐ LỚN NGẪU NHIÊN & REJECTION SAMPLING
// ----------------------------------------------------------------------------

// Hàm phụ trợ: Sinh số ngẫu nhiên gồm tối đa `bits` bit trong khoảng [0, 2^bits - 1]
// (Không ép bật bit cao nhất, dùng làm nguồn bit ngẫu nhiên đồng đều cho Rejection Sampling).
static big_t random_upto_bits(prng_ctx *ctx, size_t bits)
{
    big_t res;
    if (bits == 0)
    {
        return res;
    }

    const size_t max_bits = BIG_LIMBS * 64;
    if (bits > max_bits)
    {
        bits = max_bits;
    }

    size_t num_limbs = (bits + 63) / 64;
    for (size_t i = 0; i < num_limbs; i++)
    {
        res.digits[i] = prng_next_u64(ctx);
    }

    size_t rem_bits = bits % 64;
    if (rem_bits != 0)
    {
        uint64_t mask = (1ULL << rem_bits) - 1ULL;
        res.digits[num_limbs - 1] &= mask;
    }
    return res;
}

// Sinh số ngẫu nhiên có đúng `bits` bit (bit cao nhất (bits - 1) luôn bật = 1),
// đảm bảo kết quả nằm trong đoạn [2^(bits - 1), 2^bits - 1].
big_t random_bits(prng_ctx *ctx, size_t bits)
{
    if (bits == 0)
    {
        return big_t();
    }

    const size_t max_bits = BIG_LIMBS * 64;
    if (bits > max_bits)
    {
        bits = max_bits;
    }

    big_t res = random_upto_bits(ctx, bits);
    res.set_bit(bits - 1);
    return res;
}

// Sinh số ngẫu nhiên phân phối đều trong đoạn [lo, hi] bằng kỹ thuật Rejection Sampling.
// Tuyệt đối không dùng phép chia lấy dư % để tránh sai lệch phân phối (modulo bias).
big_t random_range(prng_ctx *ctx, const big_t &lo, const big_t &hi)
{
    if (hi <= lo)
    {
        return lo;
    }

    big_t range = hi - lo;
    size_t k = range.bit_length(); // 2^(k - 1) <= range <= 2^k - 1

    while (true)
    {
        big_t r = random_upto_bits(ctx, k);
        if (r <= range)
        {
            return lo + r;
        }
    }
}

// ----------------------------------------------------------------------------
// 2. KIỂM TRA SỐ NGUYÊN TỐ MILLER-RABIN TRÊN uint64_t (MỐC 06/11 - 08/11)
// ----------------------------------------------------------------------------

static uint64_t powmod_u64(uint64_t base, uint64_t exp, uint64_t mod)
{
    if (mod == 1)
        return 0;
    uint64_t res = 1;
    uint64_t b = base % mod;
    while (exp > 0)
    {
        if (exp & 1ULL)
        {
            res = static_cast<uint64_t>((static_cast<uint128_t>(res) * b) % mod);
        }
        b = static_cast<uint64_t>((static_cast<uint128_t>(b) * b) % mod);
        exp >>= 1;
    }
    return res;
}

static bool miller_rabin_witness_u64(uint64_t a, uint64_t d, size_t s, uint64_t n)
{
    uint64_t x = powmod_u64(a, d, n);
    if (x == 1 || x == n - 1)
    {
        return true;
    }
    for (size_t r = 1; r < s; r++)
    {
        x = static_cast<uint64_t>((static_cast<uint128_t>(x) * x) % n);
        if (x == n - 1)
        {
            return true;
        }
        if (x == 1)
        {
            return false;
        }
    }
    return false;
}

// Kiểm tra nguyên tố Miller-Rabin trên số 64-bit
bool is_probable_prime_u64(prng_ctx *ctx, uint64_t n, int rounds = 40)
{
    if (n < 2)
        return false;
    if (n == 2 || n == 3)
        return true;
    if ((n & 1ULL) == 0)
        return false;

    uint64_t d = n - 1;
    size_t s = 0;
    while ((d & 1ULL) == 0)
    {
        d >>= 1;
        s++;
    }

    if (rounds <= 0)
        rounds = 40;

    uint64_t range = n - 3; // chọn a trong [2, n - 2] <=> 2 + r với r in [0, n - 4]
    uint64_t mask = ~0ULL;
    if (range > 0)
    {
        size_t k = 0;
        uint64_t tmp = range;
        while (tmp > 0)
        {
            k++;
            tmp >>= 1;
        }
        mask = (k >= 64) ? ~0ULL : ((1ULL << k) - 1ULL);
    }

    for (int i = 0; i < rounds; i++)
    {
        uint64_t r = 0;
        if (range > 0)
        {
            do
            {
                r = prng_next_u64(ctx) & mask;
            } while (r > range);
        }
        uint64_t a = 2ULL + r;
        if (!miller_rabin_witness_u64(a, d, s, n))
        {
            return false;
        }
    }
    return true;
}

// ----------------------------------------------------------------------------
// 3. KIỂM TRA SỐ NGUYÊN TỐ MILLER-RABIN TRÊN big_t (MỐC 09/11)
// ----------------------------------------------------------------------------

// Đếm số bit 0 tận cùng (trailing zeros) của một số lớn khác 0
static size_t count_trailing_zeros(const big_t &x)
{
    for (size_t i = 0; i < BIG_LIMBS; i++)
    {
        if (x.digits[i] != 0)
        {
#if defined(__GNUC__) || defined(__clang__)
            return i * 64 + static_cast<size_t>(__builtin_ctzll(x.digits[i]));
#else
            uint64_t v = x.digits[i];
            size_t tz = 0;
            while ((v & 1ULL) == 0)
            {
                tz++;
                v >>= 1;
            }
            return i * 64 + tz;
#endif
        }
    }
    return 0;
}

// Kiểm tra số nguyên tố Miller-Rabin trên big_t với `rounds` vòng lặp
// Xác suất một hợp số vượt qua k vòng lặp tối đa là 4^(-k).
bool is_probable_prime(prng_ctx *ctx, const big_t &n, int rounds = 40)
{
    const big_t one  = big_t::from_u64(1);
    const big_t two  = big_t::from_u64(2);
    const big_t three = big_t::from_u64(3);

    // 1. Xử lý các trường hợp biên nhỏ và số chẵn
    if (n < two)
    {
        return false;
    }
    if (n == two || n == three)
    {
        return true;
    }
    if (!n.is_odd())
    {
        return false;
    }

    // 2. Phân tích n - 1 = 2^s * d (với d là số lẻ)
    big_t n_minus_1 = n - one;
    big_t n_minus_2 = n - two;
    size_t s = count_trailing_zeros(n_minus_1);
    big_t d = n_minus_1 >> s;

    if (rounds <= 0)
    {
        rounds = 40;
    }

    // 3. Thực hiện `rounds` vòng kiểm tra độc lập
    for (int i = 0; i < rounds; i++)
    {
        // Chọn cơ sở a ngẫu nhiên đều trong [2, n - 2]
        big_t a = random_range(ctx, two, n_minus_2);

        // Tính x = a^d mod n
        big_t x = modular_exponentiation(a, d, n);
        if (x == one || x == n_minus_1)
        {
            continue;
        }

        bool passed_round = false;
        for (size_t r = 1; r < s; r++)
        {
            x = (x * x) % n;
            if (x == n_minus_1)
            {
                passed_round = true;
                break;
            }
            if (x == one)
            {
                // Phát hiện căn bậc hai không tầm thường của 1 mod n -> chắc chắn là hợp số
                return false;
            }
        }

        if (!passed_round)
        {
            return false; // Hợp số (a là nhân chứng Miller-Rabin cho n)
        }
    }

    return true;
}

// ----------------------------------------------------------------------------
// 4. SINH SỐ NGUYÊN TỐ AN TOÀN & KHÓA RIÊNG
// ----------------------------------------------------------------------------

// Sinh số nguyên tố an toàn p có độ dài bitsize (mặc định 512 bit)
// p = 2q + 1, sao cho cả q và p đều là số nguyên tố (Thuộc Mốc 2: 10/11 - 12/11).
big_t generate_safe_prime(prng_ctx *ctx, size_t bitsize = 512)
{
    // TODO(Minh - Mốc 2): Sàng sơ bộ + sinh safe prime p = 2q + 1
    (void)ctx;
    (void)bitsize;
    return big_t();
}

// Sinh khóa riêng cho Alice/Bob ngẫu nhiên trong khoảng [2, p - 2]
big_t generate_private_key(prng_ctx *ctx, const big_t &p)
{
    big_t two = big_t::from_u64(2);
    if (p <= big_t::from_u64(4))
    {
        return two;
    }
    big_t p_minus_two = p - two;
    return random_range(ctx, two, p_minus_two);
}

#endif // PRIME_CPP
