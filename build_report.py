#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
build_report.py - Biên soạn và kết xuất Báo cáo Đồ án 1 (report.pdf)
Môn: Nhập môn mã hóa - mật mã | Nhóm 18 | HK1 2026-2027
"""

import os
import re
import subprocess
import markdown
import pypdf

REPO_DIR = os.path.dirname(os.path.abspath(__file__))
DOCS_DIR = os.path.join(REPO_DIR, "docs")
OUT_DIR = os.path.join(REPO_DIR, "project_01_report")
HTML_OUT = os.path.join(OUT_DIR, "report.html")
PDF_OUT = os.path.join(OUT_DIR, "report.pdf")

# Danh sách các tệp nội dung theo thứ tự
DOC_FILES = [
    "muc1_he_thong_so_lon.md",
    "muc2_kiem_tra_so_nguyen_to.md",
    "muc3_so_ngau_nhien.md",
    "muc4_ung_dung_dh.md",
    "muc5_mo_ta_chuong_trinh.md",
    "muc6_tu_danh_gia.md"
]

def clean_markdown_math(text):
    """
    Chuẩn hóa cú pháp công thức KaTeX trước khi đưa vào markdown parser
    để tránh parser biến dấu _ hoặc * trong công thức thành thẻ em/strong.
    """
    # Thay thế KaTeX block math $$...$$ bằng placeholder an toàn
    blocks = []
    def save_block(m):
        blocks.append(m.group(1))
        return f"@@BLOCKMATH{len(blocks)-1}@@"
    text = re.sub(r"\$\$(.*?)\$\$", save_block, text, flags=re.DOTALL)

    # Thay thế KaTeX inline math $...$ bằng placeholder
    inlines = []
    def save_inline(m):
        inlines.append(m.group(1))
        return f"@@INLINEMATH{len(inlines)-1}@@"
    text = re.sub(r"(?<!\\)\$(.*?)(?<!\\)\$", save_inline, text)

    return text, blocks, inlines

def restore_markdown_math(html, blocks, inlines):
    for i, b in enumerate(blocks):
        # Escaped characters inside math if any
        html = html.replace(f"@@BLOCKMATH{i}@@", f'<div class="katex-display">$${b}$$</div>')
    for i, inl in enumerate(inlines):
        html = html.replace(f"@@INLINEMATH{i}@@", f'<span class="katex-inline">${inl}$</span>')
    return html

def build_html():
    os.makedirs(OUT_DIR, exist_ok=True)
    
    combined_body = []
    
    for filename in DOC_FILES:
        filepath = os.path.join(DOCS_DIR, filename)
        if not os.path.exists(filepath):
            print(f"Warning: File {filename} not found!")
            continue
        
        with open(filepath, "r", encoding="utf-8") as f:
            content = f.read()

        # Bỏ dòng tiêu đề H1 đầu tiên và phụ trách chính (sẽ dùng cấu trúc section thống nhất)
        lines = content.splitlines()
        filtered_lines = []
        skip_header = True
        for line in lines:
            if skip_header:
                if line.startswith("# BÁO CÁO MỤC") or line.startswith("# MỤC"):
                    continue
                if "**Phụ trách" in line or "**Người thực hiện" in line or line.strip() == "---":
                    continue
                if line.strip() == "":
                    continue
                skip_header = False
            filtered_lines.append(line)
        
        section_md = "\n".join(filtered_lines)
        section_md_clean, blocks, inlines = clean_markdown_math(section_md)

        # Chuyển đổi Markdown sang HTML
        html_part = markdown.markdown(
            section_md_clean,
            extensions=["tables", "fenced_code", "sane_lists"]
        )
        html_part = restore_markdown_math(html_part, blocks, inlines)
        
        # Lấy số thứ tự mục từ tên file (vd muc1 -> 1)
        sec_num = re.search(r"muc(\d+)", filename).group(1)
        
        # Tiêu đề mục chuẩn
        sec_titles = {
            "1": "Mục 1: Hệ thống xử lý số lớn (Big Integer)",
            "2": "Mục 2: Các thuật toán kiểm tra số nguyên tố & Sinh số nguyên tố an toàn",
            "3": "Mục 3: Số ngẫu nhiên & Tầm quan trọng trong mật mã (Mục điểm cộng)",
            "4": "Mục 4: Ứng dụng thực tế của giao thức trao đổi khóa Diffie-Hellman",
            "5": "Mục 5: Mô tả chương trình, kiểm thử & Hướng dẫn sử dụng",
            "6": "Mục 6: Tự đánh giá ưu/nhược điểm, hướng cải thiện & Phân công công việc"
        }
        
        title_text = sec_titles.get(sec_num, f"Mục {sec_num}")
        
        combined_body.append(f"""
        <section class="report-section" id="muc-{sec_num}">
            <h1 class="section-title">{title_text}</h1>
            <div class="section-content">
                {html_part}
            </div>
        </section>
        """)

    full_html = f"""<!DOCTYPE html>
<html lang="vi">
<head>
<meta charset="utf-8">
<title>Báo cáo Đồ án 1: Số nguyên tố & Trao đổi khóa Diffie-Hellman</title>

<!-- KaTeX CSS & JS -->
<link rel="stylesheet" href="https://cdn.jsdelivr.net/npm/katex@0.16.9/dist/katex.min.css">
<script defer src="https://cdn.jsdelivr.net/npm/katex@0.16.9/dist/katex.min.js"></script>
<script defer src="https://cdn.jsdelivr.net/npm/katex@0.16.9/dist/contrib/auto-render.min.js"
    onload="renderMathInElement(document.body, {{
        delimiters: [
            {{left: '$$', right: '$$', display: true}},
            {{left: '$', right: '$', display: false}}
        ],
        throwOnError: false
    }});"></script>

<style>
@page {{
    size: A4;
    margin: 20mm 20mm 25mm 20mm;
    @bottom-center {{
        content: counter(page);
    }}
}}

@page :first {{
    margin: 0;
}}

body {{
    font-family: 'Times New Roman', 'Liberation Serif', serif;
    font-size: 13.5pt;
    line-height: 1.5;
    color: #111827;
    margin: 0;
    padding: 0;
    text-align: justify;
}}

/* BÌA BÁO CÁO */
.cover-page {{
    page-break-after: always;
    height: 100vh;
    padding: 25mm 25mm 20mm 25mm;
    box-sizing: border-box;
    display: flex;
    flex-direction: column;
    justify-content: space-between;
    border: 3px double #1e3a8a;
    margin: 15mm;
}}

.cover-header {{
    text-align: center;
    border-bottom: 2px solid #1e3a8a;
    padding-bottom: 12px;
}}

.cover-header .inst-top {{
    font-size: 13pt;
    font-weight: bold;
    text-transform: uppercase;
    color: #1e3a8a;
    letter-spacing: 0.5px;
}}

.cover-header .inst-sub {{
    font-size: 12pt;
    font-weight: bold;
    text-transform: uppercase;
    color: #374151;
    margin-top: 4px;
}}

.cover-header .inst-dept {{
    font-size: 11pt;
    font-style: italic;
    color: #4b5563;
    margin-top: 2px;
}}

.cover-title-box {{
    text-align: center;
    margin: 40px 0;
}}

.cover-subject {{
    font-size: 14pt;
    font-weight: bold;
    text-transform: uppercase;
    color: #0369a1;
    margin-bottom: 10px;
    letter-spacing: 1px;
}}

.cover-project-type {{
    font-size: 15pt;
    font-weight: bold;
    color: #1f2937;
    text-transform: uppercase;
    margin-bottom: 12px;
}}

.cover-main-title {{
    font-size: 21pt;
    font-weight: bold;
    text-transform: uppercase;
    color: #1e3a8a;
    line-height: 1.3;
    padding: 15px 10px;
    background: #f0f7ff;
    border: 1px solid #bfdbfe;
    border-radius: 6px;
}}

.cover-info {{
    font-size: 13pt;
    margin-top: 20px;
    line-height: 1.6;
}}

.info-table {{
    width: 100%;
    margin-top: 15px;
    border-collapse: collapse;
}}

.info-table td {{
    padding: 6px 10px;
    vertical-align: top;
    font-size: 12.5pt;
}}

.info-table th {{
    background: #e2e8f0;
    color: #0f172a;
    padding: 8px 10px;
    text-align: left;
    font-weight: bold;
    border: 1px solid #cbd5e1;
}}

.info-table td {{
    border: 1px solid #e2e8f0;
}}

.cover-footer {{
    text-align: center;
    font-size: 12pt;
    font-weight: bold;
    color: #4b5563;
    border-top: 1px solid #cbd5e1;
    padding-top: 10px;
}}

/* MỤC LỤC & TỔNG QUAN */
.toc-page {{
    page-break-after: always;
    padding-top: 10px;
}}

.toc-title {{
    font-size: 18pt;
    font-weight: bold;
    text-align: center;
    color: #1e3a8a;
    text-transform: uppercase;
    margin-bottom: 25px;
    border-bottom: 2px solid #1e3a8a;
    padding-bottom: 8px;
}}

.toc-item {{
    display: flex;
    justify-content: space-between;
    margin: 10px 0;
    font-size: 13pt;
    border-bottom: 1px dotted #94a3b8;
    padding-bottom: 2px;
}}

.toc-item.level-1 {{
    font-weight: bold;
    color: #0f172a;
    margin-top: 14px;
}}

.toc-item.level-2 {{
    margin-left: 20px;
    font-size: 12pt;
    color: #334155;
}}

/* NỘI DUNG CÁC PHẦN */
.report-section {{
    page-break-before: always;
    padding-top: 10px;
}}

h1.section-title {{
    font-size: 17pt;
    font-weight: bold;
    color: #1e3a8a;
    text-transform: uppercase;
    border-bottom: 2px solid #2563eb;
    padding-bottom: 6px;
    margin-top: 0;
    margin-bottom: 18px;
}}

h2 {{
    font-size: 14.5pt;
    font-weight: bold;
    color: #0f172a;
    margin-top: 22px;
    margin-bottom: 10px;
    border-left: 4px solid #2563eb;
    padding-left: 8px;
}}

h3 {{
    font-size: 13pt;
    font-weight: bold;
    color: #1e293b;
    margin-top: 16px;
    margin-bottom: 8px;
}}

h4 {{
    font-size: 12.5pt;
    font-weight: bold;
    color: #334155;
    margin-top: 12px;
    margin-bottom: 6px;
}}

p {{
    margin: 8px 0;
    text-indent: 1.2cm;
}}

ul, ol {{
    margin: 8px 0 8px 25px;
    padding-left: 15px;
}}

li {{
    margin-bottom: 5px;
}}

/* BẢNG BIỂU */
table {{
    width: 100%;
    border-collapse: collapse;
    margin: 16px 0;
    font-size: 11.5pt;
}}

th, td {{
    border: 1px solid #cbd5e1;
    padding: 7px 10px;
    text-align: left;
    vertical-align: middle;
}}

th {{
    background-color: #f1f5f9;
    font-weight: bold;
    color: #0f172a;
}}

tr:nth-child(even) {{
    background-color: #f8fafc;
}}

/* CODE & TRÍCH DẪN */
pre {{
    background-color: #f8fafc;
    border: 1px solid #e2e8f0;
    border-radius: 4px;
    padding: 12px;
    font-family: 'Consolas', 'Courier New', monospace;
    font-size: 10.5pt;
    line-height: 1.4;
    overflow-x: auto;
    margin: 12px 0;
    white-space: pre-wrap;
    word-break: break-all;
}}

code {{
    font-family: 'Consolas', 'Courier New', monospace;
    font-size: 11pt;
    background-color: #f1f5f9;
    padding: 2px 4px;
    border-radius: 3px;
    color: #b91c1c;
}}

pre code {{
    background-color: transparent;
    padding: 0;
    color: #0f172a;
}}

blockquote {{
    margin: 12px 0;
    padding: 8px 16px;
    background-color: #eff6ff;
    border-left: 4px solid #3b82f6;
    font-style: italic;
    color: #1e3a8a;
}}

blockquote p {{
    text-indent: 0;
}}

.katex-display {{
    margin: 14px 0 !important;
    text-align: center;
}}

hr {{
    border: 0;
    height: 1px;
    background: #e2e8f0;
    margin: 20px 0;
}}

/* ĐÁNH SỐ TRANG HEADER / FOOTER */
.page-footer {{
    position: fixed;
    bottom: 0;
    left: 0;
    right: 0;
    font-size: 10pt;
    text-align: center;
    color: #64748b;
    border-top: 1px solid #e2e8f0;
    padding-top: 4px;
}}
</style>
</head>
<body>

<!-- TRANG BÌA -->
<div class="cover-page">
    <div class="cover-header">
        <div class="inst-top">Đại học Quốc gia Thành phố Hồ Chí Minh</div>
        <div class="inst-sub">Trường Đại học Khoa học Tự nhiên</div>
        <div class="inst-dept">Khoa Công nghệ Thông tin — Bộ môn Công nghệ Tri thức</div>
    </div>

    <div class="cover-title-box">
        <div class="cover-subject">Môn học: Nhập môn mã hóa - mật mã</div>
        <div class="cover-project-type">BÁO CÁO ĐỒ ÁN 1</div>
        <div class="cover-main-title">
            SỐ NGUYÊN TỐ &amp; TRAO ĐỔI KHÓA DIFFIE-HELLMAN
        </div>
    </div>

    <div class="cover-info">
        <table class="info-table">
            <tr>
                <td style="width: 38%; font-weight: bold; color: #1e3a8a;">Giảng viên phụ trách:</td>
                <td>Thầy Trần Hải Anh Điền</td>
            </tr>
            <tr>
                <td style="font-weight: bold; color: #1e3a8a;">Nhóm thực hiện:</td>
                <td><strong>Nhóm 18</strong></td>
            </tr>
        </table>

        <div style="margin-top: 18px; font-weight: bold; color: #1e3a8a; font-size: 13pt;">
            DANH SÁCH THÀNH VIÊN NHÓM:
        </div>
        <table class="info-table" style="margin-top: 8px;">
            <thead>
                <tr>
                    <th style="width: 10%; text-align: center;">STT</th>
                    <th style="width: 25%;">Mã số sinh viên</th>
                    <th style="width: 40%;">Họ và tên</th>
                    <th style="width: 25%; text-align: center;">Tỷ lệ đóng góp</th>
                </tr>
            </thead>
            <tbody>
                <tr>
                    <td style="text-align: center;">1</td>
                    <td><strong>24120070</strong></td>
                    <td><strong>Vương Hữu Khang</strong></td>
                    <td style="text-align: center;"><strong>33.3%</strong></td>
                </tr>
                <tr>
                    <td style="text-align: center;">2</td>
                    <td><strong>24120089</strong></td>
                    <td><strong>Tạ Quang Mạnh</strong></td>
                    <td style="text-align: center;"><strong>33.3%</strong></td>
                </tr>
                <tr>
                    <td style="text-align: center;">3</td>
                    <td><strong>24120091</strong></td>
                    <td><strong>Lâm Duy Minh</strong></td>
                    <td style="text-align: center;"><strong>33.3%</strong></td>
                </tr>
            </tbody>
        </table>
    </div>

    <div class="cover-footer">
        THÀNH PHỐ HỒ CHÍ MINH — THÁNG 11 / 2026
    </div>
</div>

<!-- MỤC LỤC TỔNG QUAN -->
<div class="toc-page">
    <div class="toc-title">MỤC LỤC TỔNG QUAN</div>

    <div class="toc-item level-1">
        <span>Mục 1: Hệ thống xử lý số lớn (Big Integer)</span>
        <span>Trang 3</span>
    </div>
    <div class="toc-item level-2"><span>1.1. Cấu trúc dữ liệu tĩnh 2048-bit (big_t)</span></div>
    <div class="toc-item level-2"><span>1.2. Phép toán số học cơ bản (+, -, *) và xử lý nhớ 128-bit</span></div>
    <div class="toc-item level-2"><span>1.3. Phép chia nhiều từ Knuth Algorithm D (divmod, /, %)</span></div>
    <div class="toc-item level-2"><span>1.4. Lũy thừa modulo &amp; Các phép toán hỗ trợ (AddMod, MultMod, ExpMod)</span></div>

    <div class="toc-item level-1">
        <span>Mục 2: Thuật toán kiểm tra số nguyên tố &amp; Sinh Safe Prime</span>
        <span>Trang 8</span>
    </div>
    <div class="toc-item level-2"><span>2.1. Phân tích &amp; So sánh các thuật toán (Chia thử, Fermat, Solovay-Strassen, AKS, Miller-Rabin)</span></div>
    <div class="toc-item level-2"><span>2.2. Số nguyên tố an toàn (Safe Prime) &amp; Phòng chống tấn công Pohlig-Hellman, nhóm con nhỏ</span></div>
    <div class="toc-item level-2"><span>2.3. Kiến trúc lọc 4 tầng &amp; Sàng đồng thời 302 số nguyên tố nhỏ</span></div>
    <div class="toc-item level-2"><span>2.4. Kết quả thực nghiệm Benchmark sinh Safe Prime</span></div>

    <div class="toc-item level-1">
        <span>Mục 3: Số ngẫu nhiên trong Mật mã học (Mục điểm cộng)</span>
        <span>Trang 13</span>
    </div>
    <div class="toc-item level-2"><span>3.1. Vai trò của số ngẫu nhiên &amp; Phân loại TRNG, PRNG, CSPRNG</span></div>
    <div class="toc-item level-2"><span>3.2. Vì sao cấm sử dụng rand() và mt19937 trong mật mã</span></div>
    <div class="toc-item level-2"><span>3.3. Kiến trúc CSPRNG: Nguồn Entropy OS (rand_s / getrandom) + Keccak SHAKE128</span></div>
    <div class="toc-item level-2"><span>3.4. Rejection Sampling triệt tiêu Modulo Bias &amp; Các sự cố bảo mật lịch sử</span></div>

    <div class="toc-item level-1">
        <span>Mục 4: Ứng dụng thực tế của trao đổi khóa Diffie-Hellman</span>
        <span>Trang 17</span>
    </div>
    <div class="toc-item level-2"><span>4.1. Diffie-Hellman trong TLS 1.3 / HTTPS &amp; Tính bí mật chuyển tiếp (PFS)</span></div>
    <div class="toc-item level-2"><span>4.2. Các cuộc tấn công thực tế (Man-in-the-Middle, Tấn công Logjam CVE-2015-4000)</span></div>
    <div class="toc-item level-2"><span>4.3. Xu hướng tiến hóa: ECDHE trên đường cong Elliptic &amp; Mật mã hậu lượng tử</span></div>

    <div class="toc-item level-1">
        <span>Mục 5: Mô tả chương trình, kiểm thử &amp; Hướng dẫn sử dụng</span>
        <span>Trang 21</span>
    </div>
    <div class="toc-item level-2"><span>5.1. Môi trường phát triển, chuẩn C++14 &amp; Kiến trúc Unity Build (*.cpp)</span></div>
    <div class="toc-item level-2"><span>5.2. Hướng dẫn biên dịch và tùy biến tham số dòng lệnh (--bits, --rounds, --seed)</span></div>
    <div class="toc-item level-2"><span>5.3. Minh họa kết quả chạy thực tế của chương trình</span></div>

    <div class="toc-item level-1">
        <span>Mục 6: Tự đánh giá ưu/nhược điểm, hướng cải thiện &amp; Phân công</span>
        <span>Trang 25</span>
    </div>
    <div class="toc-item level-2"><span>6.1. Đánh giá phần PRNG, Miller-Rabin &amp; Sinh Safe Prime</span></div>
    <div class="toc-item level-2"><span>6.2. Đánh giá phần Big Integer &amp; Thuật toán chia Knuth D</span></div>
    <div class="toc-item level-2"><span>6.3. Đánh giá phần Giao thức Diffie-Hellman &amp; Lũy thừa Modulo</span></div>
    <div class="toc-item level-2"><span>6.4. Bảng tổng hợp mức độ hoàn thành &amp; Đóng góp của từng thành viên</span></div>
</div>

<!-- NỘI DUNG CHÍNH -->
{''.join(combined_body)}

</body>
</html>
"""

    with open(HTML_OUT, "w", encoding="utf-8") as f:
        f.write(full_html)
    print(f"Built HTML at: {HTML_OUT}")
    return HTML_OUT

def compile_pdf(html_path):
    chrome_path = r"C:\Program Files\Google\Chrome\Application\chrome.exe"
    if not os.path.exists(chrome_path):
        raise FileNotFoundError(f"Chrome executable not found at {chrome_path}")
    
    file_url = f"file:///{os.path.abspath(html_path).replace(os.sep, '/')}"
    pdf_abs = os.path.abspath(PDF_OUT)
    
    cmd = [
        chrome_path,
        "--headless",
        "--disable-gpu",
        "--no-pdf-header-footer",
        "--run-all-compositor-stages-before-draw",
        "--virtual-time-budget=6000",
        f"--print-to-pdf={pdf_abs}",
        file_url
    ]
    
    print("Compiling PDF with Chrome headless...")
    res = subprocess.run(cmd, capture_output=True, text=True)
    if res.returncode != 0:
        print(f"Error compiling PDF: {res.stderr}")
        return False
    
    if os.path.exists(pdf_abs) and os.path.getsize(pdf_abs) > 0:
        print(f"Successfully generated PDF: {pdf_abs} ({os.path.getsize(pdf_abs):,} bytes)")
        return True
    else:
        print("Failed to generate PDF.")
        return False

def verify_pdf():
    if not os.path.exists(PDF_OUT):
        print("PDF does not exist!")
        return
    reader = pypdf.PdfReader(PDF_OUT)
    num_pages = len(reader.pages)
    print(f"PDF verified: {num_pages} pages in total.")
    text_sample = reader.pages[0].extract_text()
    print("Page 1 preview:")
    print(text_sample[:300].strip())

if __name__ == "__main__":
    html_file = build_html()
    ok = compile_pdf(html_file)
    if ok:
        verify_pdf()
