const fs = require('fs');
let html = fs.readFileSync('C:/Users/28414/Desktop/低标注预算下长尾分布的主动学习与半监督学习联合策略-20260617-131815/index_standalone.html', 'utf8');

// 1. Update .flow-heading to match .formula-title
html = html.replace(/\.slide-12 \.flow-heading\s*\{[^}]+\}/, `.slide-12 .flow-heading {
  font-size: 17px;
  font-weight: 700;
  color: #0F172A;
  margin-bottom: 12px;
  line-height: 1.3;
  flex-shrink: 0;
}`);

// 2. Update .flow-connector to stretch and evenly space the right column
html = html.replace(/\.slide-12 \.flow-connector\s*\{([^}]+)\}/, (match, inner) => {
  // If flex: 1 is not there, add it
  if (!inner.includes('flex: 1')) {
    inner = inner + '\n  flex: 1;\n  align-items: center;';
  }
  return '.slide-12 .flow-connector {' + inner + '}';
});

// Also make sure .flow-step doesn't have too much static padding to prevent stretching
html = html.replace(/\.slide-12 \.flow-step\s*\{([^}]+)\}/, (match, inner) => {
  return '.slide-12 .flow-step {' + inner.replace(/padding:\s*4px\s*0;/, 'padding: 0;') + '}';
});

fs.writeFileSync('C:/Users/28414/Desktop/低标注预算下长尾分布的主动学习与半监督学习联合策略-20260617-131815/index_standalone.html', html, 'utf8');
console.log('Aligned left and right columns on Slide 10.');
