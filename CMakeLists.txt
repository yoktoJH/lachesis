cmake_minimum_required(VERSION 3.28)

project(LAChesis)

add_subdirectory(LAChesis-runtimelib)


add_executable(instrumented)

configure_file(config.yaml config.yaml COPYONLY)

target_sources(instrumented PRIVATE ${LLVMTOOL_ANALYZE})

target_compile_options(instrumented PRIVATE -mllvm -lachesis -g)

target_link_libraries(instrumented 
    PRIVATE 
        lachesis
)


