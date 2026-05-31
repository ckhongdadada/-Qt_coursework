# C++版技术解析文档 - 组员A（API层与路由开发）

## 1. 职责概述

在C++版本中，组员A负责HTTP API层开发、路由注册、请求解析与响应封装。使用Qt6的QHttpServer框架实现RESTful API端点，将HTTP请求转发到对应的Service层处理，并封装标准化的JSON响应。API层主要用于内部模块通信和扩展接口。

### 1.1 主要职责

| 职责领域 | 具体内容 |
|----------|----------|
| API设计 | RESTful API端点设计、请求/响应格式定义 |
| 路由开发 | QHttpServer路由注册、参数解析 |
| 请求处理 | JSON解析、参数验证、错误处理 |
| 响应封装 | 统一响应格式、状态码规范、CORS处理 |

---

## 2. 技术栈与核心依赖

### 2.1 核心框架

| 技术 | 版本 | 用途 |
|------|------|------|
| Qt6 Core | 6.x | 核心数据类型（QString、QJsonObject等） |
| Qt6 HttpServer | 6.x | HTTP服务器与路由 |
| Qt6 Network | 6.x | TCP监听与网络通信 |
| Qt6 Sql | 6.x | 数据库交互（间接依赖） |
| C++17 | - | 语言标准（结构化绑定、optional等） |
| CMake | 3.25+ | 构建系统 |

### 2.2 客户端API调用模式

**重要说明：** 客户端**不使用HTTP API**，而是**直接调用Service层**。API层仅作为内部模块通信和扩展接口使用。

#### 2.2.1 单模式架构

```
┌─────────────────────────────────────┐
│      Qt Desktop Client              │
│  ┌──────────┐                       │
│  │  Pages   │ ──直接调用──→ Service │
│  └──────────┘                       │
└─────────────────────────────────────┘
```

**为什么使用直接调用？**

| 原因 | 说明 |
|------|------|
| 性能优化 | 避免HTTP序列化/反序列化开销 |
| 延迟降低 | 避免网络延迟 |
| 简化处理 | 简化错误处理 |
| 提高性能 | 直接内存调用 |

#### 2.2.2 客户端直接调用示例

```cpp
class CoursesPage : public BasePage {
private slots:
    void onAddCourse() {
        CourseEditorDialog dialog(this);
        if (dialog.exec() == QDialog::Accepted) {
            Course course = dialog.course();
            Course created = CourseService::create(course);
            if (created.id > 0) {
                ToastNotification::display(this, "课程已创建");
                emit dataChanged(DataDomain::Courses);
            } else {
                ToastNotification::display(this, "创建失败，请重试");
            }
        }
    }
    
    void onEditCourse(int id) {
        Course course = CourseService::getById(id);
        if (course.id == 0) {
            ToastNotification::display(this, "课程不存在");
            return;
        }
        
        CourseEditorDialog dialog(this, course);
        if (dialog.exec() == QDialog::Accepted) {
            Course updated = CourseService::update(id, dialog.course());
            if (updated.id > 0) {
                ToastNotification::display(this, "课程已更新");
                emit dataChanged(DataDomain::Courses);
            }
        }
    }
    
    void onDeleteCourse(int id) {
        auto reply = QMessageBox::question(this, "确认删除", 
            "确定要删除这门课程吗？");
        if (reply == QMessageBox::Yes) {
            bool success = CourseService::remove(id);
            if (success) {
                ToastNotification::display(this, "课程已删除");
                emit dataChanged(DataDomain::Courses);
            }
        }
    }
};
```

#### 2.2.3 客户端Toast通知格式

```cpp
ToastNotification::display(parent, "操作成功");
ToastNotification::display(parent, "课程已创建");
ToastNotification::display(parent, "简历已导出");
ToastNotification::display(parent, "操作失败，请重试");
ToastNotification::display(parent, "课程不存在");
ToastNotification::display(parent, "导入失败");
ToastNotification::display(parent, "请先选择一个项目");
ToastNotification::display(parent, "请填写必填字段");
```

### 2.3 项目结构（API层）

```
cpp_project/src/
├── api/                    # API层（组员A主要负责）
│   ├── CourseApi.h         # 课程API
│   ├── RoleApi.h           # 角色API
│   ├── AchievementApi.h    # 成就API
│   ├── ExperienceApi.h     # 经验API
│   ├── ActivityApi.h       # 活动API
│   ├── GoalApi.h           # 目标API
│   ├── JobApi.h            # 岗位API
│   ├── DashboardApi.h      # 仪表盘API
│   ├── AnalyticsApi.h      # 分析API
│   ├── TimelineApi.h       # 时间轴API
│   ├── ImportApi.h         # 导入API
│   ├── ResumeApi.h         # 简历API
│   ├── AiApi.h             # AI功能API
│   └── AuthApi.h           # 认证API
├── server/
│   └── HttpServer.h        # HTTP服务器与路由注册
└── util/
    ├── JsonUtils.h         # JSON响应工具
    └── Logger.h            # 日志工具
```

---

## 3. 核心对象解析

### 3.1 HttpServer 类 (server/HttpServer.h)

**创建的对象：**

| 对象名 | 类型 | 用途 |
|--------|------|------|
| `m_server` | QHttpServer | Qt6 HTTP服务器实例 |
| `m_tcpServer` | QTcpServer | TCP监听器 |

**核心方法：**

| 方法 | 返回值 | 用途 |
|------|--------|------|
| `start(quint16 port)` | bool | 启动HTTP服务器 |
| `registerRoutes()` | void | 注册所有API路由 |

**路由注册模式：**

```cpp
m_server.route("/api/courses", QHttpServerRequest::Method::Get,
    [](const QHttpServerRequest& req) { return CourseApi::getAll(req); });

m_server.route("/api/courses/<arg>", QHttpServerRequest::Method::Get,
    [](int id, const QHttpServerRequest&) { return CourseApi::getById(id); });
```

**CORS中间件：**

```cpp
m_server.afterRequest([](QHttpServerResponse&& resp) {
    resp.setHeader("Access-Control-Allow-Origin", "*");
    resp.setHeader("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
    resp.setHeader("Access-Control-Allow-Headers", "Content-Type, Authorization, X-Auth-Token");
    return std::move(resp);
});
```

### 3.2 JsonUtils 工具类 (util/JsonUtils.h)

**创建的静态方法：**

| 方法 | 参数 | 返回值 | 用途 |
|------|------|--------|------|
| `createResponse()` | success, data, message, code | QHttpServerResponse | 创建通用响应 |
| `successResponse()` | data, message | QHttpServerResponse | 成功响应(200) |
| `createdResponse()` | data, message | QHttpServerResponse | 创建成功响应(201) |
| `errorResponse()` | message, code | QHttpServerResponse | 错误响应(400) |
| `notFoundResponse()` | message | QHttpServerResponse | 未找到响应(404) |

**统一响应格式：**

```json
{
    "success": true,
    "code": 200,
    "message": "操作成功",
    "data": { ... }
}
```

### 3.3 Logger 工具类 (util/Logger.h)

**创建的静态方法：**

| 方法 | 用途 |
|------|------|
| `info(message)` | 输出INFO级别日志 |
| `warning(message)` | 输出WARN级别日志 |
| `error(message)` | 输出ERROR级别日志 |
| `debug(message)` | 输出DEBUG级别日志 |

**日志格式：** `[2026-04-27 10:30:00] [INFO] 服务器已启动`

---

## 4. API端点详细解析

### 4.1 CourseApi 类

**创建的静态方法：**

| 方法 | HTTP方法 | 路径 | 功能 |
|------|----------|------|------|
| `getAll()` | GET | `/api/courses` | 获取所有课程 |
| `getById(id)` | GET | `/api/courses/<id>` | 获取单个课程 |
| `create(request)` | POST | `/api/courses` | 创建课程 |
| `update(id, request)` | PUT | `/api/courses/<id>` | 更新课程 |
| `remove(id)` | DELETE | `/api/courses/<id>` | 删除课程 |
| `getStatistics(request)` | GET | `/api/courses/statistics` | 获取统计数据 |

**请求解析流程：**

```cpp
static QHttpServerResponse create(const QHttpServerRequest& request) {
    QJsonDocument doc = QJsonDocument::fromJson(request.body());
    if (!doc.isObject()) return JsonUtils::errorResponse("无效的JSON数据");
    
    Course course = Course::fromDict(doc.object());
    course = CourseService::create(course);
    
    if (course.id == 0) return JsonUtils::errorResponse("创建课程失败");
    return JsonUtils::createdResponse(course.toDict(), "课程创建成功");
}
```

### 4.2 AuthApi 类

**创建的静态方法：**

| 方法 | HTTP方法 | 路径 | 功能 |
|------|----------|------|------|
| `registerUser(request)` | POST | `/api/auth/register` | 用户注册 |
| `login(request)` | POST | `/api/auth/login` | 用户登录 |
| `getMe(request)` | GET | `/api/auth/me` | 获取当前用户 |
| `updateMe(request)` | PUT | `/api/auth/me` | 更新用户资料 |
| `changePassword(request)` | POST | `/api/auth/change-password` | 修改密码 |

**Token解析逻辑：**

```cpp
static int extractUserIdFromToken(const QString& token) {
    QStringList parts = token.split('.');
    if (parts.size() >= 1) {
        QByteArray decoded = QByteArray::fromBase64(parts[0].toUtf8());
        bool ok;
        int userId = decoded.toInt(&ok);
        return ok ? userId : 0;
    }
    return 0;
}
```

### 4.3 DashboardApi 类

| 方法 | HTTP方法 | 路径 | 功能 |
|------|----------|------|------|
| `getOverview(request)` | GET | `/api/dashboard/overview` | 获取总览数据 |
| `getGpaTrend(request)` | GET | `/api/dashboard/gpa-trend` | 获取GPA趋势 |
| `getRecommendations(request)` | GET | `/api/dashboard/recommendations` | 获取推荐建议 |

### 4.4 AnalyticsApi 类

| 方法 | HTTP方法 | 路径 | 功能 |
|------|----------|------|------|
| `getSemesterComparison()` | GET | `/api/analytics/semester-comparison` | 学期对比 |
| `getPeerComparison()` | GET | `/api/analytics/peer-comparison` | 同学对比 |
| `getReport()` | GET | `/api/analytics/report` | 生成分析报告 |
| `getPeers()` | GET | `/api/analytics/peers` | 获取同学列表 |
| `createPeer(request)` | POST | `/api/analytics/peers` | 添加同学数据 |
| `updatePeer(id, request)` | PUT | `/api/analytics/peers/<id>` | 更新同学数据 |
| `deletePeer(id)` | DELETE | `/api/analytics/peers/<id>` | 删除同学数据 |

### 4.5 ResumeApi 类

| 方法 | HTTP方法 | 路径 | 功能 |
|------|----------|------|------|
| `generate(request)` | POST | `/api/resume/generate` | 生成简历数据 |
| `exportJson(request)` | POST | `/api/resume/export/json` | 导出JSON格式 |
| `exportHtml(request)` | POST | `/api/resume/export/html` | 导出HTML格式 |

### 4.6 AiApi 类

| 方法 | HTTP方法 | 路径 | 功能 |
|------|----------|------|------|
| `analyze(request)` | POST | `/api/ai/analyze` | AI分析 |
| `checkStatus(request)` | GET | `/api/ai/status` | 检查AI状态 |
| `chat(request)` | POST | `/api/ai/chat` | AI聊天 |

### 4.7 ImportApi 类

| 方法 | HTTP方法 | 路径 | 功能 |
|------|----------|------|------|
| `importEntity(entity, request)` | POST | `/api/imports/<entity>` | 导入CSV数据 |

---

## 5. 路由注册完整列表

### 5.1 路由注册顺序

HttpServer中的路由注册顺序很重要，**更具体的路径必须先注册**：

```cpp
m_server.route("/api/courses/statistics", Get, ...);  // 先注册
m_server.route("/api/courses", Get, ...);              // 后注册
m_server.route("/api/courses/<arg>", Get, ...);        // 参数路由最后
```

### 5.2 完整路由表

| 路径 | 方法 | 处理函数 |
|------|------|----------|
| `/api/courses/statistics` | GET | CourseApi::getStatistics |
| `/api/courses` | GET | CourseApi::getAll |
| `/api/courses` | POST | CourseApi::create |
| `/api/courses/<id>` | GET | CourseApi::getById |
| `/api/courses/<id>` | PUT | CourseApi::update |
| `/api/courses/<id>` | DELETE | CourseApi::remove |
| `/api/roles/statistics` | GET | RoleApi::getStatistics |
| `/api/roles` | GET/POST | RoleApi::getAll/create |
| `/api/roles/<id>` | GET/PUT/DELETE | RoleApi::getById/update/remove |
| `/api/achievements/statistics` | GET | AchievementApi::getStatistics |
| `/api/achievements` | GET/POST | AchievementApi::getAll/create |
| `/api/achievements/<id>` | GET/PUT/DELETE | AchievementApi::getById/update/remove |
| `/api/experiences/statistics` | GET | ExperienceApi::getStatistics |
| `/api/experiences` | GET/POST | ExperienceApi::getAll/create |
| `/api/experiences/<id>` | GET/PUT/DELETE | ExperienceApi::getById/update/remove |
| `/api/activities` | GET/POST | ActivityApi::getAll/create |
| `/api/activities/<id>` | GET/PUT/DELETE | ActivityApi::getById/update/remove |
| `/api/goals/statistics` | GET | GoalApi::getStatistics |
| `/api/goals` | GET/POST | GoalApi::getAll/create |
| `/api/goals/<id>` | GET/PUT/DELETE | GoalApi::getById/update/remove |
| `/api/jobs` | GET/POST | JobApi::getAll/create |
| `/api/jobs/import` | POST | JobApi::importJobs |
| `/api/jobs/<id>` | GET/PUT/DELETE | JobApi::getById/update/remove |
| `/api/jobs/<id>/requirements/<idx>/toggle` | POST | JobApi::toggleRequirement |
| `/api/dashboard/overview` | GET | DashboardApi::getOverview |
| `/api/dashboard/gpa-trend` | GET | DashboardApi::getGpaTrend |
| `/api/dashboard/recommendations` | GET | DashboardApi::getRecommendations |
| `/api/analytics/semester-comparison` | GET | AnalyticsApi::getSemesterComparison |
| `/api/analytics/peer-comparison` | GET | AnalyticsApi::getPeerComparison |
| `/api/analytics/report` | GET | AnalyticsApi::getReport |
| `/api/analytics/peers` | GET/POST | AnalyticsApi::getPeers/createPeer |
| `/api/analytics/peers/<id>` | PUT/DELETE | AnalyticsApi::updatePeer/deletePeer |
| `/api/timeline` | GET | TimelineApi::getAll |
| `/api/imports/<entity>` | POST | ImportApi::importEntity |
| `/api/resume/generate` | POST | ResumeApi::generate |
| `/api/resume/export/json` | POST | ResumeApi::exportJson |
| `/api/resume/export/html` | POST | ResumeApi::exportHtml |
| `/api/ai/analyze` | POST | AiApi::analyze |
| `/api/ai/status` | GET | AiApi::checkStatus |
| `/api/ai/chat` | POST | AiApi::chat |
| `/api/auth/register` | POST | AuthApi::registerUser |
| `/api/auth/login` | POST | AuthApi::login |
| `/api/auth/me` | GET/PUT | AuthApi::getMe/updateMe |
| `/api/auth/change-password` | POST | AuthApi::changePassword |

---

## 6. 请求处理模式

### 6.1 标准CRUD模式

```cpp
class XxxApi {
public:
    static QHttpServerResponse getAll(const QHttpServerRequest&) {
        QList<Xxx> list = XxxService::getAll();
        QJsonArray arr;
        for (const auto& item : list) arr.append(item.toDict());
        return JsonUtils::successResponse(arr);
    }

    static QHttpServerResponse getById(int id) {
        Xxx item = XxxService::getById(id);
        if (item.id == 0) return JsonUtils::notFoundResponse("资源不存在");
        return JsonUtils::successResponse(item.toDict());
    }

    static QHttpServerResponse create(const QHttpServerRequest& request) {
        QJsonDocument doc = QJsonDocument::fromJson(request.body());
        if (!doc.isObject()) return JsonUtils::errorResponse("无效的JSON数据");
        Xxx item = Xxx::fromDict(doc.object());
        item = XxxService::create(item);
        if (item.id == 0) return JsonUtils::errorResponse("创建失败");
        return JsonUtils::createdResponse(item.toDict(), "创建成功");
    }

    static QHttpServerResponse update(int id, const QHttpServerRequest& request) {
        QJsonDocument doc = QJsonDocument::fromJson(request.body());
        if (!doc.isObject()) return JsonUtils::errorResponse("无效的JSON数据");
        Xxx item = Xxx::fromDict(doc.object());
        item = XxxService::update(id, item);
        if (item.id == 0) return JsonUtils::notFoundResponse("资源不存在");
        return JsonUtils::successResponse(item.toDict(), "更新成功");
    }

    static QHttpServerResponse remove(int id) {
        bool success = XxxService::remove(id);
        if (!success) return JsonUtils::errorResponse("删除失败");
        return JsonUtils::successResponse(QJsonValue(), "删除成功");
    }
};
```

### 6.2 查询参数解析

```cpp
static QHttpServerResponse getStatistics(const QHttpServerRequest& request) {
    QString scale = "standard";
    double creditTarget = 120;
    QUrlQuery query(request.url());
    if (query.hasQueryItem("scale")) 
        scale = query.queryItemValue("scale");
    if (query.hasQueryItem("creditTarget")) 
        creditTarget = query.queryItemValue("creditTarget").toDouble();
    QJsonObject stats = CourseService::getStatistics(scale, creditTarget);
    return JsonUtils::successResponse(stats);
}
```

### 6.3 文件下载响应

```cpp
static QHttpServerResponse exportJson(const QHttpServerRequest& request) {
    QJsonDocument doc = QJsonDocument::fromJson(request.body());
    QJsonObject options = doc.isObject() ? doc.object() : QJsonObject();
    QByteArray data = ResumeService::exportJson(options);
    QHttpServerResponse response(data, QHttpServerResponse::StatusCode::Ok);
    response.setHeader("Content-Type", "application/json; charset=UTF-8");
    response.setHeader("Content-Disposition", "attachment; filename=resume.json");
    return response;
}
```

---

## 7. API层设计特点

### 7.1 核心技术方案

| 对比项 | 技术方案 |
|--------|----------|
| 路由框架 | QHttpServer route |
| 请求解析 | QJsonDocument::fromJson(request.body()) |
| 响应封装 | JsonUtils::successResponse() |
| CORS处理 | afterRequest中间件 |
| 路由参数 | `<arg>` + lambda参数 |
| 查询参数 | QUrlQuery(request.url()) |

### 7.2 设计特点

**API层设计：**

| 特点 | 说明 |
|------|------|
| 静态类方法 | 使用静态类方法组织API |
| Lambda表达式 | 使用Lambda注册路由 |
| 手动JSON序列化 | 使用QJsonObject手动处理 |

---

## 8. 开发规范

### 8.1 API类设计规范

| 规范 | 说明 |
|------|------|
| 静态方法 | 每个API类只包含静态方法 |
| 无状态 | 不持有状态，无成员变量 |
| 统一签名 | 方法签名统一：`static QHttpServerResponse methodName(...)` |
| 统一返回 | 返回值统一使用 `JsonUtils` 封装 |

### 8.2 命名规范

| 类型 | 规范 | 示例 |
|------|------|------|
| API类名 | PascalCase + Api后缀 | CourseApi, AuthApi |
| 方法名 | camelCase | getAll, getById, create, update, remove |
| 路由路径 | kebab-case | /gpa-trend, /semester-comparison |

---

## 9. 总结

### 9.1 工作成果

| 成果 | 数量 |
|------|------|
| API类 | 14个 |
| API端点 | 50+个 |
| 功能模块覆盖 | 14个 |

### 9.2 功能模块覆盖

| 模块 | API数量 | 状态 |
|------|---------|------|
| 课程管理 | 6个 | ✅ |
| 角色管理 | 6个 | ✅ |
| 成就管理 | 6个 | ✅ |
| 经验管理 | 6个 | ✅ |
| 活动管理 | 5个 | ✅ |
| 目标管理 | 6个 | ✅ |
| 岗位管理 | 7个 | ✅ |
| 仪表盘 | 3个 | ✅ |
| 分析中心 | 7个 | ✅ |
| 时间轴 | 1个 | ✅ |
| 数据导入 | 1个 | ✅ |
| 简历导出 | 3个 | ✅ |
| AI助手 | 3个 | ✅ |
| 用户认证 | 5个 | ✅ |

在C++版本中，组员A负责API层和路由开发，使用Qt6的QHttpServer框架实现了标准化的RESTful API接口。通过统一的请求解析模式、响应封装工具和CORS中间件，确保了接口的一致性和可扩展性。

---

**文档版本**: v1.1  
**更新日期**: 2026-04-29  
**适用项目版本**: v0.3.0
