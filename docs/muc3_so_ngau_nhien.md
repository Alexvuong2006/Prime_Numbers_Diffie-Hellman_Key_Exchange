# BÁO CÁO MỤC 3: SỐ NGẪU NHIÊN & TẦM QUAN TRỌNG TRONG MẬT MÃ (MỤC ĐIỂM CỘNG)

**Phụ trách chính:** Lâm Duy Minh (MSSV: 24120091)

---

## 3.1. Vai trò cốt lõi của số ngẫu nhiên trong mật mã học

Theo nguyên lý Kerckhoffs (1883), độ an toàn của một hệ mật mã không được phép dựa trên sự bí mật của thuật toán, mà phải phụ thuộc hoàn toàn vào **tính bí mật và tính không thể đoán trước của khóa (key)**. Trong giao thức Diffie-Hellman nói riêng và mật mã học hiện đại nói chung, tính ngẫu nhiên xuất hiện ở hầu hết các mắt xích trọng yếu:

1. **Sinh khóa riêng ($a, b \in [2, p - 2]$):** Nếu kẻ tấn công có thể dự đoán hoặc thu hẹp không gian tìm kiếm của khóa riêng $a$, bài toán Logarit rời rạc (DLP) sẽ bị phá vỡ tức thì bằng vét cạn trên tập con có xác suất cao, bất kể số nguyên tố $p$ lớn đến đâu (2048 hay 4096 bit).
2. **Sinh tham số miền (Safe Prime $p = 2q + 1$):** Nếu nguồn sinh ứng viên $q$ bị lặp lại hoặc có entropy thấp, hai hệ thống độc lập có thể vô tình sinh trùng tham số hoặc rò rỉ cấu trúc của số nguyên tố.
3. **Chọn cơ sở kiểm tra trong thuật toán xác suất Miller-Rabin:** Định lý Rabin đảm bảo xác suất hợp số giả nguyên tố $\le 4^{-k}$ **chỉ khi** $k$ cơ sở $a \in [2, n - 2]$ được chọn độc lập và phân phối đều. Nếu bộ sinh ngẫu nhiên bị thao túng hoặc cố định, kẻ tấn công có thể chế tạo hợp số vượt qua mọi cơ sở cố định đã biết (tương tự phương pháp của Arnault năm 1995).

---

## 3.2. Phân loại bộ sinh số ngẫu nhiên: TRNG, PRNG và CSPRNG

| Tiêu chí | TRNG (True RNG) | PRNG (Pseudorandom NG) | CSPRNG (Cryptographically Secure PRNG) |
|---|---|---|---|
| **Nguồn đầu vào** | Nhiễu vật lý (nhiệt, bức xạ, thời gian ngắt phần cứng, jitter xung nhịp CPU) | Một giá trị mầm (seed) ngắn có tính tất định | Seed có entropy cao (thường lấy từ TRNG của hệ điều hành) |
| **Bản chất** | Không tất định (Non-deterministic), không có chu kỳ | Tất định hoàn toàn (Deterministic), có chu kỳ hữu hạn | Tất định từ seed nhưng **không thể phân biệt** với chuỗi ngẫu nhiên thực sự trong thời gian đa thức |
| **Tốc độ sinh** | Chậm (phụ thuộc tốc độ thu thập entropy vật lý) | Rất nhanh | Nhanh (tương đương tốc độ băm/mã hóa khối phần cứng) |
| **Khả năng dự đoán** | Không thể dự đoán | **Dễ dàng dự đoán** nếu biết một số đầu ra liên tiếp | **Bất khả thi về mặt tính toán** để đoán bit tiếp theo hoặc khôi phục trạng thái trước |
| **Ví dụ điển hình** | Intel RDSEED, nhiễu bức xạ, bộ thu entropy hạt nhân OS | `rand()` (LCG), `std::mt19937` (Mersenne Twister) | SHAKE128/256, ChaCha20-DRBG, CTR_DRBG (NIST SP 800-90A) |
| **Ứng dụng** | Tạo seed gốc ban đầu cho CSPRNG | Mô phỏng Monte Carlo, trò chơi, đồ họa | **Sinh khóa mật mã, nonce, IV, tham số nguyên tố** |

### Hai yêu cầu bảo mật bắt buộc của một CSPRNG:
1. **Next-bit Unpredictability (Không thể dự đoán bit kế tiếp - Yao, 1982):** Cho dù kẻ tấn công biết toàn bộ $L$ bit đầu ra đầu tiên $s_1, s_2, \dots, s_L$, không tồn tại thuật toán thời gian đa thức nào có thể dự đoán bit thứ $L + 1$ với xác suất thành công lớn hơn $\frac{1}{2} + \epsilon$ (với $\epsilon$ không đáng kể).
2. **Backtracking Resistance / Forward Security (Chống truy ngược trạng thái):** Nếu trạng thái nội tại của bộ sinh bị lộ tại thời điểm $t$, kẻ tấn công không thể tính ngược lại các số ngẫu nhiên đã sinh ra trước thời điểm $t$.

---

## 3.3. Vì sao tuyệt đối không dùng `rand()` và `std::mt19937` trong mật mã?

Một sai lầm phổ biến khi lập trình là sử dụng các hàm sinh ngẫu nhiên có sẵn trong thư viện chuẩn C/C++:

1. **`std::rand()` (Linear Congruential Generator - LCG):**
   - Công thức truy hồi tuyến tính: $X_{n+1} = (a X_n + c) \bmod m$.
   - Trạng thái nội tại thường chỉ rộng **31 hoặc 32 bit** (`RAND_MAX` trên Windows MinGW chỉ là $32767 = 2^{15} - 1$).
   - Chỉ cần quan sát 2 đến 3 giá trị đầu ra liên tiếp, kẻ tấn công giải hệ phương trình đồng dư tuyến tính là tìm được toàn bộ trạng thái $X_n$ và dự đoán 100% mọi số trong tương lai cũng như quá khứ.
   - Hơn nữa, kết hợp `srand(time(NULL))` chỉ có khoảng $86.400$ giá trị seed mỗi ngày — một máy tính cá nhân có thể vét cạn toàn bộ khóa riêng 512-bit sinh từ `srand(time(NULL))` trong **chưa đầy 1 mili-giây**.

2. **`std::mt19937` (Mersenne Twister):**
   - Mặc dù có chu kỳ cực lớn ($2^{19937} - 1$) và phân phối thống kê rất tốt cho mô phỏng khoa học, Mersenne Twister hoàn toàn **không an toàn mật mã**.
   - Hàm biến đổi trạng thái (tempering) của `mt19937` là một phép biến đổi tuyến tính khả nghịch trên trường $\mathbb{F}_2$. Chỉ cần thu thập đúng **624 giá trị đầu ra 32-bit liên tiếp**, kẻ tấn công có thể nghịch đảo phép tempering, tái tạo chính xác mảng trạng thái 624 từ nhớ và dự đoán mọi đầu ra tiếp theo với xác suất 100%.

---

## 3.4. Kiến trúc CSPRNG của nhóm: Nguồn Entropy Hệ điều hành + Keccak Sponge (SHAKE128)

Để đảm bảo an toàn mật mã thực thụ và tính tương thích đa nền tảng (Windows & Linux/macOS), hệ thống PRNG của nhóm (`fips202.cpp` và `prng.cpp`) được thiết kế theo mô hình chuẩn gồm 2 tầng: **Thu thập Entropy từ Hệ điều hành (OS CSPRNG)** để tạo seed 256-bit, sau đó đưa vào **Hàm mở rộng đầu ra SHAKE128 (Keccak Sponge XOF)**.

### 3.4.1. Nguồn Entropy từ Hệ điều hành (`randombytes`)
Hạt nhân hệ điều hành (OS Kernel) liên tục thu thập nhiễu phần cứng (thời gian ngắt bàn phím/chuột, I/O ổ đĩa, gói tin mạng, bộ đếm chu kỳ CPU `RDTSC`/`RDRAND`) vào bể chứa entropy (entropy pool) và trộn qua CSPRNG cấp hạt nhân:
- **Trên Linux / macOS:** Sử dụng lời gọi hệ thống `getrandom(out, outlen, 0)` (hoặc `/dev/urandom`), trích xuất trực tiếp từ bộ sinh ChaCha20-DRBG trong nhân Linux sau khi bể entropy đã khởi tạo đủ.
- **Trên Windows (`_WIN32`):** Mã nguồn mẫu ban đầu của giảng viên chỉ gọi `getrandom()` nên không biên dịch được trên Windows. Nhóm đã port sang Windows bằng cách sử dụng hàm **`rand_s()`** (yêu cầu định nghĩa macro `#define _CRT_RAND_S` trước `<stdlib.h>`). Bên dưới, `rand_s()` gọi trực tiếp vào API mật mã của nhân Windows (`RtlGenRandom` / `BCryptGenRandom` trong `bcryptprimitives.dll`) mà không cần liên kết thêm thư viện ngoài khi biên dịch bằng 1 lệnh `g++`.
- **Các lỗi tiềm ẩn trong code mẫu đã được nhóm khắc phục triệt để:**
  1. Tham số `uint8_t outlen` trong `randombytes` mẫu bị giới hạn tối đa 255 byte $\rightarrow$ nhóm đổi sang `size_t outlen` và đọc theo vòng lặp an toàn.
  2. Trong `prng_next_u32`, biểu thức `buf[3] << 24` thực hiện dịch trái trên kiểu `int` 32-bit có dấu (do integer promotion từ `uint8_t`), gây tràn bit dấu (**Undefined Behavior** trong chuẩn C++14 khi `buf[3] >= 0x80`) $\rightarrow$ nhóm ép kiểu tường minh `static_cast<uint32_t>(buf[i]) << (8 * i)` trước khi dịch bit.
  3. Trong `main_prng.cpp` mẫu, lệnh `memcpy(seed, "seed", sizeof("seed"))` chỉ ghi 5 byte đầu và để 27 byte còn lại của `seed[32]` là rác bộ nhớ chưa khởi tạo $\rightarrow$ nhóm khởi tạo toàn bộ 32 byte (256 bit) của `seed` trực tiếp từ `randombytes(seed, 32)`.

### 3.4.2. Cấu trúc Bọt biển Keccak (Sponge Construction) và vì sao XOF SHAKE128 dùng được làm CSPRNG
SHAKE128 (chuẩn hóa trong **NIST FIPS 202**) là một hàm **XOF (Extendable-Output Function)** dựa trên cấu trúc bọt biển **Keccak-$f[1600]$**:

```
       Seed (32 byte)
             │
             ▼
     ┌───────────────┐  r = 1344 bit (Rate - tốc độ hấp thụ/vắt)
 0 ──►   XOR & f     ├──► Khối đầu ra z_0 (168 byte) ──► XOR & f ──► Khối đầu ra z_1 ...
     ├───────────────┤
 0 ──►               │  c = 256 bit  (Capacity - dung lượng bảo mật ẩn)
     └───────────────┘
       Keccak-f[1600]        [ Pha Hấp thụ (Absorb) ]          [ Pha Vắt (Squeeze) ]
       (24 vòng hoán vị)
```

- **Trạng thái nội tại $S$:** Gồm $1600\text{ bit}$ ($5 \times 5 \times 64\text{ bit}$), được chia thành hai phần:
  - **Rate ($r = 1344\text{ bit} = 168\text{ byte}$):** Phần giao tiếp với bên ngoài (nhận dữ liệu vào hoặc xuất dữ liệu ra).
  - **Capacity ($c = 256\text{ bit} = 32\text{ byte}$):** Phần trạng thái ẩn **không bao giờ** được xuất trực tiếp ra ngoài và không bị tác động trực tiếp bởi dữ liệu đầu vào.
- **Hai pha hoạt động:**
  1. **Pha hấp thụ (`prng_seed` = `shake128_init` + `shake128_absorb` + `shake128_finalize`):** Chuỗi seed 32 byte (256 bit) cùng hậu tố đệm miền (domain separation suffix `0x1F` của SHAKE) được XOR vào phần Rate, sau đó toàn bộ 1600 bit trạng thái đi qua phép hoán vị phi tuyến 24 vòng **Keccak-$f[1600]$** (gồm 5 bước $\theta, \rho, \pi, \chi, \iota$).
  2. **Pha vắt (`prng_next_u64` $\rightarrow$ `shake128_squeeze`):** Mỗi lần cần byte ngẫu nhiên, chương trình trích xuất từ phần Rate ($168\text{ byte}$/khối). Khi dùng hết 168 byte trong bộ đệm Rate, hàm gọi lại hoán vị `KeccakF1600_StatePermute` để sinh khối 168 byte kế tiếp.
- **Vì sao SHAKE128 là một CSPRNG lý tưởng?**
  - Theo định lý bảo mật của cấu trúc Sponge (Bertoni et al., Eurocrypt 2007), trong mô hình hoán vị ngẫu nhiên (Random Permutation Model), dòng bit đầu ra của SHAKE128 không thể phân biệt được với một chuỗi bit ngẫu nhiên thực sự miễn là số lượng phép truy vấn $N \ll 2^{c/2} = 2^{128}$.
  - Phần **Capacity $c = 256\text{ bit}$** luôn bị che khuất hoàn toàn: ngay cả khi kẻ tấn công quan sát được một khối đầu ra 168 byte đầy đủ (phần Rate), họ vẫn thiếu 256 bit Capacity để có thể áp dụng hoán vị ngược $f^{-1}$ nhằm tìm lại trạng thái trước đó hoặc tính khối tiếp theo. Do đó, SHAKE128 thỏa mãn đồng thời tính không thể dự đoán ở mức an toàn **128-bit bảo mật cổ điển**.

---

## 3.5. Kỹ thuật Rejection Sampling và Loại bỏ Modulo Bias

Khi đã có nguồn bit ngẫu nhiên phân phối đều từ SHAKE128, bài toán tiếp theo là sinh một số nguyên ngẫu nhiên phân phối đều trong đoạn $[lo, hi]$ (ví dụ sinh khóa riêng $a \in [2, p - 2]$ hoặc chọn cơ sở Miller-Rabin $a \in [2, n - 2]$). Denote $M = hi - lo + 1$.

### 3.5.1. Sai lầm phổ biến: Modulo Bias khi dùng phép chia lấy dư `%`
Giả sử ta sinh một số ngẫu nhiên $k$-bit $R \in [0, 2^k - 1]$ rồi tính $X = lo + (R \bmod M)$.
Nếu $M$ không phải là lũy thừa của 2, phép chia $2^k = q \cdot M + r$ có số dư $r = 2^k \bmod M > 0$. Khi đó:
- Các giá trị trong đoạn $[0, r - 1]$ được ánh xạ bởi $q + 1$ giá trị của $R$.
- Các giá trị trong đoạn $[r, M - 1]$ chỉ được ánh xạ bởi $q$ giá trị của $R$.
Ví dụ: nếu $2^k = 16$ và $M = 11$, các số $0, 1, 2, 3, 4$ có xác suất xuất hiện là $\frac{2}{16} = 12{,}5\%$, cao gấp đôi xác suất $\frac{1}{16} = 6{,}25\%$ của các số $5, 6, 7, 8, 9, 10$! Trong mật mã (đặc biệt là DSA/ECDSA và Diffie-Hellman), sự chệch phân phối (Modulo Bias) này làm rò rỉ thông tin thống kê về các bit cao của khóa bí mật.

### 3.5.2. Giải pháp của nhóm: Rejection Sampling trong `random_range`
Trong file `project_01_source/prime.cpp`, nhóm cài đặt thuật toán **Lấy mẫu loại bỏ (Rejection Sampling)** chặt chẽ theo chuẩn NIST FIPS 186-5:
1. Tính độ rộng khoảng $\text{range} = hi - lo$, và tìm số bit nhỏ nhất $k = \text{bit\_length}(\text{range})$ sao cho $2^{k-1} \le \text{range} \le 2^k - 1$.
2. Sinh một số ngẫu nhiên $r$ gồm đúng $k$ bit độc lập từ PRNG (nằm đều trong $[0, 2^k - 1]$ bằng cách cắt mặt nạ bit `mask = (1ULL << rem_bits) - 1` ở limb cao nhất).
3. Kiểm tra:
   - Nếu $r \le \text{range}$: chấp nhận và trả về $lo + r$.
   - Nếu $r > \text{range}$: **hủy bỏ $r$ và lặp lại bước 2**.

**Phân tích hiệu quả:**
- Vì mọi giá trị trong $[0, \text{range}]$ đều xuất hiện trong $[0, 2^k - 1]$ với xác suất bằng nhau là $\frac{1}{2^k}$, phân phối có điều kiện khi $r \le \text{range}$ là **phân phối đều tuyệt đối 100%** ($\Pr[X = x] = \frac{1}{\text{range} + 1}$).
- Vì $\text{range} \ge 2^{k-1}$, xác suất chấp nhận trong mỗi lần thử luôn thỏa mãn:
  $$P_{\text{accept}} = \frac{\text{range} + 1}{2^k} > \frac{2^{k-1}}{2^k} = \frac{1}{2}$$
  Do đó, số lần lặp trung bình (kỳ vọng của phân phối hình học) luôn **$< 2$ lần thử**, vừa đảm bảo an toàn tuyệt đối vừa không tốn phép chia lớn `%`.

---

## 3.6. Vai trò của Seed và Vì sao Seed cố định chỉ dùng để Debug

- Trong `prng_seed(ctx, seed, 32)`, toàn bộ đầu ra của SHAKE128 là một hàm tất định của `seed`. Hai lần chạy có cùng `seed` sẽ sinh ra cùng số nguyên tố $p$, cùng khóa riêng $a, b$ và cùng bí mật chung $s$.
- **Lợi ích khi dùng `--seed <HEX>` trong kiểm thử/gỡ lỗi:** Giúp nhóm tái lập chính xác 100% một kịch bản lỗi (deterministic replay), đối chiếu kết quả giữa các máy tính khác nhau và phục vụ giảng viên chấm bài kiểm chứng tính đúng đắn.
- **Nguy cơ chí mạng nếu dùng Seed cố định trong thực tế:** Nếu dùng seed cố định (hoặc seed dễ đoán) trong môi trường sản xuất, mọi phiên giao dịch đều sinh ra cùng một khóa riêng $a, b$, làm mất hoàn toàn tính bảo mật chuyển tiếp (Forward Secrecy) và cho phép bất kỳ ai biết mã nguồn đều tính lại được khóa bí mật chung $s$. Vì vậy, chương trình của nhóm luôn **mặc định lấy 32 byte entropy mới từ hệ điều hành (`randombytes`)** khi chạy bình thường.

---

## 3.7. Các sự cố bảo mật thực tế kinh điển do bộ sinh ngẫu nhiên yếu

1. **Thảm họa Debian OpenSSL PRNG (2008 – CVE-2008-0166):**
   - Tháng 5/2006, một lập trình viên bảo trì gói OpenSSL trên hệ điều hành Debian/Ubuntu muốn xóa cảnh báo "đọc bộ nhớ chưa khởi tạo" của công cụ Valgrind nên đã **xóa nhầm dòng lệnh `MD_Update(&m, buf, j)`** dùng để nạp entropy hệ thống vào bộ sinh ngẫu nhiên.
   - Hậu quả: Nguồn entropy duy nhất còn lại trong OpenSSL trên hàng triệu máy chủ Linux suốt **2 năm (2006–2008)** chỉ là **Process ID (`getpid()`)** của tiến trình hiện tại. Vì PID tối đa trên Linux mặc định chỉ là $32.768$, toàn bộ thế giới chỉ có tối đa $32.768$ cặp khóa SSH/TLS/OpenVPN khác nhau cho mỗi kích thước bit! Kẻ tấn công chỉ cần tạo trước danh sách $32.768$ khóa là giải mã và đăng nhập trái phép vào hàng loạt máy chủ toàn cầu.
2. **Sinh trùng thừa số nguyên tố RSA trên thiết bị nhúng (Heninger et al., USENIX Security 2012 – *"Mining Your Ps and Qs"*):**
   - Nhóm nghiên cứu quét hàng triệu chứng chỉ TLS và khóa SSH công khai trên Internet và phát hiện hàng chục nghìn bộ định tuyến (router), tường lửa sinh khóa ngay lúc vừa khởi động (boot) khi bể entropy `/dev/urandom` chưa thu thập đủ nhiễu phần cứng.
   - Nhiều thiết bị sinh ra các mô-đun RSA $N_1 = p \cdot q_1$ và $N_2 = p \cdot q_2$ có chung một số nguyên tố $p$. Chỉ bằng một phép tính ước chung lớn nhất $\gcd(N_1, N_2) = p$ mất vài mili-giây, nhóm nghiên cứu đã khôi phục hoàn toàn khóa bí mật của hàng chục nghìn thiết bị mạng.
3. **Cửa hậu Dual_EC_DRBG (2006–2013):**
   - Bộ sinh ngẫu nhiên dựa trên đường cong Elliptic do NSA thúc đẩy đưa vào chuẩn NIST SP 800-90A chứa hai điểm hằng số $P, Q$. Nếu ai biết mối quan hệ logarit rời rạc bí mật $P = eQ$, người đó chỉ cần nhìn thấy 32 byte đầu ra của bộ sinh là tính ngược được trạng thái nội tại và giải mã toàn bộ kết nối TLS. Sự cố này dẫn đến việc NIST chính thức loại bỏ hoàn toàn Dual_EC_DRBG vào năm 2014.

---

## 3.8. Tài liệu tham khảo
1. NIST FIPS PUB 202, *SHA-3 Standard: Permutation-Based Hash and Extendable-Output Functions*, August 2015.
2. G. Bertoni, J. Daemen, M. Peeters, G. Van Assche, *"Sponge Functions"*, Ecrypt Hash Workshop 2007.
3. NIST SP 800-90A Rev. 1, *Recommendation for Random Number Generation Using Deterministic Random Bit Generators*, June 2015.
4. N. Heninger, Z. Durumeric, E. Wustrow, J. A. Halderman, *"Mining Your Ps and Qs: Detection of Widespread Weak Keys in Network Devices"*, USENIX Security Symposium, 2012.
5. Debian Security Advisory DSA-1571-1, *openssl — predictable random number generator (CVE-2008-0166)*, May 2008.

