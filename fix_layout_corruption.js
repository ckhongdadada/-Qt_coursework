const fs = require('fs');
let html = fs.readFileSync('C:/Users/28414/Desktop/低标注预算下长尾分布的主动学习与半监督学习联合策略-20260617-131815/index_standalone.html', 'utf8');

// Fix Slide 10: broken class name
html = html.replace('.slide-12 .card-.slide-12 {', '.slide-12 .card-body {');

// Fix Slide 11: extra closing div
// The broken part in slide 11 looks like this:
/*
              <td>0.1726&plusmn;.000</td>
            </tr>
          </tbody>
        </table>
        </div>
      </div>

      <!-- Right: Significance + Findings -->
*/
// We need to replace `</table>\n        </div>\n      </div>` with `</table>\n      </div>`
// Let's use a regex to be safe.
html = html.replace(/<\/table>\s*<\/div>\s*<\/div>\s*<!-- Right: Significance \+ Findings -->/, '</table>\n      </div>\n\n      <!-- Right: Significance + Findings -->');

fs.writeFileSync('C:/Users/28414/Desktop/低标注预算下长尾分布的主动学习与半监督学习联合策略-20260617-131815/index_standalone.html', html, 'utf8');
console.log('Fixed broken HTML structures in Slide 10 and 11');
