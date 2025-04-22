cmake_minimum_required(VERSION 3.28)

add_library(llvm-tool)

target_include_directories(llvm-tool PRIVATE .)

set(SOURCE
        callback.h
        noise.h
        runtime_interface.cpp
        runtime.cpp
        runtime.h
        tool.cpp
        tool.h
        types.h
        thread.h
        thread.cpp
        analysers/eraser.cpp
        analysers/atomrace.cpp
        analysers/fasttrack2.cpp
)

target_sources(llvm-tool PRIVATE ${SOURCE})
