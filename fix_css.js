const fs = require('fs');
let html = fs.readFileSync('C:/Users/28414/Desktop/低标注预算下长尾分布的主动学习与半监督学习联合策略-20260617-131815/index_standalone.html', 'utf8');

let lines = html.split('\n');

// Find line 1011 to 1156 and prepend .slide-5
for (let i = 1010; i < 1156; i++) {
    // If it's the .slide-container block, we skip or remove it, but let's just make it .slide-5
    if (lines[i].trim() === '.slide-container {') {
        lines[i] = lines[i].replace('.slide-container', '.slide-5.slide-container');
    } else if (lines[i].match(/^\s*\.[a-zA-Z0-9_-]+\s*\{/)) {
        lines[i] = lines[i].replace(/^\s*\./, '        .slide-5 .');
    } else if (lines[i].match(/^\s*\.[a-zA-Z0-9_-]+\s+/)) {
        // e.g. .icon-circle svg
        lines[i] = lines[i].replace(/^\s*\./, '        .slide-5 .');
    }
}

fs.writeFileSync('C:/Users/28414/Desktop/低标注预算下长尾分布的主动学习与半监督学习联合策略-20260617-131815/index_standalone.html', lines.join('\n'), 'utf8');
console.log('Fixed unscoped CSS for slide 5');
