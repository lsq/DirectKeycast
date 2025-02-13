cmake_minimum_required(VERSION 3.25.0)
project(DirectKeycast VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_definitions(
    /D_UNICODE=1
    /DUNICODE=1
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include_directories("./includes/")

set(HEADER_FILES
    ./includes/main.h
)

set(SRC_FILES
    ./src/main.cpp
)

set(MY_EXECUTABLE_NAME "directkeycast")

add_executable(${MY_EXECUTABLE_NAME} WIN32
    ${HEADER_FILES}
    ${SRC_FILES}
)

target_include_directories(${MY_EXECUTABLE_NAME}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/includes
)
