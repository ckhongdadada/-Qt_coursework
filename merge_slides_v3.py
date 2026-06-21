#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
完整版本：处理幻灯片HTML，将本地图片转换为base64嵌入
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
        
        # 根据文件扩展名确定MIME类型
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
    # 查找所有img标签的src属性
    def replace_img_src(match):
        full_tag = match.group(0)
        src = match.group(1)
        
        # 解码URL编码的路径
        if src.startswith('file:///'):
            # 移除 file:/// 前缀
            local_path = src.replace('file:///', '')
            # URL解码
            local_path = unquote(local_path)
            # Windows路径处理
            local_path = local_path.replace('/', '\\')
            if local_path.startswith('C%3A'):
                local_path = 'C:' + local_path[4:]
            
            print(f"  → 转换图片: {Path(local_path).name}")
            
            # 转换为base64
            base64_src = read_image_as_base64(local_path)
            if base64_src:
                return full_tag.replace(src, base64_src)
            else:
                print(f"    ✗ 转换失败，保留原路径")
                return full_tag
        
        return full_tag
    
    # 替换所有图片源
    html_content = re.sub(
        r'<img[^>]+src="([^"]+)"',
        replace_img_src,
        html_content
    )
    
    return html_content

def process_slide_html(html_content):
    """处理单个幻灯片HTML：移除字体引用和脚本，转换图片"""
    # 移除@font-face声明
    html_content = re.sub(r'@font-face\s*\{[^}]*?\}', '', html_content, flags=re.DOTALL)
    
    # 强制添加白色背景到 body，防止透明导致黑屏
    if '<style>' in html_content:
        html_content = html_content.replace('<style>', '<style>\nbody { background-color: #FFFFFF !important; }\n')
    else:
        html_content = html_content.replace('</head>', '<style>body { background-color: #FFFFFF !important; }</style>\n</head>')
        
    # 替换本地 Katex CSS 为 CDN (修复数学公式不渲染问题)
    html_content = re.sub(r'<link[^>]+katex\.min\.css[^>]*>', '<link rel="stylesheet" href="https://cdn.jsdelivr.net/npm/katex@0.16.8/dist/katex.min.css" crossorigin="anonymous">', html_content)
    
    # 替换字体引用为系统字体
    html_content = html_content.replace("'Alibaba PuHuiTi 3.0'", "'Microsoft YaHei', 'PingFang SC', 'Hiragino Sans GB'")
    html_content = html_content.replace("'Alibaba PuHuiTi 3.0', sans-serif", "'Microsoft YaHei', 'PingFang SC', 'Hiragino Sans GB', Arial, sans-serif")
    html_content = html_content.replace('"Alibaba PuHuiTi 3.0"', '"Microsoft YaHei", "PingFang SC", "Hiragino Sans GB"')
    
    # 转换本地图片为base64
    html_content = convert_local_images_to_base64(html_content)
    
    # 移除编辑器专用的脚本（保留渲染图表的脚本）
    html_content = re.sub(r'<script[^>]*id="__qw_[^>]*>.*?</script>', '', html_content, flags=re.DOTALL)
    
    # 移除HTML注释（ai-slides标记）
    html_content = re.sub(r'<!--.*?-->', '', html_content, flags=re.DOTALL)
    
    # Inject KaTeX JS
    katex_scripts = """
<script defer src="https://cdn.jsdelivr.net/npm/katex@0.16.8/dist/katex.min.js"></script>
<script defer src="https://cdn.jsdelivr.net/npm/katex@0.16.8/dist/contrib/auto-render.min.js" onload="renderMathInElement(document.body, {delimiters: [{left: '$$', right: '$$', display: true}, {left: '$', right: '$', display: false}]});"></script>
"""
    html_content = html_content.replace('</body>', katex_scripts + '</body>')
    
    return html_content

def merge_slides_simple(slides_dir, output_file):
    """合并所有幻灯片，生成单个HTML文件"""
    slides_path = Path(slides_dir)
    slide_files = sorted(slides_path.glob('*.html'), key=lambda x: int(x.stem))
    
    if not slide_files:
        print("没有找到幻灯片文件！")
        return
    
    print(f"找到 {len(slide_files)} 个幻灯片文件")
    
    # 处理每个幻灯片
    processed_slides = []
    for i, slide_file in enumerate(slide_files, 1):
        print(f"\n处理 {slide_file.name}...")
        with open(slide_file, 'r', encoding='utf-8') as f:
            html_content = f.read()
        
        # 处理HTML（移除字体和脚本，转换图片）
        processed = process_slide_html(html_content)
        
        # 提取完整的HTML（从<!DOCTYPE到</html>）
        processed_slides.append({
            'number': i,
            'html': processed
        })
    
    # 生成最终HTML
    final_html = generate_final_html(processed_slides, len(slide_files))
    
    # 写入文件
    with open(output_file, 'w', encoding='utf-8') as f:
        f.write(final_html)
    
    print(f"\n{'='*60}")
    print(f"DONE 成功！合并后的文件保存在: {output_file}")
    print(f"  共合并 {len(processed_slides)} 个幻灯片")
    print(f"  文件大小: {len(final_html):,} 字节 ({len(final_html)/1024/1024:.2f} MB)")

def generate_final_html(slides, total_slides):
    """生成最终的HTML，使用iframe嵌入每个幻灯片"""
    
    # 为每个幻灯片创建iframe
    slides_html = []
    for slide in slides:
        # 将HTML转义以用于srcdoc
        html_escaped = (slide['html']
                       .replace('&', '&amp;')
                       .replace('"', '&quot;')
                       .replace('<', '&lt;')
                       .replace('>', '&gt;'))
        
        # 实际上srcdoc不需要转义<>，只需要转义引号
        html_escaped = (slide['html']
                       .replace('\\', '\\\\')
                       .replace('"', '\\"')
                       .replace('\n', '\\n')
                       .replace('\r', ''))
        
        display = "block" if slide['number'] == 1 else "none"
        
        # 使用JavaScript字符串而不是HTML属性
        slides_html.append(f'''<div class="slide-frame" id="slide-{slide['number']}" style="display: {display};"></div>''')
    
    # 创建JavaScript中的幻灯片数据
    slides_data_js = []
    for slide in slides:
        html_js = (slide['html']
                  .replace('\\', '\\\\')
                  .replace('`', '\\`')
                  .replace('$', '\\$')
                  .replace('</script>', '<\\/script>'))
        slides_data_js.append(f"  {slide['number']}: `{html_js}`,")
    
    html_template = f'''<!DOCTYPE html>
<html lang="zh-CN">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>低标注预算下长尾分布的主动学习与半监督学习联合策略</title>
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

.container {{
  width: 100%;
  height: 100vh;
  display: flex;
  align-items: center;
  justify-content: center;
  background: #000;
}}

.slide-wrapper {{
  width: 1280px;
  height: 720px;
  position: relative;
  box-shadow: 0 12px 40px rgba(0,0,0,0.6);
}}

.slide-frame {{
  position: absolute;
  top: 0;
  left: 0;
  width: 1280px;
  height: 720px;
}}

.slide-frame iframe {{
  display: block;
  pointer-events: auto;
  width: 100%;
  height: 100%;
  border: 0;
  background: #FFFFFF;
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
  font-family: 'Microsoft YaHei', Arial, sans-serif;
  transition: all 0.2s;
  font-weight: 500;
}}

.controls button:hover:not(:disabled) {{
  background: rgba(255, 255, 255, 0.24);
  transform: translateY(-1px);
}}

.controls button:disabled {{
  opacity: 0.3;
  cursor: not-allowed;
}}

.controls .counter {{
  color: #fff;
  font-size: 15px;
  font-family: 'Microsoft YaHei', Arial, sans-serif;
  min-width: 90px;
  text-align: center;
  font-weight: 500;
  letter-spacing: 0.5px;
}}

/* 快捷键提示 */
.hint {{
  position: fixed;
  top: 24px;
  right: 24px;
  background: rgba(20, 20, 20, 0.9);
  padding: 12px 20px;
  border-radius: 12px;
  color: rgba(255, 255, 255, 0.7);
  font-size: 13px;
  font-family: 'Microsoft YaHei', Arial, sans-serif;
  z-index: 10000;
  border: 1px solid rgba(255, 255, 255, 0.1);
  line-height: 1.6;
}}

.hint .key {{
  display: inline-block;
  background: rgba(255, 255, 255, 0.1);
  padding: 2px 8px;
  border-radius: 4px;
  font-family: monospace;
  font-size: 12px;
  margin: 0 2px;
}}
</style>
</head>
<body>
<div class="container">
  <div class="slide-wrapper">
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
const slidesData = {{
{chr(10).join(slides_data_js)}
}};

let currentSlide = 1;
const totalSlides = {total_slides};
const loadedSlides = new Set();

function loadSlide(n) {{
  if (loadedSlides.has(n)) return;
  
  const frameDiv = document.getElementById(`slide-${{n}}`);
  if (!frameDiv) return;
  
  const iframe = document.createElement('iframe');
  iframe.style.cssText = 'width: 1280px; height: 720px; border: 0;';
  iframe.srcdoc = slidesData[n];
  
  frameDiv.appendChild(iframe);
  loadedSlides.add(n);
}}

function showSlide(n) {{
  if (n < 1) n = 1;
  if (n > totalSlides) n = totalSlides;
  
  // 隐藏所有幻灯片
  for (let i = 1; i <= totalSlides; i++) {{
    const slide = document.getElementById(`slide-${{i}}`);
    if (slide) slide.style.display = 'none';
  }}
  
  // 加载并显示当前幻灯片
  loadSlide(n);
  const currentSlideEl = document.getElementById(`slide-${{n}}`);
  if (currentSlideEl) {{
    currentSlideEl.style.display = 'block';
  }}
  
  // 预加载前后幻灯片
  if (n > 1) loadSlide(n - 1);
  if (n < totalSlides) loadSlide(n + 1);
  
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
    slides_dir = base_dir / 'ppt-output' / 'slides'
    output_file = base_dir / 'index_standalone.html'
    
    print("=" * 60)
    print("幻灯片合并工具 v3 (支持图片嵌入)")
    print("=" * 60)
    print(f"幻灯片目录: {slides_dir}")
    print(f"输出文件: {output_file}")
    print()
    
    merge_slides_simple(slides_dir, output_file)
