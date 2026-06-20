const fs = require('fs');
let html = fs.readFileSync('C:/Users/28414/Desktop/低标注预算下长尾分布的主动学习与半监督学习联合策略-20260617-131815/index_standalone.html', 'utf8');

// Find the section for Slide 4
let slideStartIndex = html.indexOf('实验设置与数据构造');
let slideEndIndex = html.indexOf('<!-- Hexagon decorations -->', slideStartIndex);

if (slideStartIndex !== -1 && slideEndIndex !== -1) {
  let slideHtml = html.substring(slideStartIndex, slideEndIndex);

  // 1. Remove Emojis
  slideHtml = slideHtml.replace(/📊 /g, '');
  slideHtml = slideHtml.replace(/💰 /g, '');
  slideHtml = slideHtml.replace(/📈 /g, '');
  slideHtml = slideHtml.replace(/<span>⚠️<\/span>/g, '<span style="font-weight: bold; color: #D97706;">注：</span>');
  
  // Also clean up the padding for the alert if we removed the big emoji icon
  slideHtml = slideHtml.replace(/<div class="table-note" style="(.*?)">/, '<div class="table-note" style="$1">');

  // 2. Replace the formula block
  const oldFormulaBlockRegex = /<div class="formula-block">[\s\S]*?<\/div>\s*<!-- Annotation Budget -->/;
  
  const newFormulaBlock = `<div class="formula-block" style="background: #F8FAFC; border-radius: 8px; border: 1px solid #E2E8F0; padding: 16px; margin-bottom: 24px;">
        <div class="formula" style="font-size: 22px; text-align: center; margin-bottom: 16px; color: #0F172A;">
          $n_c = n_{\\max} \\cdot \\rho^{-c/(C-1)}$
        </div>
        <div style="display: grid; grid-template-columns: auto 1fr; gap: 6px 12px; font-size: 14px; color: #475569; margin-bottom: 16px; align-items: center;">
          <div style="text-align: right; font-weight: bold;">$n_c$:</div><div>类别 $c$ 的样本数</div>
          <div style="text-align: right; font-weight: bold;">$n_{\\max}$:</div><div>头类样本数</div>
          <div style="text-align: right; font-weight: bold;">$\\rho$:</div><div>长尾不平衡率 (Imbalance Factor)</div>
          <div style="text-align: right; font-weight: bold;">$C$:</div><div>总类别数 (此处 $C=10$)</div>
          <div style="text-align: right; font-weight: bold;">$c$:</div><div>类别索引 (从头类到尾类，升序排列)</div>
        </div>
        <div style="margin-top: 12px; padding-top: 12px; border-top: 1px dashed #CBD5E1;">
          <div style="font-size: 15px; font-weight: bold; color: #0F172A; margin-bottom: 6px;">分布谱系设计 — $\\rho \\in \\{1, 5, 10, 20, 50, 100\\}$</div>
          <ul style="font-size: 14px; color: #475569; margin: 0 0 0 20px; padding: 0; line-height: 1.5;">
            <li style="margin-bottom: 4px;">覆盖从均衡到极端长尾的完整谱系</li>
            <li>系统考察策略在不同不平衡程度下的表现</li>
          </ul>
        </div>
      </div>
      <!-- Annotation Budget -->`;
  
  slideHtml = slideHtml.replace(oldFormulaBlockRegex, newFormulaBlock);

  // Write back
  html = html.substring(0, slideStartIndex) + slideHtml + html.substring(slideEndIndex);
  fs.writeFileSync('C:/Users/28414/Desktop/低标注预算下长尾分布的主动学习与半监督学习联合策略-20260617-131815/index_standalone.html', html, 'utf8');
  console.log('Fixed slide 4 layout and formula.');
} else {
  console.log('Could not find slide 4 section.');
}
