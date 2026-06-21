const fs = require('fs');
let html = fs.readFileSync('C:/Users/28414/Desktop/低标注预算下长尾分布的主动学习与半监督学习联合策略-20260617-131815/index_standalone.html', 'utf8');

// Find the section for Slide 4
let slideStartIndex = html.indexOf('实验设置与数据构造');
let slideEndIndex = html.indexOf('<!-- Hexagon decorations -->', slideStartIndex);

if (slideStartIndex !== -1 && slideEndIndex !== -1) {
  let slideHtml = html.substring(slideStartIndex, slideEndIndex);

  // 1. Force left-align on the 2nd column values of the grid
  slideHtml = slideHtml.replace(/<div>类别 \$c\$ 的样本数<\/div>/, '<div style="text-align: left;">类别 $c$ 的样本数</div>');
  slideHtml = slideHtml.replace(/<div>头类样本数<\/div>/, '<div style="text-align: left;">头类样本数</div>');
  slideHtml = slideHtml.replace(/<div>长尾不平衡率 \(Imbalance Factor\)<\/div>/, '<div style="text-align: left;">长尾不平衡率 (Imbalance Factor)</div>');
  slideHtml = slideHtml.replace(/<div>总类别数 \(此处 \$C=10\$\)<\/div>/, '<div style="text-align: left;">总类别数 (此处 $C=10$)</div>');
  slideHtml = slideHtml.replace(/<div>类别索引 \(从头类到尾类，升序排列\)<\/div>/, '<div style="text-align: left;">类别索引 (从头类到尾类，升序排列)</div>');

  // Add justify-content: start; justify-items: start; to the grid itself
  slideHtml = slideHtml.replace(/align-items: center;"/, 'align-items: center; justify-content: start; justify-items: start;"');

  // Force left align on "分布谱系设计"
  slideHtml = slideHtml.replace(/<div style="font-size: 15px; font-weight: bold; color: #0F172A; margin-bottom: 6px;">分布谱系设计/, 
                                '<div style="font-size: 15px; font-weight: bold; color: #0F172A; margin-bottom: 6px; text-align: left; padding-left: 12px;">分布谱系设计');
  
  // Force left align on the ul for 分布谱系设计
  slideHtml = slideHtml.replace(/<ul style="font-size: 14px; color: #475569; margin: 0 0 0 20px; padding: 0; line-height: 1\.5;">/,
                                '<ul style="font-size: 14px; color: #475569; margin: 0 0 0 32px; padding: 0; line-height: 1.5; text-align: left; list-style-position: outside;">');

  // 2. Add F1 formula and move text to the right
  const oldInfoSubRegex = /<div class="info-sub" style="margin-top:10px;">[\s\S]*?<\/ul>\s*<\/div>/;
  
  const newInfoSub = `<div style="display: grid; grid-template-columns: 1.2fr 1fr; gap: 16px; margin-top: 16px; align-items: start;">
          <div class="f1-formula" style="background: #F8FAFC; padding: 12px; border-radius: 8px; border: 1px solid #E2E8F0; text-align: center;">
            <div style="font-size: 16px; color: #0F172A; font-family: 'Times New Roman', serif; margin-bottom: 12px;">
              $F_1 = 2 \\cdot \\frac{Precision \\cdot Recall}{Precision + Recall}$
            </div>
            <div style="font-size: 16px; color: #0F172A; font-family: 'Times New Roman', serif;">
              $Macro\\text{-}F_1 = \\frac{1}{C} \\sum_{c} F_{1,c}$
            </div>
          </div>
          <div class="info-sub" style="text-align: left;">
            <strong style="color: #0F172A; font-size: 13px;">Macro-F1 为主指标，衡量类别均衡性能：</strong>
            <ul style="margin-left: 16px; margin-top: 6px; padding: 0; font-size: 12px; color: #64748B; line-height: 1.5;">
              <li style="margin-bottom: 4px;">F1 分数是精确率与召回率的调和平均</li>
              <li style="margin-bottom: 4px;">Macro-F1 为所有类别 F1 的算术平均</li>
              <li>客观反映模型在长尾小样本类别上的表现</li>
            </ul>
          </div>
        </div>`;

  slideHtml = slideHtml.replace(oldInfoSubRegex, newInfoSub);

  // Write back
  html = html.substring(0, slideStartIndex) + slideHtml + html.substring(slideEndIndex);
  fs.writeFileSync('C:/Users/28414/Desktop/低标注预算下长尾分布的主动学习与半监督学习联合策略-20260617-131815/index_standalone.html', html, 'utf8');
  console.log('Fixed alignments and added F1 formula.');
} else {
  console.log('Could not find slide.');
}
