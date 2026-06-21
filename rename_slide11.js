const fs = require('fs');
const filepath = 'C:/Users/28414/Desktop/低标注预算下长尾分布的主动学习与半监督学习联合策略-20260617-131815/index_standalone.html';
let html = fs.readFileSync(filepath, 'utf8');
html = html.replace(/\r\n/g, '\n');

const old = '主要实验结果';
const neu = 'AL 创新策略实验结果';

if (html.includes(old)) {
    html = html.replace(old, neu);
    fs.writeFileSync(filepath, html, 'utf8');
    console.log('Title updated.');
} else {
    console.log('NOT FOUND');
}
