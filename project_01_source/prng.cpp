// ============================================================================
// prng.cpp - Bộ sinh số giả ngẫu nhiên dựa trên SHAKE128 (XOF)
// Gộp từ mã nguồn mẫu của giảng viên (prng_sample/lib/fips202/prng.h + prng.cpp).
// Phụ trách: Lâm Duy Minh (24120091)
//
// Đã hoàn thiện các mục trong PHAN_CONG_CONG_VIEC.md (Mục 2):
//   [1] Port randombytes() sang Windows bằng rand_s (_CRT_RAND_S).
//       Nhánh Linux/macOS giữ nguyên getrandom().
//   [2] Đổi tham số outlen của randombytes() từ uint8_t sang size_t.
//   [3] prng_next_u16/u32: ép kiểu buf[i] sang uint16_t/uint32_t TRƯỚC khi dịch bit
//       để tránh tràn bit dấu (Undefined Behavior trên int 32-bit trong C++14).
//   [4] Hỗ trợ lấy đủ 32 byte entropy từ randombytes() trên cả Windows và Linux.
// ============================================================================
#ifndef PRNG_CPP
#define PRNG_CPP

#ifdef _WIN32
#ifndef _CRT_RAND_S
#define _CRT_RAND_S
#endif
#include <stdlib.h>
#if defined(_CRTIMP)
extern "C" _CRTIMP int __cdecl rand_s(unsigned int*);
#else
extern "C" int __cdecl rand_s(unsigned int*);
#endif
#endif

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

void randombytes(uint8_t *out, size_t outlen);
void prng_seed(prng_ctx *ctx, const uint8_t *seed, size_t seedlen);
uint16_t prng_next_u16(prng_ctx *ctx);
uint32_t prng_next_u32(prng_ctx *ctx);
uint64_t prng_next_u64(prng_ctx *ctx);

// ----------------------------- [Phần 2] prng.cpp -----------------------------

// Lấy chuỗi byte ngẫu nhiên từ nguồn entropy của hệ điều hành:
//   - Windows (_WIN32): dùng rand_s() (gọi RtlGenRandom của OS).
//   - Linux / macOS:    dùng getrandom().
void randombytes(uint8_t *out, size_t outlen)
{
#ifdef _WIN32
    while (outlen > 0)
    {
        unsigned int r = 0;
        if (rand_s(&r) != 0)
        {
            fprintf(stderr, "Loi: rand_s() that bai khi lay entropy tu he dieu hanh Windows.\n");
            abort();
        }
        size_t chunk = (outlen < sizeof(r)) ? outlen : sizeof(r);
        for (size_t i = 0; i < chunk; i++)
        {
            out[i] = static_cast<uint8_t>((r >> (8 * i)) & 0xFFU);
        }
        out += chunk;
        outlen -= chunk;
    }
#else
    ssize_t ret;
    while (outlen > 0)
    {
        ret = getrandom(out, outlen, 0);
        if ((ret == -1) && (errno == EINTR))
            continue;
        else if (ret < 0)
            abort();
        out += static_cast<size_t>(ret);
        outlen -= static_cast<size_t>(ret);
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
    return static_cast<uint16_t>(buf[0]) |
           (static_cast<uint16_t>(buf[1]) << 8);
}

uint32_t prng_next_u32(prng_ctx *ctx)
{
    uint8_t buf[4];
    shake128_squeeze(buf, sizeof(buf), &ctx->sponge);
    return static_cast<uint32_t>(buf[0]) |
           (static_cast<uint32_t>(buf[1]) << 8) |
           (static_cast<uint32_t>(buf[2]) << 16) |
           (static_cast<uint32_t>(buf[3]) << 24);
}

uint64_t prng_next_u64(prng_ctx *ctx)
{
    uint8_t buf[8];
    shake128_squeeze(buf, sizeof(buf), &ctx->sponge);
    uint64_t ret = 0;
    for (size_t i = 0; i < 8; i++)
    {
        ret |= static_cast<uint64_t>(buf[i]) << (8 * i);
    }
    return ret;
}

#endif // PRNG_CPP
