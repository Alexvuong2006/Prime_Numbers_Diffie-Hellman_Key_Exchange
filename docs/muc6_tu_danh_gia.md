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
  - Thời gian sinh safe prime 512 bit chỉ mất trung bình **~0,26 giây** và 1024 bit chỉ mất **~1,58 giây**, vượt xa kỳ vọng ban đầu của nhóm.

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

