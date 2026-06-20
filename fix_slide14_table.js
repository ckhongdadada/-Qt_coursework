const fs = require('fs');
let html = fs.readFileSync('C:/Users/28414/Desktop/低标注预算下长尾分布的主动学习与半监督学习联合策略-20260617-131815/index_standalone.html', 'utf8');

// The CSS rule for .slide-14 .col-name is:
// .slide-14 .col-name {
//     width: 104px;
// ...

html = html.replace(/\.slide-14 \.col-name\s*{\s*width:\s*104px;/g, 
                    '.slide-14 .col-name {\n    width: 130px;\n    white-space: nowrap;');

fs.writeFileSync('C:/Users/28414/Desktop/低标注预算下长尾分布的主动学习与半监督学习联合策略-20260617-131815/index_standalone.html', html, 'utf8');
console.log('Fixed slide 14 table column width.');
