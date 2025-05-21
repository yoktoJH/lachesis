# LAChesis



## Getting started

The tool is currently extremely work in progress.
However, there are three included analysers should function properly. Currently only pthreads and C source files are supported. In theory c++ could work but there are no guarantees. 

### Requirements

To use the tool you will need [modified version of llvm project](https://gitlab.fi.muni.cz/xhoschek/llvm-project) that includes a new Pass for instrumentation and a new option for clang compilation. Build both llvm and clang using the official [guide](https://llvm.org/docs/GettingStarted.html).

### Configure CMake build
Set the the CMAKE_C_COMPILER and CMAKE_CXX_COMPILER variables in [CMakeLists.txt](./CMakeLists.txt) to the newly built clang executables.

Set your sourcefiles by either directly set them in LACHESIS_ANALYZE  [CMakeLists.txt](./CMakeLists.txt) CMake variable or in the CMake configure:

```
cmake -S . -B build -DLACHESIS_ANALYZE="path/to/a/file.c"
```

then run:
```
cmake --build build
```
and hopefully everything should build and link properly

## Configuring the analysis

### Analyzers

In order to enable one of the analysers uncomment their respective *_init() and *_terminate() functions in [runtime_interface.cpp](LAChesis-runtimelib/runtime_interface.cpp#L61). Analysers should be preferably run one at a time. The default is FastTrack enabled

### Noise injection
To modify the noise settings edit definitions at the beginning of [noise.h](LAChesis-runtimelib/noise.h#L12).
Frequency and strength should be a number from 0 to 999. 
Random strength means that the noise strenght value is randomly chosen from interval (0,strength).

All avaiable noieses can be seen in this enum:
```
enum class NoiseType
{
  NONE,
  SLEEP,
  YIELD,
  WAIT,
  INVERSE,
  DEBUG
};
```