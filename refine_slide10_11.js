const fs = require('fs');
let html = fs.readFileSync('C:/Users/28414/Desktop/低标注预算下长尾分布的主动学习与半监督学习联合策略-20260617-131815/index_standalone.html', 'utf8');

// Slide 10: flow-step-title & desc
html = html.replace(/\.slide-12 \.flow-step-title \{\s*font-size: 19px;/, '.slide-12 .flow-step-title {\n  font-size: 18px;');
html = html.replace(/\.slide-12 \.flow-step-desc \{\s*font-size: 17px;/, '.slide-12 .flow-step-desc {\n  font-size: 16px;');

// Slide 10: if they were 22px (in case my previous replacement didn't stick for some reason)
html = html.replace(/\.slide-12 \.flow-step-title \{\s*font-size: 22px;/, '.slide-12 .flow-step-title {\n  font-size: 18px;');
html = html.replace(/\.slide-12 \.flow-step-desc \{\s*font-size: 22px;/, '.slide-12 .flow-step-desc {\n  font-size: 16px;');

// Slide 11: further refine hero numbers and titles to be even more coordinated
html = html.replace(/\.hero-number\s*\{\s*font-size:\s*34px;/g, '.hero-number {\n  font-size: 32px;');
html = html.replace(/\.hero-stars\s*\{\s*font-size:\s*22px;/g, '.hero-stars {\n  font-size: 20px;');

// If they were still 40px
html = html.replace(/\.hero-number\s*\{\s*font-size:\s*40px;/g, '.hero-number {\n  font-size: 32px;');
html = html.replace(/\.hero-stars\s*\{\s*font-size:\s*28px;/g, '.hero-stars {\n  font-size: 20px;');

fs.writeFileSync('C:/Users/28414/Desktop/低标注预算下长尾分布的主动学习与半监督学习联合策略-20260617-131815/index_standalone.html', html, 'utf8');
console.log('Applied font refinements for slide 10 and 11.');
