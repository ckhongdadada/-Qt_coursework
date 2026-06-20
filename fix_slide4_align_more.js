const fs = require('fs');
let html = fs.readFileSync('C:/Users/28414/Desktop/低标注预算下长尾分布的主动学习与半监督学习联合策略-20260617-131815/index_standalone.html', 'utf8');

// Find the section for Slide 4
let slideStartIndex = html.indexOf('实验设置与数据构造');
let slideEndIndex = html.indexOf('<!-- Hexagon decorations -->', slideStartIndex);

if (slideStartIndex !== -1 && slideEndIndex !== -1) {
  let slideHtml = html.substring(slideStartIndex, slideEndIndex);

  // Fix the missed text-align: right
  slideHtml = slideHtml.replace(/<div style="text-align: right; font-weight: bold;">\$n_{\\max}\$:/g, '<div style="text-align: left; font-weight: bold; padding-left: 12px;">$n_{\\max}$:');
  
  // Just in case, replace any remaining text-align: right inside the formula block
  slideHtml = slideHtml.replace(/text-align: right;/g, 'text-align: left; padding-left: 12px;');

  // Make sure the formula is clearly indented to align nicely
  slideHtml = slideHtml.replace(/<div class="formula" style="font-size: 22px; text-align: left; margin-bottom: 16px; margin-left: 12px; color: #0F172A;">/, 
                                '<div class="formula" style="font-size: 22px; text-align: left; margin-bottom: 16px; padding-left: 12px; color: #0F172A;">');

  html = html.substring(0, slideStartIndex) + slideHtml + html.substring(slideEndIndex);
  fs.writeFileSync('C:/Users/28414/Desktop/低标注预算下长尾分布的主动学习与半监督学习联合策略-20260617-131815/index_standalone.html', html, 'utf8');
  console.log('Fixed alignment.');
} else {
  console.log('Could not find slide.');
}
