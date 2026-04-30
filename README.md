# Personal Development Planner - Stage 1

This repository records the staged development process of a C++ personal development planning system.

Stage 1 focuses on the backend foundation and data architecture. The Qt desktop interface is intentionally not included yet, so the project history can show a clear development progression.

## Stage 1 Scope

- SQLite database schema
- C++ data models
- DAO data access layer
- Service business layer
- Qt HttpServer REST API
- Standalone backend executable: `pdp_server`

## Project Structure

```text
src/
  api/       REST API handlers
  dao/       SQLite data access objects
  model/     domain models
  server/    Qt HTTP server wrapper
  service/   business services
  util/      common helpers
  main.cpp   backend server entry
resources/
  schema.sql database schema
```

## Build

```powershell
cmake -S . -B build -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=C:\Qt\6.11.0\mingw_64
cmake --build build --target pdp_server
```

## Run

```powershell
.\build\pdp_server.exe
```

Default port: `8080`.

## Next Stages

- Stage 2: Qt desktop shell and navigation layout
- Stage 3: business pages and CRUD dialogs
- Stage 4: resume editor, analysis page, import tools and AI assistant
- Stage 5: reports, documentation, packaging and final submission materials
