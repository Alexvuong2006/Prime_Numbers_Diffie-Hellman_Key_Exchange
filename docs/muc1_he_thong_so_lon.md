# BÁO CÁO MỤC 1: HỆ THỐNG XỬ LÝ SỐ NGUYÊN LỚN (BIGINT)

**Phụ trách chính:** Vương Hữu Khang (MSSV: 24120070)  
**Phần bổ sung (Chia/Mod & Lũy thừa mô-đun):** Tạ Quang Mạnh (MSSV: 24120089)

---

## 1.1. Đặt vấn đề và Mục tiêu thiết kế

Trong giao thức trao đổi khóa Diffie-Hellman, các phép toán mật mã được thực hiện trên trường hữu hạn $\mathbb{Z}_p^*$ với số nguyên tố an toàn $p$ có kích thước tối thiểu là 512 bit (hoặc 1024, 2048 bit trong thực tế). Các kiểu dữ liệu nguyên thủy trong chuẩn C++14 (như `uint64_t`) chỉ hỗ trợ tối đa 64 bit, hoàn toàn không đáp ứng được yêu cầu tính toán.

Hơn nữa, quy định của đề bài nghiêm cấm việc sử dụng các thư viện số lớn bên ngoài (như GMP, NTL, Boost.Multiprecision). Do đó, nhóm bắt buộc phải tự thiết kế và cài đặt cấu trúc số nguyên lớn từ đầu.

### Mục tiêu thiết kế của kiểu số lớn `uint_t<N>`:
1. **Không âm (Unsigned)**: Toàn bộ các giá trị trong Diffie-Hellman (số nguyên tố $p$, phần tử sinh $g$, khóa riêng $a, b$, khóa công khai $A, B$, và bí mật chung $s$) đều là các số nguyên dương nằm trong khoảng $[0, p - 1]$. Việc chỉ xử lý số không âm giúp tối ưu hóa đáng kể độ phức tạp của các thuật toán và tránh chi phí xử lý bit dấu.
2. **Độ rộng cố định (Fixed-width) qua Template**:
   - Sử dụng `template <size_t N>` với $N$ là số lượng limb 64-bit.
   - Bộ nhớ của số lớn được cấp phát tĩnh (stack/inline), không dùng cấp phát động (`std::vector` hay `malloc/new`). Điều này giúp triệt tiêu chi phí phân mảnh bộ nhớ (heap fragmentation), tận dụng tối đa bộ nhớ đệm CPU (L1/L2 cache locality) và tăng tốc độ thực thi đáng kể trong các vòng lặp tính lũy thừa lặp lại hàng nghìn lần.
3. **Quy tắc gấp đôi độ rộng (`MAX_PRIME_BITS`)**:
   - Trong phép nhân hai số $a, b < p$ (với $p$ có tối đa $B$ bit), tích $a \times b$ có độ dài tối đa là $2B$ bit trước khi thực hiện phép chia lấy dư $\bmod\ p$.
   - Do đó, nhóm định nghĩa kiểu làm việc chung `big_t` có độ rộng gấp đôi kích thước bit tối đa của $p$:
     $$\text{BIG\_LIMBS} = \frac{2 \times \text{MAX\_PRIME\_BITS}}{64}$$
     Với $\text{MAX\_PRIME\_BITS} = 1024\text{ bit}$, nhóm chọn $N = 32\text{ limbs}$ (tương đương 2048 bit). Nhờ đó, mọi phép cộng, trừ, nhân đều không bao giờ bị tràn số (overflow) ngoài tầm kiểm soát.

---

## 1.2. Cấu trúc biểu diễn dữ liệu

Cấu trúc được định nghĩa trong file `project_01_source/bigint.cpp`:

```cpp
__extension__ typedef unsigned __int128 uint128_t;

template <size_t N>
struct uint_t {
    uint64_t digits[N]; // Mảng N phần tử 64-bit
    ...
};
```

- **Thứ tự lưu trữ (Endianness)**: Little-endian theo cấp độ limb:
  - `digits[0]` lưu 64 bit có trọng số thấp nhất ($2^0 \to 2^{63}$).
  - `digits[1]` lưu 64 bit tiếp theo ($2^{64} \to 2^{127}$).
  - `digits[N - 1]` lưu 64 bit cao nhất.
  - Giá trị toán học của số nguyên được biểu diễn là:
    $$A = \sum_{i=0}^{N-1} \text{digits}[i] \times 2^{64i}$$
- **Kiểu dữ liệu mở rộng `uint128_t`**:
  Trình biên dịch `g++` (GCC/Clang) hỗ trợ kiểu mở rộng phần cứng 128-bit `__uint128_t`. Kiểu này cho phép nhân hai số 64-bit hoặc cộng hai số 64-bit có nhớ để cho ra kết quả 128-bit chỉ trong một vài chu kỳ xung nhịp CPU mà không cần phải bẻ nhỏ thành các limb 32-bit, giúp hiệu năng tăng gấp 3 đến 4 lần.

---

## 1.3. Chi tiết các thuật toán và độ phức tạp

### 1.3.1. Các hàm tiện ích & Thao tác bit
- **`is_zero()`**: Duyệt kiểm tra mọi limb xem có bằng 0 hay không. Độ phức tạp: $O(N)$.
- **`is_odd()`**: Kiểm tra bit thấp nhất: `(digits[0] & 1) != 0`. Độ phức tạp: $O(1)$.
- **`test_bit(size_t i)` & `set_bit(size_t i)`**:
  Xác định limb chứa bit: $\text{limb} = i / 64$, và vị trí bit trong limb: $\text{bit} = i \bmod 64$.
  Độ phức tạp: $O(1)$.
- **`bit_length()`**:
  Tìm limb cao nhất khác 0 (`top`). Sau đó sử dụng hàm nội tại của GCC `__builtin_clzll` (Count Leading Zeros) để xác định số bit có nghĩa trong limb cao nhất:
  $$\text{bit\_length} = \text{top} \times 64 + (64 - \text{clzll}(\text{digits}[\text{top}]))$$
  Độ phức tạp: $O(N)$.

### 1.3.2. Chuyển đổi chuỗi (I/O)
- **`from_hex(const std::string &s)`**:
  - Tự động bỏ qua tiền tố `0x` hoặc `0X` (nếu có).
  - Duyệt chuỗi hex từ phải qua trái, gom từng cụm 16 ký tự hex vào một limb 64-bit (`digits[0]`, `digits[1]`,...).
  - Độ phức tạp: $O(L)$ với $L$ là độ dài chuỗi ký tự.
- **`to_hex()`**:
  - Tìm limb cao nhất khác 0, in limb này không đệm số 0 đầu (`%llx`).
  - Các limb thấp hơn được in với đúng 16 ký tự có đệm số 0 (`%016llx`).
  - Nếu số bằng 0, trả về `"0"`.
  - Độ phức tạp: $O(N)$.
- **`to_dec()`**:
  - Để chuyển đổi số nhị phân lớn sang hệ thập phân một cách nhanh chóng, nhóm không chia từng số 10 đơn lẻ mà thực hiện phép chia lặp cho $10^{18}$ (lũy thừa lớn nhất của 10 nằm vừa trong `uint64_t`):
    - Mỗi vòng lặp, số lớn được chia cho $10^{18}$ từ limb cao xuống thấp bằng `uint128_t`.
    - Phần dư 64-bit thu được chính là 18 chữ số thập phân tương ứng.
  - Kỹ thuật này giảm số bước chia tới 18 lần.
  - Độ phức tạp: $O(N^2)$.

### 1.3.3. Các phép toán so sánh
- **`operator==`**: So sánh từng limb từ 0 đến $N - 1$. Độ phức tạp: $O(N)$.
- **`operator<`**: So sánh từ limb cao nhất $N - 1$ xuống limb thấp nhất $0$. Ngay khi gặp cặp limb khác nhau, đưa ra kết luận. Độ phức tạp: $O(N)$ trong trường hợp xấu nhất, trung bình $O(1)$.
- Các toán tử khác (`!=`, `>`, `<=`, `>=`) được suy ra trực tiếp từ `==` và `<`.

### 1.3.4. Phép cộng (`operator+`)
- Thuật toán cộng có nhớ (Ripple-carry Addition):
  ```cpp
  uint128_t carry = 0;
  for (size_t i = 0; i < N; i++) {
      uint128_t sum = (uint128_t)a.digits[i] + b.digits[i] + carry;
      res.digits[i] = (uint64_t)sum;
      carry = sum >> 64;
  }
  ```
- Kết quả được tự động lấy theo modulo $2^{64N}$.
- Độ phức tạp: $O(N)$.

### 1.3.5. Phép trừ (`operator-`)
- Giả định tiền điều kiện: $a \ge b$.
- Thuật toán trừ có mượn:
  - Nếu `a.digits[i] >= b.digits[i] + borrow`: không cần mượn, `borrow = 0`.
  - Ngược lại: mượn $2^{64}$ từ limb kế tiếp, `borrow = 1`.
- Độ phức tạp: $O(N)$.

### 1.3.6. Phép nhân (`operator*`)
- Thuật toán nhân cổ điển (Schoolbook Multiplication):
  - Nhóm tối ưu hóa bằng cách xác định số lượng limb có nghĩa thực tế của $a$ (gọi là $len_a$) và $b$ (gọi là $len_b$).
  - Bỏ qua các limb bằng 0 trong vòng lặp ngoài:
    $$\text{res}[i + j] \leftarrow \text{res}[i + j] + a[i] \times b[j] + \text{carry}$$
- Độ phức tạp thời gian: $O(len_a \times len_b) \le O(N^2)$. Trong các phép tính với số nhỏ, thời gian giảm tỷ lệ thuận với số limb thực tế.

### 1.3.7. Phép dịch bit (`<<` và `>>`)
- Được phân tách thành hai giai đoạn:
  1. Dịch chuyển limb nguyên: $\text{limb\_shift} = \text{shift} / 64$.
  2. Dịch chuyển bit lẻ trong limb: $\text{bit\_shift} = \text{shift} \bmod 64$.
- Sử dụng phép gộp bit giữa limb hiện tại và limb liền kề để dịch chính xác trong 1 lượt duyệt.
- Độ phức tạp: $O(N)$.

### 1.3.8. Sàng số nhỏ (`mod_small`)
- Tính $a \bmod m$ với $m$ là số nguyên nhỏ 32-bit (dùng để sàng các số nguyên tố nhỏ trước khi chạy Miller-Rabin).
- Áp dụng sơ đồ Horner từ limb cao nhất xuống:
  $$\text{rem} = ((\text{rem} \times 2^{64}) + \text{digits}[i]) \bmod m$$
- Độ phức tạp: $O(len_a)$.

### 1.3.9. So sánh hai hướng tiếp cận số học mô-đun (AddMod, MultMod vs. Bộ đệm $2n$-bit)
Dựa trên tài liệu tham khảo bổ sung của môn học (`pseudocode-modular-operation.pdf`), nhóm đã nghiên cứu và phân tích hai hướng tiếp cận khác nhau trong việc thực thi các phép toán mô-đun:

1. **Phương pháp 1: Giữ biến trung gian luôn trong $n$-bit (Mã giả của giảng viên)**:
   - **Cơ chế**:
     - `AddMod(x, y, p)`: Dùng điều kiện kiểm tra $x \ge p - y$ để tính $(x + y) \bmod p$ bằng phép trừ $x - (p - y)$, triệt tiêu nguy cơ tràn bit $n$.
     - `MultMod(x, y, p)`: Tính $(x \cdot y) \bmod p$ bằng kỹ thuật cộng-dồn nhị phân (Double-and-Add) kết hợp `AddMod`, duyệt qua $n$ bit của $y$.
     - `ExpMod(x, y, p)`: Lũy thừa mô-đun dạng Square-and-Multiply dựa trên `MultMod`.
   - **Ưu điểm**: Mọi biến trung gian được đảm bảo không vượt quá $n$-bit. Rất phù hợp nếu hệ thống bị ép buộc chỉ được cấp phát một struct kích thước cố định đúng $n$-bit (như `uint512_t` thuần túy).
   - **Hạn chế**: Mỗi phép nhân `MultMod` phải lặp qua toàn bộ số bit của $y$ (512 lượt gọi `AddMod`), dẫn đến độ phức tạp thực thi cao hơn.

2. **Phương pháp 2: Sử dụng bộ đệm độ rộng gấp đôi ($2n$-bit) kết hợp nhân khối (Cách nhóm cài đặt)**:
   - **Cơ chế**:
     - Định nghĩa `big_t` có độ rộng $2n$-bit (2048 bit).
     - Phép nhân hai số $n$-bit $x \times y$ được thực hiện nguyên khối qua thuật toán Schoolbook tận dụng lệnh nhân 128-bit phần cứng `uint128_t` (chỉ mất $8 \times 8 = 64$ phép nhân limb cho 512-bit).
     - Sau đó áp dụng phép chia nhị phân `divmod` / `%` để lấy phần dư modulo $p$.
   - **Ưu điểm**: Tối ưu hóa sâu phần cứng CPU, tốc độ nhanh hơn vượt trội. Thực nghiệm đo đạc lũy thừa mô-đun 512-bit cho thấy:
     - Phương pháp 2 (Nhóm): **~20.1 ms**.
     - Phương pháp 1 (Mã giả thầy): **~62.8 ms** (chậm hơn khoảng 3 lần).
   - **Kết luận**: Nhóm cài đặt đầy đủ cả hai phương án trong mã nguồn (`dh.cpp`) để đối chiếu, kiểm chứng tính chính xác (cho ra kết quả khớp 100%) và phục vụ tối ưu hóa tốc độ.

---

## 1.4. Bảng tổng kết độ phức tạp các toán tử

| Toán tử / Hàm | Mục đích | Độ phức tạp thời gian | Bộ nhớ phụ |
|---|---|:---:|:---:|
| `is_zero()`, `is_odd()` | Kiểm tra 0 / số lẻ | $O(N)$ / $O(1)$ | $O(1)$ |
| `bit_length()` | Đếm số bit có nghĩa | $O(N)$ | $O(1)$ |
| `test_bit()`, `set_bit()` | Thao tác bit đơn lẻ | $O(1)$ | $O(1)$ |
| `==`, `!=`, `<`, `>`, `<=`, `>=` | So sánh hai số lớn | $O(N)$ | $O(1)$ |
| `+`, `-` | Cộng, trừ số lớn | $O(N)$ | $O(N)$ |
| `*` | Nhân số lớn (Schoolbook) | $O(len_a \times len_b) \le O(N^2)$ | $O(N)$ |
| `<<`, `>>` | Dịch trái, dịch phải bit | $O(N)$ | $O(N)$ |
| `mod_small(a, m)` | Lấy dư với số nhỏ (sàng số) | $O(N)$ | $O(1)$ |
| `from_hex`, `to_hex` | Chuyển đổi chuỗi Hex | $O(N)$ | $O(N)$ |
| `to_dec` | Chuyển đổi chuỗi Dec | $O(N^2)$ | $O(N)$ |
| `divmod(a, b, q, r)` | Chia & lấy dư hai số lớn | $O(k \cdot N)$ (với $k$ là số bit) | $O(N)$ |
