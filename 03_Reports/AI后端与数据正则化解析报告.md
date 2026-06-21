# AI 后端与数据正则化解析报告

## 1. 独立后端的设计哲学与架构决策

### 1.1 架构分离的必要性

在最初的原型开发中，团队尝试在 Qt C++ 端直接编写正则表达式来解析 PDF 培养方案文本。这引发了两个问题：

1. **字符串处理效率**：C++ 的正则表达式库在处理多行非结构化文本时，内存管理和调试成本较高。
2. **大模型 SDK 接入困难**：各大 AI 提供商通常提供 Python SDK，而 C++ 接入需要手动拼接 JSON 与处理流式传输（SSE），网络边界情况的错误处理复杂度高。

**解决方案**：剥离 AI 与复杂文本清洗逻辑，构建独立的 Python Flask 后端（`ai_server.py`）。Qt 客户端通过 HTTP POST 将原始数据推送给该服务，由 Python 完成清洗、正则化提取后，将结构化的 JSON 返回给 C++ 持久化。C++ 专注于 UI 渲染与数据管理，Python 专注于文本解析与模型推理。

### 1.2 AI 模型选型

本系统的 AI 后端集成 **Qwen2.5-1.5B-Instruct** 模型，这是一个由阿里巴巴开源的中文化大语言模型，参数量 1.5B，适合本地部署。

**模型能力**：
- 中文理解和生成能力强，适合处理学业相关的中文文本；
- 参数量小，可在消费级 GPU 上本地推理；
- 支持 OpenAI 兼容的对话格式（ChatML）。

**部署方式**：模型文件存放于 `qwen_models/` 目录，`ai_server.py` 启动时通过 `transformers` 库加载。支持 CPU 和 GPU 两种推理模式，GPU 不可用时自动降级到 CPU。

**外部 API 扩展**：C++ 客户端的 `AiService` 同时支持连接外部云 API（如 DeepSeek），通过配置 API Key 和服务端点即可切换。当外部 API 不可用时，自动回退到本地 Flask 服务或基于关键词的规则分析。

### 1.3 进程编排策略

AI 后端采用外置脚本启动而非 `QProcess` 内嵌管理，原因如下：

1. **Python 虚拟环境隔离**：外置 `.bat` 脚本可直接调用 `venv\Scripts\activate` 激活虚拟环境，避免 C++ 代码与文件系统路径强耦合。
2. **生命周期解耦**：AI 服务与桌面客户端在操作系统中是平级的独立进程，一方崩溃不会影响另一方。
3. **调试可见性**：`.bat` 启动会弹出终端窗口，HTTP 请求和报错堆栈一目了然，便于联调。
4. **部署灵活性**：AI 后端可独立部署到远程服务器，客户端只需修改请求地址即可。

项目提供了两个启动脚本：
- `start.bat`：启动 C++ 服务端（pdp_server.exe）+ AI 后端
- `start_desktop.bat`：启动桌面客户端（pdp_desktop.exe）+ AI 后端

两个脚本均会自动检测 Python 环境、设置模型路径、在独立终端中启动 AI 服务（端口 8001），然后启动 C++ 程序。

---

## 2. 数据正则化解析管线

大型文稿（尤其是学校下发的教务培养方案 PDF）中充斥着格式不规范、OCR 错误、排版混乱的文本。直接投喂大模型不仅消耗大量 Token，而且容易产生格式错误。因此，系统确立了**"正则预处理 + AI 兜底"**的处理策略。

### 2.1 第一层：高优先级实体的正则捕获

对于核心数据（如课程代码），系统通过确定性的正则表达式进行匹配，而非依赖大模型猜测：

```python
# 严格格式匹配：例如 CS-101 或 COMP202
code_pattern = re.compile(r'^[A-Z]{2,4}[-]?[0-9]{3,4}$', re.IGNORECASE)

# 宽松格式匹配：处理 OCR 或人工输入导致的变体
loose_code_pattern = re.compile(r'^[A-Z]{1,5}[-]?[0-9]{2,5}$', re.IGNORECASE)
```

只有通过正则表达式提取到的字段，才会被系统确认为高可信度的锚点数据。

### 2.2 第二层：基于状态机的上下文感知

许多课程明细列表中只写了"课程名"和"学分"，缺少"必修"还是"选修"的分类信息。文档的结构通常是：先出现一个大标题（如"三、专业核心必修课"），随后跟着若干门课程。

系统通过正则捕获章节标题，维护一个线性状态机：

```python
overview_table_pattern = re.compile(
    r'(公共基础|学科基础|专业方向).*?(必\s*修\s*课|选\s*修\s*课)'
)
current_state_type = "UNKNOWN"

for line in document:
    match = overview_table_pattern.search(line)
    if match:
        current_state_type = match.group(2).replace(" ", "")
        continue
    # 后续课程自动继承 current_state_type ...
```

这种带记忆的状态机设计，让缺乏独立分类信息的课程数据重新获得了完整的维度。

### 2.3 第三层：隐式特征的极限提取

学生录入数据的颗粒度往往较差，例如"大学物理实验(2学分,共36学时)"这样的长字符串。系统通过分隔符和括号体系进行精准切分：

```python
inline_credit_pattern = re.compile(r'(必\s*修\s*课|选\s*修\s*课)\s*(\d+)\s*学分')
parts = re.split(r'\s{2,}|\t|[()（）]', line)  # 兼容中英文全半角括号
```

---

## 3. 健壮性与降级机制

### 3.1 多级降级策略

系统实现了多级降级机制，确保在各种异常场景下仍能提供基本功能：

| 场景 | 降级行为 |
| :--- | :--- |
| AI 服务未启动 | C++ 客户端自动切换到本地规则分析（关键词匹配） |
| 模型加载中 | `/health` 端点返回 `model_loading: true`，客户端提示等待 |
| 网络超时 | `QNetworkReply` 超时后返回错误，客户端回退到本地规则 |
| PDF 解析失败 | 返回错误信息，不影响其他功能 |

C++ 端的降级逻辑（`AiService.cpp`）：

```cpp
QJsonObject AiService::chat(const QJsonObject& data) {
    if (isAiServerAvailable()) {
        QJsonObject aiResult = chatWithAi(data);
        if (!hasAiError(aiResult)) {
            aiResult["aiPowered"] = true;
            return aiResult;
        }
        resetAiServerCheck();
    }
    // 外部AI不可用时，使用本地规则分析
    return chatLocal(data);
}
```

### 3.2 健康检查与可用性缓存

`AiService` 实现了带缓存的健康检查机制：

- 可用时缓存 15 秒（`kAiAvailableCacheMs = 15000`），避免频繁请求；
- 不可用时缓存 3 秒（`kAiUnavailableRetryMs = 3000`），快速重试；
- 健康检查超时 2 秒（`kAiHealthTimeoutMs = 2000`），避免长时间阻塞 UI。

### 3.3 本地规则分析

当 AI 服务不可用时，`AiService` 提供基于关键词匹配的本地规则分析。覆盖以下场景：

- **课程分析**：统计已完成/进行中课程数、GPA、最薄弱/最优秀科目；
- **目标追踪**：统计目标完成进度、平均进度；
- **经历分析**：统计经历数量、类型分布；
- **简历建议**：统计简历素材覆盖率；
- **成果分析**：统计成果数量、验证状态；
- **综合分析**：汇总各维度数据，给出整体发展建议。

---

## 4. 智能上下文聚合机制

为了让 AI 大模型能够基于用户的真实学业数据给出精准建议，系统设计了一套"全量业务数据快照聚合"机制。

### 4.1 C++ 端数据快照构建（buildProjectContext）

当用户发起 AI 对话时，`AiService::buildProjectContext()` 会从各个 Service 层拉取最新数据：

| 数据维度 | 来源 | 提取内容 |
| :--- | :--- | :--- |
| 课程汇总 | `CourseService::getStatistics()` | GPA、加权平均分、总学分、学分进度 |
| 课程详情 | `CourseService::getAll()` | 每门课的成绩、学分、学期、状态 |
| 数据分析 | 遍历课程列表 | 最薄弱科目、学期 GPA 趋势 |
| 目标详情 | `GoalService::getAll()` | 目标进度、状态、截止日期 |
| 经历详情 | `ExperienceService::getAll()` | 实习/项目经历、进行中状态 |
| 成就详情 | `AchievementService::getAll()` | 荣誉奖项、级别、验证状态 |
| 角色详情 | `RoleService::getAll()` | 学生干部、社团职务 |
| 活动详情 | `ActivityService::getAll()` | 课外活动参与记录 |

每个维度的数据拉取均被 `try-catch` 包裹，某张表无数据则静默跳过，不会导致聚合崩溃。

### 4.2 System Prompt 注入

快照数据被格式化为带有语义标签的中文字符串，注入到发给 AI 的 System Prompt 中：

```cpp
if (context.contains("gpa"))
    ctxParts << QString("【GPA】%1").arg(context["gpa"].toDouble(), 0, 'f', 2);
if (context.contains("creditGap"))
    ctxParts << QString("【距毕业还差】%1 学分").arg(context["creditGap"].toDouble(), 0, 'f', 1);
```

最终构建的 System Prompt 包含完整的学业背景，使 AI 能够回答"GPA 趋势如何"、"哪门课最薄弱"等需要真实数据支撑的问题。

### 4.3 设计优势

1. **数据实时性**：`buildProjectContext()` 仅在用户发送消息时触发，AI 拿到的永远是当前最新的数据快照。
2. **低系统开销**：用户正常使用软件（浏览图表、录入课程）时，上下文构建逻辑完全休眠。
3. **无状态设计**：Python 服务端不需要查询数据库或维护 Session，桌面端客户端是私密数据的唯一物理主体。

---

## 5. Flask API 端点说明

`ai_server.py` 基于 Flask 框架，默认监听 `127.0.0.1:8001`，提供以下端点：

| 端点 | 方法 | 功能 |
| :--- | :--- | :--- |
| `/health` | GET | 健康检查，返回模型加载状态 |
| `/v1/status` | GET | 模型状态查询 |
| `/v1/chat/completions` | POST | 对话补全（兼容 OpenAI 格式） |
| `/v1/analyze` | POST | 学业数据分析（课程/职业/目标/综合） |
| `/parse-pdf` | POST | PDF 培养方案解析，返回按专业分组的课程列表 |
| `/parse_supplementary` | POST | 补充课程解析（通修/通识） |

### 5.1 对话补全接口

请求格式兼容 OpenAI Chat Completions API：

```json
{
    "messages": [
        {"role": "system", "content": "你是一个学业发展规划助手"},
        {"role": "user", "content": "请分析我的课程情况"}
    ],
    "max_tokens": 256,
    "temperature": 0.3
}
```

响应格式：
```json
{
    "id": "chatcmpl-20260620100000",
    "object": "chat.completion",
    "model": "Qwen2.5-1.5B-Instruct",
    "choices": [{
        "index": 0,
        "message": {"role": "assistant", "content": "..."},
        "finish_reason": "stop"
    }]
}
```

### 5.2 PDF 培养方案解析接口

以 `multipart/form-data` 上传 PDF 文件，支持：
- 主文件：培养方案 PDF
- 补充文件 `supplementary_tongxiu`：通修课程 PDF
- 补充文件 `supplementary_tongshi`：通识课程 PDF

返回按专业分组的课程列表和学分要求，支持从 pdfplumber 或 PyPDF2 两种库中自动选择。

---

## 6. 总结

`ai_server.py` 是学业发展规划系统的 AI 中枢，其核心设计原则是：

1. **确定性优先**：对课程代码、学分等结构化数据，优先使用正则表达式精确提取，而非依赖大模型猜测。
2. **优雅降级**：AI 服务不可用时，系统自动回退到本地规则分析，保证基本功能不受影响。
3. **无状态设计**：Python 服务端不维护任何用户状态，所有上下文由 C++ 客户端在请求时实时构建并注入。
4. **进程解耦**：AI 后端作为独立进程运行，与桌面客户端生命周期隔离，支持本地和远程两种部署模式。
