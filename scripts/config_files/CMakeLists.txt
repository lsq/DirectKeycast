cmake_minimum_required(VERSION 3.25.0)
project(DirectKeycast VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_definitions(
    /D_UNICODE=1
    /DUNICODE=1
    /DNOMINMAX=1
)

# add_definitions(
#     /DFANY_DEBUG
# )

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include_directories("./includes/")

set(MSIMEUI_DIR "${CMAKE_SOURCE_DIR}/third_party/msimeui")
if(NOT EXISTS "${MSIMEUI_DIR}/CMakeLists.txt")
    message(FATAL_ERROR
        "msimeui submodule is missing at ${MSIMEUI_DIR}. "
        "Run: git submodule update --init --recursive")
endif()
add_subdirectory("${MSIMEUI_DIR}" EXCLUDE_FROM_ALL)

set(SRC_FILES
    # headers
    "./includes/config.h"
    "./includes/CTimer.h"
    "./includes/d2d.h"
    "./includes/globals.h"
    "./includes/hook.h"
    "./includes/InitConsole.h"
    "./includes/main.h"
    "./includes/msimeui_host.h"
    "./includes/settings.h"
    "./includes/theme.h"
    "./includes/tray_menu.h"
    "./includes/utils.h"
    "./includes/window.h"
    # source files
    "./src/config.cpp"
    "./src/d2d.cpp"
    "./src/hook.cpp"
    "./src/main.cpp"
    "./src/msimeui_host.cpp"
    "./src/settings.cpp"
    "./src/theme.cpp"
    "./src/tray_menu.cpp"
    "./src/utils.cpp"
    "./src/window.cpp"
)

set(MY_EXECUTABLE_NAME "directkeycast")

add_executable(${MY_EXECUTABLE_NAME} WIN32
    ${SRC_FILES}
)

target_include_directories(${MY_EXECUTABLE_NAME}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/includes
)

target_link_libraries(${MY_EXECUTABLE_NAME} PRIVATE msimeui)

add_custom_command(TARGET ${MY_EXECUTABLE_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:${MY_EXECUTABLE_NAME}>/assets/config
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_SOURCE_DIR}/assets/config/config.toml
        $<TARGET_FILE_DIR:${MY_EXECUTABLE_NAME}>/assets/config/config.toml
)
