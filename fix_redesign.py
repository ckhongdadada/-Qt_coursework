#!/usr/bin/env python3
"""
Fix redesign: Preserve original styles while adding ppt-agent-skill improvements.
"""

import re
import os
import json

BASE_DIR = os.path.dirname(os.path.abspath(__file__))
SLIDES_DIR = os.path.join(BASE_DIR, "slides")
OUTPUT_DIR = os.path.join(BASE_DIR, "ppt-output", "slides")

# Load style definition
STYLE_FILE = os.path.join(BASE_DIR, "ppt-output", "style.json")
with open(STYLE_FILE, 'r', encoding='utf-8') as f:
    STYLE = json.load(f)

def extract_styles(html_content):
    """Extract all <style> block contents."""
    styles = re.findall(r'<style[^>]*>(.*?)</style>', html_content, re.DOTALL)
    return "\n".join(styles)

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
    h1_match = re.search(r'<h1[^>]*>(.*?)</h1>', html_content, re.DOTALL)
    if h1_match:
        title = re.sub(r'<[^>]+>', '', h1_match.group(1)).strip()
        return title
    title_match = re.search(r'class="[^"]*slide-title[^"]*"[^>]*>(.*?)</', html_content, re.DOTALL)
    if title_match:
        return re.sub(r'<[^>]+>', '', title_match.group(1)).strip()
    title_match = re.search(r'class="[^"]*page-title[^"]*"[^>]*>(.*?)</', html_content, re.DOTALL)
    if title_match:
        return re.sub(r'<[^>]+>', '', title_match.group(1)).strip()
    return ""

def get_slide_label(html_content):
    """Extract slide label/category."""
    label_match = re.search(r'class="[^"]*(?:tag-label|header-label|page-label|section-label)[^"]*"[^>]*>(.*?)</', html_content, re.DOTALL)
    if label_match:
        return re.sub(r'<[^>]+>', '', label_match.group(1)).strip()
    return ""

# Global enhancements from ppt-agent-skill
GLOBAL_ENHANCEMENTS = """
/* === PPT Agent Skill Enhancements === */

/* Typography improvements */
* {
  font-feature-settings: 'kern', 'liga', 'calt', 'ss01', 'cv11', 'ccmp';
  text-rendering: optimizeLegibility;
  -webkit-font-smoothing: antialiased;
  -moz-osx-font-smoothing: grayscale;
}

/* Tabular numbers for data */
.data-number,
.table-cell,
.metric-chip,
.big-number,
.data-table td,
.data-table th {
  font-variant-numeric: tabular-nums proportional-nums;
}

/* Improved letter spacing */
.page-label,
.tag-label,
.header-label,
.section-label {
  letter-spacing: 0.22em;
}

/* Subtle hover effects for cards */
.card,
.card-item,
.pipeline-card,
.info-block {
  transition: transform 0.2s ease, box-shadow 0.2s ease;
}

.card:hover,
.card-item:hover,
.pipeline-card:hover,
.info-block:hover {
  transform: translateY(-2px);
  box-shadow: 0 8px 24px rgba(0,0,0,0.04);
}

/* Pulse animation for labels */
@keyframes pulse {
  0%, 100% { opacity: 1; transform: scale(1); }
  50% { opacity: 0.5; transform: scale(0.8); }
}

/* Corner decoration lines */
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
  background: #2563EB;
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

/* Inner frame decoration */
.inner-frame {
  position: absolute;
  inset: 16px;
  border: 1px solid rgba(10,29,58,0.06);
  border-radius: 4px;
  pointer-events: none;
  z-index: 100;
}
"""

def main():
    os.makedirs(OUTPUT_DIR, exist_ok=True)

    slides_html = []
    all_styles = []
    chart_scripts = {}

    for i in range(1, 21):
        slide_file = os.path.join(SLIDES_DIR, f"{i}.html")
        print(f"Processing slide {i}...")

        with open(slide_file, 'r', encoding='utf-8') as f:
            content = f.read()

        # Extract components
        original_styles = extract_styles(content)
        body = extract_body_content(content)
        title = get_slide_title(content)
        label = get_slide_label(content)

        # Extract chart scripts
        chart_script = extract_chart_scripts(content)
        if chart_script:
            chart_scripts[i] = chart_script

        # Remove @font-face from original styles (we'll add globally)
        cleaned_styles = re.sub(r'@font-face\s*\{[^}]+\}', '', original_styles)
        # Remove base reset (we'll add globally)
        cleaned_styles = re.sub(r'\*\s*\{[^}]+\}', '', cleaned_styles)
        cleaned_styles = cleaned_styles.strip()

        if cleaned_styles:
            all_styles.append(f"/* === Slide {i} Original Styles === */\n{cleaned_styles}")

        # Add corner lines to body
        body_with_decoration = body + '\n  <div class="corner-lines"></div>'

        slides_html.append(f'<section id="slide-{i}" class="slide-page">\n{body_with_decoration}\n</section>')

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
/* Global Reset */
* {{ margin: 0; padding: 0; box-sizing: border-box; }}

{original_styles}

{GLOBAL_ENHANCEMENTS}

/* Slide container */
.slide-page {{
  width: 1280px;
  height: 720px;
  position: relative;
  overflow: hidden;
}}
</style>
</head>
<body>
<section class="slide-page">
{body_with_decoration}
</section>
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
/* Global Reset */
* {{ margin: 0; padding: 0; box-sizing: border-box; }}

/* Combined Original Styles */
{chr(10).join(all_styles)}

/* Global Enhancements */
{GLOBAL_ENHANCEMENTS}

/* Preview Container */
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
  color: #0a1d3a;
  cursor: pointer;
  display: inline-flex;
  align-items: center;
  justify-content: center;
  transition: all 0.15s ease;
}}

#controls button:hover:not(:disabled) {{
  background: #f6f8fb;
  border-color: #2563EB;
  color: #2563EB;
}}

#controls button:disabled {{
  opacity: 0.3;
  cursor: default;
}}

#counter {{
  font-family: 'SF Mono', 'JetBrains Mono', monospace;
  font-size: 13px;
  font-variant-numeric: tabular-nums;
  color: #64748b;
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
