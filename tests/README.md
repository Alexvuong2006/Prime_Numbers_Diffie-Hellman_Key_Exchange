# Thư mục Kiểm thử (Tests & Benchmarks)

Phụ trách chính: **Tạ Quang Mạnh (24120089)**

Thư mục này chứa các file unit test C++ và script kiểm thử tự động (ví dụ Python đối chiếu với `pow(b, e, m)` hoặc thư viện kiểm chứng).

## Kế hoạch kiểm thử:
1. **Unit test BigInt**: Kiểm tra cộng, trừ, nhân, dịch bit, chuyển đổi chuỗi đối chiếu với kết quả chuẩn.
2. **Unit test Chia / Mod / ModExp**: So sánh với `divmod` và `pow` trong Python.
3. **Unit test Miller-Rabin**: Kiểm tra với các số nguyên tố nhỏ đã biết, số Carmichael (561, 41041, 825265), và các số nguyên tố chuẩn RFC 3526.
4. **Unit test Diffie-Hellman**: Chạy nhiều chu kỳ trao đổi khóa liên tiếp với seed ngẫu nhiên, đảm bảo `s_alice == s_bob` luôn đạt 100%.

*Lưu ý: Thư mục này chỉ phục vụ kiểm thử nội bộ trên GitHub, không đưa vào file zip khi nộp bài.*
