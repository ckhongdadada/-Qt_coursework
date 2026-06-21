const fs = require('fs');
let html = fs.readFileSync('C:/Users/28414/Desktop/低标注预算下长尾分布的主动学习与半监督学习联合策略-20260617-131815/index_standalone.html', 'utf8');

// 1. Left align the formula block
html = html.replace(/<div class="formula" style="font-size: 22px; text-align: center; margin-bottom: 16px; color: #0F172A;">/, 
                    '<div class="formula" style="font-size: 22px; text-align: left; margin-bottom: 16px; margin-left: 12px; color: #0F172A;">');

html = html.replace(/<div style="text-align: right; font-weight: bold;">\$n_c\$:/, '<div style="text-align: left; font-weight: bold; padding-left: 12px;">$n_c$:');
html = html.replace(/<div style="text-align: right; font-weight: bold;">\$n_\\max\$:/, '<div style="text-align: left; font-weight: bold; padding-left: 12px;">$n_{\\max}$:');
html = html.replace(/<div style="text-align: right; font-weight: bold;">\$\\rho\$:/, '<div style="text-align: left; font-weight: bold; padding-left: 12px;">$\\rho$:');
html = html.replace(/<div style="text-align: right; font-weight: bold;">\$C\$:/, '<div style="text-align: left; font-weight: bold; padding-left: 12px;">$C$:');
html = html.replace(/<div style="text-align: right; font-weight: bold;">\$c\$:/, '<div style="text-align: left; font-weight: bold; padding-left: 12px;">$c$:');

// 2. Move Evaluation Metrics block
const evalBlockRegex = /<!-- Evaluation Metrics -->[\s\S]*?<\/div>\s*<\/div>\s*<\/div>\s*<!-- Right: Data Table -->/;
let evalBlockMatch = html.match(evalBlockRegex);

if (evalBlockMatch) {
  let fullMatch = evalBlockMatch[0];
  // Extract just the Evaluation Metrics div block
  let extractionRegex = /(<!-- Evaluation Metrics -->[\s\S]*?<\/div>\s*<\/div>)\s*<\/div>\s*<!-- Right: Data Table -->/;
  let parts = fullMatch.match(extractionRegex);
  
  if (parts && parts[1]) {
    let evalBlock = parts[1];
    
    // Remove the eval block from left-col
    html = html.replace(evalBlock, '');
    
    // Insert it into right-col, after table-container
    let insertTarget = /<\/div>\s*<\/div>\s*<\/div>\s*<!-- Hexagon decorations -->/;
    html = html.replace(insertTarget, `      </div>\n      ${evalBlock}\n    </div>\n  </div>\n\n  <!-- Hexagon decorations -->`);
    
    fs.writeFileSync('C:/Users/28414/Desktop/低标注预算下长尾分布的主动学习与半监督学习联合策略-20260617-131815/index_standalone.html', html, 'utf8');
    console.log('Successfully aligned left and moved eval metrics to right.');
  } else {
    console.log('Failed to parse eval block parts.');
  }
} else {
  console.log('Failed to match eval block.');
}
