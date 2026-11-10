// ============================================================================
// prime.cpp - Số ngẫu nhiên, kiểm tra nguyên tố & sinh số nguyên tố an toàn
// Phụ trách: Lâm Duy Minh (24120091)
//
// Nhiệm vụ B & C:
//   - random_bits: sinh số ngẫu nhiên có độ dài đúng bits bit (bit cao nhất = 1)
//   - random_range: sinh số ngẫu nhiên đều trong đoạn [lo, hi] bằng Rejection Sampling
//   - is_probable_prime_u64: kiểm tra số nguyên tố Miller-Rabin trên uint64_t
//   - is_probable_prime: kiểm tra số nguyên tố Miller-Rabin trên big_t
//   - passes_safe_prime_sieve: sàng đồng thời q và p = 2q + 1 bằng các số nguyên tố nhỏ < 2000
//   - miller_rabin_base2: kiểm tra nhanh 1 vòng Miller-Rabin cơ sở 2
//   - generate_safe_prime: sinh số nguyên tố an toàn p = 2q + 1 (p và q cùng nguyên tố)
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
// 4. SÀNG SỐ NGUYÊN TỐ NHỎ & SINH SỐ NGUYÊN TỐ AN TOÀN (SAFE PRIME)
// ----------------------------------------------------------------------------

// Bảng 302 số nguyên tố lẻ nhỏ hơn 2000 (từ 3 đến 1999) dùng để sàng sơ bộ
static const uint32_t SMALL_PRIMES[] = {
       3,    5,    7,   11,   13,   17,   19,   23,   29,   31,   37,   41,
      43,   47,   53,   59,   61,   67,   71,   73,   79,   83,   89,   97,
     101,  103,  107,  109,  113,  127,  131,  137,  139,  149,  151,  157,
     163,  167,  173,  179,  181,  191,  193,  197,  199,  211,  223,  227,
     229,  233,  239,  241,  251,  257,  263,  269,  271,  277,  281,  283,
     293,  307,  311,  313,  317,  331,  337,  347,  349,  353,  359,  367,
     373,  379,  383,  389,  397,  401,  409,  419,  421,  431,  433,  439,
     443,  449,  457,  461,  463,  467,  479,  487,  491,  499,  503,  509,
     521,  523,  541,  547,  557,  563,  569,  571,  577,  587,  593,  599,
     601,  607,  613,  617,  619,  631,  641,  643,  647,  653,  659,  661,
     673,  677,  683,  691,  701,  709,  719,  727,  733,  739,  743,  751,
     757,  761,  769,  773,  787,  797,  809,  811,  821,  823,  827,  829,
     839,  853,  857,  859,  863,  877,  881,  883,  887,  907,  911,  919,
     929,  937,  941,  947,  953,  967,  971,  977,  983,  991,  997, 1009,
    1013, 1019, 1021, 1031, 1033, 1039, 1049, 1051, 1061, 1063, 1069, 1087,
    1091, 1093, 1097, 1103, 1109, 1117, 1123, 1129, 1151, 1153, 1163, 1171,
    1181, 1187, 1193, 1201, 1213, 1217, 1223, 1229, 1231, 1237, 1249, 1259,
    1277, 1279, 1283, 1289, 1291, 1297, 1301, 1303, 1307, 1319, 1321, 1327,
    1361, 1367, 1373, 1381, 1399, 1409, 1423, 1427, 1429, 1433, 1439, 1447,
    1451, 1453, 1459, 1471, 1481, 1483, 1487, 1489, 1493, 1499, 1511, 1523,
    1531, 1543, 1549, 1553, 1559, 1567, 1571, 1579, 1583, 1597, 1601, 1607,
    1609, 1613, 1619, 1621, 1627, 1637, 1657, 1663, 1667, 1669, 1693, 1697,
    1699, 1709, 1721, 1723, 1733, 1741, 1747, 1753, 1759, 1777, 1783, 1787,
    1789, 1801, 1811, 1823, 1831, 1847, 1861, 1867, 1871, 1873, 1877, 1879,
    1889, 1901, 1907, 1913, 1931, 1933, 1949, 1951, 1973, 1979, 1987, 1993,
    1997, 1999
};
static const size_t NUM_SMALL_PRIMES = sizeof(SMALL_PRIMES) / sizeof(SMALL_PRIMES[0]);

// Sàng đồng thời cho ứng viên q (bitsize - 1 bit) và p = 2q + 1 (bitsize bit)
// Với mỗi số nguyên tố lẻ nhỏ r < 2000:
//   - Nếu q mod r == 0         => r | q         => q là hợp số (nếu q > r)
//   - Nếu q mod r == (r - 1)/2 => 2q + 1 ≡ 0 (mod r) => r | p => p là hợp số (nếu p > r)
// Chỉ cần 1 phép mod_small(q, r) để kiểm tra đồng thời cả q và p = 2q + 1.
static bool passes_safe_prime_sieve(const big_t &q, size_t bitsize)
{
    for (size_t i = 0; i < NUM_SMALL_PRIMES; i++)
    {
        uint32_t r = SMALL_PRIMES[i];
        if (bitsize <= 12 && r >= (1U << (bitsize - 2)))
        {
            break;
        }
        uint64_t rem = mod_small(q, r);
        if (rem == 0 || rem == static_cast<uint64_t>((r - 1) / 2))
        {
            return false;
        }
    }
    return true;
}

// Kiểm tra nhanh 1 vòng Miller-Rabin với cơ sở cố định a = 2 (loại > 99.99% hợp số còn sót sau sàng)
static bool miller_rabin_base2(const big_t &n)
{
    const big_t one = big_t::from_u64(1);
    const big_t two = big_t::from_u64(2);
    const big_t three = big_t::from_u64(3);

    if (n < two)
        return false;
    if (n == two || n == three)
        return true;
    if (!n.is_odd())
        return false;

    big_t n_minus_1 = n - one;
    size_t s = count_trailing_zeros(n_minus_1);
    big_t d = n_minus_1 >> s;

    big_t x = modular_exponentiation(two, d, n);
    if (x == one || x == n_minus_1)
    {
        return true;
    }

    for (size_t r = 1; r < s; r++)
    {
        x = (x * x) % n;
        if (x == n_minus_1)
        {
            return true;
        }
        if (x == one)
        {
            return false;
        }
    }
    return false;
}

// Thống kê quá trình sinh safe prime gần nhất (phục vụ benchmark và báo cáo Mục 2)
struct SafePrimeStats
{
    size_t total_candidates; // Tổng số ứng viên q đã sinh
    size_t passed_sieve;     // Số cặp (q, p) vượt qua sàng nguyên tố nhỏ < 2000
    size_t passed_q_base2;   // Số ứng viên vượt qua Miller-Rabin cơ sở 2 trên q
    size_t passed_p_base2;   // Số ứng viên vượt qua Miller-Rabin cơ sở 2 trên cả q và p
};

static SafePrimeStats g_last_safe_prime_stats = {0, 0, 0, 0};

// Sinh số nguyên tố an toàn p có độ dài đúng `bitsize` bit (mặc định 512 bit)
// p = 2q + 1, sao cho cả q (Sophie Germain prime) và p (safe prime) đều là số nguyên tố.
// Quy trình tối ưu 4 tầng:
//   1. Sinh q ngẫu nhiên (bitsize - 1) bit, bit cao nhất = 1, bit 0 = 1 (q lẻ => p = 2q + 1 có đúng bitsize bit).
//   2. Sàng đồng thời q và p = 2q + 1 bằng 302 số nguyên tố lẻ nhỏ < 2000 qua mod_small(q, r).
//   3. Chạy 1 vòng Miller-Rabin cơ sở 2 trên q, sau đó chạy 1 vòng cơ sở 2 trên p = 2q + 1.
//   4. Chạy đủ `rounds` vòng Miller-Rabin cơ sở ngẫu nhiên trên cả q và p để xác nhận.
big_t generate_safe_prime(prng_ctx *ctx, size_t bitsize = 512, int rounds = 40)
{
    if (bitsize < 3)
    {
        return big_t::from_u64(5);
    }
    if (bitsize == 3)
    {
        return big_t::from_u64(7);
    }

    const big_t one = big_t::from_u64(1);
    SafePrimeStats stats = {0, 0, 0, 0};

    while (true)
    {
        stats.total_candidates++;

        // Sinh q có đúng (bitsize - 1) bit và ép lẻ (bit 0 = 1)
        // Khi đó 2^(bitsize - 2) + 1 <= q <= 2^(bitsize - 1) - 1
        // => p = 2q + 1 nằm trong [2^(bitsize - 1) + 3, 2^bitsize - 1], luôn có đúng bitsize bit!
        big_t q = random_bits(ctx, bitsize - 1);
        q.set_bit(0);

        // Tầng 1: Sàng sơ bộ đồng thời trên q và p = 2q + 1
        if (!passes_safe_prime_sieve(q, bitsize))
        {
            continue;
        }
        stats.passed_sieve++;

        // Tầng 2: Kiểm tra nhanh 1 vòng Miller-Rabin cơ sở 2 cho q
        if (!miller_rabin_base2(q))
        {
            continue;
        }
        stats.passed_q_base2++;

        // Tầng 3: Tính p = 2q + 1 và kiểm tra nhanh 1 vòng Miller-Rabin cơ sở 2 cho p
        big_t p = (q << 1) + one;
        if (!miller_rabin_base2(p))
        {
            continue;
        }
        stats.passed_p_base2++;

        // Tầng 4: Kiểm tra đầy đủ `rounds` vòng Miller-Rabin cơ sở ngẫu nhiên cho cả q và p
        if (!is_probable_prime(ctx, q, rounds))
        {
            continue;
        }
        if (!is_probable_prime(ctx, p, rounds))
        {
            continue;
        }

        g_last_safe_prime_stats = stats;
        return p;
    }
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
