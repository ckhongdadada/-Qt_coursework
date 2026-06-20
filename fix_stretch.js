const fs = require('fs');
let html = fs.readFileSync('C:/Users/28414/Desktop/低标注预算下长尾分布的主动学习与半监督学习联合策略-20260617-131815/index_standalone.html', 'utf8');

// Remove flex: 1 and min-height: 0 from .card-body to stop it from stretching to the bottom of the card
html = html.replace(/\.slide-12 \.card-body\s*\{([^}]+)\}/, (match, inner) => {
  inner = inner.replace(/\s*flex:\s*1;/, '');
  inner = inner.replace(/\s*min-height:\s*0;/, '');
  return '.slide-12 .card-body {' + inner + '}';
});

fs.writeFileSync('C:/Users/28414/Desktop/低标注预算下长尾分布的主动学习与半监督学习联合策略-20260617-131815/index_standalone.html', html, 'utf8');
console.log('Fixed card-body stretching.');
