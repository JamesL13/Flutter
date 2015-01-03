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

