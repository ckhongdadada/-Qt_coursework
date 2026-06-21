const fs = require('fs');
let html = fs.readFileSync('C:/Users/28414/Desktop/低标注预算下长尾分布的主动学习与半监督学习联合策略-20260617-131815/index_standalone.html', 'utf8');

// The containers for slide 12 and slide 14 are:
// <div class="slide-container slide-14" id="slide-12" ...> ... </div>
// <div class="slide-container slide-16" id="slide-14" ...> ... </div>

// We need to extract both blocks.
const slide12Regex = /<div class="slide-container slide-14" id="slide-12" style="display: none;">([\s\S]*?)<\/div>\s*<div class="slide-container slide-15" id="slide-13"/;
const slide14Regex = /<div class="slide-container slide-16" id="slide-14" style="display: none;">([\s\S]*?)<\/div>\s*<div class="slide-container slide-17" id="slide-15"/;

let slide12Match = html.match(slide12Regex);
let slide14Match = html.match(slide14Regex);

if (slide12Match && slide14Match) {
    let slide12Content = slide12Match[0].replace('id="slide-12"', 'id="slide-14"');
    slide12Content = slide12Content.replace(/>12 \/ 17</g, '>14 / 17<'); // Update page number
    // Remove the trailing next slide container start tag from the match to get pure content
    slide12Content = slide12Content.replace(/\s*<div class="slide-container slide-15" id="slide-13"$/, '');

    let slide14Content = slide14Match[0].replace('id="slide-14"', 'id="slide-12"');
    slide14Content = slide14Content.replace(/>14 \/ 17</g, '>12 / 17<'); // Update page number
    slide14Content = slide14Content.replace(/\s*<div class="slide-container slide-17" id="slide-15"$/, '');

    // Replace slide 12 with slide 14's content
    html = html.replace(slide12Match[0].replace(/\s*<div class="slide-container slide-15" id="slide-13"$/, ''), slide14Content);
    
    // Replace slide 14 with slide 12's content
    html = html.replace(slide14Match[0].replace(/\s*<div class="slide-container slide-17" id="slide-15"$/, ''), slide12Content);

    fs.writeFileSync('C:/Users/28414/Desktop/低标注预算下长尾分布的主动学习与半监督学习联合策略-20260617-131815/index_standalone.html', html, 'utf8');
    console.log('Successfully swapped slides 12 and 14.');
} else {
    console.log('Could not find the slides to swap.');
}
