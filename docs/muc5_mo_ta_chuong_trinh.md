# BÁO CÁO MỤC 5: MÔ TẢ CHƯƠNG TRÌNH & HƯỚNG DẪN SỬ DỤNG

**Phụ trách chính:** Vương Hữu Khang (MSSV: 24120070)

---

## 5.1. Môi trường phát triển và Yêu cầu hệ thống

Chương trình được thiết kế tương thích đa nền tảng (Cross-platform), hoạt động hoàn toàn chính xác trên cả Windows, Linux và macOS:

- **Hệ điều hành thử nghiệm:**
  - Windows 10/11 (sử dụng môi trường MSYS2 MinGW-w64 UCRT64).
  - Linux (Ubuntu 20.04 / 22.04 LTS hoặc Windows Subsystem for Linux - WSL).
- **Trình biên dịch:** `g++` (phiên bản GCC 7.0 trở lên, khuyến nghị GCC 11+ hoặc GCC 14).
  - Yêu cầu hỗ trợ kiểu phần cứng mở rộng `__uint128_t`.
- **Chuẩn ngôn ngữ:** **C++14 trở xuống** (`-std=c++14`), tuân thủ nghiêm ngặt quy định không sử dụng tính năng của C++17/C++20 và chỉ dùng thư viện chuẩn C++ Standard Library.

---

## 5.2. Kiến trúc tổ chức mã nguồn (Unity Build)

Theo quy định của đề bài:
> *"Cấu trúc bài nộp: `project_01_source/*.cpp`"*

Để đảm bảo thư mục nộp bài chỉ chứa duy nhất các file mã nguồn `.cpp` mà không cần file header rời `.h` hay `Makefile`, nhóm áp dụng mô hình kiến trúc **Unity Build**:

```
project_01_source/
├── fips202.cpp     # Cài đặt lõi Keccak, hàm băm SHA-3 & hàm XOF SHAKE128 (FIPS 202)
├── prng.cpp        # Bộ sinh số giả ngẫu nhiên PRNG dùng SHAKE128, port randombytes cho Windows
├── bigint.cpp      # Kiểu dữ liệu số lớn uint_t<N> và đầy đủ các phép toán số học
├── dh.cpp          # Lũy thừa mô-đun (modular_exponentiation) & chọn phần tử sinh g
├── prime.cpp       # Kiểm tra Miller-Rabin, sinh số nguyên tố an toàn & sinh khóa riêng
└── main.cpp        # Điểm vào chương trình, xử lý tham số CLI & điều phối luồng Diffie-Hellman
```

- Mỗi module được viết độc lập trong một file `.cpp` có include guard (`#ifndef ...`).
- File `main.cpp` đóng vai trò là file tổng hợp, `#include` các file `.cpp` khác theo đúng thứ tự phụ thuộc.
- **Ưu điểm vượt trội**:
  - Người chấm bài chỉ cần biên dịch đúng 1 file `main.cpp` duy nhất.
  - Tận dụng tối đa tính năng tối ưu hóa toàn cục của trình biên dịch (Whole Program Optimization / Inlining) mà không cần bật LTO phức tạp.
  - Loại bỏ hoàn toàn lỗi liên kết trùng lặp định nghĩa (ODR violation).

---

## 5.3. Hướng dẫn biên dịch chương trình

Mở terminal tại thư mục `project_01_source/` (hoặc thư mục gốc sau khi giải nén bài nộp):

```bash
cd project_01_source
g++ -std=c++14 -O2 -Wall -Wextra -o dh main.cpp
```

### Giải thích các cờ biên dịch:
- `-std=c++14`: Bắt buộc tuân thủ chuẩn C++14 theo đúng yêu cầu đề bài.
- `-O2`: Bật tối ưu hóa mức độ cao của trình biên dịch (tăng tốc độ thực thi các phép nhân số lớn và vòng lặp Miller-Rabin).
- `-Wall -Wextra`: Bật toàn bộ cảnh báo kiểm tra lỗi cú pháp và kiểu dữ liệu (mã nguồn được đảm bảo biên dịch sạch, không có warning).
- `-o dh`: Đặt tên file thực thi sinh ra là `dh` (hoặc `dh.exe` trên Windows).

---

## 5.4. Hướng dẫn chạy chương trình & Cấu hình tham số dòng lệnh

Chương trình cung cấp giao diện dòng lệnh linh hoạt với các tùy chọn tham số:

### 1. Xem hướng dẫn sử dụng
```bash
./dh -h
# hoặc:
./dh --help
```

### 2. Chạy mặc định (Cấu hình chuẩn của đề bài)
```bash
./dh
```
- Độ dài số nguyên tố $p$: **512 bit** (mặc định).
- Số vòng kiểm tra Miller-Rabin: **40 vòng**.
- Nguồn ngẫu nhiên: Lấy entropy trực tiếp từ hệ điều hành (`rand_s` trên Windows, `getrandom` trên Linux).

### 3. Tùy chỉnh độ lớn của số nguyên tố $p$ (`--bits`)
Để kiểm nghiệm hiệu năng hoặc đáp ứng các tiêu chuẩn bảo mật cao hơn (ví dụ 768 bit, 1024 bit):
```bash
./dh --bits 768
# hoặc:
./dh --bits 1024
```

### 4. Tùy chỉnh số vòng kiểm tra Miller-Rabin (`--rounds`)
Xác suất một hợp số vượt qua kiểm tra Miller-Rabin sau $k$ vòng không vượt quá $4^{-k}$. Với $k = 40$, xác suất sai số là $4^{-40} \approx 10^{-24}$ (đủ an toàn tuyệt đối trong thực tế). Để thay đổi số vòng kiểm tra:
```bash
./dh --bits 512 --rounds 50
```

### 5. Chạy với Seed cố định (`--seed`) để gỡ lỗi và tái lập kết quả
Đề bài cho phép sinh viên dùng seed cố định phục vụ mục đích học tập và gỡ lỗi. Người dùng có thể truyền vào chuỗi hex 64 ký tự (tương ứng đúng 32 byte):
```bash
./dh --bits 512 --seed 0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef
```

---

## 5.5. Hướng dẫn thay đổi các tham số trong mã nguồn

Nếu muốn thay đổi cố định các giá trị mặc định trực tiếp trong mã nguồn:

1. **Thay đổi giới hạn kích thước bit tối đa (`MAX_PRIME_BITS`)**:
   - Vị trí: Trong file `project_01_source/bigint.cpp` (dòng ~180).
   ```cpp
   const size_t MAX_PRIME_BITS = 1024; // Mặc định hỗ trợ tối đa 1024-bit prime
   ```
   - Nếu muốn thử nghiệm với các số nguyên tố lớn hơn (như 2048 bit), người dùng chỉ cần sửa thành `2048`. Cấu trúc `big_t` sẽ tự động mở rộng tương ứng thành 64 limbs (4096 bit) để chứa tích trung gian mà không cần sửa đổi thêm bất kỳ hàm nào.

2. **Thay đổi phần tử sinh $g$**:
   - Vị trí: Trong hàm `choose_generator()` tại `project_01_source/dh.cpp`.

---

## 5.6. Minh họa kết quả chạy thực tế (Terminal Output)

Dưới đây là output mẫu thực tế khi thực thi chương trình:

```text
===============================================================
  DO AN 1: SO NGUYEN TO & TRAO DOI KHOA DIFFIE-HELLMAN
  Nhom 18 - Mon: Nhap mon ma hoa mat ma
===============================================================
Cau hinh:
  - Do dai bit p: 512 bit
  - So vong Miller-Rabin: 40
  - PRNG Seed: Ngau nhien tu he dieu hanh (randombytes)
===============================================================

[1] Dang sinh so nguyen to an toan p (512 bit)...
    => Hoan tat trong: 4.8213 giay
    p = f0a2bc89... (chuỗi hex 512-bit)

[2] Chon phan tu sinh g...
    g = 2

[3] Sinh khoa rieng cho Alice va Bob trong khoang [2, p - 2]...
    Khoa rieng Alice a = 7b2c91a0...
    Khoa rieng Bob   b = 3f901e84...

[4] Tinh gia tri cong khai:
    Alice gui A = g^a mod p = a8193f0b...
    Bob   gui B = g^b mod p = c57d9021...

[5] Tinh bi mat chung:
    Bi mat chung Alice tinh duoc (s_A) = e94a10bc...
    Bi mat chung Bob   tinh duoc (s_B) = e94a10bc...
    Thoi gian tinh toan DH: 0.0031 giay

===============================================================
  [XAC MINH THANH CONG] Bi mat chung hai ben hoan toan TRUNG KHOP!
===============================================================
```
