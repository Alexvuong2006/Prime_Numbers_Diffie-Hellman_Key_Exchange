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

    // Chuỗi hex không có tiền tố "0x", không phân biệt hoa/thường.
    static uint_t from_hex(const std::string &s)
    {
        // TODO(Khang)
        (void)s;
        return uint_t();
    }

    std::string to_hex() const
    {
        // TODO(Khang)
        return "<TODO to_hex>";
    }

    std::string to_dec() const
    {
        // TODO(Khang)
        return "<TODO to_dec>";
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
        // TODO(Khang)
        return 0;
    }

    bool test_bit(size_t i) const
    {
        // TODO(Khang)
        (void)i;
        return false;
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
    // TODO(Khang): so sánh từ limb cao xuống limb thấp
    (void)a;
    (void)b;
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
    // TODO(Khang): cộng có nhớ từng limb bằng uint128_t
    (void)a;
    (void)b;
    return uint_t<N>();
}

// Trừ: giả định a >= b.
template <size_t N>
uint_t<N> operator-(const uint_t<N> &a, const uint_t<N> &b)
{
    // TODO(Khang): trừ có mượn
    (void)a;
    (void)b;
    return uint_t<N>();
}

// Nhân schoolbook O(n^2), kết quả lấy theo mod 2^(64N).
// Người gọi đảm bảo bit_length(a) + bit_length(b) <= 64N (xem MAX_PRIME_BITS).
template <size_t N>
uint_t<N> operator*(const uint_t<N> &a, const uint_t<N> &b)
{
    // TODO(Khang): chỉ lặp trên số limb thực dùng để tăng tốc
    (void)a;
    (void)b;
    return uint_t<N>();
}

template <size_t N>
uint_t<N> operator<<(const uint_t<N> &a, size_t shift)
{
    // TODO(Khang)
    (void)a;
    (void)shift;
    return uint_t<N>();
}

template <size_t N>
uint_t<N> operator>>(const uint_t<N> &a, size_t shift)
{
    // TODO(Khang)
    (void)a;
    (void)shift;
    return uint_t<N>();
}

// a mod m với m nhỏ (dùng cho sàng số nguyên tố nhỏ trong prime.cpp).
template <size_t N>
uint32_t mod_small(const uint_t<N> &a, uint32_t m)
{
    // TODO(Khang): Horner từ limb cao xuống, dùng uint128_t
    (void)a;
    (void)m;
    return 0;
}

// =============================== [MẠNH] chia ================================
// q = a / b, r = a % b. Yêu cầu b != 0.
// Gợi ý: làm bản shift-subtract trước, sau đó tối ưu bằng Knuth Algorithm D.
template <size_t N>
void divmod(const uint_t<N> &a, const uint_t<N> &b, uint_t<N> &q, uint_t<N> &r)
{
    // TODO(Mạnh)
    (void)a;
    (void)b;
    q = uint_t<N>();
    r = uint_t<N>();
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

