const fs = require('fs');
const filepath = 'C:/Users/28414/Desktop/低标注预算下长尾分布的主动学习与半监督学习联合策略-20260617-131815/index_standalone.html';
let html = fs.readFileSync(filepath, 'utf8');
html = html.replace(/\r\n/g, '\n');

// 1. Change title
html = html.replace('消融实验：AL创新 vs SSL创新', '消融实验：核心策略拆解');

// 2. Remove the "Innov SSL" row from the table
const rowToRemove = `<tr>
              <td>Base AL+Innov SSL</td>
              <td>Entropy</td>
              <td>Class-Aware</td>
              <td class="f1-cell" style="color: #ef4444;">0.3351</td>
              <td class="delta-neg" style="color: #ef4444;">-2.5%</td>
            </tr>`;
if (html.includes(rowToRemove)) {
    html = html.replace(rowToRemove, '');
} else {
    // try flexible match
    const regexRow = /<tr>\s*<td>Base AL\+Innov SSL<\/td>[\s\S]*?<\/tr>/g;
    html = html.replace(regexRow, '');
}

// 3. Remove the red warning box (Innov SSL failing)
const redBoxRegex = /<div style="background: rgba\(239, 68, 68, 0\.1\); padding: 20px; border-radius: 12px; border-left: 4px solid #ef4444;">[\s\S]*?<\/div>\s*<\/div>\s*<div style="background: rgba\(59, 130, 246, 0\.1\)/g;
html = html.replace(redBoxRegex, '<div style="background: rgba(59, 130, 246, 0.1)');

// 4. Update the blue box text (Joint Distribution motivation)
const blueBoxTitleOld = '转向联合分布感知 (Joint Distribution)';
const blueBoxTitleNew = '引入联合分布感知 (Joint Distribution)';
const blueBoxTextOld = '这直接促使我们放弃对 SSL 的单独创新，转而提出<strong>“联合分布感知”策略</strong>：在 AL 阶段将伪标签分布纳入类惩罚项，使 AL 能够感知并主动填补 SSL 未覆盖的弱势类别，形成真正的联合协同。';
const blueBoxTextNew = '为了进一步提升长尾类别性能，我们提出<strong>“联合分布感知”策略</strong>：在 AL 阶段将伪标签分布纳入类惩罚项，使 AL 能够感知并主动填补 SSL 未覆盖的弱势类别，形成真正的联合协同。';

html = html.replace(blueBoxTitleOld, blueBoxTitleNew);
html = html.replace(blueBoxTextOld, blueBoxTextNew);

// 5. Remove Innov SSL from glossary and adjust width
const glossaryItem = `<div style="flex: 1 1 45%;"><strong style="color:#111827;">Innov SSL (创新半监督)</strong>: 引入类别自适应阈值，动态调整长尾类别的伪标签生成难度。</div>`;
html = html.replace(glossaryItem, '');

const glossaryBaseSSL = `<div style="flex: 1 1 45%;"><strong style="color:#111827;">Base SSL (基础半监督)</strong>: 采用标准 FlexMatch，依靠固定/全局置信度阈值生成伪标签。</div>`;
const glossaryBaseSSLNew = `<div style="flex: 1 1 100%;"><strong style="color:#111827;">Base SSL (基础半监督)</strong>: 采用标准 FlexMatch，依靠固定/全局置信度阈值生成伪标签。</div>`;
html = html.replace(glossaryBaseSSL, glossaryBaseSSLNew);

// Write back
fs.writeFileSync(filepath, html, 'utf8');
console.log('Slide 12 rewritten without SSL Innovation narrative.');
