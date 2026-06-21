import requests
import json

AI_URL = 'http://localhost:8001'
PDF_DIR = r'C:\Users\28414\Desktop\培养方案文件'

# 只发主PDF
main_pdf = f'{PDF_DIR}/2023信息学院本科培养方案.pdf'
with open(main_pdf, 'rb') as f1:
    files = {'file': ('main.pdf', f1, 'application/pdf')}
    resp = requests.post(f'{AI_URL}/parse-pdf', files=files, timeout=120)
data_main = resp.json()

# 找大数据专业中MAT108的code
for m in data_main.get('majors', []):
    if '大数据' in m['name'] and '实验' not in m['name']:
        mat = [c for c in m.get('courses', []) if 'MAT108' in c.get('code', '')]
        print(f'主PDF中MAT108:')
        for c in mat:
            print(f'  code="{c["code"]}", section="{c.get("section", "")}"')
        break

# 只发补充PDF（通过parse_supplementary端点）
# 不行，没有这个端点。让我直接测试parse_supplementary_courses函数
import pdfplumber
import re

tongxiu_pdf = f'{PDF_DIR}/同修.pdf'
text = ''
with pdfplumber.open(tongxiu_pdf) as pdf:
    for page in pdf.pages:
        pt = page.extract_text()
        if pt:
            text += pt + '\n'

loose_code_pattern = re.compile(r'^[A-Z]{1,5}[-]?[0-9]{2,5}$', re.IGNORECASE)
current_type = "Required"
current_section = "通修必修"

for line in text.split('\n'):
    line = line.strip()
    if not line:
        continue
    m = re.match(r'([A-Z]{1,5}[-]?\d{2,5})\s+(.+?)\s+(必修|选修)\s+(\d+)\s+(\d+)', line, re.IGNORECASE)
    if m and 'MAT108' in m.group(1):
        code, name, ctype, hours, credits = m.groups()
        print(f'\n同修PDF中MAT108:')
        print(f'  code="{code.upper()}", section="{"通修必修" if ctype == "必修" else "通修选修"}"')
