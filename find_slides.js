const fs = require('fs');
const html = fs.readFileSync('C:/Users/28414/Desktop/低标注预算下长尾分布的主动学习与半监督学习联合策略-20260617-131815/index_standalone.html', 'utf8');

// Find ALL slide-title and slide-section-label occurrences with surrounding context
const parts = html.split('id="slide-');
console.log(`Total slide segments: ${parts.length - 1}`);
for (let i = 1; i < parts.length; i++) {
    const segment = parts[i].substring(0, 500);
    // Find any title-like content
    const titles = segment.match(/>([^<]{2,60})</g);
    if (titles) {
        const filtered = titles.filter(t => 
            t.includes('实验') || t.includes('主要') || t.includes('AL') || t.includes('SSL') || 
            t.includes('联合') || t.includes('学习曲线') || t.includes('消融') || t.includes('总结') ||
            t.includes('策略') || t.includes('设计')
        );
        if (filtered.length > 0) {
            console.log(`Slide ${i}: ${filtered.slice(0,3).join(' | ')}`);
        }
    }
}
