const fs = require('fs');
const html = fs.readFileSync('C:/Users/28414/Desktop/低标注预算下长尾分布的主动学习与半监督学习联合策略-20260617-131815/index_standalone.html', 'utf8');
const regex = /<h1 class="slide-title">([^<]+)<\/h1>/g;
let match;
let i = 1;
while ((match = regex.exec(html)) !== null) {
    console.log(`Slide ${i}: ${match[1]}`);
    i++;
}
