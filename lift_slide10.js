const fs = require('fs');
let html = fs.readFileSync('C:/Users/28414/Desktop/低标注预算下长尾分布的主动学习与半监督学习联合策略-20260617-131815/index_standalone.html', 'utf8');

// Add margin-bottom to .flow-cycle to lift the right column slightly
html = html.replace(/\.slide-12 \.flow-cycle\s*\{([^}]+)\}/, (match, inner) => {
  if (!inner.includes('margin-bottom')) {
    inner = inner + '\n  margin-bottom: 10px;';
  } else {
    inner = inner.replace(/margin-bottom:\s*[^;]+;/, 'margin-bottom: 10px;');
  }
  return '.slide-12 .flow-cycle {' + inner + '}';
});

fs.writeFileSync('C:/Users/28414/Desktop/低标注预算下长尾分布的主动学习与半监督学习联合策略-20260617-131815/index_standalone.html', html, 'utf8');
console.log('Added margin-bottom to .flow-cycle to align text.');
