# Stage 1 - 数据层与后端接口基线

本阶段建立个人发展规划系统的 C++ 工程基础，重点完成：

- SQLite 数据库 schema
- Model / DAO / Service 分层
- Qt HttpServer REST API
- 独立后端服务 `pdp_server`
- 基础 AI 服务接口占位

此阶段暂不包含 Qt 桌面界面，桌面 UI 将在后续阶段逐步加入。

## 构建

```powershell
cmake -S . -B build -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=C:\Qt\6.11.0\mingw_64
cmake --build build --target pdp_server
```

## 运行

```powershell
.\build\pdp_server.exe
```

默认监听端口：`8080`。
