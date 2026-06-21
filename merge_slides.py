#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
将多个幻灯片HTML文件合并成一个独立的HTML文件
移除外部字体依赖和播放器脚本
"""

import re
import os
from pathlib import Path

def extract_slide_content(html_content):
    """提取幻灯片的核心内容(不包括head和script)"""
    # 提取body内的主要内容，排除script标签
    body_match = re.search(r'<body>(.*?)</body>', html_content, re.DOTALL)
    if not body_match:
        return None
    
    body_content = body_match.group(1)
    
    # 移除所有script标签
    body_content = re.sub(r'<script[^>]*>.*?</script>', '', body_content, flags=re.DOTALL)
    
    return body_content.strip()

def extract_style_content(html_content):
    """提取style标签中的CSS，移除@font-face"""
    style_match = re.search(r'<style>(.*?)</style>', html_content, re.DOTALL)
    if not style_match:
        return None
    
    style_content = style_match.group(1)
    
    # 移除@font-face声明
    style_content = re.sub(r'@font-face\s*{[^}]*}', '', style_content, flags=re.DOTALL)
    
    # 替换字体引用为系统字体
    style_content = style_content.replace("'Alibaba PuHuiTi 3.0'", "'Microsoft YaHei', 'PingFang SC', 'Hiragino Sans GB'")
    style_content = style_content.replace("'Alibaba PuHuiTi 3.0', sans-serif", "'Microsoft YaHei', 'PingFang SC', 'Hiragino Sans GB', Arial, sans-serif")
    style_content = style_content.replace('"Alibaba PuHuiTi 3.0"', '"Microsoft YaHei", "PingFang SC", "Hiragino Sans GB"')
    
    return style_content.strip()

def merge_slides(slides_dir, output_file):
    """合并所有幻灯片到一个HTML文件"""
    slides_path = Path(slides_dir)
    slide_files = sorted(slides_path.glob('*.html'), key=lambda x: int(x.stem))
    
    if not slide_files:
        print("没有找到幻灯片文件！")
        return
    
    print(f"找到 {len(slide_files)} 个幻灯片文件")
    
    # 收集所有幻灯片内容和样式
    slides_content = []
    all_styles = []
    
    for i, slide_file in enumerate(slide_files, 1):
        print(f"处理 {slide_file.name}...")
        with open(slide_file, 'r', encoding='utf-8') as f:
            html_content = f.read()
        
        # 提取内容
        content = extract_slide_content(html_content)
        style = extract_style_content(html_content)
        
        if content:
            slides_content.append(content)
        # 提取每个幻灯片的样式
        if style:
            all_styles.append(style)
    
    # 生成合并后的HTML
    merged_html = generate_merged_html(slides_content, all_styles, len(slide_files))
    
    # 写入文件
    with open(output_file, 'w', encoding='utf-8') as f:
        f.write(merged_html)
    
    print(f"\n✓ 成功！合并后的文件保存在: {output_file}")
    print(f"  共合并 {len(slides_content)} 个幻灯片")
    print(f"  文件大小: {len(merged_html):,} 字节")

def generate_merged_html(slides_content, styles, total_slides):
    """生成最终的HTML"""
    
    # 合并样式
    merged_style = '\n'.join(styles) if styles else ''
    
    # 生成幻灯片HTML
    slides_html = []
    for i, content in enumerate(slides_content, 1):
        slide_id = f"slide-{i}"
        display = "block" if i == 1 else "none"
        slides_html.append(f'<div class="slide-page" id="{slide_id}" style="display: {display};">\n{content}\n</div>')
    
    html_template = f'''<!DOCTYPE html>
<html lang="zh-CN">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>低标注预算下长尾分布的主动学习与半监督学习联合策略</title>
<style>
{merged_style}

/* 页面容器样式 */
.slide-container {{
  width: 1280px;
  height: 720px;
  margin: 0 auto;
  position: relative;
  background: #000;
  overflow: hidden;
}}

.slide-page {{
  width: 1280px;
  height: 720px;
  position: absolute;
  top: 0;
  left: 0;
}}

/* 控制按钮 */
.controls {{
  position: fixed;
  bottom: 20px;
  left: 50%;
  transform: translateX(-50%);
  background: rgba(20, 20, 20, 0.9);
  padding: 12px 20px;
  border-radius: 30px;
  display: flex;
  align-items: center;
  gap: 16px;
  z-index: 10000;
  box-shadow: 0 4px 20px rgba(0, 0, 0, 0.5);
}}

.controls button {{
  background: rgba(255, 255, 255, 0.1);
  border: none;
  color: #fff;
  padding: 8px 16px;
  border-radius: 20px;
  cursor: pointer;
  font-size: 14px;
  font-family: 'Microsoft YaHei', Arial, sans-serif;
  transition: background 0.2s;
}}

.controls button:hover:not(:disabled) {{
  background: rgba(255, 255, 255, 0.2);
}}

.controls button:disabled {{
  opacity: 0.4;
  cursor: not-allowed;
}}

.controls .counter {{
  color: #fff;
  font-size: 14px;
  font-family: 'Microsoft YaHei', Arial, sans-serif;
  min-width: 80px;
  text-align: center;
}}

/* 快捷键提示 */
.hint {{
  position: fixed;
  top: 20px;
  right: 20px;
  background: rgba(20, 20, 20, 0.8);
  padding: 8px 16px;
  border-radius: 8px;
  color: rgba(255, 255, 255, 0.6);
  font-size: 12px;
  font-family: 'Microsoft YaHei', Arial, sans-serif;
  z-index: 10000;
}}
</style>
</head>
<body>
<div class="slide-container" id="slideContainer">
{chr(10).join(slides_html)}
</div>

<div class="controls">
  <button id="prevBtn" onclick="prevSlide()">← 上一页</button>
  <span class="counter" id="counter">1 / {total_slides}</span>
  <button id="nextBtn" onclick="nextSlide()">下一页 →</button>
</div>

<div class="hint">
  快捷键: ← → 翻页 | Home/End 首/末页 | 数字键 跳转
</div>

<script>
let currentSlide = 1;
const totalSlides = {total_slides};

function showSlide(n) {{
  if (n < 1) n = 1;
  if (n > totalSlides) n = totalSlides;
  
  // 隐藏所有幻灯片
  document.querySelectorAll('.slide-page').forEach(slide => {{
    slide.style.display = 'none';
  }});
  
  // 显示当前幻灯片
  const currentSlideEl = document.getElementById(`slide-${{n}}`);
  if (currentSlideEl) {{
    currentSlideEl.style.display = 'block';
  }}
  
  currentSlide = n;
  
  // 更新计数器
  document.getElementById('counter').textContent = `${{n}} / ${{totalSlides}}`;
  
  // 更新按钮状态
  document.getElementById('prevBtn').disabled = (n === 1);
  document.getElementById('nextBtn').disabled = (n === totalSlides);
}}

function nextSlide() {{
  showSlide(currentSlide + 1);
}}

function prevSlide() {{
  showSlide(currentSlide - 1);
}}

function jumpToSlide(n) {{
  showSlide(n);
}}

// 键盘快捷键
document.addEventListener('keydown', (e) => {{
  switch(e.key) {{
    case 'ArrowLeft':
    case 'PageUp':
      prevSlide();
      e.preventDefault();
      break;
    case 'ArrowRight':
    case 'PageDown':
    case ' ':
      nextSlide();
      e.preventDefault();
      break;
    case 'Home':
      jumpToSlide(1);
      e.preventDefault();
      break;
    case 'End':
      jumpToSlide(totalSlides);
      e.preventDefault();
      break;
    default:
      // 数字键跳转
      if (e.key >= '0' && e.key <= '9') {{
        const num = parseInt(e.key);
        if (num >= 1 && num <= Math.min(9, totalSlides)) {{
          jumpToSlide(num);
          e.preventDefault();
        }}
      }}
  }}
}});

// 初始化
showSlide(1);
</script>
</body>
</html>'''
    
    return html_template

if __name__ == '__main__':
    # 配置路径
    base_dir = Path(__file__).parent
    slides_dir = base_dir / 'slides'
    output_file = base_dir / 'index_standalone.html'
    
    print("=" * 60)
    print("幻灯片合并工具")
    print("=" * 60)
    print(f"幻灯片目录: {slides_dir}")
    print(f"输出文件: {output_file}")
    print()
    
    merge_slides(slides_dir, output_file)
