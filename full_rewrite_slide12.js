const fs = require('fs');
const filepath = 'C:/Users/28414/Desktop/低标注预算下长尾分布的主动学习与半监督学习联合策略-20260617-131815/index_standalone.html';
let html = fs.readFileSync(filepath, 'utf8');
html = html.replace(/\r\n/g, '\n');

// Find slide-12 content and replace with entirely new content
const slide12Start = html.indexOf('id="slide-12"');
const slide12End = html.indexOf('id="slide-13"');

if (slide12Start === -1 || slide12End === -1) {
    console.log('Could not find slide-12 boundaries');
    process.exit(1);
}

// Extract surrounding HTML tags to preserve structure
const beforeSlide12 = html.substring(0, slide12Start);
const afterSlide12 = html.substring(slide12End);

// Build the completely new slide-12 content
const newSlide12 = `id="slide-12" class="slide-container">
  <div class="slide-inner" style="height: 100%; display: flex; flex-direction: column; padding: 32px 48px 24px;">

    <!-- Header -->
    <div class="slide-section-label">ABLATION STUDY</div>
    <div class="slide-divider"></div>
    <h1 class="slide-title">在 AL 创新基础上叠加联合分布感知</h1>

    <!-- Body -->
    <div class="slide-body" style="display: flex; flex-direction: row; gap: 24px; flex: 1; margin-top: 20px; min-height: 0;">

      <!-- Left: Two insight cards -->
      <div style="flex: 1; display: flex; flex-direction: column; gap: 18px; justify-content: center;">

        <div style="background: rgba(16,185,129,0.08); padding: 22px; border-radius: 12px; border-left: 4px solid #10b981;">
          <div style="font-size: 1.15rem; color: #059669; font-weight: 700; margin-bottom: 10px;">① AL 创新已确立主要贡献</div>
          <div style="font-size: 0.95rem; line-height: 1.7; color: #374151;">
            Class-Aware 策略通过<strong>显式对弱势类加权惩罚</strong>，在 ρ=50 极端长尾场景下将 Macro-F1 从 <strong>0.2585 提升至 0.3129</strong>，实现 <strong>+21.0%</strong> 的核心增益。<br>
            这是整个策略体系中<strong>最大的单项贡献</strong>。
          </div>
        </div>

        <div style="background: rgba(59,130,246,0.08); padding: 22px; border-radius: 12px; border-left: 4px solid #3b82f6;">
          <div style="font-size: 1.15rem; color: #2563eb; font-weight: 700; margin-bottom: 10px;">② 联合分布感知带来额外增益</div>
          <div style="font-size: 0.95rem; line-height: 1.7; color: #374151;">
            在 Innov AL 基础上引入<strong>联合分布感知</strong>：AL 查询时将模型对无标注集的伪标签分布纳入类惩罚项，使 AL 能主动感知并填补 SSL 尚未覆盖的弱势类别，实现真正的<strong>联合协同</strong>，额外提升至 <strong>0.3322（+28.5%）</strong>。
          </div>
        </div>

      </div>

      <!-- Right: Comparison table -->
      <div style="flex: 1.2; display: flex; flex-direction: column; justify-content: center;">
        <table style="width: 100%; border-collapse: collapse; font-size: 0.95rem;">
          <thead>
            <tr style="border-bottom: 2px solid #e5e7eb;">
              <th style="text-align: left; padding: 10px 12px; color: #6b7280; font-weight: 600;">配置</th>
              <th style="text-align: left; padding: 10px 12px; color: #6b7280; font-weight: 600;">AL 策略</th>
              <th style="text-align: center; padding: 10px 12px; color: #6b7280; font-weight: 600;">F1 (ρ=50)</th>
              <th style="text-align: center; padding: 10px 12px; color: #6b7280; font-weight: 600;">Δ vs 基线</th>
            </tr>
          </thead>
          <tbody>
            <tr style="border-bottom: 1px solid #f3f4f6;">
              <td style="padding: 12px; color: #374151;">纯 AL 基线</td>
              <td style="padding: 12px; color: #374151;">Entropy</td>
              <td style="padding: 12px; text-align: center; color: #374151; font-weight: 600;">0.2585</td>
              <td style="padding: 12px; text-align: center; color: #9ca3af;">—</td>
            </tr>
            <tr style="border-bottom: 1px solid #f3f4f6; background: rgba(16,185,129,0.04);">
              <td style="padding: 12px; color: #374151; font-weight: 600;">Innov AL</td>
              <td style="padding: 12px; color: #374151;">Class-Aware</td>
              <td style="padding: 12px; text-align: center; color: #059669; font-weight: 700;">0.3129</td>
              <td style="padding: 12px; text-align: center; color: #059669; font-weight: 700;">+21.0%</td>
            </tr>
            <tr style="background: rgba(59,130,246,0.06);">
              <td style="padding: 12px; color: #374151; font-weight: 600;">Innov AL + 联合分布感知</td>
              <td style="padding: 12px; color: #374151;">Class-Aware + Joint</td>
              <td style="padding: 12px; text-align: center; color: #2563eb; font-weight: 700;">0.3322</td>
              <td style="padding: 12px; text-align: center; color: #2563eb; font-weight: 700;">+28.5%</td>
            </tr>
          </tbody>
        </table>

        <!-- Bottom conclusion -->
        <div style="margin-top: 20px; padding: 14px 18px; background: rgba(37,99,235,0.07); border-radius: 10px; border-left: 3px solid #2563eb;">
          <div style="font-size: 0.92rem; color: #1e40af; line-height: 1.6;">
            <strong>结论：</strong>AL 创新是主要贡献来源（+21%），在此基础上引入联合分布感知可进一步稳定提升（额外 +7.5%），两者协同构成本文最优策略。
          </div>
        </div>

        <!-- Glossary -->
        <div style="margin-top: 14px; display: flex; gap: 16px; font-size: 0.85rem; color: #6b7280; background: rgba(0,0,0,0.02); padding: 12px 16px; border-radius: 8px; border: 1px solid #f3f4f6;">
          <div style="flex: 1;"><strong style="color: #374151;">Innov AL (Class-Aware)</strong>：对标注不足的弱势类显式加大 AL 采样权重。</div>
          <div style="flex: 1;"><strong style="color: #374151;">联合分布感知</strong>：将 SSL 伪标签分布纳入 AL 惩罚项，AL 与 SSL 形成互补。</div>
        </div>
      </div>

    </div>

    <!-- Bottom bullet -->
    <div style="margin-top: 12px; font-size: 0.88rem; color: #6b7280;">
      <span style="color: #2563eb; font-weight: 600;">·</span> AL 创新 + 联合分布感知协同，ρ=50 最终 F1 提升 <strong style="color: #2563eb;">+28.5%</strong>
    </div>

  </div>
`;

html = beforeSlide12 + newSlide12 + afterSlide12;
fs.writeFileSync(filepath, html, 'utf8');
console.log('Slide 12 completely rewritten.');
