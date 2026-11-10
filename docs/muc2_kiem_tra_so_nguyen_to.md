# BÁO CÁO MỤC 2: CÁC THUẬT TOÁN KIỂM TRA SỐ NGUYÊN TỐ & SINH SỐ NGUYÊN TỐ AN TOÀN

**Phụ trách chính:** Lâm Duy Minh (MSSV: 24120091)

---

## 2.1. Tổng quan về bài toán kiểm tra tính nguyên tố (Primality Testing)

Trong mật mã học khóa công khai (RSA, Diffie-Hellman, ElGamal, DSA), việc kiểm tra một số nguyên lẻ $n$ có kích thước lớn ($512 \sim 4096\text{ bit}$) có phải là số nguyên tố hay không là tác vụ nền tảng. Khác với bài toán phân tích ra thừa số nguyên tố (Integer Factorization) vốn chưa có thuật toán thời gian đa thức trên máy tính cổ điển, bài toán kiểm tra tính nguyên tố có thể giải quyết rất hiệu quả trong thời gian đa thức theo số bit $k = \lfloor \log_2 n \rfloor + 1$.

Các thuật toán kiểm tra số nguyên tố được chia thành hai nhóm chính:
- **Thuật toán tất định (Deterministic):** Luôn trả về kết luận đúng tuyệt đối 100% (ví dụ: Chia thử, AKS). Tuy nhiên, với số lớn hàng trăm bit, các thuật toán tất định hoặc có độ phức tạp mũ (chia thử), hoặc có hằng số ẩn rất lớn (AKS), không khả thi khi phải kiểm tra hàng nghìn ứng viên liên tiếp.
- **Thuật toán xác suất (Probabilistic / Monte Carlo):** Sử dụng các tính chất số học trong nhóm nhân $\mathbb{Z}_n^*$ với cơ sở ngẫu nhiên $a$ (ví dụ: Fermat, Solovay-Strassen, **Miller-Rabin**). Nếu thuật toán kết luận *"Hợp số"*, kết luận đó đúng 100%; nếu kết luận *"Có khả năng nguyên tố (Probable Prime)"* sau $k$ vòng độc lập, xác suất sai sót nhỏ hơn một cận giảm theo hàm mũ (như $4^{-k}$), đủ nhỏ hơn cả xác suất lỗi phần cứng do tia vũ trụ.

---

## 2.2. Phân tích và So sánh các thuật toán kiểm tra số nguyên tố

### 2.2.1. Phương pháp Chia thử (Trial Division)
- **Nguyên lý:** Một hợp số $n > 1$ luôn có ít nhất một ước nguyên tố $p \le \sqrt{n}$. Thuật toán lần lượt kiểm tra $n \bmod d$ với mọi số nguyên tố $d \in [2, \lfloor\sqrt{n}\rfloor]$.
- **Độ phức tạp:** $O(\sqrt{n}) = O(2^{k/2})$ phép chia, với $k = \log_2 n$ là độ dài bit của $n$. Đây là độ phức tạp **hàm mũ** theo kích thước đầu vào.
- **Hạn chế:** Với số $n$ dài 512 bit, $\sqrt{n} \approx 2^{256} \approx 1{,}15 \times 10^{77}$ — bất khả thi về mặt tính toán.
- **Ứng dụng thực tế trong đồ án:** Mặc dù không thể dùng để chứng minh $n$ nguyên tố, chia thử với một tập nhỏ các số nguyên tố đầu tiên ($r < 2000$) lại là **bộ lọc sơ bộ (Sieve)** cực kỳ hiệu quả, giúp loại bỏ nhanh **$> 98{,}5\%$** ứng viên hợp số trước khi chạy thuật toán tốn kém hơn.

### 2.2.2. Kiểm tra Fermat (Fermat Primality Test) & Số Carmichael
- **Cơ sở lý thuyết (Định lý Fermat nhỏ):** Nếu $n$ là số nguyên tố thì với mọi $a \in [2, n - 2]$:
  $$a^{n-1} \equiv 1 \pmod n$$
- **Thuật toán:** Chọn ngẫu nhiên $k$ cơ sở $a \in [2, n - 2]$. Nếu tồn tại $a$ sao cho $a^{n-1} \not\equiv 1 \pmod n$, kết luận chắc chắn $n$ là hợp số ($a$ được gọi là *nhân chứng Fermat*).
- **Điểm yếu chí mạng – Số Carmichael (Carmichael Numbers):**
  Tồn tại vô hạn các hợp số $n$ (được chứng minh bởi Alford, Granville và Pomerance năm 1994) thỏa mãn $a^{n-1} \equiv 1 \pmod n$ với **mọi** số nguyên $a$ nguyên tố cùng nhau với $n$ ($\gcd(a, n) = 1$).
  - Định lý Korselt (1899): Hợp số lẻ $n$ là số Carmichael khi và chỉ khi $n$ không có ước chính phương (square-free) và với mọi ước nguyên tố $p \mid n$, ta có $(p - 1) \mid (n - 1)$.
  - Các số Carmichael đầu tiên:
    $$561 = 3 \times 11 \times 17, \quad 1105 = 5 \times 13 \times 17, \quad 1729 = 7 \times 13 \times 19, \quad 41041 = 7 \times 11 \times 13 \times 41$$
  - Trên các số Carmichael này, kiểm tra Fermat chỉ phát hiện được hợp số nếu tình cờ chọn trúng $a$ có $\gcd(a, n) > 1$ — xác suất này gần như bằng $0$ khi $n$ là tích của các số nguyên tố lớn. Do đó, kiểm tra Fermat **không an toàn** khi đứng độc lập.

### 2.2.3. Kiểm tra Solovay-Strassen (1977)
- **Cơ sở lý thuyết (Tiêu chuẩn Euler):** Nếu $n$ là số nguyên tố lẻ thì với mọi $a$ không chia hết cho $n$:
  $$a^{(n-1)/2} \equiv \left(\frac{a}{n}\right) \pmod n$$
  trong đó $\left(\frac{a}{n}\right) \in \{-1, 0, 1\}$ là ký hiệu Jacobi (mở rộng của ký hiệu Legendre).
- **Ưu điểm & Hạn chế:** Không có tương tự của số Carmichael: với mọi hợp số lẻ $n$, tối đa $\frac{1}{2}$ số cơ sở $a \in \mathbb{Z}_n^*$ là nhân chứng giả Euler, nên xác suất sai sau $k$ vòng là $\le 2^{-k}$. Tuy nhiên, việc tính ký hiệu Jacobi và cận sai số $2^{-k}$ đều kém hiệu quả hơn thuật toán Miller-Rabin ($4^{-k}$), nên trong thực tế Solovay-Strassen đã được thay thế hoàn toàn bởi Miller-Rabin.

### 2.2.4. Kiểm tra tất định AKS (Agrawal–Kayal–Saxena, 2002)
- **Cơ sở lý thuyết:** Dựa trên mở rộng đa thức của định lý Fermat nhỏ: $n \ge 2$ là số nguyên tố khi và chỉ khi đồng dư thức đa thức:
  $$(X + a)^n \equiv X^n + a \pmod{X^r - 1, n}$$
  đúng trong vành $\mathbb{Z}_n[X]/(X^r - 1)$ với một bậc $r = O(\log^5 n)$ phù hợp và mọi $1 \le a \le O(\sqrt{\phi(r)} \log n)$.
- **Ý nghĩa lý thuyết & Thực tiễn:** AKS là thuật toán đầu tiên trong lịch sử chứng minh bài toán `PRIMES` thuộc lớp **P** (đa thức tất định không cần giả thuyết nào), với độ phức tạp cải tiến đạt $\tilde{O}(\log^6 n)$. Tuy nhiên, do phải nhân đa thức bậc hàng triệu trên hệ số lớn, AKS chậm hơn hàng triệu lần so với Miller-Rabin ($O(k \log^3 n)$) ở kích thước 512–2048 bit nên chỉ mang ý nghĩa lý thuyết.

### 2.2.5. Thuật toán Miller-Rabin (Miller 1976, Rabin 1980) — Lựa chọn của nhóm
- **Cơ sở lý thuyết:** Khai thác đồng thời hai tính chất của trường hữu hạn $\mathbb{Z}_p$ khi $p$ là số nguyên tố lẻ:
  1. Định lý Fermat nhỏ: $a^{p-1} \equiv 1 \pmod p$ với mọi $1 \le a \le p - 1$.
  2. Trong trường $\mathbb{Z}_p$, phương trình $x^2 \equiv 1 \pmod p \iff (x - 1)(x + 1) \equiv 0 \pmod p$ chỉ có **đúng hai nghiệm tầm thường** là $x \equiv 1$ và $x \equiv -1 \equiv p - 1 \pmod p$. Nếu trong quá trình bình phương liên tiếp xuất hiện $x \not\equiv \pm 1 \pmod n$ mà $x^2 \equiv 1 \pmod n$, thì $n$ **chắc chắn là hợp số** (vì $\gcd(x - 1, n)$ và $\gcd(x + 1, n)$ là các ước thực sự của $n$).
- **Mô tả thuật toán:**
  Với số lẻ $n > 3$, phân tích $n - 1 = 2^s \cdot d$ trong đó $d$ là số lẻ và $s \ge 1$ (bằng cách đếm số bit `0` tận cùng của $n - 1$).
  Trong mỗi vòng kiểm tra (lặp $k$ vòng độc lập):
  1. Chọn ngẫu nhiên đều cơ sở $a \in [2, n - 2]$.
  2. Tính $x \leftarrow a^d \bmod n$.
  3. Nếu $x = 1$ hoặc $x = n - 1$, vòng kiểm tra hiện tại đạt (`continue` sang vòng tiếp theo).
  4. Lặp $r$ từ $1$ đến $s - 1$:
     - $x \leftarrow x^2 \bmod n$.
     - Nếu $x = n - 1$: vòng kiểm tra hiện tại đạt (`break` khỏi vòng trong).
     - Nếu $x = 1$: trả về `false` (**Hợp số** — phát hiện căn bậc hai không tầm thường của $1$).
  5. Nếu kết thúc vòng lặp trong mà không gặp $x = n - 1$: trả về `false` (**Hợp số** — vi phạm định lý Fermat $a^{n-1} \not\equiv 1 \pmod n$).
- **Định lý cận sai số Monier–Rabin (1980):**
  Với **mọi** hợp số lẻ $n > 9$ (kể cả mọi số Carmichael), số lượng "nhân chứng giả mạnh" (strong liars) trong đoạn $[1, n - 1]$ không bao giờ vượt quá $\frac{\phi(n)}{4} < \frac{n - 1}{4}$.
  Do đó, nếu chọn độc lập $k$ cơ sở ngẫu nhiên đều:
  $$\Pr[\text{Hợp số } n \text{ vượt qua } k \text{ vòng Miller-Rabin}] \le \left(\frac{1}{4}\right)^k = 4^{-k} = 2^{-2k}$$
  Với cấu hình mặc định của nhóm là **$k = 40$ vòng**, xác suất một hợp số lọt lưới không quá $4^{-40} = 2^{-80} \approx 8{,}27 \times 10^{-25}$ (còn trong trường hợp trung bình với số ngẫu nhiên 512 bit theo định lý Damgård–Landrock–Pomerance, xác suất sai thực tế nhỏ hơn $2^{-160}$).

### Bảng tổng hợp so sánh các thuật toán kiểm tra nguyên tố

| Thuật toán | Phân loại | Độ phức tạp ($b = \log_2 n$) | Xác suất sai sau $k$ vòng | Bị đánh lừa bởi số Carmichael? | Tính khả thi ở 512–2048 bit |
|---|---|:---:|:---:|:---:|:---:|
| **Chia thử (Trial Division)** | Tất định | $O(2^{b/2})$ | $0$ | Không | ❌ Chỉ dùng sàng ước nhỏ |
| **Fermat** | Xác suất | $O(k \cdot b^3)$ | Không chặn được trên số Carmichael | **Có** (561, 1105, 1729...) | ⚠️ Không an toàn độc lập |
| **Solovay-Strassen** | Xác suất | $O(k \cdot b^3)$ | $\le 2^{-k}$ | Không | Chậm hơn Miller-Rabin |
| **Miller-Rabin** | Xác suất | $O(k \cdot b^3)$ | **$\le 4^{-k}$** | **Không** | ✅ **Chuẩn công nghiệp** |
| **AKS** | Tất định | $\tilde{O}(b^6)$ | $0$ | Không | ❌ Quá chậm trong thực tế |

---

## 2.3. Số nguyên tố an toàn (Safe Prime) & Vì sao bắt buộc dùng trong Diffie-Hellman

### 2.3.1. Định nghĩa Safe Prime và Sophie Germain Prime
- Một số nguyên tố $q$ được gọi là **số nguyên tố Sophie Germain** nếu $p = 2q + 1$ cũng là số nguyên tố.
- Khi đó, $p = 2q + 1$ được gọi là **số nguyên tố an toàn (Safe Prime)**.

### 2.3.2. Lý do mật mã học: Chống tấn công Pohlig-Hellman và Tấn công nhóm con nhỏ
Độ an toàn của giao thức Diffie-Hellman trên nhóm nhân $\mathbb{Z}_p^*$ phụ thuộc vào độ khó của bài toán Logarit rời rạc (DLP): tìm $a$ khi biết $g, p$ và $A = g^a \bmod p$.
- **Nếu chọn số nguyên tố $p$ ngẫu nhiên thông thường:** Cấp của nhóm tuần hoàn $|\mathbb{Z}_p^*| = p - 1$ là một số chẵn ngẫu nhiên và hầu như luôn có nhiều ước nguyên tố nhỏ hoặc trung bình:
  $$p - 1 = \prod_{i=1}^{m} q_i^{e_i}$$
  Khi đó, kẻ tấn công có thể áp dụng **thuật toán Pohlig-Hellman (1978)** kết hợp **tấn công giam cầm nhóm con nhỏ (Small-Subgroup Confinement Attack - Lim & Lee, 1997)**:
  1. Kẻ tấn công chiếu bài toán DLP từ nhóm lớn cấp $p - 1$ xuống từng nhóm con cấp $q_i^{e_i}$ (hoặc chủ động gửi khóa công khai giả $B'$ có bậc $q_i$ nhỏ).
  2. Giải tìm $a \bmod q_i^{e_i}$ trong thời gian chỉ $O\left(\sum e_i (\log p + \sqrt{q_i})\right)$ bằng thuật toán Baby-step Giant-step / Pollard's rho.
  3. Ghép các phần dư bằng Định lý Thặng dư Trung Hoa (CRT) để thu hồi nhiều bit (hoặc toàn bộ) khóa riêng $a$ của nạn nhân!
- **Khi sử dụng Safe Prime $p = 2q + 1$ (với $q$ nguyên tố lớn $(b-1)$ bit):**
  - Phân tích thừa số nguyên tố của cấp nhóm là tối giản tuyệt đối:
    $$|\mathbb{Z}_p^*| = p - 1 = 2q$$
  - Theo định lý Lagrange, mọi phần tử $x \in \mathbb{Z}_p^*$ chỉ có thể có bậc thuộc tập $\{1, 2, q, 2q\}$.
  - Nhóm chọn phần tử sinh $g = h^2 \bmod p \neq 1$ (cụ thể $g = 4$), là phần tử sinh của **nhóm con thặng dư bậc hai $Q_p$ có cấp nguyên tố $q$**. Mọi phần tử khác đơn vị trong nhóm con này đều có bậc đúng bằng số nguyên tố $(b-1)$-bit $q$, làm vô hiệu hóa hoàn toàn thuật toán Pohlig-Hellman và chống rò rỉ bit chẵn/lẻ của khóa riêng qua ký hiệu Legendre.

---

## 2.4. Thiết kế và Tối ưu hóa cài đặt trong `project_01_source/prime.cpp`

Theo Định lý Số nguyên tố (Prime Number Theorem), mật độ số nguyên tố xung quanh $2^b$ xấp xỉ $\frac{1}{\ln(2^b)} = \frac{1}{b \ln 2}$. Để cả $q$ và $p = 2q + 1$ đồng thời là số nguyên tố (cặp nguyên tố Cunningham độ dài 2), xác suất một số lẻ ngẫu nhiên $q$ tạo thành safe prime chỉ vào khoảng:
$$P(\text{safe prime}) \approx \frac{2 \cdot C_2}{(\ln q)^2}$$
Với $b = 512\text{ bit}$, trung bình phải thử khoảng **$30.000 \sim 60.000$ ứng viên lẻ $q$** mới tìm được một safe prime! Nếu chạy trực tiếp Miller-Rabin trên từng ứng viên, chương trình sẽ mất hàng chục giây đến hàng phút.

Để giải quyết triệt để bài toán hiệu năng, nhóm đã thiết kế **Kiến trúc lọc 4 tầng (4-Stage Filtering Pipeline)** trong hàm `generate_safe_prime`:

### Tầng 1: Sinh ứng viên chuẩn hóa bit (`random_bits`)
- Sinh $q$ có đúng `bitsize - 1` bit từ CSPRNG SHAKE128: bật bit cao nhất tại vị trí `bitsize - 2` và bật bit thấp nhất tại vị trí `0` ($q$ lẻ).
- Nhờ $2^{\text{bitsize}-2} + 1 \le q \le 2^{\text{bitsize}-1} - 1$, số $p = 2q + 1 = (q \ll 1) + 1$ luôn thỏa mãn:
  $$2^{\text{bitsize}-1} + 3 \le p \le 2^{\text{bitsize}} - 1$$
  Tức là $p$ **luôn luôn có độ dài chính xác bằng `bitsize` bit** và $p \equiv 3 \pmod 4$.

### Tầng 2: Sàng đồng thời (Simultaneous Small-Prime Sieve) trên cả $q$ và $p = 2q + 1$
- Nhóm lập bảng tĩnh `SMALL_PRIMES` gồm toàn bộ **302 số nguyên tố lẻ nhỏ hơn 2000** (từ $3, 5, 7, \dots$ đến $1999$).
- Thay vì phải tính modulo trên cả $q$ và $p$, nhóm khai thác đồng dư thức đại số:
  $$p = 2q + 1 \equiv 0 \pmod r \iff 2q \equiv -1 \equiv r - 1 \pmod r \iff q \equiv \frac{r - 1}{2} \pmod r$$
  (do mọi $r \in \text{SMALL\_PRIMES}$ đều là số lẻ nên $\frac{r-1}{2}$ là số nguyên).
- Như vậy, với mỗi số nguyên tố nhỏ $r$, hàm `passes_safe_prime_sieve` chỉ cần gọi **1 lần duy nhất** hàm chia nhanh `rem = mod_small(q, r)` (độ phức tạp $O(N)$ dùng sơ đồ Horner 128-bit):
  - Nếu `rem == 0`: $r \mid q \implies q$ là hợp số $\rightarrow$ **loại ngay**.
  - Nếu `rem == (r - 1) / 2`: $r \mid (2q + 1) \implies p$ là hợp số $\rightarrow$ **loại ngay**.
- **Hiệu quả toán học:** Với mỗi số nguyên tố lẻ $r$, ta loại được $\frac{2}{r}$ không gian ứng viên (cụ thể riêng với $r = 3$, mọi $q$ nguyên tố $> 3$ có $2q + 1$ nguyên tố đều buộc phải thỏa $q \equiv 2 \pmod 3$, tức chỉ mình $r = 3$ đã loại ngay $66{,}7\%$ số ứng viên ở phép chia đầu tiên!). Tổng tỷ lệ ứng viên bị loại bởi bảng 302 số nguyên tố nhỏ là:
  $$1 - \prod_{3 \le r < 2000} \left(1 - \frac{2}{r}\right) \approx \mathbf{98{,}56\%}$$
  Chỉ còn khoảng **$1{,}44\%$** ứng viên vượt qua sàng để bước vào Tầng 3.

### Tầng 3: Kiểm tra nhanh 1 vòng Miller-Rabin cơ sở 2 (`miller_rabin_base2`) tuần tự trên $q$ rồi trên $p$
- Với ứng viên $q$ vượt qua sàng, nhóm chưa chạy ngay 40 vòng Miller-Rabin (vì xác suất $q$ là hợp số vẫn khoảng $96\%$, và ngay cả khi $q$ thực sự là số nguyên tố thì xác suất $p = 2q + 1$ là hợp số vẫn lên tới $\sim 96\%$).
- Nhóm thực hiện:
  1. Chạy **1 vòng Miller-Rabin cơ sở cố định $a = 2$** trên $q$: `miller_rabin_base2(q)`. Phép kiểm tra này chỉ tốn đúng 1 lần `modular_exponentiation` nhưng loại bỏ $> 99{,}99\%$ hợp số $q$ còn sót lại sau sàng.
  2. Ngay khi $q$ vượt qua cơ sở 2, tính $p = (q \ll 1) + 1$ và chạy **1 vòng Miller-Rabin cơ sở $a = 2$** trên $p$: `miller_rabin_base2(p)`.
- **Ý nghĩa tối ưu đột phá:** Việc kiểm tra `miller_rabin_base2(p)` **trước** khi chạy 40 vòng trên $q$ giúp tiết kiệm $39$ phép lũy thừa mô-đun cho mỗi số nguyên tố $q$ mà $2q + 1$ lại là hợp số (xảy ra trung bình $\sim 30$ lần trước khi gặp safe prime thực sự), giúp tốc độ tổng thể tăng thêm **hơn 10 lần**!

### Tầng 4: Xác nhận nghiêm ngặt bằng `rounds` vòng Miller-Rabin cơ sở ngẫu nhiên
- Chỉ khi cả $q$ và $p = 2q + 1$ đều vượt qua cơ sở 2 (hầu như chắc chắn 100% đã là safe prime thực sự), chương trình mới gọi `is_probable_prime(ctx, q, rounds)` và `is_probable_prime(ctx, p, rounds)` với $40$ cơ sở ngẫu nhiên độc lập trong $[2, n - 2]$ để đảm bảo độ tin cậy mật mã học $\ge 1 - 4^{-40}$.

---

## 2.5. Kết quả thực nghiệm & Benchmark (512 / 768 / 1024 bit)

Nhóm đã xây dựng bộ công cụ đo đạc tự động tại `tests/bench_safe_prime.cpp` (biên dịch với `g++ -std=c++14 -O2`, số vòng Miller-Rabin $k = 40$) để thống kê chi tiết quá trình sinh Safe Prime ở ba mức kích thước **512 bit**, **768 bit** và **1024 bit**:

### Bảng thống kê kết quả Benchmark trung bình

| Kích thước Safe Prime $p$ | Số lần chạy thử | Số ứng viên $q$ đã sinh (TB) | Số cặp $(q, p)$ qua sàng $< 2000$ (TB) | Tỷ lệ lọc của sàng đồng thời | Số $q$ nguyên tố qua MR cơ sở 2 (TB) | Thời gian sinh trung bình |
|:---:|:---:|:---:|:---:|:---:|:---:|:---:|
| **512 bit** | 10 lần | $58.425{,}9$ | $840{,}9$ | **$98{,}56\%$** | $34{,}3$ | **$0{,}2591\text{ giây}$** ($259{,}1\text{ ms}$) |
| **768 bit** | 5 lần | $140.370{,}6$ | $1.998{,}8$ | **$98{,}58\%$** | $47{,}8$ | **$1{,}3220\text{ giây}$** ($1.322{,}0\text{ ms}$) |
| **1024 bit** | 3 lần | $83.093{,}7$ | $1.202{,}0$ | **$98{,}55\%$** | $20{,}3$ | **$1{,}5797\text{ giây}$** ($1.579{,}7\text{ ms}$) |

### Nhận xét từ số liệu thực nghiệm:
1. **Độ khớp hoàn hảo với lý thuyết số:** Tỷ lệ lọc thực nghiệm của bước sàng đồng thời đạt **$98{,}55\% \sim 98{,}58\%$**, trùng khớp chính xác tới chữ số thập phân thứ hai với công thức lý thuyết Mertens $1 - \prod_{3 \le r < 2000}(1 - 2/r) \approx 98{,}56\%$.
2. **Tốc độ vượt trội:** Nhờ sự kết hợp giữa thuật toán chia Knuth D 64-bit (`bigint.cpp` / `dh.cpp`) và kiến trúc lọc 4 tầng (`prime.cpp`), việc sinh một số nguyên tố an toàn 512 bit chỉ mất trung bình **~0,26 giây** (nhiều lần chạy chỉ mất **0,07 – 0,12 giây**), và ngay cả ở mức **1024 bit** cũng chỉ mất trung bình **~1,58 giây**.

---

## 2.6. Tài liệu tham khảo
1. G. L. Miller, *"Riemann's Hypothesis and Tests for Primality"*, Journal of Computer and System Sciences, 13(3): 300–317, 1976.
2. M. O. Rabin, *"Probabilistic algorithm for testing primality"*, Journal of Number Theory, 12(1): 128–138, 1980.
3. M. Agrawal, N. Kayal, N. Saxena, *"PRIMES is in P"*, Annals of Mathematics, 160(2): 781–793, 2004.
4. S. C. Pohlig, M. E. Hellman, *"An Improved Algorithm for Computing Logarithms over GF(p) and Its Cryptographic Significance"*, IEEE Trans. Information Theory, 24(1): 106–110, 1978.
5. A. Menezes, P. van Oorschot, S. Vanstone, *Handbook of Applied Cryptography*, CRC Press, Chapter 4 (Public-Key Parameters & Prime Generation).

