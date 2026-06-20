const fs = require('fs');
const html = fs.readFileSync('C:/Users/28414/Desktop/低标注预算下长尾分布的主动学习与半监督学习联合策略-20260617-131815/index_standalone.html', 'utf8');

// Check slide 13 content
const parts = html.split('id="slide-13"');
if (parts.length > 1) {
    const slide13 = parts[1].substring(0, 2000);
    console.log('=== Slide 13 content (first 2000 chars) ===');
    console.log(slide13);
} else {
    console.log('Slide 13 not found!');
}
