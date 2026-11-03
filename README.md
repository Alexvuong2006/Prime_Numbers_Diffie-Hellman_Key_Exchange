# Đồ án 1: Số nguyên tố & Trao đổi khóa Diffie-Hellman

> **Môn học:** Nhập môn mã hóa – mật mã  
> **Bộ môn:** Công nghệ Tri thức – Trường Đại học Khoa học Tự nhiên, ĐHQG-HCM  
> **Giảng viên thực hành:** Thầy Trần Hải Anh Điền  
> **Hạn nộp (Deadline):** 17/11/2026  

---

## 👥 Thông tin Nhóm 18

| STT | Mã số sinh viên | Họ và tên | GitHub / Liên hệ |
|:---:|:---:|---|---|
| 1 | **24120070** | Vương Hữu Khang | [@Alexvuong2006](https://github.com/Alexvuong2006) |
| 2 | **24120089** | Tạ Quang Mạnh | |
| 3 | **24120091** | Lâm Duy Minh | |

Chi tiết phân công nhiệm vụ, API thống nhất giữa các thành viên và tiến độ thực hiện được trình bày tại:  
👉 **[Tài liệu phân công công việc & Kế hoạch đồ án (PHAN_CONG_CONG_VIEC.md)](PHAN_CONG_CONG_VIEC.md)**

---

## 🎯 Mục tiêu đồ án

1. **Hệ thống số nguyên lớn (`BigInt`)**: Tự xây dựng cấu trúc số nguyên lớn không âm (512-bit, 1024-bit, 2048-bit) bằng mảng limb 64-bit little-endian và toán tử 128-bit (`__uint128_t`), không phụ thuộc vào thư viện bên ngoài (GMP, NTL, Boost,...).
2. **Bộ sinh số giả ngẫu nhiên an toàn (PRNG)**: Tận dụng cơ chế Sponge & hàm mở rộng đầu ra (XOF) **SHAKE128** (chuẩn FIPS 202) từ thư viện tham khảo của môn học; nạp seed an toàn từ nguồn entropy của hệ điều hành (`rand_s` trên Windows, `getrandom` trên Linux/macOS).
3. **Kiểm tra số nguyên tố Miller-Rabin**: Kiểm định xác suất số nguyên tố ngẫu nhiên với số vòng lặp $k \ge 40$, kết hợp sàng lọc sơ bộ bằng số nguyên tố nhỏ.
4. **Sinh số nguyên tố an toàn (Safe Prime)**: Tự động sinh số nguyên tố lớn $p \ge 512\text{ bit}$ thỏa mãn điều kiện $p = 2q + 1$ (với $q$ cũng là số nguyên tố – Sophie Germain prime).
5. **Giao thức trao đổi khóa Diffie-Hellman**:
   - Chọn phần tử sinh $g$ phù hợp trên nhóm $\mathbb{Z}_p^*$.
   - Sinh khóa riêng ngẫu nhiên $a, b \in [2, p - 2]$.
   - Tính toán giá trị công khai: $A = g^a \bmod p$ và $B = g^b \bmod p$.
   - Tính bí mật chung hai phía: $s_A = B^a \bmod p$ và $s_B = A^b \bmod p$.
   - Xác minh hai bên đồng thuận khóa bí mật chung $s_A = s_B$.
6. **Báo cáo chuyên sâu**: Trình bày cấu trúc số lớn, phân tích thuật toán kiểm tra số nguyên tố, khảo sát vai trò của tính ngẫu nhiên trong mật mã học, phân tích ứng dụng thực tế của Diffie-Hellman (TLS 1.3 / ECDHE), và tự đánh giá chương trình.

---

## 🛠️ Yêu cầu kỹ thuật & Môi trường

- **Ngôn ngữ:** C/C++ (chuẩn **C++14 trở xuống**, tuân thủ C++ Standard Library).
- **Trình biên dịch:** `g++` hỗ trợ kiểu `__uint128_t` (GCC 7+ trên Linux/WSL hoặc MinGW-w64 / MSYS2 UCRT64 trên Windows).
- **Mô hình tổ chức:** **Unity Build** — toàn bộ mã nguồn nộp bài gồm các file `.cpp` nằm trong thư mục `project_01_source/`, biên dịch thông qua một file `main.cpp` duy nhất (không phụ thuộc file header rời `.h` hay `Makefile`).

---

## 🚀 Hướng dẫn biên dịch & Chạy chương trình

### 1. Biên dịch

Mở terminal tại thư mục gốc repository hoặc bên trong `project_01_source/`:

```bash
cd project_01_source
g++ -std=c++14 -O2 -Wall -Wextra -o dh main.cpp
```

*(Lệnh biên dịch hoạt động đồng nhất trên cả Windows, Linux và macOS).*

### 2. Chạy chương trình

- **Chạy mặc định (512-bit, 40 vòng Miller-Rabin, seed ngẫu nhiên từ HĐH):**
  ```bash
  ./dh
  ```

- **Tùy chỉnh số bit của số nguyên tố $p$ và số vòng kiểm tra:**
  ```bash
  ./dh --bits 512 --rounds 40
  ```

- **Chạy với seed cố định (phục vụ kiểm thử, gỡ lỗi và tái lập kết quả):**
  ```bash
  ./dh --bits 512 --seed 0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef
  ```

- **Xem menu trợ giúp:**
  ```bash
  ./dh --help
  ```

---

## 📂 Cấu trúc Repository

```
Prime_Numbers_Diffie-Hellman_Key_Exchange/
├── README.md                       # Tài liệu tổng quan dự án (file này)
├── PHAN_CONG_CONG_VIEC.md          # Phân công chi tiết, API, tiến độ & quy ước làm việc
├── .gitignore                      # Cấu hình bỏ qua các file nhị phân, file sinh tự động
│
├── project_01_source/              # [QUAN TRỌNG] Thư mục nộp bài - CHỈ CHỨA FILE .cpp
│   ├── fips202.cpp                 # Thuật toán Keccak / SHAKE128 (FIPS 202)
│   ├── prng.cpp                    # PRNG nền SHAKE128 & hàm lấy entropy hệ điều hành
│   ├── bigint.cpp                  # Kiểu dữ liệu số lớn uint_t<N>, các phép toán số học & chia
│   ├── dh.cpp                      # Lũy thừa mô-đun (modular_exponentiation) & chọn g
│   ├── prime.cpp                   # Miller-Rabin, sinh safe prime & sinh khóa riêng
│   └── main.cpp                    # Chương trình chính thực thi luồng trao đổi khóa DH
│
├── docs/                           # Bản thảo các mục báo cáo (Mục 1 - 6) trước khi đóng gói PDF
│   └── README.md
│
└── tests/                          # Bộ kiểm thử đơn vị (Unit Tests) & script Python kiểm chứng
    └── README.md
```

---

## 📦 Quy định đóng gói bài nộp

Bài nộp cuối cùng nén theo định dạng `.zip` với cấu trúc:

```
24120070-24120089-24120091.zip
├── project_01_source/
│   └── *.cpp
└── project_01_report/
    └── *.pdf
```

*(Mọi thành phần không biên dịch được hoặc sinh lỗi thời gian chạy sẽ không có điểm theo quy định của môn học).*
