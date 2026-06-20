import os, re
from html.parser import HTMLParser
class MLStripper(HTMLParser):
    def __init__(self):
        super().__init__()
        self.reset()
        self.strict = False
        self.convert_charrefs= True
        self.fed = []
    def handle_data(self, d):
        self.fed.append(d)
    def get_data(self):
        return ''.join(self.fed)
def strip_tags(html):
    s = MLStripper()
    s.feed(html)
    return s.get_data()

for i in range(1, 21):
    path = f'C:/Users/28414/Desktop/低标注预算下长尾分布的主动学习与半监督学习联合策略-20260617-131815/ppt-output/slides/{i}.html'
    if not os.path.exists(path):
        continue
    with open(path, 'r', encoding='utf-8') as f:
        html = f.read()
    title_match = re.search(r'<h1[^>]*>(.*?)</h1>', html)
    label_match = re.search(r'<div class="page-label">(.*?)</div>', html)
    if not title_match:
        title_match = re.search(r'<h2[^>]*>(.*?)</h2>', html)
    title = strip_tags(title_match.group(1)).strip() if title_match else '首页/结尾'
    label = strip_tags(label_match.group(1)).strip() if label_match else ''
    print(f'Slide {i}: [{label}] {title}')
