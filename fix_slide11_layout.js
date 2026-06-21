const fs = require('fs');
let html = fs.readFileSync('C:/Users/28414/Desktop/低标注预算下长尾分布的主动学习与半监督学习联合策略-20260617-131815/index_standalone.html', 'utf8');

// 1. Remove grey colors from the baseline rows in Slide 11
html = html.replace(/<tr style="color: #64748B; background: #F8FAFC;">/g, '<tr>');
html = html.replace(/<td style="font-weight: 400; color: #64748B;">/g, '<td>');

// 2. Fix the layout of hero-row and hero-stat
html = html.replace(/\.hero-row\s*\{([^}]+)\}/, `.hero-row {
  display: flex;
  flex-direction: column;
  gap: 12px;
  margin-bottom: 10px;
}`);

html = html.replace(/\.hero-stat\s*\{([^}]+)\}/, `.hero-stat {
  display: flex;
  flex-direction: row;
  align-items: baseline;
  gap: 16px;
}`);

html = html.replace(/\.hero-number\s*\{([^}]+)\}/, (match, inner) => {
  // Replace font-size if needed, ensure white-space and min-width
  let newInner = inner.replace(/font-size:\s*[^;]+;/, 'font-size: 26px;');
  if (!newInner.includes('white-space:')) {
    newInner += '\n  white-space: nowrap;';
  }
  if (!newInner.includes('min-width:')) {
    newInner += '\n  min-width: 150px;';
  }
  return '.hero-number {' + newInner + '}';
});

fs.writeFileSync('C:/Users/28414/Desktop/低标注预算下长尾分布的主动学习与半监督学习联合策略-20260617-131815/index_standalone.html', html, 'utf8');
console.log('Fixed slide 11 fonts and layout.');
