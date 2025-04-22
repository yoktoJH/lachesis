# lachesis



## Getting started

The tool is currently extremely work in progress but the three included analysers should function properly. Currently only pthreads and C source files are supported. In theory c++ could work but there are no guarantees. 
To use the tool you will need [modified version of llvm project ](https://gitlab.fi.muni.cz/xhoschek/llvm-project). Build both llvm and clang using the official [guide](https://llvm.org/docs/GettingStarted.html).

Change the CMAKE_C_COMPILER and CMAKE_CXX_COMPILER variables in [CMakeLists.txt](./CMakeLists.txt) to the new clang executables you just built.

Set your sourcefiles by either directly writing them to LACHESIS_ANALYZE  [CMakeLists.txt](./CMakeLists.txt) or using the command line:

```
cmake -S . -B build -DLACHESIS_ANALYZE="path/to/a/file.c"
```

then run:
```
cmake --build build
```
and hopefully everything should build and link properly

In order to enable one of the analysers uncomment their respective *_init() and *_terminate() functions in [runtime_interface.cpp](./runtime_interface.cpp#L61). Analysers should be preferably run one at a time. 
To modify noise settings edit definitions at the beginning of [noise.h](./noise.h#L12).
Frequency and strength should be a number from 0 to 999. Random strength noise has the strenght value randomly chosen from interval (0,strength).