import os
import re

slides_dir = 'C:/Users/28414/Desktop/低标注预算下长尾分布的主动学习与半监督学习联合策略-20260617-131815/ppt-output/slides'
output_path = 'C:/Users/28414/Desktop/低标注预算下长尾分布的主动学习与半监督学习联合策略-20260617-131815/index_standalone.html'

total_slides = 20

# Read common CSS from slide 1
with open(os.path.join(slides_dir, '1.html'), 'r', encoding='utf-8') as f:
    s1_text = f.read()
    
style_match = re.search(r'<style>(.*?)</style>', s1_text, flags=re.DOTALL)
common_style = style_match.group(1) if style_match else ''

html_template = f'''<!DOCTYPE html>
<html lang="zh-CN">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>低标注预算下长尾分布的主动学习与半监督学习联合策略</title>
<link rel="stylesheet" href="https://fonts.googleapis.com/css2?family=Inter+Tight:wght@500;600;700;800&family=Instrument+Serif:ital@0;1&family=Inter:wght@400;500;600;700&display=swap">
<link rel="stylesheet" href="https://cdn.jsdelivr.net/npm/katex@0.16.9/dist/katex.min.css">
<script src="https://cdn.jsdelivr.net/npm/katex@0.16.9/dist/katex.min.js"></script>
<script src="https://cdn.jsdelivr.net/npm/katex@0.16.9/dist/contrib/auto-render.min.js"></script>
<script src="https://cdn.jsdelivr.net/npm/chart.js@3.9.1/dist/chart.min.js"></script>
<style>
{common_style}

/* Preview specific styles */
body {{
  margin: 0;
  overflow: hidden;
  background: #111;
  display: flex;
  justify-content: center;
  align-items: center;
  height: 100vh;
}}

#stage-wrap {{
  position: relative;
  width: 1280px;
  height: 720px;
  box-shadow: 0 10px 40px rgba(0,0,0,0.5);
  overflow: hidden;
}}

#deck {{
  width: 100%;
  height: 100%;
  position: relative;
}}

.slide {{
  position: absolute;
  top: 0; left: 0;
  width: 100%; height: 100%;
  background: #fff;
  opacity: 0;
  pointer-events: none;
  transition: opacity 0.4s ease, transform 0.4s cubic-bezier(0.4, 0.0, 0.2, 1);
  transform: translateX(20px);
}}

.slide.active {{
  opacity: 1;
  pointer-events: auto;
  transform: translateX(0);
  z-index: 10;
}}

.slide.prev {{
  transform: translateX(-20px);
}}

/* Controls */
#controls {{
  position: fixed;
  bottom: 20px;
  left: 50%;
  transform: translateX(-50%);
  background: rgba(20,20,20,0.9);
  border-radius: 30px;
  padding: 10px 20px;
  display: flex;
  gap: 16px;
  align-items: center;
  z-index: 1000;
  color: white;
  font-family: var(--font-body);
  transition: opacity 0.3s ease;
}}

#controls button {{
  background: transparent;
  border: none;
  color: white;
  cursor: pointer;
  padding: 4px;
  border-radius: 4px;
  display: flex;
  align-items: center;
  justify-content: center;
  opacity: 0.8;
}}

#controls button:hover {{ background: rgba(255,255,255,0.1); opacity: 1; }}
#controls .divider {{ width: 1px; height: 16px; background: rgba(255,255,255,0.2); }}
</style>
</head>
<body>

<div id="stage-wrap">
  <div id="deck">
'''

slides_content = []
for i in range(1, total_slides + 1):
    file_path = os.path.join(slides_dir, f'{i}.html')
    with open(file_path, 'r', encoding='utf-8') as f:
        html = f.read()
    
    # Extract body content
    body_match = re.search(r'<body>(.*?)</body>', html, flags=re.DOTALL)
    if body_match:
        content = body_match.group(1).strip()
    else:
        # Fallback if no body tag
        content = html
        
    slides_content.append(f'<div class="slide" id="slide-{i}">\n{content}\n</div>')

html_template += '\n'.join(slides_content)

html_template += '''
  </div>
</div>

<div id="controls">
  <button id="btn-prev" title="Previous (Left Arrow)">
    <svg width="20" height="20" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><polyline points="15 18 9 12 15 6"></polyline></svg>
  </button>
  <span id="page-indicator" style="font-variant-numeric: tabular-nums;">1 / 20</span>
  <button id="btn-next" title="Next (Right Arrow)">
    <svg width="20" height="20" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><polyline points="9 18 15 12 9 6"></polyline></svg>
  </button>
  <div class="divider"></div>
  <button id="btn-full" title="Fullscreen (F)">
    <svg width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M8 3H5a2 2 0 0 0-2 2v3m18 0V5a2 2 0 0 0-2-2h-3m0 18h3a2 2 0 0 0 2-2v-3M3 16v3a2 2 0 0 0 2 2h3"></path></svg>
  </button>
</div>

<script>
(function() {
  const TOTAL = 20;
  let current = 1;
  
  const slides = document.querySelectorAll('.slide');
  const indicator = document.getElementById('page-indicator');
  const stageWrap = document.getElementById('stage-wrap');
  
  function updateSlides() {
    slides.forEach((el, idx) => {
      const num = idx + 1;
      el.classList.remove('active', 'prev');
      if (num === current) {
        el.classList.add('active');
      } else if (num < current) {
        el.classList.add('prev');
      }
    });
    indicator.textContent = current + ' / ' + TOTAL;
    
    // Auto-render KaTeX if needed for the current slide
    if (typeof renderMathInElement !== 'undefined') {
      renderMathInElement(slides[current-1], {
        delimiters: [
          {left: '$$', right: '$$', display: true},
          {left: '$', right: '$', display: false},
          {left: '\\\\(', right: '\\\\)', display: false},
          {left: '\\\\[', right: '\\\\]', display: true}
        ],
        throwOnError: false
      });
    }
  }
  
  function go(dir) {
    const next = current + dir;
    if (next >= 1 && next <= TOTAL) {
      current = next;
      updateSlides();
      return true;
    }
    return false;
  }
  
  document.getElementById('btn-prev').onclick = () => go(-1);
  document.getElementById('btn-next').onclick = () => go(1);
  document.getElementById('btn-full').onclick = () => {
    if (!document.fullscreenElement) {
      document.body.requestFullscreen().catch(err => {});
    } else {
      document.exitFullscreen();
    }
  };
  
  window.addEventListener('keydown', e => {
    if (e.key === 'ArrowLeft') go(-1);
    if (e.key === 'ArrowRight' || e.key === ' ') go(1);
    if (e.key === 'f' || e.key === 'F') document.getElementById('btn-full').click();
  });
  
  function fitStage() {
    const w = window.innerWidth;
    const h = window.innerHeight;
    const scale = Math.min(w / 1280, h / 720);
    stageWrap.style.transform = `scale(${scale})`;
  }
  
  window.addEventListener('resize', fitStage);
  
  fitStage();
  updateSlides();
})();
</script>
</body>
</html>
'''

with open(output_path, 'w', encoding='utf-8') as f:
    f.write(html_template)

print("Generated index_standalone.html successfully!")
