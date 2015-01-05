# The MIT License (MIT)
# 
# Flutter
# Copyright (c) 2015 William Whitacre
# 
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

#
# CMakeLists.txt - project build file.
#

cmake_minimum_required(VERSION 2.8.8)

project(Flutter C)

if( EXISTS "${Flutter_SOURCE_DIR}/.git" )

# TODO: make this work with annotated tags.
execute_process(COMMAND git describe --tags --abbrev=4 HEAD
               COMMAND sed -e "s/-/./g"
               OUTPUT_VARIABLE Flutter_VERSION
               OUTPUT_STRIP_TRAILING_WHITESPACE)

execute_process( COMMAND bash -c "echo \"${Flutter_VERSION}\" > ${Flutter_SOURCE_DIR}/etc/version" )

string(REGEX REPLACE "(.*)\\.(.*)\\.(.*)\\.(.*)" "\\1" Flutter_MAJOR_VERSION ${Flutter_VERSION})
string(REGEX REPLACE "(.*)\\.(.*)\\.(.*)\\.(.*)" "\\2" Flutter_MINOR_VERSION ${Flutter_VERSION})
string(REGEX REPLACE "(.*)\\.(.*)\\.(.*)\\.(.*)" "\\3" Flutter_PATCH_VERSION ${Flutter_VERSION})

execute_process( COMMAND bash -c "echo \"${Flutter_MAJOR_VERSION}\" > ${Flutter_SOURCE_DIR}/etc/vmaj" )
execute_process( COMMAND bash -c "echo \"${Flutter_MINOR_VERSION}\" > ${Flutter_SOURCE_DIR}/etc/vmin" )
execute_process( COMMAND bash -c "echo \"${Flutter_PATCH_VERSION}\" > ${Flutter_SOURCE_DIR}/etc/vpat" )

else( EXISTS "${Flutter_SOURCE_DIR}/.git" )

execute_process(COMMAND cat ${Flutter_SOURCE_DIR}/etc/version
               OUTPUT_VARIABLE Flutter_VERSION
               OUTPUT_STRIP_TRAILING_WHITESPACE)

endif( EXISTS "${Flutter_SOURCE_DIR}/.git" )

execute_process( COMMAND bash -c "${Flutter_SOURCE_DIR}/etc/version.sh > ${Flutter_SOURCE_DIR}/src/version.h" )

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

