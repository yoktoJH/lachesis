cmake_minimum_required(VERSION 3.28)

project(LAChesis)

set(CMAKE_C_COMPILER             "")#llvm-project/build/bin/clang"
set(CMAKE_CXX_COMPILER             "")#llvm-project/build/bin/clang++


add_subdirectory(LAChesis-runtimelib)

et(LLVMTOOL_ANALYZE  "" CACHE FILEPATH "path to a file to be analyzed")

add_executable(instrumented)

target_sources(instrumented PRIVATE ${LLVMTOOL_ANALYZE})

target_compile_options(instrumented PRIVATE -mllvm -lachesis -g)

target_link_libraries(instrumented 
    PRIVATE 
        llvm-tool
)


