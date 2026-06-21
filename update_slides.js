const fs = require('fs');
const filepath = 'C:/Users/28414/Desktop/低标注预算下长尾分布的主动学习与半监督学习联合策略-20260617-131815/index_standalone.html';
let html = fs.readFileSync(filepath, 'utf8');
let changed = false;

// 1. Slide 12 hero metric
const slide12LeftOld = `<div class="hero-col">
        <div class="hero-number">+16.8%</div>
        <div class="hero-label">Innov AL + Base SSL</div>
        <div class="hero-sublabel">vs 纯AL基线(0.3438)</div>
      </div>`;
const slide12LeftNew = `<div class="hero-col" style="display: flex; flex-direction: column; justify-content: center; gap: 20px;">
        <div style="background: rgba(239, 68, 68, 0.1); padding: 20px; border-radius: 12px; border-left: 4px solid #ef4444;">
            <div style="font-size: 1.2rem; color: #ef4444; font-weight: 600; margin-bottom: 10px;">为什么 Innov SSL 会失效？</div>
            <div style="font-size: 1rem; line-height: 1.6; color: #d1d5db;">
                在低预算下，Innov SSL 的<strong style="color:#fca5a5;">伪标签准确率骤降至 56.6%</strong>（基准 71.2%）。错误伪标签引发确认偏差，导致性能较纯AL基线<strong style="color:#fca5a5;">下降 2.5%</strong>。
            </div>
        </div>
        <div style="background: rgba(59, 130, 246, 0.1); padding: 20px; border-radius: 12px; border-left: 4px solid #3b82f6;">
            <div style="font-size: 1.2rem; color: #60a5fa; font-weight: 600; margin-bottom: 10px;">渐进式 SSL 设计动机</div>
            <div style="font-size: 1rem; line-height: 1.6; color: #d1d5db;">
                这直接促使我们提出“渐进式 SSL”：<strong>早期采用固定高阈值</strong>保证质量，<strong>后期切换自适应阈值</strong>促进尾类召回，从而实现极端长尾下的稳定提升。
            </div>
        </div>
      </div>`;
if (html.includes(slide12LeftOld)) {
    html = html.replace(slide12LeftOld, slide12LeftNew);
    changed = true;
    console.log('Slide 12 left metric updated.');
} else {
    console.log('Slide 12 left metric NOT found.');
}

// 2. Slide 12 table
const slide12TableOld = `<tr>
              <td>AL+Base SSL</td>
              <td>Entropy</td>
              <td>FlexMatch</td>
              <td class="f1-cell">0.3639</td>
              <td class="delta-pos">+5.8%</td>
            </tr>`;
const slide12TableNew = `<tr>
              <td>AL+Base SSL</td>
              <td>Entropy</td>
              <td>FlexMatch</td>
              <td class="f1-cell">0.3639</td>
              <td class="delta-pos">+5.8%</td>
            </tr>
            <tr>
              <td>Base AL+Innov SSL</td>
              <td>Entropy</td>
              <td>Class-Aware</td>
              <td class="f1-cell" style="color: #ef4444;">0.3351</td>
              <td class="delta-neg" style="color: #ef4444;">-2.5%</td>
            </tr>`;
if (html.includes(slide12TableOld)) {
    html = html.replace(slide12TableOld, slide12TableNew);
    changed = true;
    console.log('Slide 12 table row updated.');
} else {
    console.log('Slide 12 table row NOT found.');
}

// 3. Slide 13 body
let parts = html.split('id="slide-13"');
if (parts.length > 1) {
    let slide13Content = parts[1];
    let imgRegex = /(<div class="content-left">)(\s*<img src="data:image\/png;base64,[^"]+"[^>]*>)(\s*<\/div>)/;
    let match = slide13Content.match(imgRegex);
    if (match) {
        let newLeft = `
    <div class="slide-body" style="display: flex; flex-direction: row; align-items: stretch; height: 100%; gap: 20px; padding: 20px;">
    <!-- Left: zoomed learning curve image -->
    <div class="content-left" style="flex: 2; display: flex; justify-content: center; align-items: center; max-height: 550px; overflow: hidden; background: rgba(255,255,255,0.05); border-radius: 12px; padding: 10px;">
        ${match[2].replace('<img', '<img style="width: 100%; height: auto; object-fit: contain; transform: scale(1.15); transform-origin: center;"')}
    </div>
    <!-- Right: Analysis -->
    <div class="content-right" style="flex: 1; display: flex; flex-direction: column; justify-content: center; gap: 20px;">
        <div style="background: rgba(16, 185, 129, 0.1); padding: 25px; border-radius: 12px; border-left: 4px solid #10b981;">
            <div style="font-size: 1.3rem; color: #10b981; font-weight: 600; margin-bottom: 15px;">实验现象分析</div>
            <div style="font-size: 1.1rem; line-height: 1.7; color: #d1d5db;">
                <ul style="padding-left: 20px; margin: 0;">
                    <li style="margin-bottom: 15px;"><strong>策略优势的确立时机</strong>：<br/>类别自适应策略在<strong style="color:white;">第3轮之后</strong>迅速建立并拉开显著优势。</li>
                    <li><strong>学习曲线的变化</strong>：<br/>当纯 AL 和基础策略逐渐陷入瓶颈时，Class-Aware 联合策略依然能够保持连续攀升的趋势，成功突破标注瓶颈。</li>
                </ul>
            </div>
        </div>
    </div>
    `;
        let bodyRegex = /<div class="slide-body">\s*<!-- Left: zoomed learning curve image -->\s*<div class="content-left">\s*<img src="data:image\/png;base64,[^"]+"[^>]*>\s*<\/div>/;
        if (bodyRegex.test(slide13Content)) {
            parts[1] = slide13Content.replace(bodyRegex, newLeft);
            html = parts.join('id="slide-13"');
            changed = true;
            console.log('Slide 13 body updated.');
        } else {
            console.log('Slide 13 bodyRegex failed.');
        }
    } else {
        console.log('Slide 13 img tag NOT found.');
    }
}

if (changed) {
    fs.writeFileSync(filepath, html, 'utf8');
    console.log('Changes saved.');
}
