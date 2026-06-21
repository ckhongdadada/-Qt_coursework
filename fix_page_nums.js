const fs = require('fs');
let html = fs.readFileSync('C:/Users/28414/Desktop/低标注预算下长尾分布的主动学习与半监督学习联合策略-20260617-131815/index_standalone.html', 'utf8');

let slideCount = 17;

// Find all slide containers and update the page numbers inside them.
// We can use a regex to replace any "X / 19" or "X / 17" with the correct count.
let slideIndex = 1;
html = html.replace(/<div class=\"slide-container[^>]*>([\s\S]*?)<\/div>\s*(?=<div class=\"slide-container|<\/div>\s*<\/div>\s*<\/body>)/g, (match, inner) => {
    // This regex approach might be brittle if divs are not matched.
    return match;
});

// Let's just do a simpler global replace for the denominators:
html = html.replace(/\b(\d+)\s*\/\s*\d+\b/g, (match, num) => {
    return num + ' / ' + slideCount;
});

// Wait, the numbers themselves (the numerator) might be wrong now because of the deleted slides!
// Let's fix the numerators properly.
// A better way: split the HTML by `<div class="slide-container` and then update the first occurrence of `\d+ \/ \d+` in each chunk.
let chunks = html.split('<div class="slide-container');
for (let i = 1; i < chunks.length; i++) {
    chunks[i] = chunks[i].replace(/\b\d+\s*\/\s*\d+\b/, i + ' / ' + slideCount);
}
html = chunks.join('<div class="slide-container');

fs.writeFileSync('C:/Users/28414/Desktop/低标注预算下长尾分布的主动学习与半监督学习联合策略-20260617-131815/index_standalone.html', html, 'utf8');
console.log('Fixed page numbers to X / 17');
