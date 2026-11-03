// ============================================================================
// bigint.cpp - Kiểu số nguyên không âm độ rộng cố định (theo gợi ý u512_sample.h)
//
// Biểu diễn: digits[0..N-1] là các limb 64-bit, little-endian (digits[0] thấp nhất).
// Tích/tổng trung gian dùng uint128_t (phần mở rộng của GCC/Clang -> dùng g++).
//
// Phụ trách:
//   [KHANG] Vương Hữu Khang (24120070): biểu diễn, chuyển đổi chuỗi, so sánh,
//           cộng, trừ, nhân, dịch bit, mod_small.
//   [MẠNH]  Tạ Quang Mạnh (24120089): chia, mod (divmod, /, %).
// Mỗi người chỉ sửa trong khu vực của mình.
// ============================================================================
#ifndef BIGINT_CPP
#define BIGINT_CPP

#include <stdint.h>
#include <stddef.h>
#include <string>

__extension__ typedef unsigned __int128 uint128_t;

template <size_t N> // N limb 64-bit
struct uint_t
{
    uint64_t digits[N];

    // Constructor mặc định khởi tạo mảng toàn số 0
    uint_t()
    {
        for (size_t i = 0; i < N; i++)
            digits[i] = 0;
    }

    // ------------------------------ [KHANG] ---------------------------------
    static uint_t from_u64(uint64_t v)
    {
        uint_t r;
        r.digits[0] = v;
        return r;
    }

    // Chuỗi hex có thể có hoặc không có tiền tố "0x"/"0X", không phân biệt hoa/thường.
    static uint_t from_hex(const std::string &s)
    {
        uint_t r;
        size_t start = 0;
        if (s.length() >= 2 && s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) {
            start = 2;
        }

        std::string clean;
        clean.reserve(s.length() - start);
        for (size_t i = start; i < s.length(); i++) {
            char c = s[i];
            if ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F')) {
                clean.push_back(c);
            }
        }
        if (clean.empty()) return r;

        // Xử lý little-endian: 16 ký tự hex tương ứng với 1 limb 64-bit (từ phải qua trái)
        size_t len = clean.length();
        size_t limb_idx = 0;
        while (len > 0 && limb_idx < N) {
            size_t chunk_len = (len >= 16) ? 16 : len;
            size_t chunk_start = len - chunk_len;
            std::string chunk = clean.substr(chunk_start, chunk_len);
            uint64_t val = 0;
            for (char c : chunk) {
                val <<= 4;
                if (c >= '0' && c <= '9') val |= static_cast<uint64_t>(c - '0');
                else if (c >= 'a' && c <= 'f') val |= static_cast<uint64_t>(c - 'a' + 10);
                else if (c >= 'A' && c <= 'F') val |= static_cast<uint64_t>(c - 'A' + 10);
            }
            r.digits[limb_idx++] = val;
            len = chunk_start;
        }
        return r;
    }

    std::string to_hex() const
    {
        int top = static_cast<int>(N) - 1;
        while (top >= 0 && digits[top] == 0) {
            top--;
        }
        if (top < 0) return "0";

        char buf[32];
        std::snprintf(buf, sizeof(buf), "%llx", static_cast<unsigned long long>(digits[top]));
        std::string s = buf;

        for (int i = top - 1; i >= 0; i--) {
            std::snprintf(buf, sizeof(buf), "%016llx", static_cast<unsigned long long>(digits[i]));
            s += buf;
        }
        return s;
    }

    std::string to_dec() const
    {
        if (is_zero()) return "0";

        const uint64_t BASE10 = 1000000000000000000ULL; // 10^18
        uint_t tmp = *this;
        std::string s;

        while (!tmp.is_zero()) {
            uint128_t rem = 0;
            for (int i = static_cast<int>(N) - 1; i >= 0; i--) {
                uint128_t cur = (rem << 64) | tmp.digits[i];
                tmp.digits[i] = static_cast<uint64_t>(cur / BASE10);
                rem = cur % BASE10;
            }

            uint64_t chunk = static_cast<uint64_t>(rem);
            char buf[32];
            if (tmp.is_zero()) {
                std::snprintf(buf, sizeof(buf), "%llu", static_cast<unsigned long long>(chunk));
                s = std::string(buf) + s;
            } else {
                std::snprintf(buf, sizeof(buf), "%018llu", static_cast<unsigned long long>(chunk));
                s = std::string(buf) + s;
            }
        }
        return s;
    }

    bool is_zero() const
    {
        for (size_t i = 0; i < N; i++)
            if (digits[i] != 0)
                return false;
        return true;
    }

    bool is_odd() const { return (digits[0] & 1) != 0; }

    // Số bit có nghĩa (0 nếu giá trị bằng 0).
    size_t bit_length() const
    {
        int top = static_cast<int>(N) - 1;
        while (top >= 0 && digits[top] == 0) {
            top--;
        }
        if (top < 0) return 0;

        #if defined(__GNUC__) || defined(__clang__)
        int lz = __builtin_clzll(digits[top]);
        return static_cast<size_t>(top) * 64 + (64 - lz);
        #else
        uint64_t v = digits[top];
        size_t bits = 0;
        while (v > 0) { bits++; v >>= 1; }
        return static_cast<size_t>(top) * 64 + bits;
        #endif
    }

    bool test_bit(size_t i) const
    {
        size_t limb = i / 64;
        if (limb >= N) return false;
        size_t bit = i % 64;
        return ((digits[limb] >> bit) & 1ULL) != 0;
    }

    void set_bit(size_t i)
    {
        size_t limb = i / 64;
        if (limb < N) {
            digits[limb] |= (1ULL << (i % 64));
        }
    }
};

typedef uint_t<8> uint512_t;
typedef uint_t<16> uint1024_t;
typedef uint_t<32> uint2048_t;

// Kiểu làm việc chung: rộng gấp đôi số bit tối đa của p để tích a*b (a, b < p)
// không tràn. Muốn sinh p lớn hơn 1024 bit thì tăng MAX_PRIME_BITS (bội của 64).
const size_t MAX_PRIME_BITS = 1024;
const size_t BIG_LIMBS = 2 * MAX_PRIME_BITS / 64;
typedef uint_t<BIG_LIMBS> big_t;

// ============================== [KHANG] so sánh ==============================
template <size_t N>
bool operator==(const uint_t<N> &a, const uint_t<N> &b)
{
    for (size_t i = 0; i < N; i++)
        if (a.digits[i] != b.digits[i])
            return false;
    return true;
}

template <size_t N>
bool operator!=(const uint_t<N> &a, const uint_t<N> &b) { return !(a == b); }

template <size_t N>
bool operator<(const uint_t<N> &a, const uint_t<N> &b)
{
    for (int i = static_cast<int>(N) - 1; i >= 0; i--) {
        if (a.digits[i] < b.digits[i]) return true;
        if (a.digits[i] > b.digits[i]) return false;
    }
    return false;
}

template <size_t N>
bool operator>(const uint_t<N> &a, const uint_t<N> &b) { return b < a; }

template <size_t N>
bool operator<=(const uint_t<N> &a, const uint_t<N> &b) { return !(b < a); }

template <size_t N>
bool operator>=(const uint_t<N> &a, const uint_t<N> &b) { return !(a < b); }

// ============================= [KHANG] số học ===============================
// Cộng: kết quả lấy theo mod 2^(64N) (tràn bị bỏ).
template <size_t N>
uint_t<N> operator+(const uint_t<N> &a, const uint_t<N> &b)
{
    uint_t<N> res;
    uint128_t carry = 0;
    for (size_t i = 0; i < N; i++) {
        uint128_t sum = static_cast<uint128_t>(a.digits[i]) + b.digits[i] + carry;
        res.digits[i] = static_cast<uint64_t>(sum);
        carry = sum >> 64;
    }
    return res;
}

// Trừ: giả định a >= b.
template <size_t N>
uint_t<N> operator-(const uint_t<N> &a, const uint_t<N> &b)
{
    uint_t<N> res;
    uint128_t borrow = 0;
    for (size_t i = 0; i < N; i++) {
        uint128_t sub = static_cast<uint128_t>(b.digits[i]) + borrow;
        uint128_t ai = a.digits[i];
        if (ai >= sub) {
            res.digits[i] = static_cast<uint64_t>(ai - sub);
            borrow = 0;
        } else {
            res.digits[i] = static_cast<uint64_t>((static_cast<uint128_t>(1) << 64) + ai - sub);
            borrow = 1;
        }
    }
    return res;
}

// Nhân schoolbook O(n^2), kết quả lấy theo mod 2^(64N).
// Người gọi đảm bảo bit_length(a) + bit_length(b) <= 64N (xem MAX_PRIME_BITS).
template <size_t N>
uint_t<N> operator*(const uint_t<N> &a, const uint_t<N> &b)
{
    uint_t<N> res;
    int len_a = static_cast<int>(N);
    while (len_a > 0 && a.digits[len_a - 1] == 0) len_a--;
    int len_b = static_cast<int>(N);
    while (len_b > 0 && b.digits[len_b - 1] == 0) len_b--;

    if (len_a == 0 || len_b == 0) return res;

    for (int i = 0; i < len_a; i++) {
        if (a.digits[i] == 0) continue;
        uint128_t carry = 0;
        uint128_t ai = a.digits[i];
        for (int j = 0; j < len_b; j++) {
            if (static_cast<size_t>(i + j) >= N) break;
            uint128_t prod = ai * b.digits[j] + res.digits[i + j] + carry;
            res.digits[i + j] = static_cast<uint64_t>(prod);
            carry = prod >> 64;
        }
        for (size_t k = i + len_b; carry > 0 && k < N; k++) {
            uint128_t sum = static_cast<uint128_t>(res.digits[k]) + carry;
            res.digits[k] = static_cast<uint64_t>(sum);
            carry = sum >> 64;
        }
    }
    return res;
}

template <size_t N>
uint_t<N> operator<<(const uint_t<N> &a, size_t shift)
{
    if (shift == 0) return a;
    if (shift >= N * 64) return uint_t<N>();

    size_t limb_shift = shift / 64;
    size_t bit_shift = shift % 64;
    uint_t<N> res;

    if (bit_shift == 0) {
        for (size_t i = limb_shift; i < N; i++) {
            res.digits[i] = a.digits[i - limb_shift];
        }
    } else {
        for (size_t i = limb_shift; i < N; i++) {
            uint64_t low = a.digits[i - limb_shift] << bit_shift;
            uint64_t high = (i > limb_shift) ? (a.digits[i - limb_shift - 1] >> (64 - bit_shift)) : 0;
            res.digits[i] = low | high;
        }
    }
    return res;
}

template <size_t N>
uint_t<N> operator>>(const uint_t<N> &a, size_t shift)
{
    if (shift == 0) return a;
    if (shift >= N * 64) return uint_t<N>();

    size_t limb_shift = shift / 64;
    size_t bit_shift = shift % 64;
    uint_t<N> res;

    if (bit_shift == 0) {
        for (size_t i = 0; i + limb_shift < N; i++) {
            res.digits[i] = a.digits[i + limb_shift];
        }
    } else {
        for (size_t i = 0; i + limb_shift < N; i++) {
            uint64_t low = a.digits[i + limb_shift] >> bit_shift;
            uint64_t high = (i + limb_shift + 1 < N) ? (a.digits[i + limb_shift + 1] << (64 - bit_shift)) : 0;
            res.digits[i] = low | high;
        }
    }
    return res;
}

// a mod m với m nhỏ (dùng cho sàng số nguyên tố nhỏ trong prime.cpp).
template <size_t N>
uint32_t mod_small(const uint_t<N> &a, uint32_t m)
{
    if (m == 0) return 0;
    int top = static_cast<int>(N) - 1;
    while (top >= 0 && a.digits[top] == 0) top--;
    if (top < 0) return 0;

    uint128_t rem = 0;
    for (int i = top; i >= 0; i--) {
        rem = ((rem << 64) | a.digits[i]) % m;
    }
    return static_cast<uint32_t>(rem);
}

// =============================== [MẠNH] chia ================================
// q = a / b, r = a % b. Yêu cầu b != 0.
// Bản cơ bản: thuật toán chia nhị phân (shift-subtract).
// TODO(Mạnh): Tối ưu thuật toán chia bằng Knuth Algorithm D (chia theo limb 64-bit với uint128_t)
// để tăng tốc độ tính toán cho modular_exponentiation và Miller-Rabin.
template <size_t N>
void divmod(const uint_t<N> &a, const uint_t<N> &b, uint_t<N> &q, uint_t<N> &r)
{
    q = uint_t<N>();
    r = uint_t<N>();
    if (b.is_zero()) {
        return; // Chia cho 0
    }
    if (a < b) {
        r = a;
        return;
    }
    if (a == b) {
        q = uint_t<N>::from_u64(1);
        return;
    }

    size_t len_a = a.bit_length();
    for (int i = static_cast<int>(len_a) - 1; i >= 0; i--) {
        r = r << 1;
        if (a.test_bit(static_cast<size_t>(i))) {
            r.digits[0] |= 1ULL;
        }
        if (r >= b) {
            r = r - b;
            q.set_bit(static_cast<size_t>(i));
        }
    }
}

template <size_t N>
uint_t<N> operator/(const uint_t<N> &a, const uint_t<N> &b)
{
    uint_t<N> q, r;
    divmod(a, b, q, r);
    return q;
}

template <size_t N>
uint_t<N> operator%(const uint_t<N> &a, const uint_t<N> &b)
{
    uint_t<N> q, r;
    divmod(a, b, q, r);
    return r;
}

#endif // BIGINT_CPP

