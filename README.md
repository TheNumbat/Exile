# Exile
Handmade-style voxel engine &amp; game.  

[Blog](https://thenumbat.github.io/)

Objective is to create a featureful low-level programming environment & voxel game engine with minimal libraries, to dig into and perfect its systems, and to create a modded-minecraft-like game on top of the engine.

Goals:
   - High quality graphics (lighting, render distance, fast movement, large worlds)
   - Interesting and variable procedural terrain & feature generation
   - Powerful &amp; complex player-managed systems (e.g. modded minecraft/factorio)
   - Smooth multiplayer and efficient servers
   - Flexible &amp; powerful modding support

### Build

Install [meson](https://mesonbuild.com/Getting-meson.html) and [ninja](https://github.com/ninja-build/ninja/releases) if necessary.

Commands are run in the repository root.

Windows:

```
meson build
meson configure build -D buildtype=debugoptimized
ninja -C build
```

Linux - ubuntu/debian (probably works, but only with clang):

```
sudo apt install clang libclang-dev libsdl2-dev ninja-build
pip3 install meson
```
```
CXX=clang++ meson build
meson configure build -D buildtype=debugoptimized -D platform=sdl
ninja -C build
```

### Run

In the top level directory:

```
cd data
..\build\main.exe
```
### Screenshots
![shot0](https://i.imgur.com/trQ4E0s.png)
![shot1](https://i.imgur.com/ckmEU0W.png)

### Tools
- Engine & Game:
  - C+: C with member functions, function/operator overloading, lambdas, and templates (no STL, no exceptions, minimal RAII, minimal inheritance)
  - Custom standard library (data structures, threading, type info, serialization, platform, allocators)
  - Win32/Unix APIs
  - SSE4.2
- Assets:
  - STB media libraries
  - C++14/STL
- Metaprogram:
  - libclang for C++ parsing
  - C++14/STL

### Current Features

See todo.txt for plans.

- Game
  - World generation 
    - Parallelized chunk generation, updates, and voxel lighting
  - Hybrid voxel terrain meshing
  - Voxel face rendering pipeline
  - Time and environment assets
  - Block data framework
  - Player movement

- Engine
  - Assets
    - Images
    - Fonts, font atlases
  - Debugging
    - Debug message system
    - Profiler
      - Timing
      - Calls/call locations
      - Allocations/memory safety
      - Threaded timing/work delegation
    - Debug console and command system
    - Runtime variable inspection interface
  - Input
    - Event parsing from the platform layer
    - Context state machines
  - Interface
    - Dear ImGui implementation and reflection-based add-ons
    - [depreciated] Custom immediate mode GUI system
  - Log
    - Thread safe log formatting and output
    - HTML and custom output
  - Rendering
    - Textures
    - Shaders
    - Meshes
    - Command lists
    - State management
    - OpenGL setup
  - Async
    - Atomics
    - Thread pool
    - Futures

- Lib
  - Allocators
    - General
    - Arena
    - Pool
  - Array
  - Buffer
  - Queue
  - Hash Map
  - Stack
  - Vector
  - Strings
  - Thread pool and futures
  - Reflection
    - Type info 
    - Serialization
  - SIMD vector and matrix math
- Platform Layer
  - Windowing
  - Input
  - Filesystem
  - Threading
  - OpenGL
  - Dynamic code compilation/loading
  - Architectures
    - Win32
    - SDL2
- Meta
  - Metaprogram to generate reflection data
  - Asset bulider to generate asset packs
  - Batch build system for asset builder, metaprogram, engine, tests, game
