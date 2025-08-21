cmake_minimum_required(VERSION 3.25.0)
project(DirectKeycast VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_definitions(
    /D_UNICODE=1
    /DUNICODE=1
)

# add_definitions(
#     /DFANY_DEBUG
# )

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include_directories("./includes/")

set(SRC_FILES
    # headers
    "./includes/config.h"
    "./includes/CTimer.h"
    "./includes/d2d.h"
    "./includes/globals.h"
    "./includes/hook.h"
    "./includes/InitConsole.h"
    "./includes/main.h"
    "./includes/utils.h"
    "./includes/window.h"
    # source files
    "./src/config.cpp"
    "./src/d2d.cpp"
    "./src/hook.cpp"
    "./src/main.cpp"
    "./src/utils.cpp"
    "./src/window.cpp"
)

set(MY_EXECUTABLE_NAME "directkeycast")
# must before add_executable
if(MSVC)
    add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
# else()
    # add_compile_options("-finput-charset=UTF-8 -fexec-charset=UTF-8")
endif()
add_executable(${MY_EXECUTABLE_NAME} WIN32
    ${SRC_FILES}
)

target_include_directories(${MY_EXECUTABLE_NAME}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/includes
)
if (MINGW)
    target_link_libraries(${MY_EXECUTABLE_NAME} d2d1 dwrite dwmapi -municode)
endif()
