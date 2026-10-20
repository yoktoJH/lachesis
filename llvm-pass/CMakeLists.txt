find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "LLVM_DIR: ${LLVM_DIR}")
message(STATUS "LLVM include dirs: ${LLVM_INCLUDE_DIRS}")
message(STATUS "LLVM definitions: ${LLVM_DEFINITIONS}")

list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")
include(AddLLVM)

include_directories(SYSTEM ${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_llvm_pass_plugin(Lachesis
  Lachesis.cpp
)