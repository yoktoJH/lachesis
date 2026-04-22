cmake_minimum_required(VERSION 3.28)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
add_library(lachesis)

target_include_directories(lachesis PRIVATE .)

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
        config.h
        config.cpp
)

target_sources(lachesis PRIVATE ${SOURCE})

target_link_libraries(lachesis PUBLIC ryml::ryml)