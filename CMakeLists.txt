cmake_minimum_required(VERSION 3.28)
include(FetchContent)

set(RYML_REPO_URL https://github.com/biojppm/rapidyaml CACHE STRING "")
set(RYML_BRANCH_NAME master CACHE STRING "")
message(STATUS "FetchContent from repo: ${RYML_REPO_URL}")
message(STATUS "FetchContent from branch: ${RYML_BRANCH_NAME}")
include(FetchContent)
FetchContent_Declare(ryml
    GIT_REPOSITORY ${RYML_REPO_URL}
    GIT_TAG ${RYML_BRANCH_NAME}
    GIT_SHALLOW FALSE  # ensure submodules are checked out
)
FetchContent_MakeAvailable(ryml)

#set(CMAKE_C_COMPILER             "/home/jozinko/bakalarka/llvm-project/build/bin/clang")#llvm-project/build/bin/clang"
#set(CMAKE_CXX_COMPILER             "/home/jozinko/bakalarka/llvm-project/build/bin/clang++")#llvm-project/build/bin/clang++

project(LAChesis)

add_subdirectory(LAChesis-runtimelib)

#set(LLVMTOOL_ANALYZE "" CACHE FILEPATH "path to a file to be analyzed")

add_executable(instrumented)

configure_file(config.yaml config.yaml COPYONLY)

target_sources(instrumented PRIVATE ${LLVMTOOL_ANALYZE})

target_compile_options(instrumented PRIVATE -mllvm -lachesis -g)

target_link_libraries(instrumented 
    PRIVATE 
        lachesis
)


