const fs = require('fs');
let html = fs.readFileSync('C:/Users/28414/Desktop/低标注预算下长尾分布的主动学习与半监督学习联合策略-20260617-131815/index_standalone.html', 'utf8');

// 1. Remove .flow-return
html = html.replace(/<div class="flow-return">[\s\S]*?<\/div>\s*<\/div>/, '');

// 2. Make formula-title bold and darker
html = html.replace(/\.slide-12 \.formula-title \{\s*font-size: 17px;\s*font-weight: 500;\s*color: #475569;/, '.slide-12 .formula-title {\n  font-size: 17px;\n  font-weight: 700;\n  color: #0F172A;');

// Fallback if font-size was not exactly 17px or something
html = html.replace(/\.slide-12 \.formula-title \{([^}]+)\}/, (match, inner) => {
  let newInner = inner.replace(/font-weight:\s*500/, 'font-weight: 700').replace(/color:\s*#475569/, 'color: #0F172A');
  return '.slide-12 .formula-title {' + newInner + '}';
});

// 3. Make core-idea-label bold
html = html.replace(/\.slide-12 \.core-idea-label \{([^}]+)\}/, (match, inner) => {
  let newInner = inner.replace(/font-weight:\s*500/, 'font-weight: 700');
  return '.slide-12 .core-idea-label {' + newInner + '}';
});

fs.writeFileSync('C:/Users/28414/Desktop/低标注预算下长尾分布的主动学习与半监督学习联合策略-20260617-131815/index_standalone.html', html, 'utf8');
console.log('Removed flow-return and bolded concepts.');
