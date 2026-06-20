import re

with open('C:/Users/28414/Desktop/低标注预算下长尾分布的主动学习与半监督学习联合策略-20260617-131815/ppt-output/preview.html', 'r', encoding='utf-8') as f:
    preview_text = f.read()

with open('C:/Users/28414/Desktop/低标注预算下长尾分布的主动学习与半监督学习联合策略-20260617-131815/ppt-output/slides/4.html', 'r', encoding='utf-8') as f:
    slide4_text = f.read()

# Extract just the <div class="slide"> ... </div> from slide4_text
match4 = re.search(r'<div class="slide".*?</div>\s*<script', slide4_text, flags=re.DOTALL)
if match4:
    slide4_content = match4.group(0).rsplit('<script', 1)[0]
else:
    # fallback
    slide4_content = slide4_text[slide4_text.find('<div class="slide"'):slide4_text.rfind('</div>')+6]

# Replace in preview.html
# Find the slide with EXPERIMENT SETUP or slide-4
preview_text_new = re.sub(
    r'<div class="slide"[^>]*>.*?<div class="page-num">4 / 20</div>\s*</div>',
    slide4_content,
    preview_text,
    flags=re.DOTALL
)

with open('C:/Users/28414/Desktop/低标注预算下长尾分布的主动学习与半监督学习联合策略-20260617-131815/ppt-output/preview.html', 'w', encoding='utf-8') as f:
    f.write(preview_text_new)

print("Updated preview.html successfully!")
