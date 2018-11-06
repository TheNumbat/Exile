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

### Screenshots
![shot1](https://i.imgur.com/mUqnzkU.png)

### Current Features

See todo.txt for plans.

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
- Engine
  - Assets
    - Images
    - Fonts, font atlases
  - Debugging
    - Debug message system
    - Profiler
      - Time
      - Calls
      - Allocations
      - Worker threads
    - Debug console and command system
    - Debug runtime variable interface
  - Input
    - Event parsing
    - Context state machines
  - Interface
    - Dear ImGui implementation and reflection add-ons
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
- Game
  - World generation
  - Time
  - Block data and setup
  - Voxel terrain meshing and rendering
  - Sky and star rendering
  - Movement
  - Threaded terrain generation
- Meta
  - Metaprogram to generate reflection data
  - Asset bulider to generate asset packs
  - Batch build system for asset builder, metaprogram, engine, tests, game
