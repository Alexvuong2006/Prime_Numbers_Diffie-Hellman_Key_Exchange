# PHÂN CÔNG CÔNG VIỆC – ĐỒ ÁN 1

## Số nguyên tố & Trao đổi khóa Diffie-Hellman

| Thông tin | Chi tiết |
|---|---|
| Môn học | Nhập môn mã hóa – mật mã |
| Nhóm | 18 |
| Giảng viên thực hành | Trần Hải Anh Điền |
| Ngôn ngữ | C/C++ (**chỉ C++14 trở xuống**, chỉ dùng C++ Standard Library + thư viện PRNG giảng viên cung cấp) |
| Thời gian | 04/11/2026 – **17/11/2026 (deadline)** |
| Repository | https://github.com/Alexvuong2006/Prime_Numbers_Diffie-Hellman_Key_Exchange |

### Thành viên

| MSSV | Họ và tên |
|---|---|
| 24120070 | Vương Hữu Khang |
| 24120089 | Tạ Quang Mạnh |
| 24120091 | Lâm Duy Minh |

---

## 1. Tóm tắt yêu cầu đề bài (Lab01.pdf)

1. **A – `modular_exponentiation`**: tính `(base^exponent) mod mod` hiệu quả trên số lớn.
2. **B – `generate_safe_prime`**: sinh số nguyên tố an toàn `p` (p nguyên tố **và** `(p-1)/2` nguyên tố), **tự cài đặt, không dùng thư viện số lớn/số nguyên tố**; kiểm tra nguyên tố tự viết (vd. Miller-Rabin).
3. **C – `generate_private_key`**: sinh khóa riêng ngẫu nhiên trong đoạn `[2, p-2]`; kích thước `p` chỉnh được trong code, **tối thiểu 512 bit**.
4. **D – Diffie-Hellman**: Alice/Bob tính `A = g^a mod p`, `B = g^b mod p`, trao đổi, tính bí mật chung và **xác minh hai bên trùng khớp**. Phải tìm hiểu và chọn phần tử sinh `g` hợp lý (đề ghi rõ: *“sinh viên cần tìm hiểu và chọn giá trị khác”* thay vì mặc định `g = 2`).
5. **Báo cáo (PDF)** gồm 6 mục:
   1. Hệ thống xử lý số lớn (mô tả kiểu dữ liệu số lớn tự cài đặt và cách các toán tử hoạt động).
   2. Các thuật toán kiểm tra số nguyên tố.
   3. Số ngẫu nhiên & tầm quan trọng trong mật mã, cách tạo bộ sinh số ngẫu nhiên an toàn *(điểm cộng)*.
   4. Một ứng dụng thực tế của Diffie-Hellman.
   5. Mô tả chương trình: cách chạy, cách thay đổi tham số (độ lớn số nguyên tố, …).
   6. Tự đánh giá: ưu/nhược điểm, hướng cải thiện.
6. **Bài nộp**: `24120070-24120089-24120091.zip` (MSSV xếp theo thứ tự danh sách lớp)

   ```
   24120070-24120089-24120091.zip
   ├── project_01_source/   (*.cpp — CHỈ được nộp file .cpp, không .h/.c/Makefile)
   └── project_01_report/   (*.pdf)
   ```

> ⚠️ Phần nào **không biên dịch được hoặc chạy lỗi sẽ không có điểm** → mọi code merge vào `main` phải biên dịch được với `-std=c++14`.

---

## 2. Ghi chú về mã nguồn tham khảo `prng_sample` (giảng viên cung cấp)

Thư mục `prng_sample` gồm:

| File | Nội dung | Nhóm dùng như thế nào |
|---|---|---|
| `lib/fips202/fips202.c/.h` | Cài đặt Keccak/SHA-3, SHAKE128/256 (bản tham chiếu của Kyber, public domain) | Giữ nguyên thuật toán, **gộp `.h` + `.c` thành `fips202.cpp`**, dùng làm lõi PRNG |
| `lib/fips202/prng.cpp/.h` | PRNG dựa trên SHAKE128 (XOF): `prng_seed`, `prng_next_u16/u32/u64`, `randombytes` (lấy entropy từ OS) | **Gộp `.h` vào `prng.cpp`**; nguồn ngẫu nhiên duy nhất cho `random_bits`, Miller-Rabin, khóa riêng |
| `src/u512_sample.h` | Gợi ý kiểu `uint512_t` = `uint64_t digits[8]` (little-endian, dùng `__uint128_t` cho tích trung gian) | Làm cơ sở thiết kế kiểu số lớn của nhóm (mở rộng 1024/2048 bit), viết trong `bigint.cpp` |
| `src/dh_sample.h` | Chữ ký gợi ý: `modular_exponentiation`, `generate_safe_prime(prng_ctx*, size_t bitsize = 512)`, `generate_private_key(prng_ctx*, const uint512_t&)` | Nhóm **theo đúng chữ ký này** (truyền `prng_ctx*`) |
| `src/main_prng.cpp`, `Makefile` | Ví dụ seed + sinh số ngẫu nhiên | Chỉ tham khảo; bài nộp **không dùng Makefile** mà build bằng 1 lệnh `g++` |

**Các vấn đề phát hiện trong code mẫu cần xử lý:**

1. `randombytes()` dùng `getrandom()` + `<sys/random.h>` → **chỉ chạy trên Linux/macOS**, không biên dịch được trên Windows. Cần port bằng `#ifdef _WIN32` — ưu tiên **`rand_s`** (định nghĩa `_CRT_RAND_S` ở **dòng đầu tiên của `main.cpp`**, trước mọi `#include`) để không phải link thêm thư viện; giữ nguyên nhánh `getrandom` cho Linux/macOS.
2. Tham số `uint8_t outlen` trong `randombytes` → chỉ lấy tối đa 255 byte mỗi lần gọi; nên đổi thành `size_t`.
3. `prng_next_u32`: `buf[3] << 24` dịch trên `int` → có thể tràn bit dấu (hành vi không xác định trong C++14); cần ép kiểu `(uint32_t)buf[3] << 24` trước khi dịch.
4. `main_prng.cpp`: `memcpy(seed, "seed", sizeof("seed"))` chỉ ghi 5/32 byte, phần còn lại **chưa khởi tạo** → seed không xác định. Chương trình của nhóm phải: (a) mặc định seed bằng `randombytes()` (entropy OS), (b) có tùy chọn **seed cố định** để debug/tái lập kết quả (đề mẫu cho phép dùng seed cố định trong lab).
5. `__uint128_t` là phần mở rộng của **GCC/Clang**, MSVC không hỗ trợ → cả nhóm dùng **g++** (Windows: MSYS2/MinGW-w64 UCRT64 hoặc WSL).
6. `Makefile` mẫu có `-lntl -lgf2x -lgmp` (NTL/GMP) → **tuyệt đối không dùng** các thư viện này (vi phạm yêu cầu tự cài đặt); Makefile mẫu cũng chưa có `-std=c++14`.
7. Mẫu chia thành `.h` + `.c` + `.cpp`, nhưng bài nộp **chỉ được chứa `.cpp`** → toàn bộ code phải tổ chức lại theo mục 3 (không còn file header).

### Ghi chú bổ sung: Tài liệu `pseudocode-modular-operation.pdf` (Giảng viên cung cấp thêm)
Tài liệu hướng dẫn 4 thuật toán mô-đun quan trọng:
- `mod(x, y)`: Tính $x \% y$ bằng dịch bit và trừ (đã cài đặt trong `divmod` ở `bigint.cpp`).
- `AddMod(x, y, p)`: Tính $(x + y) \% p$ trong $n$-bit (kiểm tra $x \ge p - y$ để trừ $x - (p - y)$, tránh tràn số $n$-bit).
- `MultMod(x, y, p)`: Tính $(x \cdot y) \% p$ bằng kỹ thuật Double-and-Add nhị phân kết hợp `AddMod`.
- `ExpMod(x, y, p)`: Lũy thừa mô-đun Square-and-Multiply dùng `MultMod`.
*(Toàn bộ 4 thuật toán này đã được Khang hỗ trợ viết sẵn trong `dh.cpp` và kiểm thử tự động trong `tests/test_bigint.cpp` để Mạnh và nhóm tham khảo/đối chiếu).*

---

## 3. Kiến trúc chương trình & giao diện thống nhất (API)

Phát triển theo hướng gợi ý của `u512_sample.h`: **số nguyên không âm, độ rộng cố định**, mảng limb `uint64_t` little-endian, tích trung gian dùng `__uint128_t`.

```cpp
// ===== bigint.cpp =====
#define uint128_t __uint128_t

template <size_t N>                 // N limb 64-bit
struct uint_t {
    uint64_t digits[N];             // digits[0] là limb thấp nhất
    uint_t();                       // = 0
};
typedef uint_t<8>  uint512_t;
typedef uint_t<16> uint1024_t;
typedef uint_t<32> uint2048_t;

// Kiểu làm việc chung: rộng gấp đôi số bit tối đa của p để tích a*b (a, b < p) không tràn
const size_t MAX_PRIME_BITS = 1024;                 // bit_size tối đa hỗ trợ
typedef uint_t<2 * MAX_PRIME_BITS / 64> big_t;

// Tiện ích:  from_u64, from_hex, to_hex, to_dec, is_zero, is_odd, bit_length, test_bit
// So sánh:   ==, !=, <, <=, >, >=
// Số học:    +, - (giả định a >= b), *, <<, >>
// Chia:      divmod(a, b, q, r), /, %, mod_small(a, m)   (mod_small dùng cho sàng)

// ===== prng.cpp (từ mẫu, đã gộp header + port) =====
void randombytes(uint8_t *out, size_t outlen);
void prng_seed(prng_ctx *ctx, const uint8_t *seed, size_t seedlen);
uint64_t prng_next_u64(prng_ctx *ctx);  // ...

// ===== prime.cpp (Minh) & dh.cpp (Mạnh) — chữ ký theo dh_sample.h =====
big_t random_bits(prng_ctx *ctx, size_t bits);                           // đúng 'bits' bit, bit cao nhất = 1
big_t random_range(prng_ctx *ctx, const big_t &lo, const big_t &hi);     // phân phối đều trong [lo, hi]
bool  is_probable_prime(prng_ctx *ctx, const big_t &n, int rounds);      // Miller-Rabin
big_t modular_exponentiation(const big_t &base, const big_t &exp, const big_t &mod);
big_t generate_safe_prime(prng_ctx *ctx, size_t bitsize = 512);
big_t generate_private_key(prng_ctx *ctx, const big_t &p);               // trong [2, p-2]
big_t choose_generator(const big_t &p);                                  // chọn g phù hợp với safe prime p
```

**Tổ chức mã nguồn khi chỉ được nộp `.cpp` — “unity build”:**

- Mỗi mảng code nằm trong **một file `.cpp` riêng** (mỗi người sở hữu file của mình → hạn chế conflict), **không có file `.h`**.
- Mỗi file có include guard (`#ifndef BIGINT_CPP` / `#define BIGINT_CPP` / `#endif`) và tự `#include` các header **chuẩn** nó cần.
- `main.cpp` `#include` các file `.cpp` còn lại **theo đúng thứ tự phụ thuộc**, nên chỉ cần biên dịch **một file `main.cpp`**. Các file khác **không** đưa vào lệnh biên dịch (tránh lỗi trùng định nghĩa).

```
project_01_source/           # đúng cấu trúc nộp bài, chỉ chứa .cpp
├── fips202.cpp              # Minh: gộp fips202.h + fips202.c của mẫu (giữ nguyên thuật toán)
├── prng.cpp                 # Minh: gộp prng.h + prng.cpp của mẫu, port Windows, sửa lỗi
├── bigint.cpp               # Khang (phần cơ bản) + Mạnh (khu vực chia/mod)
├── prime.cpp                # Minh: random_bits, random_range, Miller-Rabin, safe prime, private key
├── dh.cpp                   # Mạnh: modular_exponentiation, choose_generator
└── main.cpp                 # Mạnh: luồng Diffie-Hellman; Khang: tham số dòng lệnh, thứ tự #include
tests/                       # Mạnh: unit test (.cpp), script đối chiếu Python — KHÔNG nộp
docs/                        # bản nháp từng mục báo cáo — KHÔNG nộp
project_01_report/
└── report.pdf
```

Thứ tự `#include` trong `main.cpp`:

```cpp
#define _CRT_RAND_S          // phải đứng trước mọi #include (dùng rand_s trên Windows)
#include "fips202.cpp"
#include "prng.cpp"
#include "bigint.cpp"
#include "dh.cpp"            // modular_exponentiation (prime.cpp cần dùng cho Miller-Rabin)
#include "prime.cpp"
// ... main()
```

**Lệnh biên dịch chuẩn** (chạy tại `project_01_source/`, giống nhau trên Windows/Linux):

```bash
g++ -std=c++14 -O2 -Wall -Wextra -o dh main.cpp
./dh
```

---

## 4. Phân công chi tiết

### 👤 24120070 – Vương Hữu Khang
**Mảng chính: Kiểu số lớn (phần cơ bản) + build/tích hợp + hoàn thiện bài nộp**

**Lập trình**
- [x] Khởi tạo cấu trúc repo theo mục 3: tạo khung các file `.cpp` (include guard, khu vực của từng người), `main.cpp` với thứ tự `#include`, `.gitignore`; đảm bảo khung build được bằng đúng 1 lệnh `g++ ... main.cpp`.
- [x] Cài đặt `uint_t<N>` phần cơ bản trong `bigint.cpp` (đã qua unit test và **đối chiếu 30.000 phép tính ngẫu nhiên với Python, 0 sai lệch** — xem `tests/crosscheck_bigint.*`):
  - Constructor, `from_u64`, `from_hex`, `to_hex`, `to_dec`.
  - `is_zero`, `is_odd`, `bit_length`, `test_bit`, `set_bit`.
  - So sánh `==, !=, <, <=, >, >=`.
  - Cộng/trừ có nhớ (dùng `uint128_t`), nhân schoolbook O(n²) có cắt theo số limb thực dùng, dịch bit `<<`, `>>`, `mod_small`.
  - **Đã bàn giao lõi số lớn cho Mạnh & Minh** (vượt tiến độ trước mốc 06/11).
- [ ] Tích hợp code của Minh và Mạnh; đảm bảo build sạch warning trên cả Windows (MSYS2) và Linux/WSL.
- [x] Tham số dòng lệnh: `--bits <n>`, `--rounds <k>`, `--seed <hex>` (đã tích hợp đầy đủ trong `main.cpp`).
  - Đã kiểm tra đầu vào: `--bits` > 0 và ≤ `MAX_PRIME_BITS`, `--rounds` ∈ [1, 1000], `--seed` đúng 64 ký tự hex.
- [x] Tạo sẵn thư mục `project_01_report/` (chứa `report.pdf` khi hoàn thành).

**Báo cáo**
- [x] **Mục 1 – Hệ thống xử lý số lớn**: đã soạn thảo bản thảo chi tiết trong `docs/muc1_he_thong_so_lon.md` (biểu diễn limb 64-bit, tối ưu gấp đôi `MAX_PRIME_BITS`, thuật toán và độ phức tạp các toán tử; chờ Mạnh bổ sung phần chia Knuth D & lũy thừa mô-đun).
- [x] **Mục 5 – Mô tả chương trình**: đã soạn thảo bản thảo chi tiết trong `docs/muc5_mo_ta_chuong_trinh.md` (môi trường g++/MSYS2, kiến trúc Unity Build, cách biên dịch, chạy, đổi `bits`/`rounds`/`seed`/`MAX_PRIME_BITS`, ví dụ output).
- [ ] Gộp báo cáo thành 1 PDF (bìa, mục lục, bảng phân công & mức độ hoàn thành, tài liệu tham khảo) — *thực hiện khi các mục 2, 3, 4, 6 hoàn tất*.

**Hoàn thiện bài nộp**
- [ ] Rà soát `project_01_source/` **chỉ chứa file `.cpp`** (không `.h`, `.c`, `.exe`, Makefile, file test).
- [ ] Đóng gói `24120070-24120089-24120091.zip`, giải nén thử và build lại từ đầu trước khi nộp.

---

### 👤 24120091 – Lâm Duy Minh
**Mảng chính: PRNG + Kiểm tra nguyên tố + Sinh số nguyên tố an toàn + Sinh khóa riêng (Nhiệm vụ B, C)**

**Lập trình**
- [ ] **Hoàn thiện PRNG từ mẫu**: gộp `fips202.h` + `fips202.c` → `fips202.cpp`, `prng.h` + `prng.cpp` → `prng.cpp` (không còn header); sửa các lỗi 1–4 ở mục 2 (port `randombytes` sang Windows bằng `rand_s`, `outlen` → `size_t`, ép kiểu trước khi dịch bit, khởi tạo seed đủ 32 byte từ `randombytes`).
- [ ] **`random_bits(ctx, bits)`**: ghép các `prng_next_u64` thành số đúng `bits` bit (bật bit cao nhất; với ứng viên nguyên tố bật thêm bit thấp nhất).
- [ ] **`random_range(ctx, lo, hi)`**: **rejection sampling** để phân phối đều (không dùng `% n` gây lệch).
- [ ] **`is_probable_prime` (Miller-Rabin)**: `n-1 = 2^s · d`, cơ sở ngẫu nhiên trong `[2, n-2]`, số vòng cấu hình được (mặc định 40); xử lý `n < 4`, số chẵn. Viết & test trước trên `uint64_t` trong lúc chờ phép chia số lớn.
- [ ] **Sàng sơ bộ** bằng các số nguyên tố nhỏ (vd. < 2000, dùng `mod_small`) để loại nhanh ứng viên.
- [ ] **`generate_safe_prime(ctx, bitsize)`**: sinh `q` (`bitsize-1` bit) → kiểm tra `q` và `p = 2q + 1` cùng nguyên tố; tối ưu: sàng đồng thời cho `q` và `p` (loại `q` nếu `q ≡ 0` hoặc `q ≡ (r-1)/2 mod r`), chạy 1 vòng Miller-Rabin/Fermat cơ sở 2 trước rồi mới chạy đủ số vòng.
- [ ] **`generate_private_key(ctx, p)`**: `random_range(ctx, 2, p-2)`.
- [ ] Benchmark sinh safe prime 512 / 768 / 1024 bit (số ứng viên đã thử, thời gian trung bình) – gửi số liệu cho Khang.

**Báo cáo**
- [ ] **Mục 2 – Kiểm tra số nguyên tố**: chia thử, Fermat (số Carmichael), Miller-Rabin (cơ sở lý thuyết, xác suất sai ≤ 4^(-k)), Solovay-Strassen, AKS (giới thiệu); safe prime/Sophie Germain prime và lý do dùng trong DH; mô tả cài đặt của nhóm.
- [ ] **Mục 3 – Số ngẫu nhiên (điểm cộng)**: TRNG/PRNG/CSPRNG; nguồn entropy của OS (`getrandom`, `rand_s`/`RtlGenRandom`, `BCryptGenRandom`); **SHAKE128/Keccak sponge – vì sao một XOF dùng được làm CSPRNG** (theo thư viện mẫu); vai trò của seed và vì sao seed cố định chỉ dùng để debug; sự cố thực tế do ngẫu nhiên yếu (vd. Debian OpenSSL 2008); vì sao không dùng `rand()`/`std::mt19937`.

---

### 👤 24120089 – Tạ Quang Mạnh
**Mảng chính: Chia/mod số lớn + Lũy thừa mô-đun + Giao thức Diffie-Hellman + Kiểm thử (Nhiệm vụ A, D)**

**Lập trình**
- [ ] **Phép chia & mod cho `uint_t<N>`** (khu vực riêng trong `bigint.cpp`): `divmod`, `/`, `%`. Làm trước bản chia nhị phân (shift-subtract) để có kết quả đúng sớm, sau đó tối ưu bằng **Knuth Algorithm D** (chia theo limb 64-bit với `uint128_t`).
  - ⚠️ **Ưu tiên cao:** bản shift-subtract Khang đặt sẵn đo được ≈ 120 µs/phép `mulmod` 512-bit (≈ 90 ms mỗi `modular_exponentiation` 512-bit) — quá chậm để sinh safe prime (cần hàng nghìn lần Miller-Rabin). Knuth D cần nhanh hơn ≥ 10 lần; làm **trước 09/11** vì Minh phụ thuộc vào tốc độ này. Đối chiếu kết quả bằng `tests/crosscheck_bigint.py` (phải giữ 0 sai lệch).
- [ ] **`modular_exponentiation`**: **square-and-multiply** duyệt bit số mũ, giảm mod sau mỗi phép nhân; xử lý `mod = 1`, `exp = 0`.
  - 💡 **Gợi ý từ tài liệu `pseudocode-modular-operation.pdf`:** Có thể đối chiếu giữa thuật toán nhân khối `(base * base) % mod` với thuật toán `ExpMod`/`MultMod`/`AddMod` của thầy (đã có sẵn code tham chiếu trong `dh.cpp`). So sánh hiệu năng hai phương pháp để đưa vào Báo cáo Mục 1 & 4.
- [ ] **`choose_generator(p)`**: với `p = 2q + 1`, các bậc phần tử chỉ có thể là 1, 2, q, 2q → chọn `g` sao cho `g ≠ ±1` và kiểm tra `g^q mod p` để biết `g` sinh nhóm con bậc `q` (thặng dư bậc hai) hay cả nhóm bậc `2q`; trình bày lựa chọn của nhóm và lý do (vd. dùng nhóm con bậc `q` để tránh rò rỉ 1 bit qua ký hiệu Legendre).
- [ ] **`main.cpp` – luồng Diffie-Hellman**: seed PRNG → sinh `p`, `g` → khóa riêng `a`, `b` → `A`, `B` → bí mật chung hai phía → in kết quả (hex), xác minh trùng khớp, in thời gian từng bước.
- [ ] **Kiểm thử** (`tests/`):
  - Unit test cho số lớn (+, -, *, /, %, <<, >>, chuyển đổi chuỗi) và `modular_exponentiation`, đối chiếu với Python (`pow(b, e, m)`) trên bộ test ngẫu nhiên.
  - Test Miller-Rabin với số nguyên tố/hợp số đã biết (gồm số Carmichael 561, 41041, 825265) và safe prime chuẩn RFC 3526.
  - Chạy DH nhiều lần với seed ngẫu nhiên và seed cố định, đảm bảo bí mật chung luôn khớp.
  - Báo lỗi cho người phụ trách qua GitHub Issues.

**Báo cáo**
- [ ] **Bổ sung Mục 1**: thuật toán chia/mod và lũy thừa mô-đun (mô tả, độ phức tạp, vì sao hiệu quả với số lớn).
- [ ] **Mục 4 – Ứng dụng thực tế của Diffie-Hellman**: ít nhất một ứng dụng chi tiết (khuyến nghị **TLS 1.3 – (EC)DHE**, hoặc SSH/IPsec IKE/Signal), có sơ đồ luồng; tấn công man-in-the-middle, Logjam; vì sao DH cần đi kèm xác thực.

---

### 👥 Công việc chung (cả 3 thành viên)
- [ ] **Mục 6 – Tự đánh giá**: mỗi người viết ưu/nhược điểm và hướng cải thiện cho phần mình làm; Khang tổng hợp.
- [ ] Review chéo: mỗi Pull Request phải được **ít nhất 1 thành viên khác** duyệt trước khi merge vào `main`.
- [ ] Đọc lại toàn bộ báo cáo lần cuối (chính tả, thuật ngữ thống nhất, trích dẫn nguồn).

---

## 5. Bảng tổng hợp

| Hạng mục | Khang (24120070) | Minh (24120091) | Mạnh (24120089) |
|---|:---:|:---:|:---:|
| Cấu trúc repo, khung unity build, tích hợp | **Chính** | | |
| Số lớn: biểu diễn, I/O, so sánh, +, -, *, shift | **Chính** | | Review |
| Số lớn: chia, mod | Review | | **Chính** |
| PRNG (port từ mẫu, `random_bits`, `random_range`) | Review | **Chính** | |
| A – `modular_exponentiation` | | Review | **Chính** |
| B – Miller-Rabin + `generate_safe_prime` | | **Chính** | Test |
| C – `generate_private_key` | | **Chính** | Test |
| D – Diffie-Hellman (`main`, chọn `g`) | Tham số CLI | | **Chính** |
| Kiểm thử & benchmark | Hỗ trợ | Benchmark prime | **Chính** |
| BC Mục 1 – Số lớn | **Chính** | | Phần chia/mod/modexp |
| BC Mục 2 – Kiểm tra nguyên tố | | **Chính** | |
| BC Mục 3 – Số ngẫu nhiên (điểm cộng) | | **Chính** | |
| BC Mục 4 – Ứng dụng DH | | | **Chính** |
| BC Mục 5 – Mô tả chương trình | **Chính** | | |
| BC Mục 6 – Tự đánh giá | Tổng hợp | Phần mình | Phần mình |
| Gộp báo cáo PDF + đóng gói nộp | **Chính** | Đọc duyệt | Đọc duyệt |

---

## 6. Tiến độ (04/11 – 17/11/2026)

| Thời gian | Khang | Minh | Mạnh | Mốc kiểm tra |
|---|---|---|---|---|
| **T4 04/11 – T5 05/11** | Thống nhất API, tạo khung repo (các file `.cpp` + `main.cpp`) | Gộp & port PRNG sang Windows, sửa lỗi mẫu | Cài môi trường g++, chuẩn bị bộ test Python | Khung project build & chạy được PRNG |
| **T6 06/11 – CN 08/11** | So sánh, +, -, shift (**xong 06/11**), nhân, I/O | `random_bits`, `random_range`; Miller-Rabin trên `uint64_t` | Chia shift-subtract, `%`; unit test số lớn | |
| **T2 09/11** | Hoàn thiện & sửa lỗi số lớn | Chuyển Miller-Rabin sang `big_t` | Hoàn thiện chia/mod | **Mốc 1:** số lớn đủ phép toán, qua unit test |
| **T3 10/11 – T5 12/11** | Tích hợp, tham số CLI | Sàng, `generate_safe_prime`, `generate_private_key` | `modular_exponentiation`, tối ưu chia (Knuth D) | **Mốc 2 (12/11):** sinh được safe prime 512 bit |
| **T6 13/11 – T7 14/11** | Sửa lỗi tích hợp, tối ưu | Benchmark 512/768/1024 bit | `choose_generator`, `main` DH, test tổng thể | **Mốc 3 (14/11):** DH hoàn chỉnh, bí mật chung khớp |
| **08/11 – 14/11 (song song)** | Viết Mục 1, 5 | Viết Mục 2, 3 | Viết Mục 4 + phần Mục 1 | Bản nháp đủ các mục trong `docs/` |
| **CN 15/11 – T2 16/11** | Gộp báo cáo PDF | Đọc duyệt, viết Mục 6 phần mình | Đọc duyệt, viết Mục 6 phần mình | `report.pdf` bản cuối (tối 16/11) |
| **T3 17/11** | Build lần cuối, đóng gói zip, **nộp** | Kiểm tra bài nộp | Kiểm tra bài nộp | **Nộp trước deadline** |

> Nếu một phần trễ mốc, người phụ trách báo ngay trong nhóm để phân bổ hỗ trợ — **Mốc 1 quan trọng nhất** vì mọi phần khác phụ thuộc vào kiểu số lớn.

---

## 7. Quy ước làm việc trên GitHub

- **Nhánh**: `main` luôn biên dịch được. Mỗi người làm trên nhánh riêng:
  - `feature/bigint-core` (Khang), `feature/bigint-div-modexp`, `feature/diffie-hellman` (Mạnh), `feature/prng-prime` (Minh), `docs/<tên-mục>` cho báo cáo.
- **Commit message**: `<loại>: <mô tả ngắn>` — ví dụ `feat: add Miller-Rabin test`, `fix: carry bug in operator+`, `docs: section 4 TLS`, `test: modexp vs python`.
- **Pull Request**: mô tả thay đổi + cách test; cần 1 người khác duyệt; không push trực tiếp code chưa test lên `main`.
- **Môi trường**: g++ hỗ trợ `__uint128_t` (MSYS2 UCRT64 / MinGW-w64 / WSL); luôn build với `-std=c++14`.
- **Không** dùng thư viện ngoài (GMP, NTL, Boost, OpenSSL…) và tính năng C++17 trở lên (`std::optional`, structured bindings, `if constexpr`…).
- **Lỗi/việc phát sinh**: tạo GitHub Issue, gán người phụ trách.

---

## 8. Tiêu chí hoàn thành (Definition of Done)

- [ ] Biên dịch được với `-std=c++14`, chỉ dùng C++ Standard Library + thư viện PRNG giảng viên cung cấp.
- [ ] Sinh được safe prime ≥ 512 bit trong thời gian hợp lý; `bits` chỉnh được.
- [ ] Khóa riêng nằm trong `[2, p-2]`, sinh từ PRNG SHAKE128 được seed bằng entropy của hệ điều hành.
- [ ] Phần tử sinh `g` được chọn có cơ sở và giải thích trong báo cáo.
- [ ] Bí mật chung của Alice và Bob trùng khớp qua nhiều lần chạy.
- [ ] Báo cáo PDF đủ 6 mục theo đề, có bảng phân công & mức độ hoàn thành, tài liệu tham khảo.
- [ ] File nộp đúng tên và cấu trúc thư mục, nộp trước 17/11/2026.

