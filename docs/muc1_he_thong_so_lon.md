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

---

## 1.5. Phép chia và lấy dư số lớn (`divmod`, `/`, `%`)

### 1.5.1. Vì sao phép chia là điểm nghẽn
Mọi phép nhân trong lũy thừa mô-đun đều phải rút gọn `mod p`. Với p 512 bit, một lần sinh số nguyên tố an toàn cần hàng nghìn lần Miller-Rabin, mỗi lần là một lũy thừa mô-đun (~768 phép nhân-mô-đun), nên tốc độ của `%` quyết định tốc độ toàn chương trình.

Phiên bản đầu dùng **chia nhị phân (shift-subtract)**: duyệt từng bit của số bị chia, mỗi bước dịch và so sánh/trừ cả số lớn, độ phức tạp $O(k \cdot N)$ với $k$ là số bit của số bị chia. Đo thực tế: **72 µs** cho một phép `mulmod` 512 bit.

### 1.5.2. Thuật toán Knuth D (TAOCP tập 2, mục 4.3.1)
Thay vì xử lý từng bit, Knuth D tính **mỗi lần một limb 64-bit của thương**, giống phép chia dài ở tiểu học nhưng trong hệ cơ số $b = 2^{64}$.

Gọi số bị chia $u$ có $m$ limb, số chia $v$ có $n$ limb (bỏ các limb cao bằng 0):

1. **Chuẩn hóa (D1).** Dịch trái cả $u$ và $v$ đi $s = \text{clz}(v_{n-1})$ bit để bit cao nhất của $v_{n-1}$ bằng 1. Điều kiện $v_{n-1} \ge 2^{63}$ bảo đảm ước lượng ở bước sau lệch tối đa 2 đơn vị.
2. **Với mỗi $j = m-n, \dots, 0$ (D2–D7):**
   - **Ước lượng (D3):** $\hat q = \lfloor (u_{j+n}\cdot 2^{64} + u_{j+n-1}) / v_{n-1} \rfloor$ dùng `uint128_t`.
   - **Hiệu chỉnh:** trong khi $\hat q \ge 2^{64}$ hoặc $\hat q \cdot v_{n-2} > \hat r \cdot 2^{64} + u_{j+n-2}$ thì giảm $\hat q$ đi 1 (lặp tối đa 2 lần). Bước này loại gần như mọi ước lượng thừa.
   - **Nhân-trừ (D4):** $u_{j..j+n} \leftarrow u_{j..j+n} - \hat q \cdot v$, dùng cờ mượn.
   - **Cộng bù (D5–D6):** nếu kết quả âm (xác suất khoảng $2/2^{64}$ với dữ liệu ngẫu nhiên) thì $\hat q \leftarrow \hat q - 1$ và cộng lại $v$.
   - Gán $q_j = \hat q$.
3. **Khử chuẩn hóa (D8):** phần dư $r = (u_{0..n-1}) \gg s$.

Trường hợp số chia chỉ có 1 limb ($n = 1$) dùng đường ngắn: chia tuần tự từ limb cao xuống thấp bằng `uint128_t`.

### 1.5.3. Độ phức tạp và kết quả đo
Số vòng ngoài là $m - n + 1$, mỗi vòng $O(n)$ nên tổng là $O(n(m-n+1))$ phép nhân 64×64→128. Trong `modular_exponentiation`, số bị chia là tích $2n$ limb và số chia $n$ limb nên mỗi phép `%` tốn $O(n^2)$, tương đương một phép nhân.

| Phép đo (g++ -O2) | Chia nhị phân | Knuth D |
|---|:---:|:---:|
| `mulmod` 512 bit | 72 µs | 0,44 µs |
| `modular_exponentiation`, mũ 512 bit | ~55 ms | 0,24 ms |
| `modular_exponentiation`, mũ 768 bit | – | 0,63 ms |
| `modular_exponentiation`, mũ 1024 bit | – | 1,17 ms |

Nhanh hơn khoảng **160 lần**, vượt xa mục tiêu ≥ 10 lần của nhóm.

Cập nhật bảng 1.4: `divmod(a, b, q, r)` — Chia & lấy dư — $O(n(m-n+1))$ — bộ nhớ phụ $O(N)$.

### 1.5.4. Kiểm thử tính đúng đắn
- `tests/crosscheck_bigint.py`: 30.000 phép tính ngẫu nhiên đối chiếu với số nguyên Python, **0 sai lệch**.
- `tests/crosscheck_div_stress.py`: 50.016 phép chia/mod với các số "khó" (limb toàn 0, $2^{63}$, $2^{64}-1$, …). Đo bằng bộ đếm tạm, bộ test này đi qua nhánh **cộng bù 3.242 lần** và nhánh hiệu chỉnh $\hat q$ 53.062 lần, **0 sai lệch**. (Test ngẫu nhiên thường gần như không bao giờ chạm nhánh cộng bù nên cần bộ test riêng này.)

---

## 1.6. Lũy thừa mô-đun (`modular_exponentiation`)

### 1.6.1. Bài toán
Tính $r = b^e \bmod m$ với $e$ cỡ 512 bit. Nhân $e$ lần là bất khả thi ($e \approx 2^{512}$).

### 1.6.2. Square-and-Multiply
Viết $e = \sum_{i=0}^{k-1} e_i 2^i$ ($e_i \in \{0,1\}$, $k$ = số bit). Duyệt bit từ cao xuống thấp:

```
result = 1
for i = k-1 down to 0:
    result = result * result mod m          # bình phương
    if e_i == 1: result = result * b mod m  # nhân
```

Ví dụ $3^{13} \bmod 7$: $13 = 1101_2$. Khởi tạo $1$; bit 1: $1^2 \cdot 3 = 3$; bit 1: $3^2 \cdot 3 = 27 \equiv 6$; bit 0: $6^2 = 36 \equiv 1$; bit 1: $1^2 \cdot 3 = 3$. Kết quả $3$ (đúng: $3^{13} = 1594323 = 7 \cdot 227760 + 3$).

### 1.6.3. Vì sao hiệu quả với số lớn
- **Số phép nhân:** $k$ phép bình phương và trung bình $k/2$ phép nhân, tức $\approx 1{,}5k$ phép `mulmod` (~768 với $k = 512$) thay vì $2^{512}$.
- **Kích thước bị chặn:** rút gọn `mod m` ngay sau mỗi phép nhân nên mọi giá trị trung gian $< m$ và tích $< m^2$, vừa trong `big_t` (rộng gấp đôi `MAX_PRIME_BITS`, xem mục 1.1). Không bao giờ phải lưu số $b^e$ khổng lồ.
- **Tổng độ phức tạp:** $O(k \cdot M(n))$ với $M(n) = O(n^2)$ là chi phí một phép nhân + chia $n$ limb. Với 512 bit: $O(512 \cdot 8^2)$ phép nhân limb.

### 1.6.4. Trường hợp biên đã xử lý
`mod = 1 → 0`; `mod = 0 → 0` (không xác định, tránh chia 0); `exp = 0 → 1`; `base ≥ mod` được rút gọn trước; `base = 0`.

### 1.6.5. Kiểm thử
`tests/crosscheck_modexp.py` đối chiếu 4.452 ca (ngẫu nhiên với modulus 8–1024 bit, ca biên, định lý Fermat nhỏ trên $2^{521}-1$) với hàm `pow(b, e, m)` của Python: **0 sai lệch**. Có thể nâng cấp nếu còn thời gian: nhân Montgomery (tránh phép chia) hoặc cửa sổ trượt (giảm số phép nhân còn $\approx k(1 + 1/(w+1))$).

### Tài liệu tham khảo (phần bổ sung)
- D. E. Knuth, *The Art of Computer Programming, Vol. 2: Seminumerical Algorithms*, mục 4.3.1, Algorithm D.
- H. S. Warren, *Hacker's Delight*, chương 9 (chia số nhiều từ, `divmnu`).
- A. Menezes, P. van Oorschot, S. Vanstone, *Handbook of Applied Cryptography*, mục 14.6 (lũy thừa) và 14.2.4 (chia).
