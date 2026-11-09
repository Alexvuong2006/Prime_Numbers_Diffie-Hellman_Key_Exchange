# Doi chieu modular_exponentiation (C++) voi pow(b, e, m) cua Python.
# Chay: python crosscheck_modexp.py ./crosscheck_modexp     (ky vong: mismatch 0)
# Python chi lam dap an chuan, khong thuoc chuong trinh nop.
import random, subprocess, sys
random.seed(89)
h = lambda v: format(v, 'x')
cases = []
# bien: mod = 1, exp = 0, base = 0, base >= mod, exp = 1
for m in (1, 2, 3, 97, (1 << 64) + 13, (1 << 521) - 1):
    for b in (0, 1, 2, m - 1, m, m + 5, 12345678901234567890):
        for e in (0, 1, 2, 3, 65537, (1 << 64)):
            if m > 0: cases.append((b, e, m))
# ngau nhien theo cac do dai khac nhau (mod toi da 1024 bit = MAX_PRIME_BITS)
for _ in range(4000):
    mb = random.choice([8, 31, 64, 65, 128, 256, 512, 513, 768, 1024])
    m = random.getrandbits(mb) | (1 << (mb - 1))
    if random.random() < 0.5: m |= 1
    b = random.getrandbits(random.choice([1, 64, mb, mb + 200]))
    e = random.getrandbits(random.choice([1, 8, 64, 256, mb]))
    cases.append((b, e, m))
# dinh ly Fermat nho: a^(p-1) = 1 voi p nguyen to Mersenne 2^521 - 1
P = (1 << 521) - 1
for _ in range(200):
    cases.append((random.randrange(2, P), P - 1, P))
inp = "\n".join(f"{h(b)} {h(e)} {h(m)}" for b, e, m in cases) + "\n"
out = subprocess.run([sys.argv[1]], input=inp, capture_output=True, text=True).stdout.split("\n")
bad = 0
for i, (b, e, m) in enumerate(cases):
    exp = h(pow(b, e, m))
    if out[i] != exp:
        bad += 1
        if bad <= 5: print("MISMATCH", h(b)[:30], h(e)[:30], h(m)[:30], "got", out[i][:40], "exp", exp[:40])
print("total", len(cases), "mismatch", bad)
