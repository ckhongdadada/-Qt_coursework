cmake_minimum_required(VERSION 3.25)

file(STRINGS "${CMAKE_SOURCE_DIR}/VERSION" PDP_VERSION LIMIT_COUNT 1)
project(pdp LANGUAGES CXX VERSION ${PDP_VERSION})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

configure_file(
    "${CMAKE_SOURCE_DIR}/src/config/Version.h.in"
    "${CMAKE_BINARY_DIR}/generated/Version.h"
)

if(MSVC)
    add_compile_options(/utf-8)
elseif(MINGW OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    add_compile_options(-finput-charset=utf-8 -fexec-charset=utf-8)
endif()

option(PDP_BUILD_SERVER "Build the standalone HTTP server" ON)
option(PDP_BUILD_TESTS "Build smoke tests" OFF)

find_package(Qt6 REQUIRED COMPONENTS Core Sql HttpServer Network Test)

set(PDP_COMMON_SOURCES
    src/model/Course.cpp
    src/dao/DaoBase.cpp
    src/service/AiService.cpp
)

set(PDP_COMMON_INCLUDE_DIRS
    ${PROJECT_SOURCE_DIR}/src
    "${CMAKE_BINARY_DIR}/generated"
)

if(PDP_BUILD_SERVER)
    add_executable(pdp_server
        src/main.cpp
        ${PDP_COMMON_SOURCES}
    )

    target_link_libraries(pdp_server PRIVATE
        Qt6::Core
        Qt6::Sql
        Qt6::HttpServer
        Qt6::Network
    )

    target_include_directories(pdp_server PRIVATE
        ${PDP_COMMON_INCLUDE_DIRS}
    )
endif()

file(COPY ${CMAKE_SOURCE_DIR}/resources/schema.sql DESTINATION ${CMAKE_BINARY_DIR}/resources)

if(PDP_BUILD_SERVER)
    install(TARGETS pdp_server RUNTIME DESTINATION bin)
endif()
