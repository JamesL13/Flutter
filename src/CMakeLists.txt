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
# src/CMakeLists.txt - build targets
#
cmake_minimum_required(VERSION 2.8.8)

file(GLOB Flutter_SRCS *.c)

add_executable(Flutter ${Flutter_SRCS} ${LUA_SRCS})
include_directories(Flutter
        ${LUA_INCLUDE_DIRS}
        ${MHD_INCLUDE_DIRS}
        ${PostgreSQL_INCLUDE_DIRS}
)
target_link_libraries(Flutter m
        ${MHD_LIBRARIES}
        ${PostgreSQL_LIBRARIES}
)
set_target_properties(Flutter PROPERTIES COMPILE_FLAGS "-O3 -Wall")

add_executable(Flutter_d ${Flutter_SRCS} ${LUA_SRCS})
include_directories(Flutter_d
        ${LUA_INCLUDE_DIRS}
        ${MHD_INCLUDE_DIRS}
        ${PostgreSQL_INCLUDE_DIRS}
)
target_link_libraries(Flutter_d m
        ${MHD_LIBRARIES}
        ${PostgreSQL_LIBRARIES}
)
set_target_properties(Flutter_d PROPERTIES COMPILE_FLAGS "-g -Wall")

