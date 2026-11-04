# Thư mục Kiểm thử (Tests & Benchmarks)

Phụ trách chính: **Tạ Quang Mạnh (24120089)**. Phần kiểm thử `BigInt` cơ bản do **Khang (24120070)** viết sẵn.

> Thư mục này chỉ phục vụ kiểm thử nội bộ trên GitHub, **không đưa vào file zip khi nộp bài**.

## Các file hiện có

| File | Mô tả | Trạng thái |
|---|---|:---:|
| `test_bigint.cpp` | Unit test `BigInt`: chuyển đổi hex/dec, so sánh, `+ - *`, dịch bit, `mod_small`, chia/mod | ✅ |
| `crosscheck_bigint.cpp` | Chương trình C++ đọc lệnh từ stdin (`add/sub/mul/div/mod/shl/shr/dec/bl/msm`) và in kết quả; cuối chương trình đo tốc độ `mulmod` 512-bit | ✅ |
| `crosscheck_bigint.py` | Script sinh 30.000 phép tính ngẫu nhiên, chạy `crosscheck_bigint` rồi đối chiếu với số nguyên Python. Python **chỉ làm đáp án chuẩn**, không thuộc chương trình nộp | ✅ |

## Cách chạy

Chạy từ thư mục `tests/` (cần g++ hỗ trợ C++14; Windows dùng MSYS2 UCRT64):

```bash
# 1) Unit test BigInt
g++ -std=c++14 -O2 -Wall -Wextra -o test_bigint test_bigint.cpp
./test_bigint

# 2) Đối chiếu với Python (cần Python 3)
g++ -std=c++14 -O2 -o crosscheck_bigint crosscheck_bigint.cpp
python crosscheck_bigint.py ./crosscheck_bigint     # kỳ vọng: "mismatch 0"
```

Sau mỗi lần sửa `bigint.cpp` (đặc biệt khi tối ưu `divmod` bằng Knuth D), **phải chạy lại cả hai** và giữ kết quả `mismatch 0`.

## Còn phải làm (Mạnh)

1. **Unit test `modular_exponentiation`**: so sánh với `pow(b, e, m)` của Python (mở rộng `crosscheck_bigint` thêm lệnh `powmod`).
2. **Unit test Miller-Rabin**: số nguyên tố nhỏ đã biết, số Carmichael (561, 41041, 825265), safe prime chuẩn RFC 3526.
3. **Unit test Diffie-Hellman**: chạy nhiều lần, với cả seed ngẫu nhiên và seed cố định, đảm bảo `s_alice == s_bob` luôn đúng.
4. **Benchmark** sinh safe prime ở 512 / 768 / 1024 bit (phối hợp với Minh).
