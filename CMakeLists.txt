cmake_minimum_required(VERSION 3.28)
project(LAChesis)

add_library(lachesis-lib)

target_include_directories(lachesis-lib PRIVATE .)


target_sources(lachesis-lib PRIVATE 
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
        analysers/fasttrack2.cpp)

# set to modified llvm build executables
set(CMAKE_C_COMPILER             "")
set(CMAKE_CXX_COMPILER             "")

set(LACHESIS_ANALYZE  "" CACHE FILEPATH "path to a file to be analyzed")

add_executable(instrumented_executable)

target_sources(instrumented_executable PRIVATE ${LLVMTOOL_ANALYZE})

target_compile_options(instrumented_executable PRIVATE -mllvm -lachesis -g)

target_link_libraries(instrumented_executable 
    PRIVATE 
        llvm-tool
)