cmake_minimum_required(VERSION 3.4.1)

enable_language(ASM)    #支持汇编
add_executable(SharkInject main.cpp PtraceInject.cpp ptrace_utils.cpp module_utils.cpp)

find_library(log-lib log)

target_link_libraries(SharkInject ${log-lib} )