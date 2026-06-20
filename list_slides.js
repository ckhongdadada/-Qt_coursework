const fs = require('fs');
const html = fs.readFileSync('C:/Users/28414/Desktop/低标注预算下长尾分布的主动学习与半监督学习联合策略-20260617-131815/index_standalone.html', 'utf8');

// Find slide 4 context - it's about "主要实验结果"
const parts = html.split('id="slide-');
console.log(`Total slides found: ${parts.length - 1}`);
for (let i = 1; i < parts.length; i++) {
    const segment = parts[i].substring(0, 300);
    const slideNum = segment.match(/^(\d+)"/);
    const title = segment.match(/slide-title">([^<]+)</);
    const label = segment.match(/slide-label">([^<]+)</);
    if (slideNum || title) {
        console.log(`\n--- Slide segment ${i} ---`);
        console.log('Title:', title ? title[1] : 'N/A');
        console.log('Label:', label ? label[1] : 'N/A');
    }
}
