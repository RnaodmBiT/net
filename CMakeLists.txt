cmake_minimum_required(VERSION 3.2)
project(tk-net)

option(TK_BUILD_SHARED "TK_BUILD_SHARED" ON)

add_subdirectory(thirdparty/enet-1.3.13)
include_directories(thirdparty/enet-1.3.13/include)

file(GLOB SOURCES src/*.cpp)
file(GLOB HEADERS include/*.hpp)
add_definitions(-DTK_BUILD_NET)

set(BUILD_MODE STATIC)
if (TK_BUILD_SHARED)
    set(BUILD_MODE SHARED)
    add_definitions(-DTK_SHARED)
endif()

add_library(tk-net ${BUILD_MODE} ${SOURCES} ${HEADERS})
target_link_libraries(tk-net tk-core enet)