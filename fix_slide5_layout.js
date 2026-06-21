const fs = require('fs');
let html = fs.readFileSync('C:/Users/28414/Desktop/低标注预算下长尾分布的主动学习与半监督学习联合策略-20260617-131815/index_standalone.html', 'utf8');

let startIndex = html.indexOf('TML模型与DL模型简述');
let endIndex = html.indexOf('六种基础主动学习策略');

if (startIndex !== -1 && endIndex !== -1) {
    let slideHtml = html.substring(startIndex, endIndex);

    // 1. Add grid-auto-rows: 1fr to the grid container
    slideHtml = slideHtml.replace(
        /grid-template-columns: 1fr 1fr; gap: 24px;/,
        'grid-template-columns: 1fr 1fr; grid-auto-rows: 1fr; gap: 24px;'
    );

    // 2. Add flex column to the cards
    slideHtml = slideHtml.replace(
        /padding: 24px; border-top: 4px solid/g,
        'padding: 24px; display: flex; flex-direction: column; border-top: 4px solid'
    );

    // 3. Add margin-top: auto to the bottom boxes
    slideHtml = slideHtml.replace(
        /border: 1px solid #E2E8F0; font-size: 14px; color: #475569;"/g,
        'border: 1px solid #E2E8F0; font-size: 14px; color: #475569; margin-top: auto;"'
    );

    slideHtml = slideHtml.replace(
        /border: 1px solid #BFDBFE; font-size: 14px; color: #1E3A8A;"/g,
        'border: 1px solid #BFDBFE; font-size: 14px; color: #1E3A8A; margin-top: auto;"'
    );

    html = html.substring(0, startIndex) + slideHtml + html.substring(endIndex);
    fs.writeFileSync('C:/Users/28414/Desktop/低标注预算下长尾分布的主动学习与半监督学习联合策略-20260617-131815/index_standalone.html', html, 'utf8');
    console.log('Fixed slide 5 box heights.');
} else {
    console.log('Could not find slide bounds.');
}
