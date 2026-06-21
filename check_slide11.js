const fs = require('fs');
const html = fs.readFileSync('C:/Users/28414/Desktop/低标注预算下长尾分布的主动学习与半监督学习联合策略-20260617-131815/index_standalone.html', 'utf8');

// Check slide 11 content (first 3000 chars)
const parts = html.split('id="slide-11"');
if (parts.length > 1) {
    const slide11 = parts[1].substring(0, 3000);
    console.log('=== Slide 11 content (first 3000 chars) ===');
    console.log(slide11);
} else {
    console.log('Slide 11 not found!');
}
