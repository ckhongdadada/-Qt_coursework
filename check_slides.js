const fs = require('fs');
const html = fs.readFileSync('C:/Users/28414/Desktop/低标注预算下长尾分布的主动学习与半监督学习联合策略-20260617-131815/index_standalone.html', 'utf8');

const slideIds = ['slide-11', 'slide-12', 'slide-13', 'slide-14', 'slide-15', 'slide-16', 'slide-17'];

slideIds.forEach(id => {
    const match = html.match(new RegExp(`<div class="slide-container[^"]*" id="${id}"[^>]*>`));
    if (match) {
        console.log(`ID: ${id} -> ${match[0]}`);
    }
});
