# BÁO CÁO MỤC 5: MÔ TẢ CHƯƠNG TRÌNH & HƯỚNG DẪN SỬ DỤNG

**Phụ trách chính:** Vương Hữu Khang (MSSV: 24120070)

---

## 5.1. Môi trường phát triển và Yêu cầu hệ thống

Chương trình được thiết kế tương thích đa nền tảng (Cross-platform), hoạt động hoàn toàn chính xác trên cả Windows, Linux và macOS:

- **Hệ điều hành thử nghiệm:**
  - Windows 10/11 (sử dụng môi trường MSYS2 MinGW-w64 UCRT64).
  - Linux (Ubuntu 20.04 / 22.04 LTS hoặc Windows Subsystem for Linux - WSL).
- **Trình biên dịch:** `g++` (phiên bản GCC 7.0 trở lên, khuyến nghị GCC 11+ hoặc GCC 14/15).
  - Yêu cầu hỗ trợ kiểu phần cứng mở rộng `__uint128_t` / `unsigned __int128`.
- **Chuẩn ngôn ngữ:** **C++14 trở xuống** (`-std=c++14`), tuân thủ nghiêm ngặt quy định không sử dụng tính năng của C++17/C++20 và chỉ dùng thư viện chuẩn C++ Standard Library.

---

## 5.2. Kiến trúc tổ chức mã nguồn (Unity Build)

Theo quy định của đề bài:
> *"Cấu trúc bài nộp: `project_01_source/*.cpp`"*

Để đảm bảo thư mục nộp bài chỉ chứa duy nhất các file mã nguồn `.cpp` mà không cần file header rời `.h` hay `Makefile`, nhóm áp dụng mô hình kiến trúc **Unity Build**:

```text
project_01_source/
├── fips202.cpp     # Cài đặt lõi Keccak, hàm băm SHA-3 & hàm XOF SHAKE128 (FIPS 202)
├── prng.cpp        # Bộ sinh số ngẫu nhiên CSPRNG dùng SHAKE128, port rand_s cho Windows
├── bigint.cpp      # Kiểu dữ liệu số lớn big_t, các toán tử số học & chia Knuth D
├── dh.cpp          # Lũy thừa modulo, chọn phần tử sinh g và kiểm tra khóa công khai
├── prime.cpp       # Rejection sampling, Miller-Rabin, sàng đồng thời & sinh Safe Prime
└── main.cpp        # Điểm vào chương trình, xử lý tham số CLI & điều phối luồng Diffie-Hellman
```

- Mỗi module được viết độc lập trong một file `.cpp` có include guard (`#ifndef ...`).
- File `main.cpp` đóng vai trò là file tổng hợp, `#include` các file `.cpp` khác theo đúng thứ tự phụ thuộc.
- **Ưu điểm vượt trội**:
  - Người chấm bài chỉ cần biên dịch đúng 1 file `main.cpp` duy nhất bằng 1 lệnh `g++`.
  - Tận dụng tối đa tính năng tối ưu hóa toàn cục của trình biên dịch (Whole Program Optimization / Inlining) mà không cần bật LTO phức tạp.
  - Loại bỏ hoàn toàn lỗi liên kết trùng lặp định nghĩa (ODR violation).

---

## 5.3. Hướng dẫn biên dịch chương trình

Mở terminal tại thư mục `project_01_source/` (hoặc thư mục gốc sau khi giải nén bài nộp):

```bash
cd project_01_source
g++ -std=c++14 -O2 -Wall -Wextra -Wpedantic -o dh main.cpp
```

### Giải thích các cờ biên dịch:
- `-std=c++14`: Bắt buộc tuân thủ chuẩn C++14 theo đúng yêu cầu đề bài.
- `-O2`: Bật tối ưu hóa mức độ cao của trình biên dịch (tăng tốc độ thực thi các phép nhân số lớn và vòng lặp Miller-Rabin).
- `-Wall -Wextra -Wpedantic`: Bật toàn bộ cảnh báo kiểm tra lỗi cú pháp và kiểu dữ liệu (mã nguồn được đảm bảo biên dịch sạch tuyệt đối, không có warning).
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
Xác suất một hợp số vượt qua kiểm tra Miller-Rabin sau $k$ vòng không vượt quá $4^{-k}$. Với $k = 40$, xác suất sai số là $4^{-40} \approx 8{,}27 \times 10^{-25}$ (đủ an toàn tuyệt đối trong thực tế). Để thay đổi số vòng kiểm tra:
```bash
./dh --bits 512 --rounds 50
```

### 5. Chạy với Seed cố định (`--seed`) để gỡ lỗi và tái lập kết quả
Đề bài cho phép dùng seed cố định phục vụ mục đích kiểm thử và tái hiện kết quả. Người dùng có thể truyền vào chuỗi hex 64 ký tự (tương ứng đúng 32 byte):
```bash
./dh --bits 512 --seed 0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef
```

---

## 5.5. Hướng dẫn thay đổi các tham số trong mã nguồn

Nếu muốn thay đổi cố định các giá trị mặc định trực tiếp trong mã nguồn:

1. **Thay đổi giới hạn kích thước bit tối đa (`BIG_LIMBS`)**:
   - Vị trí: Trong file `project_01_source/bigint.cpp` (dòng 23):
   ```cpp
   static const size_t BIG_LIMBS = 32; // 32 limbs x 64 bit = 2048 bit
   ```
   - Cấu trúc `big_t` hỗ trợ số nguyên lớn tới 2048 bit, đủ dung lượng chứa kết quả tích trung gian của các số 1024-bit. Nếu cần mở rộng lên 4096 bit, chỉ cần điều chỉnh `BIG_LIMBS = 64`.

2. **Thay đổi phần tử sinh $g$**:
   - Vị trí: Trong hàm `choose_generator()` tại `project_01_source/dh.cpp`:
   - Mặc định chọn $g = 4 = 2^2 \bmod p$, sinh nhóm con thặng dư bậc hai cấp $q = (p - 1)/2$ để triệt tiêu hoàn toàn rủi ro rò rỉ parity của khóa bí mật.

---

## 5.6. Minh họa kết quả chạy thực tế (Terminal Output)

Kết quả thực thi thực tế của chương trình trên máy thử nghiệm với cấu hình chuẩn 512-bit:

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
    => Hoan tat trong: 0.383135 giay
    p = 86aee1daa34b41b726b9eea8f1e664c3e1d4b249e10573d1c85a25bcb3197031f87779e9690c53c30ba9fdd1b7f20f71a5b6ceba60f4052f1bff879cb072612f
    (p co dung 512 bit)

[2] Chon phan tu sinh g...
    g = 4
    Kiem tra g != 1 va g^q mod p == 1: DUNG (g sinh nhom con bac nguyen to q = (p-1)/2)

[3] Sinh khoa rieng cho Alice va Bob trong khoang [2, p - 2]...
    Khoa rieng Alice a = 4df54fa0252e372f66c186113e72ed087b3bbde7aa76c555750c9ccb150e692dc89f86095c60e6d0adfd3b939ac74a2e89d0b1fee33b3a3dd88855cb1f0ce698
    Khoa rieng Bob   b = 687ea30dbbd2fde264573eba30078b4b1819dc13b76ad76b9421258660302bc4e4695fa2f7bb791beaadc88a655d87e98d718f5b41ee5196a0d906085ebf6bee

[4] Tinh gia tri cong khai:
    Alice gui A = g^a mod p = 3521abedb37bd83bb9983e48fbd0cef6ec72dfebabcbf1169ca4914572fc98e2211b885dce1bc4db4888705cdd95054bfc30508ac5310aa74b2fb7d42326b9e0
    Bob   gui B = g^b mod p = 35a2bf8d0085bc85d4b067adeb1a2fe92d1cdda9667b3f39a346b77325b9271218f102cd6085c60db9378b6b3e2f42ddbba48ec328b7c390b5eed7db2a0fe0de
    Thoi gian tinh A, B: 0.001157 giay
    Kiem tra gia tri cong khai hop le (trong [2, p-2] va A^q = B^q = 1): DUNG

[5] Tinh bi mat chung:
    Bi mat chung Alice tinh duoc (s_A) = 5fd013cfbc23fa6fbbf0f3421a2baf01140d54ac9061ae0ef529f9829fed8646a2985342b26f45290f10375b4d84a314c91a8a4aa8174f786708c575e476db0a
    Bi mat chung Bob   tinh duoc (s_B) = 5fd013cfbc23fa6fbbf0f3421a2baf01140d54ac9061ae0ef529f9829fed8646a2985342b26f45290f10375b4d84a314c91a8a4aa8174f786708c575e476db0a
    Thoi gian tinh toan DH: 0.0014605 giay

===============================================================
  [XAC MINH THANH CONG] Bi mat chung hai ben hoan toan TRUNG KHOP!
===============================================================
```
