cmake_minimum_required(VERSION 2.8.8)

project(Flutter C)

set(Flutter_MAJOR_VERSION 0)
set(Flutter_MINOR_VERSION 3)
set(Flutter_PATCH_VERSION 0)
set(Flutter_VERSION
  ${Flutter_MAJOR_VERSION}.${Flutter_MINOR_VERSION}.${Flutter_PATCH_VERSION})

set(LUA_VERSION 5.2.0)
set(LUA_CJSON_VERSION 2.1.0)

message(STATUS "Building Flutter v${Flutter_VERSION}")
message(STATUS "  Static compile LUA v${LUA_VERSION}")
message(STATUS "  Static compile LUA-cjson v${LUA_CJSON_VERSION}")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${Flutter_SOURCE_DIR}/modules/")

find_package(LibMicroHTTPD)
find_package(PostgreSQL)

option(USE_INTERNAL_FPCONV "Use internal strtod() / g_fmt() code for performance" ON)
option(MULTIPLE_THREADS "Support multi-threaded apps with internal fpconv - recommended" ON)

set(LUA_INCLUDE_DIRS
    ${Flutter_SOURCE_DIR}/lua-static/lua-${LUA_VERSION}/src
    ${Flutter_SOURCE_DIR}/lua-static/luasql
    ${Flutter_SOURCE_DIR}/lua-static
)

foreach (source_dir ${LUA_INCLUDE_DIRS})
    file(GLOB LUA_SRCS ${LUA_SRCS} ${source_dir}/*.c)
endforeach(source_dir)

file(GLOB LUA_SRCS ${LUA_SRCS} 
    ${Flutter_SOURCE_DIR}/lua-static/lua-cjson-2.1.0/lua_cjson.c
    ${Flutter_SOURCE_DIR}/lua-static/lua-cjson-2.1.0/strbuf.c
    ${Flutter_SOURCE_DIR}/lua-static/lua-cjson-2.1.0/fpconv.c
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
add_subdirectory(src)

