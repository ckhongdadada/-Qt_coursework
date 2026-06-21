import re
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

with open('C:/Users/28414/Desktop/低标注预算下长尾分布的主动学习与半监督学习联合策略-20260617-131815/index_standalone.html', 'r', encoding='utf-8') as f:
    text = f.read()

slides = re.split(r'<div class="slide"', text)
print(f'Found {len(slides)-1} slides')

for i in range(1, len(slides)):
    slide_html = slides[i]
    title_match = re.search(r'<h1[^>]*>(.*?)</h1>', slide_html)
    page_label_match = re.search(r'<div class="page-label">(.*?)</div>', slide_html)
    
    title = strip_tags(title_match.group(1)).strip() if title_match else 'No Title'
    label = strip_tags(page_label_match.group(1)).strip() if page_label_match else 'No Label'
    
    print(f'Slide {i}: [{label}] {title}')
