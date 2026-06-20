const fs = require('fs');
const filepath = 'C:/Users/28414/Desktop/低标注预算下长尾分布的主动学习与半监督学习联合策略-20260617-131815/index_standalone.html';
let html = fs.readFileSync(filepath, 'utf8');

// Normalize newlines to \n to make string replacement work
html = html.replace(/\r\n/g, '\n');

let changed = false;

const legendBlock = `
        <!-- Glossary / Legend -->
        <div style="margin-top: 20px; display: flex; flex-wrap: wrap; gap: 15px; font-size: 0.9rem; color: #9ca3af; background: rgba(255,255,255,0.02); padding: 15px; border-radius: 8px; border: 1px solid rgba(255,255,255,0.05);">
            <div style="flex: 1 1 45%;"><strong style="color:#e5e7eb;">Base AL (基础主动学习)</strong>: 采用标准策略（如 Entropy），未针对长尾分布进行特殊优化。</div>
            <div style="flex: 1 1 45%;"><strong style="color:#e5e7eb;">Innov AL (创新主动学习)</strong>: 采用提出的 Class-Aware 策略，主动关注并挖掘长尾弱势类别。</div>
            <div style="flex: 1 1 45%;"><strong style="color:#e5e7eb;">Base SSL (基础半监督)</strong>: 采用标准 FlexMatch，依靠固定/全局置信度阈值生成伪标签。</div>
            <div style="flex: 1 1 45%;"><strong style="color:#e5e7eb;">Innov SSL (创新半监督)</strong>: 引入类别自适应阈值，动态调整长尾类别的伪标签生成难度。</div>
        </div>
      </div>
    </div>`;

const tableColEnd = `        </table>
      </div>
    </div>`;

if (html.includes(tableColEnd)) {
    // Only replace the first occurrence in slide-12
    let parts = html.split('id="slide-12"');
    if (parts.length > 1) {
        if (parts[1].includes(tableColEnd)) {
            parts[1] = parts[1].replace(tableColEnd, `        </table>\n` + legendBlock);
            html = parts[0] + 'id="slide-12"' + parts[1];
            changed = true;
            console.log('Glossary added to slide 12.');
        } else {
            console.log('tableColEnd not found in slide-12.');
        }
    }
}

if (changed) {
    fs.writeFileSync(filepath, html, 'utf8');
    console.log('Changes saved.');
}
