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

#include <assert.h>
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

// Tính (base^exp) mod `mod` bằng thuật toán Square-and-Multiply (bình phương và nhân),
// duyệt các bit của số mũ từ bit cao nhất xuống bit thấp nhất (left-to-right binary):
//
//     result = 1
//     for i = bit_length(exp)-1 .. 0:
//         result = result^2 mod m                  (bình phương)
//         if bit i của exp bằng 1:
//             result = result * base mod m         (nhân)
//
// Mỗi bước nhân đều được rút gọn theo `mod` ngay (dùng divmod Knuth D), nên giá trị
// trung gian luôn < mod và tích luôn < mod^2. Số phép nhân-mô-đun là
// ~ 1.5 * bit_length(exp) thay vì exp phép nhân như cách làm ngây thơ.
//
// Tiền điều kiện: mod.bit_length() <= MAX_PRIME_BITS để tích a*b (a, b < mod) nằm
// gọn trong big_t (2 * MAX_PRIME_BITS bit), không bị tràn.
// Quy ước biên: mod == 0 -> 0 (không xác định, tránh chia cho 0); mod == 1 -> 0;
//               exp == 0 -> 1 % mod = 1.
big_t modular_exponentiation(const big_t &base, const big_t &exp, const big_t &mod)
{
    big_t one = big_t::from_u64(1);

    if (mod.is_zero() || mod == one) {
        return big_t();
    }
    assert(mod.bit_length() <= MAX_PRIME_BITS);

    big_t b = base % mod; // đưa base về [0, mod)
    big_t result = one;   // 1 < mod nên đã nằm trong [0, mod)

    size_t nbits = exp.bit_length();
    for (size_t i = nbits; i-- > 0;) {
        result = (result * result) % mod;
        if (exp.test_bit(i)) {
            result = (result * b) % mod;
        }
    }
    return result;
}

// Chọn phần tử sinh g cho Diffie-Hellman với p là số nguyên tố an toàn p = 2q + 1.
//
// Cơ sở toán học: Z_p* là nhóm cyclic bậc p - 1 = 2q (q nguyên tố) nên bậc của mỗi
// phần tử chỉ có thể là 1, 2, q hoặc 2q (theo định lý Lagrange):
//   - bậc 1: chỉ có g = 1;        - bậc 2: chỉ có g = p - 1 (= -1);
//   - bậc q: các thặng dư bậc hai khác 1  (g = h^2 mod p), kiểm tra g^q = 1;
//   - bậc 2q: các phi-thặng dư bậc hai (g^q = -1) -> g là phần tử sinh của cả Z_p*.
//
// Nhóm chọn g sinh NHÓM CON BẬC q (tất cả thặng dư bậc hai) với g = h^2 mod p,
// h = 2, 3, ... (h = 2 cho g = 4 với mọi p > 5). Lý do:
//   1. Với g bậc 2q, giá trị công khai g^a mod p để lộ 1 bit về tính chẵn lẻ của a
//      qua ký hiệu Legendre (g^a là thặng dư bậc hai <=> a chẵn). Nhóm con bậc q
//      không có rò rỉ này.
//   2. Mọi lũy thừa của g nằm trong nhóm con bậc q, nhóm bậc nguyên tố lớn -> chống
//      được tấn công nhóm con nhỏ (Pohlig-Hellman chỉ còn bài toán DLP bậc q).
//   3. Kiểm tra g^q mod p == 1 và g != 1 là đủ để khẳng định g có bậc đúng bằng q
//      (vì q nguyên tố), không cần phân tích p - 1 ra thừa số nào khác.
//
// Nếu p không có dạng an toàn (ví dụ p chưa sinh xong) hàm trả về 2 để chương trình
// vẫn chạy được, nhưng khi đó không còn bảo đảm về bậc của g.
big_t choose_generator(const big_t &p)
{
    big_t one = big_t::from_u64(1);
    big_t two = big_t::from_u64(2);

    if (p < big_t::from_u64(7)) {
        return two;
    }
    big_t p_minus_1 = p - one;
    big_t q = p_minus_1 >> 1; // q = (p - 1) / 2

    for (uint64_t h = 2; h < 1000000; h++) {
        big_t hb = big_t::from_u64(h);
        if (hb >= p_minus_1) break;

        big_t g = (hb * hb) % p; // thặng dư bậc hai
        if (g == one) continue;
        if (modular_exponentiation(g, q, p) == one) {
            return g; // bậc của g chia hết q và g != 1 => bậc đúng bằng q
        }
    }
    return two;
}

// Kiểm tra giá trị công khai V (A hoặc B) nhận được từ phía đối tác:
//   - phải nằm trong [2, p - 2]  (loại V = 0, 1, p - 1 -> bí mật chung đoán được);
//   - phải thuộc nhóm con bậc q:  V^q mod p == 1  (chống tấn công nhóm con nhỏ).
bool is_valid_public_value(const big_t &v, const big_t &p)
{
    big_t one = big_t::from_u64(1);
    big_t two = big_t::from_u64(2);
    if (p < big_t::from_u64(7)) return false;
    if (v < two || v > p - two) return false;
    big_t q = (p - one) >> 1;
    return modular_exponentiation(v, q, p) == one;
}

#endif // DH_CPP
