// ============================================================================
// prng.cpp - Bộ sinh số giả ngẫu nhiên dựa trên SHAKE128 (XOF)
// Gộp từ mã nguồn mẫu của giảng viên (prng_sample/lib/fips202/prng.h + prng.cpp).
// Phụ trách: Lâm Duy Minh (24120091)
//
// TODO(Minh) – xem mục 2 trong PHAN_CONG_CONG_VIEC.md:
//   [1] Port randombytes() sang Windows bằng rand_s (_CRT_RAND_S đã được
//       định nghĩa ở dòng đầu main.cpp). Nhánh Linux/macOS giữ getrandom().
//   [2] Đổi tham số outlen của randombytes() từ uint8_t sang size_t.
//   [3] prng_next_u16/u32: ép kiểu buf[i] sang uint16_t/uint32_t TRƯỚC khi dịch bit.
//   [4] Seed mặc định: lấy đủ 32 byte từ randombytes() (xem main.cpp).
// ============================================================================
#ifndef PRNG_CPP
#define PRNG_CPP

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#ifndef _WIN32
#include <sys/random.h>
#endif

#include "fips202.cpp"

// ------------------------------ [Phần 1] prng.h ------------------------------
typedef struct
{
    keccak_state sponge;
} prng_ctx;

void randombytes(uint8_t *out, uint8_t outlen);
void prng_seed(prng_ctx *ctx, const uint8_t *seed, size_t seedlen);
uint16_t prng_next_u16(prng_ctx *ctx);
uint32_t prng_next_u32(prng_ctx *ctx);
uint64_t prng_next_u64(prng_ctx *ctx);

// ----------------------------- [Phần 2] prng.cpp -----------------------------

// This functions supports for Linux and macOS. For Windows,
// you may need to use a different method to get random bytes.
void randombytes(uint8_t *out, uint8_t outlen)
{
#ifdef _WIN32
    // TODO(Minh) [1]: cài đặt bằng rand_s(). Tạm thời dừng chương trình để
    // không bao giờ dùng nhầm dữ liệu không ngẫu nhiên làm seed.
    (void)out;
    (void)outlen;
    fprintf(stderr, "randombytes(): chua port sang Windows - hay chay voi --seed <hex>\n");
    abort();
#else
    ssize_t ret;
    while (outlen > 0)
    {
        ret = getrandom(out, outlen, 0);
        if ((ret == -1) && (errno == EINTR))
            continue;
        else if (ret < 0)
            abort();
        out += ret;
        outlen -= ret;
    }
#endif
}

void prng_seed(prng_ctx *ctx, const uint8_t *seed, size_t seedlen)
{
    shake128_init(&ctx->sponge);
    shake128_absorb(&ctx->sponge, seed, seedlen);
    shake128_finalize(&ctx->sponge);
}

uint16_t prng_next_u16(prng_ctx *ctx)
{
    uint8_t buf[2];
    shake128_squeeze(buf, sizeof(buf), &ctx->sponge);
    return (uint16_t)buf[0] | (uint16_t)(buf[1] << 8); // TODO(Minh) [3]
}

uint32_t prng_next_u32(prng_ctx *ctx)
{
    uint8_t buf[4];
    shake128_squeeze(buf, sizeof(buf), &ctx->sponge);
    // TODO(Minh) [3]: buf[3] << 24 dịch trên int, có thể tràn bit dấu.
    return (uint32_t)buf[0] | (uint32_t)(buf[1] << 8) | (uint32_t)(buf[2] << 16) | (uint32_t)(buf[3] << 24);
}

uint64_t prng_next_u64(prng_ctx *ctx)
{
    uint8_t buf[8];
    shake128_squeeze(buf, sizeof(buf), &ctx->sponge);
    uint64_t ret = 0;
    for (size_t i = 0; i < 8; i++)
    {
        ret |= (uint64_t)buf[i] << (8 * i);
    }
    return ret;
}

#endif // PRNG_CPP

