const fs = require('fs');
let html = fs.readFileSync('C:/Users/28414/Desktop/低标注预算下长尾分布的主动学习与半监督学习联合策略-20260617-131815/index_standalone.html', 'utf8');

// Reduce font sizes and margins in slide 15 (.slide-17)
html = html.replace(/\.slide-17 \.section-label,\s*\.slide-17 \.findings-heading\s*\{[^}]+\}/, `.slide-17 .section-label, .slide-17 .findings-heading {
    font-size: 22px;
    font-weight: 700;
    color: #0F172A;
    margin-bottom: 12px;
}`);

html = html.replace(/\.slide-17 \.findings-heading\s*\{[^}]+\}/, `.slide-17 .findings-heading {
    margin-top: 10px;
}`);

html = html.replace(/\.slide-17 \.concept-card\s*\{[^}]+\}/, `.slide-17 .concept-card {
    background: #F8FAFC;
    border: 1px solid #E2E8F0;
    border-radius: 10px;
    padding: 12px 16px;
    margin-bottom: 12px;
    box-shadow: 0 2px 10px rgba(0, 0, 0, 0.02);
}`);

html = html.replace(/\.slide-17 \.card-title\s*\{[^}]+\}/, `.slide-17 .card-title {
    font-size: 19px;
    font-weight: 700;
    color: #0F172A;
    margin-bottom: 6px;
    display: flex;
    align-items: center;
    gap: 10px;
}`);

html = html.replace(/\.slide-17 \.card-text\s*\{[^}]+\}/, `.slide-17 .card-text {
    font-size: 17px;
    font-weight: 400;
    color: #334155;
    line-height: 1.4;
}`);

html = html.replace(/\.slide-17 \.finding-item\s*\{[^}]+\}/, `.slide-17 .finding-item {
    display: flex;
    align-items: flex-start;
    gap: 10px;
    margin-bottom: 8px;
}`);

html = html.replace(/\.slide-17 \.finding-text\s*\{[^}]+\}/, `.slide-17 .finding-text {
    font-size: 17px;
    font-weight: 400;
    color: #334155;
    line-height: 1.4;
}`);

html = html.replace(/\.slide-17 \.finding-bullet\s*\{[^}]+\}/, `.slide-17 .finding-bullet {
    flex-shrink: 0;
    width: 6px;
    height: 6px;
    margin-top: 9px;
}`);

// Check if content padding can be reduced slightly vertically
html = html.replace(/\.slide-17 \.content\s*\{([^}]+)\}/, (match, inner) => {
    return '.slide-17 .content {' + inner.replace(/padding:\s*[^;]+;/, 'padding: 16px 56px 20px;') + '}';
});

fs.writeFileSync('C:/Users/28414/Desktop/低标注预算下长尾分布的主动学习与半监督学习联合策略-20260617-131815/index_standalone.html', html, 'utf8');
console.log('Fixed slide 15 overflow.');
