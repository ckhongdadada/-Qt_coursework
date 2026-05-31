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
option(PDP_BUILD_DESKTOP "Build the desktop application" ON)

find_package(Qt6 REQUIRED COMPONENTS Core Sql HttpServer Network Widgets PrintSupport Test)

set(PDP_COMMON_SOURCES
    src/model/Course.cpp
    src/dao/DaoBase.cpp
    src/service/AiService.cpp
)

set(PDP_COMMON_INCLUDE_DIRS
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/src/client
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

if(PDP_BUILD_DESKTOP)
    qt_add_executable(pdp_desktop
        src/main_desktop.cpp
        src/client/MainWindow.cpp
        src/client/dialogs/ProfileEditorDialog.cpp
        src/client/dialogs/CourseEditorDialog.cpp
        src/client/dialogs/GoalEditorDialog.cpp
        src/client/dialogs/RoleEditorDialog.cpp
        src/client/dialogs/AchievementEditorDialog.cpp
        src/client/dialogs/ExperienceEditorDialog.cpp
        src/client/dialogs/ActivityEditorDialog.cpp
        src/client/dialogs/JobEditorDialog.cpp
        src/client/dialogs/PeerEditorDialog.cpp
        src/client/utils/UiHelpers.cpp
  src/client/widgets/ToastNotification.cpp
        src/client/widgets/SidebarWidget.cpp
        src/client/widgets/AiPanelWidget.cpp
        src/client/widgets/NavigationListWidget.cpp
        src/client/widgets/TimeInfoCard.cpp
        src/client/widgets/StudentInfoCard.cpp
        src/client/widgets/AiStatusBar.cpp
        src/client/widgets/AiConversationWidget.cpp
        src/client/widgets/ResumePreviewWidget.cpp
        src/client/widgets/ResumePreviewDialog.cpp
        src/client/widgets/ResumeEditorPanel.cpp
        src/client/widgets/ResumeCandidatePanel.cpp
        src/client/widgets/SemesterAnalysisWidget.cpp
        src/client/widgets/PeerBenchmarkWidget.cpp
        src/client/widgets/MetricGridWidget.cpp
        src/client/widgets/SuggestionListWidget.cpp
        src/client/widgets/CrudPageShell.cpp
        src/client/pages/BasePage.cpp
        src/client/pages/CoursesPage.cpp
        src/client/pages/RolesPage.cpp
        src/client/pages/AchievementsPage.cpp
        src/client/pages/ExperiencesPage.cpp
        src/client/pages/ActivitiesPage.cpp
        src/client/pages/GoalsPage.cpp
        src/client/pages/JobsPage.cpp
        src/client/pages/AnalysisPage.cpp
        src/client/pages/TimelinePage.cpp
        src/client/pages/ResumePage.cpp
        src/client/pages/OverviewPage.cpp
        src/client/pages/ImportsPage.cpp
        src/client/core/AppShellController.cpp
        src/client/core/DataRefreshCoordinator.cpp
        src/client/core/BackendRuntimeController.cpp
        src/client/core/AiContextMediator.cpp
        src/client/core/CrudPageController.cpp
        resources/resources.qrc
        ${PDP_COMMON_SOURCES}
    )

    target_link_libraries(pdp_desktop PRIVATE
        Qt6::Core
        Qt6::Sql
        Qt6::HttpServer
        Qt6::Network
        Qt6::Widgets
        Qt6::PrintSupport
    )

    target_include_directories(pdp_desktop PRIVATE
        ${PDP_COMMON_INCLUDE_DIRS}
        "${CMAKE_BINARY_DIR}/generated"
    )

    if(WIN32)
        set_target_properties(pdp_desktop PROPERTIES WIN32_EXECUTABLE TRUE)
    endif()
endif()

file(COPY ${CMAKE_SOURCE_DIR}/resources/schema.sql DESTINATION ${CMAKE_BINARY_DIR}/resources)

if(EXISTS "${CMAKE_SOURCE_DIR}/frontend_dist")
    file(COPY ${CMAKE_SOURCE_DIR}/frontend_dist DESTINATION ${CMAKE_BINARY_DIR})
endif()

if(PDP_BUILD_SERVER)
    install(TARGETS pdp_server
        RUNTIME DESTINATION bin
    )
endif()

if(PDP_BUILD_DESKTOP)
    install(TARGETS pdp_desktop
        RUNTIME DESTINATION bin
    )
endif()

option(PDP_BUILD_TESTS "Build the test executable" ON)

if(PDP_BUILD_TESTS)
    add_executable(smoke_test
        tests/smoke_test.cpp
        ${PDP_COMMON_SOURCES}
    )

    target_link_libraries(smoke_test PRIVATE
        Qt6::Core
        Qt6::Sql
        Qt6::Test
        Qt6::Network
    )

    target_include_directories(smoke_test PRIVATE
        ${PDP_COMMON_INCLUDE_DIRS}
        "${CMAKE_BINARY_DIR}/generated"
    )
endif()
