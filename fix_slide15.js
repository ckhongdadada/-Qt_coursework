const fs = require('fs');
let html = fs.readFileSync('C:/Users/28414/Desktop/低标注预算下长尾分布的主动学习与半监督学习联合策略-20260617-131815/index_standalone.html', 'utf8');

const replacementCss = `
.slide-17 .content {
    display: flex;
    padding: 24px 64px 32px;
    gap: 48px;
    flex: 1;
    min-height: 0;
}

.slide-17 .chart-col {
    flex: 0 0 52%;
    display: flex;
    flex-direction: column;
    min-height: 0;
}

.slide-17 .chart-label {
    font-size: 22px;
    font-weight: 700;
    color: #475569;
    letter-spacing: 0.1em;
    text-transform: uppercase;
    margin-bottom: 12px;
}

.slide-17 .chart-wrap {
    flex: 1;
    position: relative;
    min-height: 0;
    background: #FFFFFF;
    border-radius: 12px;
    border: 1px solid #E2E8F0;
    box-shadow: 0 4px 20px rgba(0, 0, 0, 0.03);
    padding: 16px;
}

.slide-17 .chart-wrap canvas {
    position: absolute;
    top: 16px; left: 16px;
    width: calc(100% - 32px) !important;
    height: calc(100% - 32px) !important;
}

.slide-17 .findings-col {
    flex: 1;
    display: flex;
    flex-direction: column;
    justify-content: flex-start;
    padding-top: 0;
    min-height: 0;
    overflow: hidden;
}

.slide-17 .section-label, .slide-17 .findings-heading {
    font-size: 26px;
    font-weight: 700;
    color: #0F172A;
    margin-bottom: 20px;
}

.slide-17 .findings-heading {
    margin-top: 16px;
}

.slide-17 .concept-card {
    background: #F8FAFC;
    border: 1px solid #E2E8F0;
    border-radius: 12px;
    padding: 20px 24px;
    margin-bottom: 20px;
    box-shadow: 0 4px 16px rgba(0, 0, 0, 0.02);
}

.slide-17 .card-title {
    font-size: 21px;
    font-weight: 700;
    color: #0F172A;
    margin-bottom: 10px;
    display: flex;
    align-items: center;
    gap: 12px;
}

.slide-17 .card-accent-line {
    width: 5px;
    height: 20px;
    border-radius: 2.5px;
    flex-shrink: 0;
}

.slide-17 .card-text {
    font-size: 19px;
    font-weight: 400;
    color: #334155;
    line-height: 1.6;
}

.slide-17 .card-text strong {
    font-weight: 700;
    color: #0F172A;
}

.slide-17 .pro {
    color: #2563EB;
    font-weight: 600;
}

.slide-17 .con {
    color: #64748B;
    font-weight: 600;
}

.slide-17 .separator {
    display: none;
}

.slide-17 .finding-item {
    display: flex;
    align-items: flex-start;
    gap: 14px;
    margin-bottom: 14px;
}

.slide-17 .finding-bullet {
    flex-shrink: 0;
    width: 8px;
    height: 8px;
    margin-top: 11px;
}

.slide-17 .finding-bullet svg {
    display: block;
    width: 100%;
    height: 100%;
}

.slide-17 .finding-text {
    font-size: 20px;
    font-weight: 400;
    color: #334155;
    line-height: 1.5;
}

.slide-17 .finding-text strong {
    font-weight: 700;
    color: #2563EB;
}
`;

// Extract everything from .slide-17 .content { to right before .slide-17 .hex-decorations {
let startIndex = html.indexOf('.slide-17 .content {');
let endIndex = html.indexOf('.slide-17 .hex-decorations {');

if (startIndex !== -1 && endIndex !== -1) {
  html = html.substring(0, startIndex) + replacementCss + '\n' + html.substring(endIndex);
}

fs.writeFileSync('C:/Users/28414/Desktop/低标注预算下长尾分布的主动学习与半监督学习联合策略-20260617-131815/index_standalone.html', html, 'utf8');
console.log('Fixed slide 15 layout.');
