#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
create_submission_zip.py - Đóng gói và kiểm tra tính hợp lệ bài nộp Đồ án 1
Theo đúng quy định trong Lab01.pdf:
  - Tên file zip: <MSSV1-MSSV2-MSSV3>.zip (sắp xếp tăng dần theo MSSV)
  - Cấu trúc chỉ gồm:
      project_01_source/*.cpp (CHỈ file .cpp, không .h, không Makefile, không .exe)
      project_01_report/*.pdf (CHỈ file .pdf)
"""

import os
import zipfile
import hashlib
import subprocess

REPO_DIR = os.path.dirname(os.path.abspath(__file__))
SOURCE_DIR = os.path.join(REPO_DIR, "project_01_source")
REPORT_DIR = os.path.join(REPO_DIR, "project_01_report")

MSSV_LIST = ["24120070", "24120089", "24120091"]
ZIP_NAME = "-".join(sorted(MSSV_LIST)) + ".zip"
ZIP_PATH = os.path.join(os.path.dirname(REPO_DIR), ZIP_NAME) # Đặt tại D:\Nhập môn Crypto\
ZIP_LOCAL_PATH = os.path.join(REPO_DIR, ZIP_NAME)           # Đặt tại repo

EXPECTED_CPP_FILES = [
    "bigint.cpp",
    "dh.cpp",
    "fips202.cpp",
    "main.cpp",
    "prime.cpp",
    "prng.cpp"
]

def verify_source_compilation():
    print("[1/5] Kiểm tra biên dịch mã nguồn chuẩn C++14...")
    main_cpp = os.path.join(SOURCE_DIR, "main.cpp")
    test_exe = os.path.join(REPO_DIR, "temp_check.exe")
    
    cmd = [
        "g++", "-std=c++14", "-O2", "-Wall", "-Wextra", "-Wpedantic",
        "-o", "temp_check.exe", "project_01_source/main.cpp"
    ]
    res = subprocess.run(cmd, cwd=REPO_DIR, capture_output=True, text=True)
    if res.returncode != 0:
        print("LỖI BIÊN DỊCH C++:")
        print(res.stderr)
        return False
    
    if os.path.exists(test_exe):
        os.remove(test_exe)
    print("  => Biên dịch thành công 100% không warning/error!")
    return True

def create_zip(target_zip_path):
    print(f"[2/5] Đang tạo file zip tại: {target_zip_path}...")
    
    with zipfile.ZipFile(target_zip_path, "w", compression=zipfile.ZIP_DEFLATED) as zf:
        # 1. Đóng gói project_01_source/*.cpp
        for fname in EXPECTED_CPP_FILES:
            fpath = os.path.join(SOURCE_DIR, fname)
            if not os.path.exists(fpath):
                raise FileNotFoundError(f"Thiếu file mã nguồn: {fname}")
            arcname = f"project_01_source/{fname}"
            zf.write(fpath, arcname)
            print(f"  + Thêm: {arcname}")
            
        # 2. Đóng gói project_01_report/*.pdf
        report_pdf = os.path.join(REPORT_DIR, "report.pdf")
        if not os.path.exists(report_pdf):
            raise FileNotFoundError("Thiếu file báo cáo report.pdf!")
        arcname = "project_01_report/report.pdf"
        zf.write(report_pdf, arcname)
        print(f"  + Thêm: {arcname}")

def verify_zip_integrity(zip_path):
    print(f"[3/5] Kiểm tra cấu trúc bài nộp bên trong {os.path.basename(zip_path)}...")
    with zipfile.ZipFile(zip_path, "r") as zf:
        infolist = zf.infolist()
        all_ok = True
        
        print("\n--- DANH SÁCH FILE TRONG GÓI NỘP BÀI ---")
        for info in infolist:
            name = info.filename
            size = info.file_size
            comp_size = info.compress_size
            
            # Kiểm tra quy tắc
            if name.startswith("project_01_source/"):
                if not name.endswith(".cpp"):
                    print(f"  ❌ VI PHẠM: Thư mục project_01_source chứa file không phải .cpp: {name}")
                    all_ok = False
                else:
                    print(f"  [OK] {name:<35} ({size:>8,} bytes -> zip {comp_size:>7,} bytes)")
            elif name.startswith("project_01_report/"):
                if not name.endswith(".pdf"):
                    print(f"  ❌ VI PHẠM: Thư mục project_01_report chứa file không phải .pdf: {name}")
                    all_ok = False
                else:
                    print(f"  [OK] {name:<35} ({size:>8,} bytes -> zip {comp_size:>7,} bytes)")
            else:
                print(f"  ❌ VI PHẠM: Tồn tại file ngoài thư mục quy định: {name}")
                all_ok = False
                
        print("------------------------------------------")
        if all_ok:
            print("  => CẤU TRÚC BÀI NỘP HOÀN TOÀN HỢP LỆ VỚI YÊU CẦU ĐỀ BÀI!")
        return all_ok

def calculate_checksum(zip_path):
    print(f"[4/5] Tính mã băm SHA-256 của file zip...")
    h = hashlib.sha256()
    with open(zip_path, "rb") as f:
        while chunk := f.read(65536):
            h.update(chunk)
    sha256_hash = h.hexdigest()
    print(f"  SHA-256 ({os.path.basename(zip_path)}): {sha256_hash}")
    return sha256_hash

if __name__ == "__main__":
    if not verify_source_compilation():
        exit(1)
        
    create_zip(ZIP_LOCAL_PATH)
    create_zip(ZIP_PATH)
    
    if verify_zip_integrity(ZIP_PATH):
        h = calculate_checksum(ZIP_PATH)
        print("\n[5/5] TỔNG KẾT ĐÓNG GÓI THÀNH CÔNG:")
        print(f"  - File nộp: {ZIP_PATH}")
        print(f"  - Kích thước: {os.path.getsize(ZIP_PATH):,} bytes")
        print(f"  - Sẵn sàng nộp bài lên trang môn học!")
