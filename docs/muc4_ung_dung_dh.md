# BÁO CÁO MỤC 4: ỨNG DỤNG THỰC TẾ CỦA TRAO ĐỔI KHÓA DIFFIE-HELLMAN

**Phụ trách:** Tạ Quang Mạnh (MSSV: 24120089)

---

## 4.1. Tóm tắt giao thức
Alice và Bob thống nhất công khai số nguyên tố an toàn $p = 2q + 1$ và phần tử sinh $g$ (nhóm con bậc $q$). Alice chọn bí mật $a$, gửi $A = g^a \bmod p$; Bob chọn $b$, gửi $B = g^b \bmod p$. Cả hai tính $s = B^a = A^b = g^{ab} \bmod p$. Kẻ nghe lén thấy $p, g, A, B$ nhưng muốn tìm $s$ phải giải bài toán logarit rời rạc (hoặc Diffie-Hellman tính toán), được coi là khó với $p$ đủ lớn. Giao thức chỉ cho phép **thỏa thuận** một bí mật chung, không mã hóa dữ liệu; $s$ được đưa qua hàm dẫn xuất khóa (KDF) để tạo khóa đối xứng.

## 4.2. Ứng dụng chính: TLS 1.3 (HTTPS)
TLS 1.3 (RFC 8446) là giao thức bảo vệ HTTPS. Trong TLS 1.3 **mọi bắt tay đều dùng (EC)DHE** (Diffie-Hellman *ephemeral*): hai bên sinh cặp khóa mới cho mỗi phiên rồi hủy.

### Luồng bắt tay đơn giản hóa
```
 Client (trình duyệt)                                   Server (website)
 --------------------                                   ----------------
 sinh a, A = g^a                      ClientHello
                                      + key_share(A) + danh sách nhóm hỗ trợ
                                  ------------------------------------->
                                                        sinh b, B = g^b
                                                        s = A^b
                                      ServerHello + key_share(B)
                                      {Certificate}          (chứng chỉ)
                                      {CertificateVerify}    (chữ ký trên toàn bộ bắt tay)
                                      {Finished}
                                  <-------------------------------------
 s = B^a (cùng giá trị s)
 kiểm tra chứng chỉ + chữ ký
                                      {Finished}
                                  ------------------------------------->
 ===== từ đây dữ liệu HTTP mã hóa bằng khóa dẫn xuất từ s (HKDF) =====
```
Các điểm chính:
1. **Trao đổi khóa:** `key_share` ở ClientHello/ServerHello mang giá trị công khai $A$, $B$. Nhóm hỗ trợ gồm đường cong elliptic (x25519, secp256r1) và nhóm hữu hạn có tên `ffdhe2048…ffdhe8192` (RFC 7919), là các **số nguyên tố an toàn** có $g = 2$, đúng loại mà chương trình của nhóm sinh ra.
2. **Dẫn xuất khóa:** $s$ không dùng trực tiếp mà đưa vào HKDF cùng giá trị băm toàn bộ bản ghi bắt tay để tạo khóa mã hóa cho từng giai đoạn.
3. **Xác thực:** server ký bản ghi bắt tay bằng khóa riêng ứng với chứng chỉ (`CertificateVerify`); client kiểm tra chuỗi chứng chỉ. Đây là phần ngăn tấn công man-in-the-middle (mục 4.3).
4. **Bảo mật chuyển tiếp (forward secrecy):** $a, b$ chỉ tồn tại trong một phiên rồi bị xóa. Nếu sau này khóa riêng của server bị lộ, kẻ tấn công **không** giải mã được lưu lượng cũ đã ghi lại, vì không thể khôi phục $s$.

### Liên hệ với chương trình của nhóm
| Trong TLS 1.3 | Trong chương trình |
|---|---|
| Nhóm `ffdhe` (safe prime, $g=2$) | `generate_safe_prime` sinh $p = 2q+1$; `choose_generator` chọn $g$ bậc $q$ |
| `key_share` của client / server | $A = g^a \bmod p$, $B = g^b \bmod p$ |
| Khóa ephemeral | `generate_private_key` mỗi phiên, trong $[2, p-2]$ |
| Kiểm tra `key_share` hợp lệ | `is_valid_public_value` (thuộc $[2,p-2]$, $V^q = 1$) |
| Dẫn xuất khóa | Ngoài phạm vi bài (chỉ in $s$) |

Lưu ý: RFC 7919 dùng $g = 2$, hợp lệ khi $p \equiv 7 \pmod 8$ (khi đó 2 là thặng dư bậc hai, sinh nhóm con bậc $q$). Nhóm chọn $g = h^2 \bmod p$ ($g = 4$) để đúng với **mọi** safe prime mà vẫn nằm trong nhóm con bậc $q$.

## 4.3. Tấn công man-in-the-middle & vì sao DH cần xác thực
DH thuần túy **không xác thực danh tính**. Mallory chặn đường truyền, thực hiện hai phiên DH riêng: với Alice (khóa $s_1$) và với Bob (khóa $s_2$), rồi giải mã/mã hóa lại mọi tin nhắn. Cả hai nạn nhân đều thấy "bí mật chung" hợp lệ mà không biết có người chen giữa. Cách phòng: gắn DH với xác thực, trong TLS là **chữ ký số của server trên bản ghi bắt tay** + chứng chỉ do CA ký (trong SSH là khóa host đã biết; trong IPsec IKE là chữ ký hoặc khóa chia sẻ trước).

## 4.4. Các tấn công thực tế liên quan
- **Logjam (2015, Adrian và cộng sự, ACM CCS 2015):** (1) kẻ tấn công chen giữa ép server hạ cấp xuống `DHE_EXPORT` với $p$ chỉ 512 bit; (2) bước tiền tính của thuật toán sàng trường số (NFS) chỉ phụ thuộc $p$, nên tính một lần cho số nguyên tố dùng chung rồi giải từng logarit rời rạc trong thời gian ngắn. Nhiều server dùng chung vài số nguyên tố 512/1024 bit nên rất đáng giá để tiền tính. Bài học: tham số nhỏ + dùng chung + cho phép hạ cấp là nguy hiểm. TLS 1.3 loại bỏ nhóm xuất khẩu và nhóm tùy ý, chỉ cho phép nhóm có tên được kiểm định; khuyến nghị tối thiểu 2048 bit.
- **Tấn công nhóm con nhỏ:** nếu $p-1$ có ước nhỏ và kẻ tấn công gửi giá trị công khai thuộc nhóm con nhỏ, bí mật chung chỉ có ít giá trị, từ đó suy ra thông tin về khóa riêng. Dùng **số nguyên tố an toàn** ($p-1 = 2q$) và kiểm tra giá trị nhận được ($2 \le V \le p-2$, $V^q \equiv 1$) loại bỏ nguy cơ này. Đây chính là lý do đề bài yêu cầu safe prime.

## 4.5. Các ứng dụng khác
- **SSH:** trao đổi khóa phiên (`diffie-hellman-group14-sha256`, `curve25519-sha256`).
- **IPsec / IKEv2:** thỏa thuận khóa cho VPN, dùng các nhóm MODP của RFC 3526 (1536–8192 bit).
- **Signal / WhatsApp:** X3DH và Double Ratchet dựa trên DH trên đường cong (X25519), đổi khóa liên tục cho mỗi tin nhắn.

## 4.6. Tài liệu tham khảo
1. W. Diffie, M. Hellman, "New Directions in Cryptography", *IEEE Trans. Information Theory*, 22(6), 1976.
2. E. Rescorla, *RFC 8446: The Transport Layer Security (TLS) Protocol Version 1.3*, 2018.
3. D. Gillmor, *RFC 7919: Negotiated Finite Field Diffie-Hellman Ephemeral Parameters for TLS*, 2016.
4. D. Harkins, D. Carrel, *RFC 2409: The Internet Key Exchange (IKE)*, 1998 (Oakley Group 2, 1024 bit).
5. D. Adrian et al., "Imperfect Forward Secrecy: How Diffie-Hellman Fails in Practice", *ACM CCS 2015* (weakdh.org).
6. Giáo trình: S. Galbraith, *Mathematics of Public Key Cryptography*, chương 20 (liên kết trong đề bài).
