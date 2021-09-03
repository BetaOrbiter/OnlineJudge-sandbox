cmake_minimum_required(VERSION 2.8)
project(y_judge C)
LINK_LIBRARIES("pthread")
LINK_LIBRARIES("seccomp")
set(CMAKE_C_FLAGS "-O3 -std=gnu99 -pie -fPIC -pthread")
add_executable(
        judge_core
        src/main.c
        src/common/common.c
        src/common/common.h
        src/executant/executant.c
        src/executant/executant.h
        src/monitor/monitor.c
        src/monitor/monitor.h
        )