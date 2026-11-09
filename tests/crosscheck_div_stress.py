# Stress test phep chia/mod (Knuth D) voi cac so "kho": limb toan 0, 1, 2^63, 2^64-1,...
# Muc tieu: ep chuong trinh di qua nhanh hieu chinh q_hat va nhanh "add back".
# Chay:  python crosscheck_div_stress.py ./crosscheck_bigint     (ky vong: mismatch 0)
# Python chi dong vai tro dap an chuan, khong thuoc chuong trinh nop.
import random
import subprocess
import sys

random.seed(20261104)
M = 1 << 64
SPECIAL = [0, 1, 2, 3, (1 << 63) - 1, 1 << 63, (1 << 63) + 1, M - 3, M - 2, M - 1]


def limbs_to_int(limbs):  # limbs[0] la limb thap nhat
    return sum(l << (64 * i) for i, l in enumerate(limbs))


def special_number(max_limbs):
    n = random.randint(1, max_limbs)
    return limbs_to_int([random.choice(SPECIAL) for _ in range(n)])


def h(v):
    return format(v, "x")


cases = []

# 1) Cac truong hop kinh dien gay add-back (Hacker's Delight divmnu, mo rong len 64-bit)
cases.append((limbs_to_int([3, 0, 1 << 63]), limbs_to_int([1, 0, 1 << 61])))
cases.append((limbs_to_int([0, M - 2, 1 << 63]), limbs_to_int([M - 1, 1 << 63])))
cases.append((limbs_to_int([0, 0, 1 << 63, M - 1]), limbs_to_int([M - 1, 0, 1 << 63])))
cases.append((limbs_to_int([M - 1] * 6), limbs_to_int([M - 1] * 3)))
cases.append((limbs_to_int([M - 1] * 6), limbs_to_int([0] * 2 + [1])))
cases.append(((1 << 1024) - 1, (1 << 512) - 1))
cases.append(((1 << 1024), (1 << 512) + 1))
cases.append(((1 << 1000) - 1, (1 << 999)))

# 2) Ngau nhien tu tap limb dac biet
for _ in range(20000):
    a = special_number(16)
    b = special_number(random.choice([2, 3, 4, 8]))
    if b == 0:
        b = 1
    cases.append((a, b))

# 3) Ngau nhien: thuong gan gioi han (a = b*q + r voi q, r cuc tri)
for _ in range(5000):
    b = special_number(6) | 1
    q = special_number(6)
    r = random.choice([0, 1, b - 1, b // 2, max(b - 2, 0)])
    cases.append((b * q + r, b))

ops, exp = [], []
for a, b in cases:
    ops.append(f"div {h(a)} {h(b)}")
    exp.append(h(a // b))
    ops.append(f"mod {h(a)} {h(b)}")
    exp.append(h(a % b))

r = subprocess.run([sys.argv[1]], input="\n".join(ops) + "\n", capture_output=True, text=True)
out = r.stdout.split("\n")
bad = 0
for i, (o, e) in enumerate(zip(ops, exp)):
    if out[i] != e:
        bad += 1
        if bad <= 5:
            print("MISMATCH", o[:100], "got", out[i][:60], "exp", e[:60])
print("total", len(ops), "mismatch", bad)
if r.stderr.strip():
    print(r.stderr.strip())
