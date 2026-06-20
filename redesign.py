#!/usr/bin/env python3
"""
Redesign slides using ppt-agent-skill's blue_white style.
Applies Apple-grade typography and design principles.
"""

import re
import os
import json

BASE_DIR = os.path.dirname(os.path.abspath(__file__))
SLIDES_DIR = os.path.join(BASE_DIR, "slides")
OUTPUT_DIR = os.path.join(BASE_DIR, "ppt-output", "slides")
STYLE_FILE = os.path.join(BASE_DIR, "ppt-output", "style.json")

# Load style definition
with open(STYLE_FILE, 'r', encoding='utf-8') as f:
    STYLE = json.load(f)

# CSS Variables from blue_white style
CSS_VARS = """
:root {
  /* Background */
  --bg-primary: #ffffff;
  --bg-gradient-to: #f6f8fb;

  /* Card */
  --card-from: #ffffff;
  --card-to: #f6f8fb;
  --card-border: rgba(10,29,58,0.06);
  --card-radius: 12px;

  /* Text */
  --text-primary: #0a1d3a;
  --text-secondary: #64748b;

  /* Accent */
  --accent-primary: #2563EB;
  --accent-primary-dark: #1D4ED8;
  --accent-secondary: #059669;

  /* Typography */
  --font-display: -apple-system, 'SF Pro Display', 'Inter Tight', 'Inter', BlinkMacSystemFont, sans-serif;
  --font-body: -apple-system, 'SF Pro Text', 'Inter', sans-serif;
  --font-serif: 'Instrument Serif', 'Fraunces', Georgia, serif;
  --font-mono: 'SF Mono', 'JetBrains Mono', monospace;

  /* Letter Spacing */
  --ls-display: -0.042em;
  --ls-headline: -0.02em;
  --ls-body: -0.005em;
  --ls-label: 0.22em;
}
"""

# Base template with Apple-grade typography
BASE_STYLE = """
/* Reset & Base */
* { margin: 0; padding: 0; box-sizing: border-box; }

.slide {
  width: 1280px;
  height: 720px;
  position: relative;
  overflow: hidden;
  background: var(--bg-primary);
  font-family: var(--font-body);
  color: var(--text-primary);
  font-feature-settings: 'kern', 'liga', 'calt', 'ss01', 'cv11', 'ccmp';
  text-rendering: optimizeLegibility;
  -webkit-font-smoothing: antialiased;
  -moz-osx-font-smoothing: grayscale;
}

/* Top accent bar - subtle */
.top-bar {
  position: absolute;
  top: 0;
  left: 0;
  width: 100%;
  height: 2px;
  background: linear-gradient(90deg, var(--accent-primary), var(--accent-primary-dark));
  opacity: 0.8;
}

/* Page header with dot-pulse label */
.page-header {
  padding: 48px 64px 0;
}

.page-label {
  display: inline-flex;
  align-items: center;
  gap: 10px;
  font-size: 11px;
  font-weight: 600;
  color: var(--accent-primary);
  text-transform: uppercase;
  letter-spacing: var(--ls-label);
  margin-bottom: 16px;
}

.page-label::before {
  content: '';
  width: 6px;
  height: 6px;
  border-radius: 50%;
  background: var(--accent-primary);
  animation: pulse 2s ease-in-out infinite;
}

@keyframes pulse {
  0%, 100% { opacity: 1; transform: scale(1); }
  50% { opacity: 0.5; transform: scale(0.8); }
}

.page-title {
  font-family: var(--font-display);
  font-size: 32px;
  font-weight: 600;
  color: var(--text-primary);
  letter-spacing: var(--ls-headline);
  line-height: 1.2;
}

/* Serif italic for emphasis */
.page-title em,
.highlight em {
  font-family: var(--font-serif);
  font-style: italic;
  font-weight: 400;
  color: var(--accent-primary);
}

/* Header divider */
.header-divider {
  width: 100%;
  height: 1px;
  background: var(--card-border);
  margin: 20px 0;
}

/* Content area */
.content {
  padding: 24px 64px 48px;
  flex: 1;
}

/* Cards grid - Bento style */
.cards-grid {
  display: grid;
  gap: 16px;
}

.cards-grid.cols-3 { grid-template-columns: repeat(3, 1fr); }
.cards-grid.cols-2 { grid-template-columns: repeat(2, 1fr); }
.cards-grid.cols-4 { grid-template-columns: repeat(4, 1fr); }

/* Card - Apple minimal */
.card {
  background: linear-gradient(135deg, var(--card-from), var(--card-to));
  border: 1px solid var(--card-border);
  border-radius: var(--card-radius);
  padding: 24px;
  transition: transform 0.2s ease, box-shadow 0.2s ease;
}

.card:hover {
  transform: translateY(-2px);
  box-shadow: 0 8px 24px rgba(0,0,0,0.04);
}

.card-label {
  font-size: 11px;
  font-weight: 600;
  color: var(--accent-primary);
  text-transform: uppercase;
  letter-spacing: var(--ls-label);
  margin-bottom: 12px;
}

.card-title {
  font-family: var(--font-display);
  font-size: 18px;
  font-weight: 600;
  color: var(--text-primary);
  letter-spacing: var(--ls-headline);
  margin-bottom: 8px;
}

.card-body {
  font-size: 14px;
  color: var(--text-secondary);
  letter-spacing: var(--ls-body);
  line-height: 1.6;
}

/* Data numbers - tabular-nums */
.data-number {
  font-family: var(--font-display);
  font-variant-numeric: tabular-nums proportional-nums;
  font-weight: 700;
  color: var(--text-primary);
}

.data-number.large {
  font-size: 48px;
  letter-spacing: var(--ls-display);
}

.data-number.medium {
  font-size: 32px;
  letter-spacing: var(--ls-headline);
}

/* Tables */
.data-table {
  width: 100%;
  border-collapse: collapse;
}

.data-table th {
  font-size: 11px;
  font-weight: 600;
  color: var(--text-secondary);
  text-transform: uppercase;
  letter-spacing: var(--ls-label);
  padding: 12px 16px;
  text-align: left;
  border-bottom: 2px solid var(--card-border);
}

.data-table td {
  font-size: 14px;
  color: var(--text-primary);
  padding: 12px 16px;
  border-bottom: 1px solid var(--card-border);
  font-variant-numeric: tabular-nums;
}

.data-table tr:last-child td {
  border-bottom: none;
}

/* Page number */
.page-number {
  position: absolute;
  bottom: 24px;
  right: 48px;
  font-size: 12px;
  color: var(--text-secondary);
  font-variant-numeric: tabular-nums;
}

/* Corner decoration lines - Apple style */
.corner-lines {
  position: absolute;
  top: 24px;
  right: 24px;
  width: 40px;
  height: 40px;
  pointer-events: none;
}

.corner-lines::before,
.corner-lines::after {
  content: '';
  position: absolute;
  background: var(--accent-primary);
  opacity: 0.15;
}

.corner-lines::before {
  top: 0;
  right: 0;
  width: 1px;
  height: 20px;
}

.corner-lines::after {
  top: 0;
  right: 0;
  width: 20px;
  height: 1px;
}

/* Inner frame - subtle border */
.inner-frame {
  position: absolute;
  inset: 16px;
  border: 1px solid var(--card-border);
  border-radius: 4px;
  pointer-events: none;
}

/* Hexagon decorations - minimal */
.hex-decoration {
  position: absolute;
  pointer-events: none;
  opacity: 0.08;
}

/* Formulas */
.formula-block {
  background: var(--bg-gradient-to);
  border-left: 3px solid var(--accent-primary);
  padding: 16px 20px;
  border-radius: 0 8px 8px 0;
  margin: 16px 0;
}

.formula {
  font-family: var(--font-mono);
  font-size: 16px;
  color: var(--text-primary);
}

/* KaTeX overrides */
.katex { font-size: 1em; }
.katex-display { margin: 0; }

/* Chart container */
.chart-container {
  position: relative;
  width: 100%;
  height: 100%;
}

.chart-container canvas {
  width: 100% !important;
  height: 100% !important;
}
"""

def extract_body_content(html_content):
    """Extract content between <body> and </body>, excluding scripts."""
    body_match = re.search(r'<body[^>]*>(.*?)</body>', html_content, re.DOTALL)
    if not body_match:
        return ""
    body = body_match.group(1)
    body = re.sub(r'<script[^>]*>.*?</script>', '', body, flags=re.DOTALL)
    return body.strip()

def extract_chart_scripts(html_content):
    """Extract inline Chart.js scripts."""
    all_scripts = re.findall(r'<script(?:\s[^>]*)?>(.*?)</script>', html_content, re.DOTALL)
    chart_scripts = []
    for s in all_scripts:
        s_stripped = s.strip()
        if not s_stripped or '__qw_' in s_stripped:
            continue
        if 'new Chart' in s_stripped:
            chart_scripts.append(s_stripped)
    return "\n".join(chart_scripts)

def get_slide_title(html_content):
    """Extract slide title from HTML."""
    # Try h1 first
    h1_match = re.search(r'<h1[^>]*>(.*?)</h1>', html_content, re.DOTALL)
    if h1_match:
        title = re.sub(r'<[^>]+>', '', h1_match.group(1)).strip()
        return title

    # Try .slide-title
    title_match = re.search(r'class="[^"]*slide-title[^"]*"[^>]*>(.*?)</', html_content, re.DOTALL)
    if title_match:
        return re.sub(r'<[^>]+>', '', title_match.group(1)).strip()

    # Try .page-title
    title_match = re.search(r'class="[^"]*page-title[^"]*"[^>]*>(.*?)</', html_content, re.DOTALL)
    if title_match:
        return re.sub(r'<[^>]+>', '', title_match.group(1)).strip()

    return ""

def get_slide_label(html_content):
    """Extract slide label/category."""
    # Look for uppercase labels
    label_match = re.search(r'class="[^"]*(?:tag-label|header-label|page-label|section-label)[^"]*"[^>]*>(.*?)</', html_content, re.DOTALL)
    if label_match:
        return re.sub(r'<[^>]+>', '', label_match.group(1)).strip()
    return ""

def wrap_slide(slide_num, body, title="", label="", is_cover=False, is_end=False):
    """Wrap slide body with proper structure."""
    if is_cover:
        # Cover page - minimal with large title
        return f"""
<section id="slide-{slide_num}" class="slide-page">
  <div class="inner-frame"></div>
  <div style="display:flex; flex-direction:column; justify-content:center; align-items:center; height:100%; padding:0 120px;">
    <div class="page-label" style="margin-bottom:32px;">{label or 'MACHINE LEARNING'}</div>
    <h1 style="font-family:var(--font-display); font-size:56px; font-weight:700; color:var(--text-primary); letter-spacing:var(--ls-display); text-align:center; line-height:1.15;">
      {title}
    </h1>
    <div style="width:80px; height:2px; background:var(--accent-primary); margin:32px 0;"></div>
    <p style="font-size:18px; color:var(--text-secondary); text-align:center; letter-spacing:var(--ls-body);">
      主动学习与半监督学习的协同框架
    </p>
    <p style="font-size:14px; color:var(--text-secondary); margin-top:48px; letter-spacing:var(--ls-body);">
      作者：某某 &nbsp;|&nbsp; 指导：某老师 &nbsp;|&nbsp; 2026年6月
    </p>
  </div>
  <div class="corner-lines"></div>
  <div class="page-number">{slide_num} / 20</div>
</section>
"""
    elif is_end:
        # End page
        return f"""
<section id="slide-{slide_num}" class="slide-page">
  <div class="inner-frame"></div>
  <div style="display:flex; flex-direction:column; justify-content:center; align-items:center; height:100%; padding:0 120px;">
    <div class="page-label" style="margin-bottom:32px;">THANK YOU</div>
    <h1 style="font-family:var(--font-display); font-size:48px; font-weight:700; color:var(--text-primary); letter-spacing:var(--ls-display); text-align:center;">
      {title or '谢谢'}
    </h1>
    <div style="width:80px; height:2px; background:var(--accent-primary); margin:32px 0;"></div>
    <p style="font-size:18px; color:var(--text-secondary); text-align:center;">
      Q & A
    </p>
  </div>
  <div class="corner-lines"></div>
  <div class="page-number">{slide_num} / 20</div>
</section>
"""
    else:
        # Regular content page
        return f"""
<section id="slide-{slide_num}" class="slide-page">
  <div class="top-bar"></div>
  <div class="page-header">
    <div class="page-label">{label or 'SECTION'}</div>
    <h1 class="page-title">{title}</h1>
    <div class="header-divider"></div>
  </div>
  <div class="content">
    {body}
  </div>
  <div class="corner-lines"></div>
  <div class="page-number">{slide_num} / 20</div>
</section>
"""

def main():
    os.makedirs(OUTPUT_DIR, exist_ok=True)

    slides_html = []
    chart_scripts = {}

    for i in range(1, 21):
        slide_file = os.path.join(SLIDES_DIR, f"{i}.html")
        print(f"Processing slide {i}...")

        with open(slide_file, 'r', encoding='utf-8') as f:
            content = f.read()

        # Extract components
        body = extract_body_content(content)
        title = get_slide_title(content)
        label = get_slide_label(content)

        # Extract chart scripts
        chart_script = extract_chart_scripts(content)
        if chart_script:
            chart_scripts[i] = chart_script

        # Determine page type
        is_cover = (i == 1)
        is_end = (i == 20)

        # Wrap with proper structure
        slide_html = wrap_slide(i, body, title, label, is_cover, is_end)
        slides_html.append(slide_html)

        # Save individual slide
        with open(os.path.join(OUTPUT_DIR, f"{i}.html"), 'w', encoding='utf-8') as f:
            f.write(f"""<!DOCTYPE html>
<html lang="zh-CN">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>{title}</title>
<link rel="stylesheet" href="https://fonts.googleapis.com/css2?family=Inter+Tight:wght@500;600;700;800&family=Instrument+Serif:ital@0;1&family=Inter:wght@400;500;600;700&display=swap">
<link rel="stylesheet" href="https://cdn.jsdelivr.net/npm/katex@0.16.9/dist/katex.min.css">
<style>
{CSS_VARS}
{BASE_STYLE}
</style>
</head>
<body>
{slide_html}
</body>
</html>""")

    # Create combined preview
    chart_init_js = ""
    if chart_scripts:
        chart_init_js = "var slideChartScripts = {\n"
        for i, script in chart_scripts.items():
            chart_init_js += f"  {i}: function() {{\n{script}\n  }},\n"
        chart_init_js += "};\n"

    combined_html = f"""<!DOCTYPE html>
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
{CSS_VARS}
{BASE_STYLE}

/* Combined preview styles */
#stage-wrap {{
  position: fixed;
  inset: 0;
  display: flex;
  align-items: center;
  justify-content: center;
  background: #000;
}}

#stage {{
  width: 1280px;
  height: 720px;
  transform-origin: center center;
  background: #fff;
  box-shadow: 0 20px 60px rgba(0,0,0,0.3);
  position: relative;
  overflow: hidden;
}}

.slide-page {{
  width: 1280px;
  height: 720px;
  position: absolute;
  top: 0;
  left: 0;
  overflow: hidden;
}}

/* Controls */
#controls {{
  position: fixed;
  left: 50%;
  bottom: 24px;
  transform: translateX(-50%);
  display: flex;
  align-items: center;
  gap: 16px;
  padding: 12px 20px;
  background: rgba(255,255,255,0.95);
  border: 1px solid rgba(0,0,0,0.08);
  border-radius: 12px;
  box-shadow: 0 4px 20px rgba(0,0,0,0.1);
  backdrop-filter: blur(20px);
  transition: opacity .3s, transform .3s;
  opacity: 1;
  z-index: 100;
}}

#controls.fade {{
  opacity: 0;
  transform: translateX(-50%) translateY(10px);
  pointer-events: none;
}}

#controls button {{
  width: 36px;
  height: 36px;
  border-radius: 8px;
  border: 1px solid rgba(0,0,0,0.08);
  background: #fff;
  color: var(--text-primary);
  cursor: pointer;
  display: inline-flex;
  align-items: center;
  justify-content: center;
  transition: all 0.15s ease;
}}

#controls button:hover:not(:disabled) {{
  background: var(--bg-gradient-to);
  border-color: var(--accent-primary);
  color: var(--accent-primary);
}}

#controls button:disabled {{
  opacity: 0.3;
  cursor: default;
}}

#counter {{
  font-family: var(--font-mono);
  font-size: 13px;
  font-variant-numeric: tabular-nums;
  color: var(--text-secondary);
  min-width: 60px;
  text-align: center;
}}

#hint {{
  position: fixed;
  right: 24px;
  bottom: 24px;
  font-size: 11px;
  color: rgba(0,0,0,0.3);
  letter-spacing: 0.05em;
}}
</style>
</head>
<body>
  <div id="stage-wrap">
    <div id="stage">
      {"".join(slides_html)}
    </div>
  </div>

  <div id="controls">
    <button id="prev" aria-label="Previous">
      <svg width="14" height="14" viewBox="0 0 14 14" fill="none"><path d="M9 3L5 7L9 11" stroke="currentColor" stroke-width="1.6" stroke-linecap="round" stroke-linejoin="round"/></svg>
    </button>
    <span id="counter">1 / 20</span>
    <button id="next" aria-label="Next">
      <svg width="14" height="14" viewBox="0 0 14 14" fill="none"><path d="M5 3L9 7L5 11" stroke="currentColor" stroke-width="1.6" stroke-linecap="round" stroke-linejoin="round"/></svg>
    </button>
    <button id="fs" aria-label="Fullscreen">
      <svg width="14" height="14" viewBox="0 0 14 14" fill="none"><path d="M2 5V2H5 M9 2H12V5 M12 9V12H9 M5 12H2V9" stroke="currentColor" stroke-width="1.5" stroke-linecap="round" stroke-linejoin="round"/></svg>
    </button>
  </div>
  <div id="hint">← → Space · F fullscreen · Esc exit</div>

<script>
(function() {{
  var TOTAL = 20;
  var idx = 0;
  var fadeTimer = null;
  var chartInitialized = {{}};

  {chart_init_js}

  var stage = document.getElementById("stage");
  var stageWrap = document.getElementById("stage-wrap");
  var counter = document.getElementById("counter");
  var prevBtn = document.getElementById("prev");
  var nextBtn = document.getElementById("next");
  var fsBtn = document.getElementById("fs");
  var controls = document.getElementById("controls");
  var hint = document.getElementById("hint");

  function fitStage() {{
    var rect = stageWrap.getBoundingClientRect();
    if (rect.width === 0 || rect.height === 0) return;
    var scale = Math.min(rect.width / 1280, rect.height / 720);
    stage.style.transform = "scale(" + scale + ")";
  }}

  function renderSlide() {{
    var pages = document.querySelectorAll(".slide-page");
    for (var i = 0; i < pages.length; i++) {{
      pages[i].style.display = "none";
    }}
    var slideNum = idx + 1;
    var current = document.getElementById("slide-" + slideNum);
    if (current) {{
      current.style.display = "block";
    }}

    if (typeof slideChartScripts !== 'undefined' && slideChartScripts[slideNum]) {{
      if (typeof Chart !== 'undefined') {{
        Object.keys(Chart.instances).forEach(function(key) {{
          Chart.instances[key].destroy();
        }});
      }}
      requestAnimationFrame(function() {{
        setTimeout(function() {{
          try {{
            slideChartScripts[slideNum]();
            chartInitialized[slideNum] = true;
          }} catch(e) {{
            console.warn('Chart init failed:', e);
          }}
        }}, 100);
      }});
    }}

    counter.textContent = slideNum + " / " + TOTAL;
    prevBtn.disabled = idx <= 0;
    nextBtn.disabled = idx >= TOTAL - 1;
  }}

  function go(delta) {{
    var next = idx + delta;
    if (next < 0 || next >= TOTAL) return false;
    idx = next;
    renderSlide();
    return true;
  }}

  function jumpTo(n) {{
    if (n < 1 || n > TOTAL) return false;
    idx = n - 1;
    renderSlide();
    return true;
  }}

  function toggleFullscreen() {{
    if (document.fullscreenElement) {{
      document.exitFullscreen();
    }} else {{
      document.documentElement.requestFullscreen().catch(function() {{}});
    }}
  }}

  function showControls() {{
    controls.classList.remove("fade");
    hint.style.opacity = "1";
    if (fadeTimer) clearTimeout(fadeTimer);
    fadeTimer = setTimeout(function() {{
      controls.classList.add("fade");
      hint.style.opacity = "0";
    }}, 2500);
  }}

  prevBtn.addEventListener("click", function() {{ go(-1); }});
  nextBtn.addEventListener("click", function() {{ go(1); }});
  fsBtn.addEventListener("click", function() {{ toggleFullscreen(); }});

  document.addEventListener("keydown", function(e) {{
    if (e.key === "ArrowLeft" || e.key === "PageUp") {{ if (go(-1)) e.preventDefault(); }}
    else if (e.key === "ArrowRight" || e.key === "PageDown" || e.key === " ") {{ if (go(1)) e.preventDefault(); }}
    else if (e.key === "Home") {{ if (jumpTo(1)) e.preventDefault(); }}
    else if (e.key === "End") {{ if (jumpTo(TOTAL)) e.preventDefault(); }}
    else if (e.key === "f" || e.key === "F") {{ toggleFullscreen(); e.preventDefault(); }}
    else if (e.key === "Escape") {{ if (document.fullscreenElement) document.exitFullscreen(); }}
    else if (/^[0-9]$/.test(e.key)) {{
      var n = parseInt(e.key, 10);
      if (n >= 1 && jumpTo(n)) e.preventDefault();
    }}
  }});

  document.addEventListener("mousemove", function(e) {{
    if (e.clientY > window.innerHeight - 150) showControls();
  }});

  if (typeof ResizeObserver !== "undefined") {{
    new ResizeObserver(fitStage).observe(stageWrap);
  }} else {{
    window.addEventListener("resize", fitStage);
  }}

  fitStage();
  renderSlide();
  showControls();
}})();
</script>
</body>
</html>"""

    # Save combined preview
    with open(os.path.join(BASE_DIR, "ppt-output", "preview.html"), 'w', encoding='utf-8') as f:
        f.write(combined_html)

    print(f"\nDone! Output saved to ppt-output/")
    print(f"  - preview.html (combined preview)")
    print(f"  - slides/ (individual slides)")
    print(f"  - style.json (style definition)")

if __name__ == "__main__":
    main()
