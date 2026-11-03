// ============================================================================
// dh.cpp - Trao đổi khóa Diffie-Hellman & Lũy thừa mô-đun
// Phụ trách: Tạ Quang Mạnh (24120089)
//
// Nhiệm vụ A & D:
//   - modular_exponentiation: (base^exp) % mod
//   - choose_generator: chọn phần tử sinh g cho nhóm Z_p* (với p là safe prime)
// ============================================================================
#ifndef DH_CPP
#define DH_CPP

#include "bigint.cpp"

// Tính (base^exp) % mod bằng thuật toán Square-and-Multiply (bình phương và nhân)
// TODO(Mạnh):
//   1. Xử lý các trường hợp biên: mod == 1 -> return 0; exp == 0 -> return 1 % mod.
//   2. Duyệt các bit của exp từ bit cao nhất (hoặc thấp nhất).
//   3. Giảm modulo sau mỗi phép nhân (dùng toán tử * và % của BigInt).
big_t modular_exponentiation(big_t base, big_t exp, const big_t &mod)
{
    if (mod == big_t::from_u64(1)) {
        return big_t();
    }
    big_t result = big_t::from_u64(1);
    base = base % mod;

    // TODO(Mạnh): Hoàn thiện vòng lặp square-and-multiply
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
