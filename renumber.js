const fs = require('fs');
let html = fs.readFileSync('C:/Users/28414/Desktop/低标注预算下长尾分布的主动学习与半监督学习联合策略-20260617-131815/index_standalone.html', 'utf8');

// fix the missing slide-8 by renumbering id='slide-X'
let slideCounter = 1;
html = html.replace(/id=\"slide-(\d+)\"/g, (match, num) => {
    return 'id=\"slide-' + (slideCounter++) + '\"';
});
console.log('Total slides after renumbering:', slideCounter - 1);
html = html.replace(/const totalSlides = \d+;/, 'const totalSlides = ' + (slideCounter - 1) + ';');

fs.writeFileSync('C:/Users/28414/Desktop/低标注预算下长尾分布的主动学习与半监督学习联合策略-20260617-131815/index_standalone.html', html, 'utf8');
console.log('Fixed missing slide ID sequence.');
