// ============================================================================
// dh.cpp - Trao đổi khóa Diffie-Hellman & Lũy thừa mô-đun
// Phụ trách: Tạ Quang Mạnh (24120089)
//
// Nhiệm vụ A & D:
//   - modular_exponentiation: (base^exp) % mod
//   - choose_generator: chọn phần tử sinh g cho nhóm Z_p* (với p là safe prime)
//   - Bổ sung: AddMod, MultMod, ExpMod theo tài liệu pseudocode-modular-operation.pdf
// ============================================================================
#ifndef DH_CPP
#define DH_CPP

#include "bigint.cpp"

// ----------------------------------------------------------------------------
// CÁC THUẬT TOÁN THEO pseudocode-modular-operation.pdf (GIẢNG VIÊN CUNG CẤP)
// Đảm bảo mọi biến trung gian luôn nằm vừa trong n-bit, không cần bộ nhớ phụ lớn.
// ----------------------------------------------------------------------------

// Tính (x + y) % p trong n-bit (tránh tràn số 512-bit)
template <size_t N>
uint_t<N> AddMod(uint_t<N> x, uint_t<N> y, const uint_t<N> &p)
{
    x = x % p;
    y = y % p;
    uint_t<N> p_minus_y = p - y;
    if (x >= p_minus_y) {
        return x - p_minus_y;
    }
    return x + y;
}

// Tính (x * y) % p bằng kỹ thuật Double-and-Add nhị phân kết hợp AddMod
// Đảm bảo biến trung gian luôn nằm trong n-bit
template <size_t N>
uint_t<N> MultMod(uint_t<N> x, const uint_t<N> &y, const uint_t<N> &p)
{
    uint_t<N> ret;
    x = x % p;
    size_t n = y.bit_length();
    for (size_t i = 0; i < n; i++) {
        if (y.test_bit(i)) {
            ret = AddMod(ret, x, p);
        }
        x = AddMod(x, x, p);
    }
    return ret;
}

// Tính (x^y) % p theo mã giả ExpMod của giảng viên (Square-and-Multiply qua MultMod)
template <size_t N>
uint_t<N> ExpMod(const uint_t<N> &x, const uint_t<N> &y, const uint_t<N> &p)
{
    if (p == uint_t<N>::from_u64(1)) {
        return uint_t<N>();
    }
    uint_t<N> ret = uint_t<N>::from_u64(1);
    uint_t<N> base = x % p;
    size_t n = y.bit_length();
    for (size_t i = 0; i < n; i++) {
        if (y.test_bit(i)) {
            ret = MultMod(ret, base, p);
        }
        base = MultMod(base, base, p);
    }
    return ret;
}

// ----------------------------------------------------------------------------
// THUẬT TOÁN LŨY THỪA MÔ-ĐUN CẢI TIẾN CỦA NHÓM (DÙNG BỘ ĐỆM 2n-BIT & NHÂN KHỐI)
// ----------------------------------------------------------------------------

// Tính (base^exp) % mod bằng thuật toán Square-and-Multiply kết hợp nhân khối
// Tận dụng uint128_t phần cứng, nhanh hơn phương pháp bitwise khoảng 3 lần.
big_t modular_exponentiation(big_t base, big_t exp, const big_t &mod)
{
    if (mod == big_t::from_u64(1)) {
        return big_t();
    }
    big_t result = big_t::from_u64(1);
    base = base % mod;

    while (!exp.is_zero()) {
        if (exp.is_odd()) {
            result = (result * base) % mod;
        }
        base = (base * base) % mod;
        exp = exp >> 1;
    }
    return result;
}

// Chọn phần tử sinh g cho nhóm con hoặc nhóm Z_p*
// Với p = 2q + 1 (safe prime), các bậc nguyên tố của phần tử trong Z_p* chỉ có thể là 1, 2, q, 2q.
// TODO(Mạnh):
//   - Kiểm tra g: thường chọn g = 2 hoặc số nguyên tố nhỏ.
//   - Nếu chọn sinh nhóm con bậc q (thặng dư bậc hai): kiểm tra g^q mod p == 1 và g != 1.
//   - Trình bày lý do chọn g vào báo cáo mục 4.
big_t choose_generator(const big_t &p)
{
    // Mặc định ban đầu chọn g = 2 (sinh viên cần kiểm tra và chọn phù hợp)
    // TODO(Mạnh): Hoàn thiện kiểm tra bậc của g với p
    (void)p;
    return big_t::from_u64(2);
}

#endif // DH_CPP
