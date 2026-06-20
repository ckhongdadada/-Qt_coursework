import re
with open('C:/Users/28414/Desktop/低标注预算下长尾分布的主动学习与半监督学习联合策略-20260617-131815/index_standalone.html', 'r', encoding='utf-8') as f:
    text = f.read()
print('Count of div class slide:', len(re.findall(r'<div class="slide"', text)))
print('Count of section class slide:', len(re.findall(r'<section class="slide"', text)))
