const fs = require('fs');
let html = fs.readFileSync('C:/Users/28414/Desktop/低标注预算下长尾分布的主动学习与半监督学习联合策略-20260617-131815/index_standalone.html', 'utf8');

// Fix the corrupted item-body class
html = html.replace(/\.slide-19 \.item-\.slide-19/g, '.slide-19 .item-body');

// Replace the CSS block for slide 19 layout
const newCss = `
.slide-19 .content {
    flex: 1;
    display: flex;
    gap: 40px;
    padding: 24px 64px 48px;
}

.slide-19 .column {
    flex: 1;
    display: flex;
    flex-direction: column;
    background: #F8FAFC;
    border-radius: 16px;
    padding: 32px 40px;
    border: 1px solid #E2E8F0;
    box-shadow: 0 10px 30px -5px rgba(0, 0, 0, 0.05);
}

.slide-19 .section-title {
    font-size: 26px;
    color: #0F172A;
    font-weight: 700;
    margin-bottom: 24px;
    line-height: 1.3;
    display: flex;
    align-items: center;
    gap: 12px;
    border-bottom: 2px solid rgba(37, 99, 235, 0.1);
    padding-bottom: 16px;
}

.slide-19 .section-title::before {
    content: '';
    display: block;
    width: 6px;
    height: 24px;
    background: #2563EB;
    border-radius: 4px;
}

.slide-19 .item {
    display: flex;
    gap: 14px;
    padding: 16px 0;
    border-bottom: 1px dashed #CBD5E1;
    font-size: 20px;
    color: #1E293B;
    line-height: 1.5;
    align-items: flex-start;
}

.slide-19 .item:last-child {
    border-bottom: none;
}

.slide-19 .item-num {
    color: #2563EB;
    font-weight: 700;
    font-size: 24px;
    flex-shrink: 0;
    line-height: 1.2;
}

.slide-19 .item-body {
    flex: 1;
}

.slide-19 .outlook-title {
    font-weight: 600;
    color: #0F172A;
    font-size: 22px;
}

.slide-19 .outlook-desc {
    color: #64748B;
    font-size: 17px;
    line-height: 1.5;
    margin-top: 6px;
}
`;

// Find the start of .slide-19 .content and replace until .slide-19 .qa-footer
let startIndex = html.indexOf('.slide-19 .content {');
let endIndex = html.indexOf('.slide-19 .qa-footer {');

if (startIndex !== -1 && endIndex !== -1) {
  html = html.substring(0, startIndex) + newCss.trim() + '\n\n' + html.substring(endIndex);
}

fs.writeFileSync('C:/Users/28414/Desktop/低标注预算下长尾分布的主动学习与半监督学习联合策略-20260617-131815/index_standalone.html', html, 'utf8');
console.log('Fixed slide 19 layout.');
