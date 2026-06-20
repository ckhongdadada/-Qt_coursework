const fs = require('fs');
let html = fs.readFileSync('C:/Users/28414/Desktop/低标注预算下长尾分布的主动学习与半监督学习联合策略-20260617-131815/index_standalone.html', 'utf8');

// Fix slide 10 overflow
html = html.replace(/\.slide-12 \.flow-step-title \{\s*font-size: 22px;/, '.slide-12 .flow-step-title {\n  font-size: 19px;');
html = html.replace(/\.slide-12 \.flow-step-desc \{\s*font-size: 22px;/, '.slide-12 .flow-step-desc {\n  font-size: 17px;');

// Fix slide 11 right column font sizes
const cssReplacements = [
  { class: '.hero-number', old: '40px', new: '34px' },
  { class: '.hero-desc', old: '18px', new: '15px' },
  { class: '.hero-stars', old: '28px', new: '22px' },
  { class: '.hero-context', old: '18px', new: '15px' },
  { class: '.sig-message', old: '18px', new: '15px' },
  { class: '.supporting-title', old: '18px', new: '15px' },
  { class: '.supporting-stat', old: '20px', new: '18px' },
  { class: '.supporting-note', old: '18px', new: '14px' },
  { class: '.findings-heading', old: '18px', new: '16px' },
  { class: '.finding-rho', old: '18px', new: '15px' },
  { class: '.finding-text', old: '18px', new: '15px' },
  { class: '.finding-note', old: '18px', new: '14px' },
  { class: '.sig-callout-label', old: '18px', new: '15px' }
];

for (let r of cssReplacements) {
  let regex = new RegExp('(\\' + r.class + '\\s*\\{[^}]*?font-size:\\s*)' + r.old);
  html = html.replace(regex, '$1' + r.new);
}

// Fix slide 11 left column table (add baselines)
const baselineRows = `            <tr style="color: #64748B; background: #F8FAFC;">
              <td style="font-weight: 400; color: #64748B;">Random</td>
              <td>0.1100&plusmn;.000</td>
              <td>0.1736&plusmn;.000</td>
              <td>0.1725&plusmn;.000</td>
              <td>0.1722&plusmn;.000</td>
            </tr>
            <tr style="color: #64748B; background: #F8FAFC;">
              <td style="font-weight: 400; color: #64748B;">Entropy</td>
              <td>0.4247&plusmn;.000</td>
              <td>0.1734&plusmn;.000</td>
              <td>0.1726&plusmn;.000</td>
              <td>0.1726&plusmn;.000</td>
            </tr>
          </tbody>`;

html = html.replace('</tbody>', baselineRows);
html = html.replace(/<div class="baseline-ref">[\s\S]*?<\/div>\s*<\/div>/, '</div>');

// Write back
fs.writeFileSync('C:/Users/28414/Desktop/低标注预算下长尾分布的主动学习与半监督学习联合策略-20260617-131815/index_standalone.html', html, 'utf8');
console.log('Fixed slide 10 and 11 layout and data.');
