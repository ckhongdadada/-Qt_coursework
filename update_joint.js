const fs = require('fs');
const filepath = 'C:/Users/28414/Desktop/低标注预算下长尾分布的主动学习与半监督学习联合策略-20260617-131815/index_standalone.html';
let html = fs.readFileSync(filepath, 'utf8');

// The text we want to replace
const oldTitle = '渐进式 SSL 设计动机';
const newTitle = '转向联合分布感知 (Joint Distribution)';

const oldText = '这直接促使我们提出“渐进式 SSL”：<strong>早期采用固定高阈值</strong>保证质量，<strong>后期切换自适应阈值</strong>促进尾类召回，从而实现极端长尾下的稳定提升。';
const newText = '这直接促使我们放弃对 SSL 的单独创新，转而提出<strong>“联合分布感知”策略</strong>：在 AL 阶段将伪标签分布纳入类惩罚项，使 AL 能够感知并主动填补 SSL 未覆盖的弱势类别，形成真正的联合协同。';

if (html.includes(oldTitle) && html.includes(oldText)) {
    html = html.replace(oldTitle, newTitle);
    html = html.replace(oldText, newText);
    fs.writeFileSync(filepath, html, 'utf8');
    console.log("Successfully updated to Joint Distribution.");
} else {
    console.log("Could not find the target text to replace.");
}
