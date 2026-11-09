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
#include <stdio.h>
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
// q = a / b, r = a % b. Yêu cầu b != 0 (nếu b == 0 thì q = r = 0).
//
// Thuật toán: Knuth, TAOCP tập 2, mục 4.3.1, Algorithm D (chia theo limb 64-bit).
//   0. Số bị chia a có m_len limb, số chia b có n limb (đã bỏ các limb cao bằng 0).
//   1. Chuẩn hóa: dịch trái cả a, b đi s bit sao cho bit cao nhất của limb cao nhất
//      của b bằng 1 (s = clz). Việc này đảm bảo ước lượng q_hat sai tối đa 2 đơn vị.
//   2. Với mỗi j = m_len - n, ..., 0 tính một limb của thương:
//        q_hat = (u[j+n]*2^64 + u[j+n-1]) / v[n-1]      (dùng uint128_t)
//        hiệu chỉnh q_hat bằng limb thứ hai v[n-2]       (tối đa 2 lần giảm)
//        u[j..j+n] -= q_hat * v ; nếu âm thì q_hat-- và cộng lại v (xác suất ~ 2/2^64)
//   3. Phần dư = u[0..n-1] >> s (khử chuẩn hóa).
// Độ phức tạp: O(n * (m_len - n + 1)) phép nhân 64x64 -> 128 bit.
// Với phép mod trong modular_exponentiation (a = tích 2n limb, b = p n limb): O(n^2).
// Chia cho số chỉ có 1 limb dùng đường ngắn (Short division).
//
// Cho phép q hoặc r trùng (alias) với a hoặc b: kết quả được tính vào biến tạm.
template <size_t N>
void divmod(const uint_t<N> &a, const uint_t<N> &b, uint_t<N> &q, uint_t<N> &r)
{
    uint_t<N> quo, rem; // khởi tạo = 0

    // Độ dài thực (số limb có nghĩa) của b
    size_t n = N;
    while (n > 0 && b.digits[n - 1] == 0) n--;
    if (n == 0) { // chia cho 0: trả về q = r = 0
        q = quo;
        r = rem;
        return;
    }

    if (a < b) {
        q = quo;
        r = a;
        return;
    }

    size_t m_len = N; // độ dài thực của a (a >= b > 0 nên m_len >= n >= 1)
    while (m_len > 0 && a.digits[m_len - 1] == 0) m_len--;

    if (n == 1) {
        // ---- Chia cho một limb: chia từ limb cao xuống thấp ----
        const uint64_t d = b.digits[0];
        uint128_t cur_rem = 0;
        for (size_t i = m_len; i-- > 0;) {
            uint128_t cur = (cur_rem << 64) | a.digits[i];
            quo.digits[i] = static_cast<uint64_t>(cur / d);
            cur_rem = cur % d;
        }
        rem.digits[0] = static_cast<uint64_t>(cur_rem);
    } else {
        // ---- Knuth Algorithm D ----
        // D1. Chuẩn hóa
        const unsigned s = static_cast<unsigned>(__builtin_clzll(b.digits[n - 1]));
        uint64_t v[N];     // b << s, n limb
        uint64_t u[N + 1]; // a << s, m_len + 1 limb

        if (s == 0) {
            for (size_t i = 0; i < n; i++) v[i] = b.digits[i];
            for (size_t i = 0; i < m_len; i++) u[i] = a.digits[i];
            u[m_len] = 0;
        } else {
            for (size_t i = n - 1; i > 0; i--)
                v[i] = (b.digits[i] << s) | (b.digits[i - 1] >> (64 - s));
            v[0] = b.digits[0] << s;

            u[m_len] = a.digits[m_len - 1] >> (64 - s);
            for (size_t i = m_len - 1; i > 0; i--)
                u[i] = (a.digits[i] << s) | (a.digits[i - 1] >> (64 - s));
            u[0] = a.digits[0] << s;
        }

        const uint128_t BASE = static_cast<uint128_t>(1) << 64;
        const uint64_t v_hi = v[n - 1];
        const uint64_t v_lo = v[n - 2];

        // D2..D7. Lặp từ limb cao nhất của thương xuống thấp nhất
        for (size_t jj = m_len - n + 1; jj-- > 0;) {
            const size_t j = jj;

            // D3. Ước lượng q_hat và hiệu chỉnh
            uint128_t num = (static_cast<uint128_t>(u[j + n]) << 64) | u[j + n - 1];
            uint128_t q_hat = num / v_hi;
            uint128_t r_hat = num % v_hi;
            while (q_hat >= BASE ||
                   q_hat * v_lo > ((r_hat << 64) | u[j + n - 2])) {
                q_hat--;
                r_hat += v_hi;
                if (r_hat >= BASE) break;
            }
            uint64_t qh = static_cast<uint64_t>(q_hat);

            // D4. Nhân và trừ: u[j..j+n] -= qh * v
            uint64_t mul_carry = 0, borrow = 0;
            for (size_t i = 0; i < n; i++) {
                uint128_t p = static_cast<uint128_t>(qh) * v[i] + mul_carry;
                mul_carry = static_cast<uint64_t>(p >> 64);
                uint64_t p_lo = static_cast<uint64_t>(p);

                uint64_t x = u[i + j];
                uint64_t t = x - p_lo;
                uint64_t b1 = (x < p_lo) ? 1 : 0;
                uint64_t t2 = t - borrow;
                uint64_t b2 = (t < borrow) ? 1 : 0;
                u[i + j] = t2;
                borrow = b1 | b2;
            }
            uint64_t x_top = u[j + n];
            uint64_t t_top = x_top - mul_carry;
            uint64_t b1_top = (x_top < mul_carry) ? 1 : 0;
            uint64_t t2_top = t_top - borrow;
            uint64_t b2_top = (t_top < borrow) ? 1 : 0;
            u[j + n] = t2_top;

            // D5/D6. Nếu kết quả âm: q_hat quá lớn 1 đơn vị -> giảm và cộng lại v
            if ((b1_top | b2_top) != 0) {
                qh--;
                uint64_t add_carry = 0;
                for (size_t i = 0; i < n; i++) {
                    uint128_t sum = static_cast<uint128_t>(u[i + j]) + v[i] + add_carry;
                    u[i + j] = static_cast<uint64_t>(sum);
                    add_carry = static_cast<uint64_t>(sum >> 64);
                }
                u[j + n] += add_carry; // tràn ra ngoài được bỏ qua (mod 2^64)
            }
            quo.digits[j] = qh;
        }

        // D8. Khử chuẩn hóa phần dư: r = u[0..n-1] >> s
        if (s == 0) {
            for (size_t i = 0; i < n; i++) rem.digits[i] = u[i];
        } else {
            for (size_t i = 0; i + 1 < n; i++)
                rem.digits[i] = (u[i] >> s) | (u[i + 1] << (64 - s));
            rem.digits[n - 1] = u[n - 1] >> s;
        }
    }

    q = quo;
    r = rem;
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

