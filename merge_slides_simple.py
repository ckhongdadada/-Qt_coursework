#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
最简单版本：直接提取body内容，不使用iframe
"""

import re
import os
import base64
from pathlib import Path
from urllib.parse import unquote

def read_image_as_base64(image_path):
    """读取图片文件并转换为base64"""
    try:
        with open(image_path, 'rb') as f:
            image_data = f.read()
        
        ext = Path(image_path).suffix.lower()
        mime_types = {
            '.png': 'image/png',
            '.jpg': 'image/jpeg',
            '.jpeg': 'image/jpeg',
            '.gif': 'image/gif',
            '.svg': 'image/svg+xml',
            '.webp': 'image/webp'
        }
        mime_type = mime_types.get(ext, 'image/png')
        
        base64_data = base64.b64encode(image_data).decode('utf-8')
        return f"data:{mime_type};base64,{base64_data}"
    except Exception as e:
        print(f"  ⚠ 无法读取图片 {image_path}: {e}")
        return None

def convert_local_images_to_base64(html_content):
    """将HTML中的本地图片路径转换为base64"""
    def replace_img_src(match):
        full_tag = match.group(0)
        src = match.group(1)
        
        if src.startswith('file:///'):
            local_path = src.replace('file:///', '')
            local_path = unquote(local_path)
            local_path = local_path.replace('/', '\\')
            if local_path.startswith('C%3A'):
                local_path = 'C:' + local_path[4:]
            
            print(f"  → 转换图片: {Path(local_path).name}")
            
            base64_src = read_image_as_base64(local_path)
            if base64_src:
                return full_tag.replace(src, base64_src)
            else:
                print(f"    ✗ 转换失败")
                return full_tag
        
        return full_tag
    
    html_content = re.sub(
        r'<img[^>]+src="([^"]+)"',
        replace_img_src,
        html_content
    )
    
    return html_content

def extract_body_and_style(html_content):
    """提取body内容、style和外部脚本引用"""
    # 提取style
    style_match = re.search(r'<style>(.*?)</style>', html_content, re.DOTALL)
    style = style_match.group(1) if style_match else ''
    
    # 移除@font-face
    style = re.sub(r'@font-face\s*{[^}]*?}', '', style, flags=re.DOTALL)
    
    # 替换字体
    style = style.replace("'Alibaba PuHuiTi 3.0'", "'Microsoft YaHei', 'PingFang SC', 'Hiragino Sans GB'")
    style = style.replace("'Alibaba PuHuiTi 3.0', sans-serif", "'Microsoft YaHei', 'PingFang SC', 'Hiragino Sans GB', Arial, sans-serif")
    style = style.replace('"Alibaba PuHuiTi 3.0"', '"Microsoft YaHei", "PingFang SC", "Hiragino Sans GB"')
    
    # 提取外部脚本引用（如Chart.js）
    external_scripts = []
    for match in re.finditer(r'<script\s+src="([^"]+)"[^>]*></script>', html_content):
        src = match.group(1)
        if 'http' in src or 'cdn' in src:  # 只保留外部CDN脚本
            external_scripts.append(src)
    
    # 提取body内容
    body_match = re.search(r'<body>(.*?)</body>', html_content, re.DOTALL)
    body = body_match.group(1) if body_match else ''
    
    # 提取内联script（绘图代码等）
    inline_scripts = []
    for match in re.finditer(r'<script(?![^>]*\bsrc\b)[^>]*>(.*?)</script>', body, re.DOTALL):
        script_content = match.group(1)
        # 排除桥接脚本
        if '__qw_sel_bridge' not in script_content and '__qw_export_runtime' not in script_content:
            inline_scripts.append(script_content)
    
    # 移除body中的所有script标签
    body = re.sub(r'<script[^>]*>.*?</script>', '', body, flags=re.DOTALL)
    
    return style.strip(), body.strip(), external_scripts, inline_scripts

def merge_slides(slides_dir, output_file):
    """合并所有幻灯片"""
    slides_path = Path(slides_dir)
    slide_files = sorted(slides_path.glob('*.html'), key=lambda x: int(x.stem))
    
    if not slide_files:
        print("没有找到幻灯片文件！")
        return
    
    print(f"找到 {len(slide_files)} 个幻灯片文件")
    
    slides_data = []
    
    for i, slide_file in enumerate(slide_files, 1):
        print(f"\n处理 {slide_file.name}...")
        with open(slide_file, 'r', encoding='utf-8') as f:
            html_content = f.read()
        
        # 转换图片
        html_content = convert_local_images_to_base64(html_content)
        
        # 提取样式、body、外部脚本和内联脚本
        style, body, external_scripts, inline_scripts = extract_body_and_style(html_content)
        
        slides_data.append({
            'number': i,
            'style': style,
            'body': body,
            'external_scripts': external_scripts,
            'inline_scripts': inline_scripts
        })
    
    # 生成HTML
    final_html = generate_html(slides_data, len(slide_files))
    
    with open(output_file, 'w', encoding='utf-8') as f:
        f.write(final_html)
    
    print(f"\n{'='*60}")
    print(f"✓ 成功！")
    print(f"  文件: {output_file}")
    print(f"  幻灯片: {len(slides_data)} 个")
    print(f"  大小: {len(final_html):,} 字节 ({len(final_html)/1024/1024:.2f} MB)")

def generate_html(slides, total_slides):
    """生成最终HTML"""
    
    # 收集所有外部脚本（去重）
    all_external_scripts = []
    for slide in slides:
        for script in slide.get('external_scripts', []):
            if script not in all_external_scripts:
                all_external_scripts.append(script)
    
    # 合并所有样式 - 使用更精确的作用域
    all_styles = []
    for slide in slides:
        style = slide['style']
        # 为每个选择器添加.slide-N前缀
        # 处理body选择器
        style = re.sub(r'\bbody\s*{', f'.slide-{slide["number"]} {{', style)
        # 处理其他选择器（类、标签等）
        style = re.sub(r'\n([.#][\w-]+)', f'\n.slide-{slide["number"]} \\1', style)
        # 处理元素选择器
        style = re.sub(r'\n([\w]+)\s*{', f'\n.slide-{slide["number"]} \\1 {{', style)
        all_styles.append(style)
    
    # 生成幻灯片HTML
    slides_html = []
    for slide in slides:
        display = "block" if slide['number'] == 1 else "none"
        
        # 添加内联脚本
        inline_script_html = ''
        if slide.get('inline_scripts'):
            for script in slide['inline_scripts']:
                # 修改脚本中的字体引用
                script = script.replace("'Alibaba PuHuiTi 3.0'", "'Microsoft YaHei', 'PingFang SC', 'Hiragino Sans GB'")
                inline_script_html += f'\n<script>\n{script}\n</script>'
        
        slides_html.append(f'''<div class="slide-container slide-{slide['number']}" id="slide-{slide['number']}" style="display: {display};">
{slide['body']}{inline_script_html}
</div>''')
    
    # 外部脚本标签
    external_scripts_html = '\n'.join([f'<script src="{src}"></script>' for src in all_external_scripts])
    
    html = f'''<!DOCTYPE html>
<html lang="zh-CN">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>低标注预算下长尾分布的主动学习与半监督学习联合策略</title>
{external_scripts_html}
<style>
* {{
  margin: 0;
  padding: 0;
  box-sizing: border-box;
}}

html, body {{
  width: 100%;
  height: 100vh;
  overflow: hidden;
  background: #000;
  font-family: 'Microsoft YaHei', 'PingFang SC', 'Hiragino Sans GB', Arial, sans-serif;
}}

.main-wrapper {{
  width: 100%;
  height: 100vh;
  display: flex;
  align-items: center;
  justify-content: center;
  background: #000;
}}

.stage {{
  width: 1280px;
  height: 720px;
  position: relative;
  box-shadow: 0 12px 40px rgba(0,0,0,0.6);
  background: #fff;
}}

.slide-container {{
  position: absolute;
  top: 0;
  left: 0;
  width: 1280px;
  height: 720px;
  overflow: hidden;
}}

/* 控制按钮 */
.controls {{
  position: fixed;
  bottom: 30px;
  left: 50%;
  transform: translateX(-50%);
  background: rgba(20, 20, 20, 0.95);
  padding: 14px 24px;
  border-radius: 40px;
  display: flex;
  align-items: center;
  gap: 20px;
  z-index: 10000;
  box-shadow: 0 8px 32px rgba(0, 0, 0, 0.6);
  border: 1px solid rgba(255, 255, 255, 0.1);
}}

.controls button {{
  background: rgba(255, 255, 255, 0.12);
  border: none;
  color: #fff;
  padding: 10px 20px;
  border-radius: 24px;
  cursor: pointer;
  font-size: 15px;
  font-weight: 500;
  transition: all 0.2s;
}}

.controls button:hover:not(:disabled) {{
  background: rgba(255, 255, 255, 0.24);
}}

.controls button:disabled {{
  opacity: 0.3;
  cursor: not-allowed;
}}

.controls .counter {{
  color: #fff;
  font-size: 15px;
  min-width: 90px;
  text-align: center;
  font-weight: 500;
}}

.hint {{
  position: fixed;
  top: 24px;
  right: 24px;
  background: rgba(20, 20, 20, 0.9);
  padding: 12px 20px;
  border-radius: 12px;
  color: rgba(255, 255, 255, 0.7);
  font-size: 13px;
  z-index: 10000;
  border: 1px solid rgba(255, 255, 255, 0.1);
}}

.hint .key {{
  display: inline-block;
  background: rgba(255, 255, 255, 0.1);
  padding: 2px 8px;
  border-radius: 4px;
  font-family: monospace;
  font-size: 12px;
}}

/* 幻灯片样式 */
{chr(10).join(all_styles)}
</style>
</head>
<body>
<div class="main-wrapper">
  <div class="stage">
{chr(10).join(slides_html)}
  </div>
</div>

<div class="controls">
  <button id="prevBtn" onclick="prevSlide()">← 上一页</button>
  <span class="counter" id="counter">1 / {total_slides}</span>
  <button id="nextBtn" onclick="nextSlide()">下一页 →</button>
</div>

<div class="hint">
  <span class="key">←</span> <span class="key">→</span> 翻页 |
  <span class="key">Home</span> <span class="key">End</span> 首/末页 |
  <span class="key">1-9</span> 快速跳转
</div>

<script>
let currentSlide = 1;
const totalSlides = {total_slides};

function showSlide(n) {{
  if (n < 1) n = 1;
  if (n > totalSlides) n = totalSlides;
  
  // 隐藏所有
  for (let i = 1; i <= totalSlides; i++) {{
    const slide = document.getElementById('slide-' + i);
    if (slide) slide.style.display = 'none';
  }}
  
  // 显示当前
  const current = document.getElementById('slide-' + n);
  if (current) current.style.display = 'block';
  
  currentSlide = n;
  document.getElementById('counter').textContent = n + ' / ' + totalSlides;
  document.getElementById('prevBtn').disabled = (n === 1);
  document.getElementById('nextBtn').disabled = (n === totalSlides);
}}

function nextSlide() {{ showSlide(currentSlide + 1); }}
function prevSlide() {{ showSlide(currentSlide - 1); }}
function jumpToSlide(n) {{ showSlide(n); }}

document.addEventListener('keydown', function(e) {{
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
      if (e.key >= '0' && e.key <= '9') {{
        const num = parseInt(e.key);
        if (num >= 1 && num <= Math.min(9, totalSlides)) {{
          jumpToSlide(num);
          e.preventDefault();
        }}
      }}
  }}
}});

showSlide(1);
console.log('幻灯片已加载: ' + totalSlides + ' 页');
</script>
</body>
</html>'''
    
    return html

if __name__ == '__main__':
    base_dir = Path(__file__).parent
    slides_dir = base_dir / 'slides'
    output_file = base_dir / 'index_standalone.html'
    
    print("=" * 60)
    print("幻灯片合并工具 - 简化版")
    print("=" * 60)
    
    merge_slides(slides_dir, output_file)
