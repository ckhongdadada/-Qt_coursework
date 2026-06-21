const fs = require('fs');
let html = fs.readFileSync('C:/Users/28414/Desktop/低标注预算下长尾分布的主动学习与半监督学习联合策略-20260617-131815/index_standalone.html', 'utf8');

// Define replacements for the left column of Slide 10
const replacements = [
  { class: '.formula-title', old: '20px', new: '17px' },
  { class: '.formula-block', old: '22px', new: '19px' },
  { class: '.formula-explain', old: '22px', new: '17px' },
  { class: '.core-idea-label', old: '20px', new: '17px' },
  { class: '.core-idea-text', old: '22px', new: '17px' }
];

for (let r of replacements) {
  let regex = new RegExp('(\\.slide-12\\s+' + r.class.replace(/\./g, '\\.') + '\\s*\\{[^}]*?font-size:\\s*)' + r.old);
  html = html.replace(regex, '$1' + r.new);
}

fs.writeFileSync('C:/Users/28414/Desktop/低标注预算下长尾分布的主动学习与半监督学习联合策略-20260617-131815/index_standalone.html', html, 'utf8');
console.log('Reduced font sizes in the left column of Slide 10.');
