const fs = require('fs');
let html = fs.readFileSync('C:/Users/28414/Desktop/低标注预算下长尾分布的主动学习与半监督学习联合策略-20260617-131815/index_standalone.html', 'utf8');

// Find the section for Slide 4
let slideStartIndex = html.indexOf('实验设置与数据构造');
let slideEndIndex = html.indexOf('<!-- Hexagon decorations -->', slideStartIndex);

if (slideStartIndex !== -1 && slideEndIndex !== -1) {
  let slideHtml = html.substring(slideStartIndex, slideEndIndex);

  // Replace the formula-block inline style to override width: fit-content and margin: 0 auto
  slideHtml = slideHtml.replace(/<div class="formula-block" style="background: #F8FAFC; border-radius: 8px; border: 1px solid #E2E8F0; padding: 16px; margin-bottom: 24px;">/, 
                                '<div class="formula-block" style="background: #F8FAFC; border-radius: 8px; border: 1px solid #E2E8F0; padding: 16px; margin-bottom: 24px; width: 100%; box-sizing: border-box; margin-left: 0; margin-right: 0;">');

  html = html.substring(0, slideStartIndex) + slideHtml + html.substring(slideEndIndex);
  fs.writeFileSync('C:/Users/28414/Desktop/低标注预算下长尾分布的主动学习与半监督学习联合策略-20260617-131815/index_standalone.html', html, 'utf8');
  console.log('Fixed grey box stretching.');
} else {
  console.log('Could not find slide.');
}
