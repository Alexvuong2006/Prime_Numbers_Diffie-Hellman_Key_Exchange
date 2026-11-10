# BÁO CÁO MỤC 6: TỰ ĐÁNH GIÁ ƯU/NHƯỢC ĐIỂM & HƯỚNG CẢI THIỆN

**Phụ trách tổng hợp:** Vương Hữu Khang (24120070)  
**Đóng góp nội dung:** Lâm Duy Minh (24120091), Vương Hữu Khang (24120070), Tạ Quang Mạnh (24120089)

---

## 6.1. Đánh giá phần Bộ sinh số ngẫu nhiên (PRNG), Kiểm tra nguyên tố & Sinh Safe Prime
**Người thực hiện:** Lâm Duy Minh (MSSV: 24120091)

### 1. Ưu điểm đã đạt được:
- **An toàn mật mã và tương thích đa nền tảng (Cross-platform CSPRNG):**
  - Port thành công hàm lấy entropy hệ điều hành `randombytes()` sang Windows thông qua `rand_s()` (`RtlGenRandom`) song song với `getrandom()` trên Linux/macOS mà không cần phụ thuộc bất kỳ thư viện liên kết ngoài nào.
  - Khắc phục triệt để các lỗi tiềm ẩn trong mã nguồn tham khảo ban đầu: mở rộng tham số `outlen` từ `uint8_t` lên `size_t`, ép kiểu tường minh trước khi dịch bit trong `prng_next_u16`/`prng_next_u32` để loại bỏ Undefined Behavior (tràn bit dấu `int` trong C++14), và nạp đủ 256 bit (32 byte) entropy cho trạng thái Sponge của SHAKE128.
- **Phân phối ngẫu nhiên đều tuyệt đối (Zero Modulo Bias):**
  - Cài đặt thuật toán **Rejection Sampling** trong `random_range(ctx, lo, hi)` dựa trên mặt nạ bit tối tiểu $2^k - 1$, đảm bảo khóa riêng $a, b \in [2, p - 2]$ và các cơ sở kiểm tra Miller-Rabin có phân phối đều 100%, không dùng phép chia lấy dư `%` gây chệch thống kê.
- **Tối ưu hóa tốc độ sinh Safe Prime bằng kiến trúc lọc 4 tầng:**
  - Áp dụng **sàng đồng thời (Simultaneous Sieve)** trên cả $q$ và $p = 2q + 1$ chỉ bằng 1 phép chia nhỏ `mod_small(q, r)` cho mỗi số nguyên tố lẻ $r < 2000$ (kiểm tra $q \bmod r \in \{0, (r - 1)/2\}$), loại bỏ ngay **$\sim 98{,}56\%$** ứng viên hợp số với chi phí tính toán không đáng kể.
  - Đặt bước kiểm tra nhanh 1 vòng Miller-Rabin cơ sở 2 (`miller_rabin_base2`) cho cả $q$ và $p$ **trước** khi chạy đủ 40 vòng Miller-Rabin cơ sở ngẫu nhiên, giúp giảm số lần chạy Miller-Rabin 40 vòng từ hàng chục lần xuống đúng **1 lần duy nhất** cho mỗi safe prime tìm được.
  - Thời gian sinh safe prime 512 bit chỉ mất trung bình **~0,26 – 0,38 giây** và 1024 bit chỉ mất **~4,5 giây**, vượt xa kỳ vọng ban đầu của nhóm.

### 2. Hạn chế tồn đọng:
- **Chưa chống tấn công kênh kề theo thời gian (Timing Side-Channel):**
  - Các vòng lặp trong `modular_exponentiation` và `is_probable_prime` hiện sử dụng nhánh điều kiện phụ thuộc vào bit của số mũ/khóa bí mật và thoát sớm (early exit) khi phát hiện hợp số. Trong môi trường thực tế nếu kẻ tấn công đo được chính xác thời gian thực thi hoặc năng lượng tiêu thụ, một phần thông tin về số mũ có thể bị rò rỉ.
- **Sinh ứng viên tuần tự trên một luồng (Single-threaded):**
  - Quá trình thử các ứng viên $q$ trong `generate_safe_prime` hiện chạy trên 1 luồng CPU duy nhất. Khi tăng kích thước lên 2048 bit hoặc 4096 bit, số lượng ứng viên cần thử tăng lên hàng trăm nghìn, làm thời gian chờ trên 1 nhân kéo dài.
- **Chưa tự động tái nạp entropy (Reseeding):**
  - Bộ sinh SHAKE128 hiện được khởi tạo seed 1 lần khi bắt đầu chương trình. Mặc dù dung lượng $c = 256\text{ bit}$ của Keccak cho phép xuất hàng tỷ byte an toàn, một CSPRNG chuẩn công nghiệp (như NIST SP 800-90A) thường kết hợp cơ chế tự động nạp lại entropy (reseed) định kỳ sau một ngưỡng dữ liệu nhất định để đảm bảo tính chống xâm phạm trạng thái (prediction resistance).

### 3. Hướng cải thiện trong tương lai:
- **Tìm kiếm bước nhảy tăng dần trên bảng dư (Incremental Sieve Table):**
  - Thay vì sinh mới hoàn toàn ứng viên $q$ từ PRNG sau mỗi lần bị loại bởi sàng, có thể sinh một số ngẫu nhiên gốc $q_0$, lưu bảng số dư `rem[i] = q_0 % SMALL_PRIMES[i]`, sau đó tăng $q \leftarrow q + 2$ và cập nhật bảng số dư bằng phép cộng đơn giản `rem[i] = (rem[i] + 2) % r` trong $O(1)$ mà không cần gọi lại `mod_small`.
- **Song song hóa đa luồng (`std::thread` trong C++11/14):**
  - Khởi tạo $T$ luồng phần cứng (`std::thread::hardware_concurrency()`), mỗi luồng giữ một ngữ cảnh `prng_ctx` được phân tách miền (domain-separated seed) để tìm kiếm ứng viên safe prime độc lập và dừng ngay khi luồng đầu tiên tìm thấy, giúp tăng tốc gần tuyến tính theo số nhân CPU cho kích thước 2048–4096 bit.
- **Bổ sung kiểm tra Lucas (Baillie–PSW Primality Test):**
  - Kết hợp 1 vòng Miller-Rabin cơ sở 2 với 1 phép kiểm tra giả nguyên tố Lucas mạnh (Strong Lucas Probable Prime Test) theo chuẩn FIPS 186-5; cho đến nay chưa tìm thấy bất kỳ hợp số nào vượt qua đồng thời cả hai phép kiểm tra này.

---

## 6.2. Đánh giá phần Hệ thống xử lý số lớn (Big Integer) & Thuật toán phép chia Knuth D
**Người thực hiện:** Vương Hữu Khang (MSSV: 24120070) & Tạ Quang Mạnh (MSSV: 24120089)

### 1. Ưu điểm đã đạt được:
- **Cấu trúc dữ liệu tĩnh (Zero Heap Allocation):**
  - Kiểu `big_t` được cấp phát hoàn toàn trên Stack với kích thước cố định 2048 bit (32 từ nhớ 64-bit `uint64_t`). Tuyệt đối không dùng cấp phát động (`malloc`, `new`, `std::vector`), không gây phân mảnh bộ nhớ (heap fragmentation), tận dụng tối đa bộ đệm L1/L2 Cache của CPU.
- **Thuật toán nhân trường học tối ưu carry bằng kiểu 128-bit:**
  - Cài đặt phép nhân $O(n^2)$ sử dụng kiểu nội tại `__int128_t` / `unsigned __int128` có sẵn trên GCC/Clang 64-bit, tích lũy nhớ (carry) trực tiếp trong 1 thao tác duy nhất mà không cần tách từ 32-bit, đạt hiệu năng tối đa trên vi kiến trúc x86_64.
- **Thuật toán chia Knuth Algorithm D (The Art of Computer Programming, Vol 2):**
  - Thay thế hoàn toàn phép chia nhị phân (dịch bit - so sánh - trừ) tốn kém bằng thuật toán chia nhiều từ Knuth D:
    * Bước chuẩn hóa (Normalization): Dịch trái số chia $V$ và số bị chia $U$ sao cho bit cao nhất của từ nhớ đầu tiên $v_{n-1} \ge 2^{63}$, giúp độ chính xác của ước lượng thương $\hat{q}$ đạt tới $100\%$ hoặc chỉ lệch tối đa $1$ đơn vị.
    * Ước lượng thương và hiệu chỉnh: Tính thương thử $\hat{q} = \lfloor (u_j B + u_{j+1}) / v_1 \rfloor$ và hiệu chỉnh thông qua điều kiện $v_2 \hat{q} > (u_j B + u_{j+1} - \hat{q} v_1) B + u_{j+2}$.
    * Tăng tốc phép nhân - trừ và phục hồi nếu có mượn âm (Add-back step).
  - Tích hợp đường tắt chia ngắn (Short Division) cho trường hợp số chia chỉ có 1 limb 64-bit và phép chia cho lũy thừa của 2 bằng dịch bit phải `>>`.
  - Giảm thời gian thực hiện phép chia lấy dư 512-bit từ **$116\text{ µs}$ xuống còn $0{,}91\text{ µs}$ (tăng tốc hơn 100 lần)**.
- **Độ tin cậy được bảo chứng:**
  - Đã chạy kiểm thử áp lực đối chiếu kết quả với thư viện số nguyên lớn của Python (`tests/crosscheck_div_stress.py`) với hơn **50.000 ca kiểm thử ngẫu nhiên và ca biên ngặt nghèo**, đạt tỷ lệ chính xác $100\%$, không có bất kỳ sai lệch nào.
  - Cài đặt đầy đủ các hàm hỗ trợ theo pseudocode của giảng viên: `AddMod`, `MultMod`, `ExpMod`.

### 2. Hạn chế tồn đọng:
- **Giới hạn kích thước tối đa 2048 bit:**
  - Cấu trúc tĩnh cố định `BIG_LIMBS = 32` (2048 bit). Mặc dù đáp ứng hoàn hảo cho đề tài (yêu cầu $\ge 512$ bit, an toàn thực tế 1024-2048 bit), thư viện chưa hỗ trợ số lớn động không giới hạn (Arbitrary-precision BigInt).
- **Chưa tối ưu phép nhân Karatsuba / Toom-Cook / FFT:**
  - Phép nhân hiện tại có độ phức tạp trường học $O(n^2)$. Với kích thước 512-1024 bit (8 đến 16 từ nhớ 64-bit), thuật toán trường học thực tế vẫn chạy rất nhanh nhờ ít overhead; tuy nhiên nếu mở rộng lên 4096 hay 8192 bit, Karatsuba ($O(n^{1{,}58})$) sẽ mang lại lợi thế vượt trội.
- **Chưa triển khai biểu diễn Montgomery (Montgomery Representation):**
  - Trong phép tính lũy thừa modulo $a^e \bmod m$, mỗi phép nhân hiện tại vẫn phải gọi một phép chia lấy dư `%` bằng thuật toán Knuth D ($O(n^2)$). Cài đặt phép nhân Montgomery sẽ cho phép chuyển đổi sang miền Montgomery và thực hiện rút gọn modulo (Montgomery Reduction) chỉ bằng các phép nhân và dịch bit, không cần phép chia nào.

### 3. Hướng cải thiện trong tương lai:
- **Tích hợp phép nhân Montgomery:**
  - Chuyển đổi $x \mapsto \tilde{x} = x \cdot R \bmod m$ với $R = 2^{64 \cdot n}$, áp dụng thuật toán CIOS (Coarsely Integrated Operand Scanning) để tối ưu hóa triệt để hàm `modular_exponentiation`.
- **Tận dụng chỉ thị phần cứng SIMD / Crypto Extensions:**
  - Sử dụng các lệnh mở rộng x86_64 hiện đại như `MULX` (nhân không ảnh hưởng cờ), `ADCX` / `ADOX` (cộng đồng thời hai chuỗi nhớ trên hai cờ CF và OF riêng biệt của Intel Broadwell trở lên) để nhân đôi băng thông tính toán số học lớn.

---

## 6.3. Đánh giá phần Giao thức Trao đổi khóa Diffie-Hellman & Lũy thừa Modulo
**Người thực hiện:** Tạ Quang Mạnh (MSSV: 24120089) & Vương Hữu Khang (MSSV: 24120070)

### 1. Ưu điểm đã đạt được:
- **Tối ưu hóa Lũy thừa modulo bằng Square-and-Multiply:**
  - Triển khai thuật toán bình phương và nhân liên tiếp (Left-to-Right Binary Exponentiation), giảm số lượng phép nhân modulo từ $O(e)$ xuống còn $O(\log e)$ (tối đa $2 \log_2 e$ phép nhân, trung bình $1{,}5 \log_2 e$).
  - Đối chiếu độ chính xác với hàm chuẩn `pow(base, exp, mod)` của Python qua 4.452 trường hợp thử nghiệm: kết quả khớp tuyệt đối $100\%$.
- **Lựa chọn phần tử sinh an toàn trên nhóm con cấp nguyên tố:**
  - Không chọn $g = 2$ tùy tiện khi chưa chứng minh bậc; thay vào đó, chương trình chọn $g = 4 = 2^2 \bmod p$, là phần tử sinh chắc chắn của nhóm con thặng dư bậc hai $Q_p$ có cấp nguyên tố $q = (p - 1)/2$.
  - Mọi phần tử trong nhóm con đều có cấp lớn $q$, triệt tiêu hoàn toàn khả năng rò rỉ parity của khóa riêng $a$ và làm vô hiệu hóa tấn công Pohlig-Hellman.
- **Xác thực khóa công khai chặt chẽ (Public Key Validation):**
  - Hàm `is_valid_public_value(v, p)` kiểm tra đầy đủ cả 2 điều kiện:
    1. $2 \le v \le p - 2$.
    2. $v^q \equiv 1 \pmod p$ (xác nhận $v$ nằm đúng trong nhóm con cấp $q$, không bị ép vào nhóm con nhỏ cấp 1 hoặc 2 do kẻ tấn công Man-in-the-Middle gửi đến).

### 2. Hạn chế tồn đọng:
- **Nhạy cảm với tấn công kênh kề thời gian (Timing Analysis / SPA):**
  - Trong vòng lặp Square-and-Multiply, bước nhân `res = (res * base) % mod` chỉ được thực thi khi bit của số mũ bằng 1. Sự chênh lệch thời gian giữa các bit 0 (chỉ bình phương) và bit 1 (bình phương + nhân) có thể bị kẻ tấn công đo đạc bằng dao động ký hoặc bộ đếm chu kỳ để khôi phục từng bit khóa riêng.
- **Chưa có cơ chế xác thực danh tính hai đầu (Unauthenticated DH):**
  - Giao thức Diffie-Hellman cổ điển nguyên bản không có chữ ký số xác thực danh tính của Alice và Bob, do đó dễ bị tấn công xen giữa (Man-in-the-Middle) nếu kênh truyền bị kiểm soát hoàn toàn bởi kẻ tấn công. Trong thực tế, DH luôn phải kết hợp với chứng chỉ số X.509 và chữ ký số RSA/ECDSA (như trong TLS 1.3).

### 3. Hướng cải thiện trong tương lai:
- **Triển khai thuật toán Montgomery Ladder (Constant-time Exponentiation):**
  - Cài đặt Montgomery Ladder đảm bảo với mọi bit của số mũ (dù là 0 hay 1), chương trình đều thực hiện đúng 1 phép bình phương và 1 phép nhân. Kết hợp kỹ thuật thanh ghi tạm (dummy variable) để loại bỏ hoàn toàn dấu vết thời gian và năng lượng tiêu thụ (chống SPA/DPA).
- **Mở rộng sang Elliptic Curve Diffie-Hellman (ECDH):**
  - Triển khai trao đổi khóa trên đường cong Elliptic (ví dụ Curve25519 theo chuẩn RFC 7748). ECDH mang lại độ an toàn tương đương (128-bit security) nhưng chỉ cần khóa 256-bit (thay vì 2048-bit hoặc 3072-bit trong DH cổ điển), giúp tốc độ tính toán nhanh hơn hàng chục lần và giảm kích thước gói tin trao đổi qua mạng.

---

## 6.4. Bảng tự đánh giá mức độ hoàn thành & Đóng góp của từng thành viên

Cả 3 thành viên trong nhóm đều thể hiện tinh thần trách nhiệm cao, chủ động nghiên cứu cơ sở lý thuyết, đóng góp mã nguồn chất lượng, kiểm thử chéo nghiêm ngặt và phối hợp viết báo cáo đầy đủ, hoàn thành trước thời hạn quy định.

| STT | Họ và tên | MSSV | Nhiệm vụ chính được phân công | Mức độ hoàn thành | Tỷ lệ đóng góp |
| :---: | :--- | :---: | :--- | :---: | :---: |
| 1 | **Vương Hữu Khang** | 24120070 | Kiến trúc tổng thể; Cài đặt BigInt cơ sở (`+`, `-`, `*`, chuyển đổi hex/dec); Cài đặt PRNG Keccak/SHAKE128; Viết Test suite, Pseudocode modular arithmetic (`AddMod`, `MultMod`, `ExpMod`); Soạn thảo Mục 1 (chính), Mục 5, tổng hợp Mục 6 và xuất báo cáo PDF. | **100%** | **33.3%** |
| 2 | **Tạ Quang Mạnh** | 24120089 | Cài đặt phép chia số lớn Knuth Algorithm D (`divmod`, `/`, `%`); Cài đặt Diffie-Hellman (`ExpMod`, `choose_generator`, `is_valid_public_value`); Viết Test suite phép chia, stress test đối chiếu Python; Soạn thảo Mục 1.5, 1.6 và Mục 4 (Ứng dụng thực tế của DH trong TLS 1.3). | **100%** | **33.3%** |
| 3 | **Lâm Duy Minh** | 24120091 | Port PRNG entropy sang Windows (`rand_s`); Cài đặt Rejection Sampling (`random_range`); Cài đặt Miller-Rabin (`uint64_t` và `big_t`); Cài đặt Sàng đồng thời 302 số nguyên tố nhỏ & Sinh Safe Prime 4 tầng; Viết Benchmark thực nghiệm; Soạn thảo Mục 2, Mục 3 (điểm cộng) và Mục 6.1. | **100%** | **33.3%** |
