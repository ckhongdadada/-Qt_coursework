const fs = require('fs');
let html = fs.readFileSync('C:/Users/28414/Desktop/低标注预算下长尾分布的主动学习与半监督学习联合策略-20260617-131815/index_standalone.html', 'utf8');

let divDepth = 0;
let lines = html.split('\n');
for (let i = 0; i < lines.length; i++) {
  let line = lines[i];
  if (line.includes('<div class="slide-container')) {
    console.log('Slide start at line ' + (i+1) + ', depth: ' + divDepth);
  }
  let openMatches = line.match(/<div[^>]*>/gi);
  let closeMatches = line.match(/<\/div>/gi);
  let o = openMatches ? openMatches.length : 0;
  let c = closeMatches ? closeMatches.length : 0;
  divDepth += o - c;
}
console.log('Final depth: ' + divDepth);
