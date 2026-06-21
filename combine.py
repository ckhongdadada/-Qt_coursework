#!/usr/bin/env python3
"""Combine all 20 slide HTML files into a single HTML without iframes.
Uses per-slide style toggling to avoid CSS conflicts."""

import re
import os

BASE_DIR = os.path.dirname(os.path.abspath(__file__))
SLIDES_DIR = os.path.join(BASE_DIR, "slides")
OUTPUT_FILE = os.path.join(BASE_DIR, "combined.html")


def extract_all_styles(html_content):
    """Extract all <style> block contents."""
    styles = re.findall(r'<style[^>]*>(.*?)</style>', html_content, re.DOTALL)
    combined = "\n".join(styles)
    return combined.strip()


def extract_body_content(html_content):
    """Extract content between <body> and </body>, excluding scripts."""
    body_match = re.search(r'<body[^>]*>(.*?)</body>', html_content, re.DOTALL)
    if not body_match:
        return ""
    body = body_match.group(1)
    # Remove script tags
    body = re.sub(r'<script[^>]*>.*?</script>', '', body, flags=re.DOTALL)
    return body.strip()


def has_chart_js(html_content):
    """Check if the slide uses Chart.js."""
    return 'chart.js' in html_content.lower() or 'new Chart' in html_content


def extract_chart_scripts(html_content):
    """Extract inline <script> blocks that create Chart.js charts."""
    # Find all script blocks (not src= external ones, not __qw ones)
    all_scripts = re.findall(r'<script(?:\s[^>]*)?>(.*?)</script>', html_content, re.DOTALL)
    chart_scripts = []
    for s in all_scripts:
        s_stripped = s.strip()
        if not s_stripped:
            continue
        if '__qw_' in s_stripped:
            continue
        if 'new Chart' in s_stripped:
            chart_scripts.append(s_stripped)
    return "\n".join(chart_scripts)


def fix_image_paths(body, slide_num):
    """Fix file:/// image paths to use relative paths if possible."""
    # Replace file:/// URLs with relative paths where possible
    def replace_path(match):
        original = match.group(0)
        url = match.group(1)
        # If it's a file:/// URL, try to extract just the filename
        if 'file:///' in url:
            # Extract filename from the URL (handle URL encoding)
            filename = url.split('/')[-1]
            # Decode URL encoding
            import urllib.parse
            filename = urllib.parse.unquote(filename)
            # Check if image exists in current directory or assets subdirectory
            local_path = os.path.join(BASE_DIR, filename)
            assets_path = os.path.join(BASE_DIR, 'assets', filename)
            if os.path.exists(local_path):
                return f'src="{filename}"'
            elif os.path.exists(assets_path):
                return f'src="assets/{filename}"'
            # Otherwise keep original but warn
            print(f"  Warning: Image not found locally: {filename}")
        return original

    # Match src="..." patterns
    result = re.sub(r'src="([^"]+)"', replace_path, body)
    return result


def main():
    slides_html = []
    slides_styles = []
    slides_chart_scripts = {}

    for i in range(1, 21):
        slide_file = os.path.join(SLIDES_DIR, f"{i}.html")
        print(f"Processing slide {i}...")

        with open(slide_file, 'r', encoding='utf-8') as f:
            content = f.read()

        # Extract CSS
        css = extract_all_styles(content)
        slides_styles.append(css)

        # Extract body content
        body = extract_body_content(content)

        # Fix image paths
        body = fix_image_paths(body, i)

        # Extract Chart.js scripts if present
        if has_chart_js(content):
            chart_script = extract_chart_scripts(content)
            if chart_script.strip():
                slides_chart_scripts[i] = chart_script

        # Wrap in a section
        display = "block" if i == 1 else "none"
        section = f'<section id="slide-{i}" class="slide-page" style="display:{display};">\n{body}\n</section>'
        slides_html.append(section)

    # Build style tags for each slide
    style_tags = []
    for i, css in enumerate(slides_styles, 1):
        if css.strip():
            # Remove @font-face (shared globally) and base reset
            cleaned = re.sub(r'@font-face\s*\{[^}]+\}', '', css)
            cleaned = re.sub(r'\*\s*\{[^}]+\}', '', cleaned)
            # Remove link tags to katex CSS (we'll load it globally)
            cleaned = re.sub(r'<link[^>]*katex[^>]*>', '', cleaned)
            # Replace "body" selector with "#slide-N" so styles apply to our wrapper
            # Handle various body selector patterns
            cleaned = re.sub(r'(?<![.\w#])body(?=\s*\{)', f'#slide-{i}', cleaned)
            cleaned = re.sub(r'(?<![.\w#])body(?=\s*,)', f'#slide-{i}', cleaned)
            cleaned = cleaned.strip()
            if cleaned:
                style_tags.append(f'<style id="style-slide-{i}" data-slide="{i}">\n{cleaned}\n</style>')

    # Build Chart.js re-init scripts
    chart_init_js = ""
    if slides_chart_scripts:
        chart_init_js = "var slideChartScripts = {\n"
        for i, script in slides_chart_scripts.items():
            # Wrap each chart script in a function
            chart_init_js += f"  {i}: function() {{\n{script}\n  }},\n"
        chart_init_js += "};\n"

    html = f"""<!DOCTYPE html>
<html lang="zh-CN">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>低标注预算下长尾分布的主动学习与半监督学习联合策略</title>
<link rel="stylesheet" href="https://cdn.jsdelivr.net/npm/katex@0.16.9/dist/katex.min.css">
<script src="https://cdn.jsdelivr.net/npm/katex@0.16.9/dist/katex.min.js"></script>
<script src="https://cdn.jsdelivr.net/npm/katex@0.16.9/dist/contrib/auto-render.min.js"></script>
<script src="https://cdn.jsdelivr.net/npm/chart.js@3.9.1/dist/chart.min.js"></script>
<style>
@font-face {{ font-family: 'Alibaba PuHuiTi 3.0'; src: url('file:///C%3A/Users/28414/.qoderwork/assets/ai-slides/fonts/AlibabaPuHuiTi-3-55-Regular.ttf') format('truetype'); font-weight: 400; font-style: normal; font-display: swap; }}
@font-face {{ font-family: 'Alibaba PuHuiTi 3.0'; src: url('file:///C%3A/Users/28414/.qoderwork/assets/ai-slides/fonts/AlibabaPuHuiTi-3-65-Medium.ttf') format('truetype'); font-weight: 500; font-style: normal; font-display: swap; }}
@font-face {{ font-family: 'Alibaba PuHuiTi 3.0'; src: url('file:///C%3A/Users/28414/.qoderwork/assets/ai-slides/fonts/AlibabaPuHuiTi-3-85-Bold.ttf') format('truetype'); font-weight: 700; font-style: normal; font-display: swap; }}

* {{ margin: 0; padding: 0; box-sizing: border-box; }}
html, body {{
  width: 100%;
  height: 100%;
  overflow: hidden;
  background: #000;
  font-family: 'Alibaba PuHuiTi 3.0', sans-serif;
}}

#stage-wrap {{
  position: fixed;
  inset: 0;
  display: flex;
  align-items: center;
  justify-content: center;
}}

#stage {{
  width: 1280px;
  height: 720px;
  transform-origin: center center;
  background: #fff;
  box-shadow: 0 12px 40px rgba(0,0,0,.6);
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
  font-family: 'Alibaba PuHuiTi 3.0', sans-serif;
  color: #0F172A;
}}

/* Don't override slide internal backgrounds - let them control their own */
.slide-page > * {{
  /* Allow child elements to set their own backgrounds */
}}

/* Controls */
#controls {{
  position: fixed;
  left: 50%;
  bottom: 12px;
  transform: translateX(-50%);
  display: flex;
  align-items: center;
  gap: 12px;
  padding: 8px 14px;
  background: rgba(20,20,20,.78);
  border: 1px solid rgba(255,255,255,.12);
  border-radius: 999px;
  backdrop-filter: blur(12px);
  transition: opacity .25s;
  opacity: 1;
  z-index: 100;
}}

#controls.fade {{ opacity: 0; pointer-events: none; }}

#controls button {{
  width: 30px;
  height: 30px;
  border-radius: 50%;
  border: 0;
  background: rgba(255,255,255,.1);
  color: #fff;
  cursor: pointer;
  display: inline-flex;
  align-items: center;
  justify-content: center;
  font-size: 14px;
  line-height: 1;
  padding: 0;
}}

#controls button:hover:not(:disabled) {{ background: rgba(255,255,255,.2); }}
#controls button:disabled {{ opacity: .35; cursor: default; }}
#controls button.active {{ background: rgba(255,255,255,.32); }}
#controls button svg {{ display: block; }}

#counter {{
  font-variant-numeric: tabular-nums;
  font-size: 13px;
  min-width: 64px;
  text-align: center;
  user-select: none;
  color: #fff;
}}

#hint {{
  position: fixed;
  right: 16px;
  bottom: 16px;
  font-size: 11px;
  color: rgba(255,255,255,.45);
  user-select: none;
  pointer-events: none;
}}

#hint.fade {{ opacity: 0; }}
</style>

{chr(10).join(style_tags)}

</head>
<body>
  <div id="stage-wrap">
    <div id="stage">
      {chr(10).join(slides_html)}
    </div>
  </div>

  <div id="controls" role="toolbar" aria-label="Presenter controls">
    <button id="prev" type="button" aria-label="Previous">
      <svg width="14" height="14" viewBox="0 0 14 14" fill="none"><path d="M9 3L5 7L9 11" stroke="currentColor" stroke-width="1.6" stroke-linecap="round" stroke-linejoin="round"/></svg>
    </button>
    <span id="counter">1 / 20</span>
    <button id="next" type="button" aria-label="Next">
      <svg width="14" height="14" viewBox="0 0 14 14" fill="none"><path d="M5 3L9 7L5 11" stroke="currentColor" stroke-width="1.6" stroke-linecap="round" stroke-linejoin="round"/></svg>
    </button>
    <button id="fs" type="button" aria-label="Fullscreen">
      <svg width="14" height="14" viewBox="0 0 14 14" fill="none"><path d="M2 5V2H5 M9 2H12V5 M12 9V12H9 M5 12H2V9" stroke="currentColor" stroke-width="1.5" stroke-linecap="round" stroke-linejoin="round"/></svg>
    </button>
  </div>
  <div id="hint">&larr; &rarr; Space &middot; F fullscreen &middot; Esc exit</div>

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

  function enableSlideStyles(slideNum) {{
    // Disable all slide styles
    var allStyles = document.querySelectorAll('style[data-slide]');
    for (var i = 0; i < allStyles.length; i++) {{
      allStyles[i].disabled = true;
    }}
    // Enable current slide's style
    var currentStyle = document.getElementById('style-slide-' + slideNum);
    if (currentStyle) {{
      currentStyle.disabled = false;
    }}
  }}

  function renderSlide() {{
    // Hide all slides
    var pages = document.querySelectorAll(".slide-page");
    for (var i = 0; i < pages.length; i++) {{
      pages[i].style.display = "none";
    }}
    // Show current slide
    var slideNum = idx + 1;
    var current = document.getElementById("slide-" + slideNum);
    if (current) {{
      current.style.display = "block";
    }}

    // Toggle styles
    enableSlideStyles(slideNum);

    // Initialize Chart.js if needed - MUST be after display:block
    if (typeof slideChartScripts !== 'undefined' && slideChartScripts[slideNum]) {{
      // Destroy ALL existing charts first
      if (typeof Chart !== 'undefined') {{
        Object.keys(Chart.instances).forEach(function(key) {{
          Chart.instances[key].destroy();
        }});
      }}
      // Wait for layout to complete, then init chart
      requestAnimationFrame(function() {{
        setTimeout(function() {{
          try {{
            slideChartScripts[slideNum]();
            chartInitialized[slideNum] = true;
          }} catch(e) {{
            console.warn('Chart init failed for slide ' + slideNum, e);
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
      if (document.exitFullscreen) document.exitFullscreen();
    }} else {{
      var root = document.documentElement;
      if (root && root.requestFullscreen) {{
        var p = root.requestFullscreen();
        if (p && typeof p.catch === "function") p.catch(function() {{}});
      }}
    }}
  }}

  function showControls() {{
    controls.classList.remove("fade");
    hint.classList.remove("fade");
    if (fadeTimer) clearTimeout(fadeTimer);
    fadeTimer = setTimeout(function() {{
      controls.classList.add("fade");
      hint.classList.add("fade");
    }}, 2000);
  }}

  prevBtn.addEventListener("click", function() {{ go(-1); }});
  nextBtn.addEventListener("click", function() {{ go(1); }});
  fsBtn.addEventListener("click", function() {{ toggleFullscreen(); }});

  document.addEventListener("fullscreenchange", function() {{
    if (document.fullscreenElement) {{
      fsBtn.classList.add("active");
    }} else {{
      fsBtn.classList.remove("active");
    }}
  }});

  function handleKeydown(e) {{
    if (e.key === "ArrowLeft" || e.key === "PageUp") {{ if (go(-1)) e.preventDefault(); }}
    else if (e.key === "ArrowRight" || e.key === "PageDown" || e.key === " ") {{ if (go(1)) e.preventDefault(); }}
    else if (e.key === "Home") {{ if (jumpTo(1)) e.preventDefault(); }}
    else if (e.key === "End") {{ if (jumpTo(TOTAL)) e.preventDefault(); }}
    else if (e.key === "f" || e.key === "F") {{ toggleFullscreen(); e.preventDefault(); }}
    else if (e.key === "Escape") {{ if (document.fullscreenElement && document.exitFullscreen) document.exitFullscreen(); }}
    else if (/^[0-9]$/.test(e.key)) {{
      var n = parseInt(e.key, 10);
      if (n >= 1 && jumpTo(n)) e.preventDefault();
    }}
  }}
  document.addEventListener("keydown", handleKeydown);

  function maybeWakeControls(e) {{
    if (!e || typeof e.clientY !== "number") return;
    if (e.clientY > window.innerHeight - 200) showControls();
  }}
  document.addEventListener("mousemove", maybeWakeControls);

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

    with open(OUTPUT_FILE, 'w', encoding='utf-8') as f:
        f.write(html)

    print(f"\nDone! Combined HTML written to: {OUTPUT_FILE}")
    print(f"Total slides: 20")
    print(f"Slides with Chart.js: {list(slides_chart_scripts.keys())}")


if __name__ == "__main__":
    main()
