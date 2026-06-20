const fs = require('fs');
let html = fs.readFileSync('C:/Users/28414/Desktop/低标注预算下长尾分布的主动学习与半监督学习联合策略-20260617-131815/index_standalone.html', 'utf8');

// Find the section for Slide 6
let slideStartIndex = html.indexOf('六种基础主动学习策略');
let slideEndIndex = html.indexOf('<script>', slideStartIndex);

if (slideStartIndex !== -1 && slideEndIndex !== -1) {
  let slideHtml = html.substring(slideStartIndex, slideEndIndex);

  // 1. Update Grid container
  slideHtml = slideHtml.replace(
    /gap: 20px; margin-top: 24px; padding: 0 16px;/, 
    'gap: 36px 32px; margin-top: 56px; padding: 0 48px;'
  );

  // 2. Update Cards
  // Replace padding: 20px; with padding: 32px 36px; min-height: 240px;
  slideHtml = slideHtml.replace(/padding: 20px;/g, 'padding: 32px 36px; min-height: 220px;');
  // Replace box-shadow: 0 4px 12px rgba(0,0,0,0.05); with a nicer shadow
  slideHtml = slideHtml.replace(/box-shadow: 0 4px 12px rgba\(0,0,0,0\.05\);/g, 'box-shadow: 0 10px 30px rgba(0,0,0,0.05);');
  // Replace border-top: 4px solid with border-top: 6px solid
  slideHtml = slideHtml.replace(/border-top: 4px solid/g, 'border-top: 6px solid');
  // Replace border-radius: 12px; with border-radius: 16px;
  slideHtml = slideHtml.replace(/border-radius: 12px;/g, 'border-radius: 16px;');

  // 3. Update Card header container
  slideHtml = slideHtml.replace(/margin-bottom: 12px;/g, 'margin-bottom: 24px;');

  // 4. Update Card title (font-size: 18px -> 24px, gap: 6px -> 12px)
  slideHtml = slideHtml.replace(/font-size: 18px; font-weight: bold; color: #0F172A; display: flex; align-items: center; gap: 6px;/g, 'font-size: 24px; font-weight: bold; color: #0F172A; display: flex; align-items: center; gap: 12px;');

  // 5. Update Circle number (width:24px; height:24px -> 36px, font-size:14px -> 18px)
  slideHtml = slideHtml.replace(/width:24px; height:24px; border-radius:50%; display:flex; align-items:center; justify-content:center; font-size:14px;/g, 'width:36px; height:36px; border-radius:50%; display:flex; align-items:center; justify-content:center; font-size:18px;');

  // 6. Update Tag on the right (padding: 4px 10px; border-radius: 20px; font-size: 12px -> 6px 16px, 15px)
  slideHtml = slideHtml.replace(/padding: 4px 10px; border-radius: 20px; font-size: 12px;/g, 'padding: 6px 16px; border-radius: 20px; font-size: 15px;');

  // 7. Update Card body text (font-size: 14px -> 18px)
  slideHtml = slideHtml.replace(/font-size: 14px; color: #475569; line-height: 1\.6;/g, 'font-size: 18px; color: #475569; line-height: 1.6;');

  // 8. Update monospace formula span sizes
  slideHtml = slideHtml.replace(/padding:2px 4px; border-radius:4px;/g, 'padding:4px 8px; border-radius:6px;');

  // Write back
  html = html.substring(0, slideStartIndex) + slideHtml + html.substring(slideEndIndex);
  fs.writeFileSync('C:/Users/28414/Desktop/低标注预算下长尾分布的主动学习与半监督学习联合策略-20260617-131815/index_standalone.html', html, 'utf8');
  console.log('Fixed slide 6 layout.');
} else {
  console.log('Could not find slide 6 section.');
}
