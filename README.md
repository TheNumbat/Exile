# exile
Handmade-style voxel engine &amp; game.  

Objective is to create a featureful low-level programming environment & voxel game engine with minimal libraries, to dig into and perfect critical and noncritical systems, and to create a modded-minecraft-like game on top of the engine.

Gameplay goals:
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
  - Sean Barret's media processing libraries
  - C++14/STL
- Metaprogram:
  - libclang for C++ parsing
  - C++14/STL

### Screenshots
![shot1](https://i.imgur.com/OjoIbKH.png)

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
    - Font atlases
  - Debugging
    - Debug message system
    - Profiler
      - Time
      - Calls
      - Allocations
  - Input
    - Event parsing
    - Context state machines
  - Interface
    - Immediate mode controls
    - Window formatting
    - Reflection based components
  - Log
    - Thread safe log formatting and output
    - HTML and custom output
  - Rendering
    - Textures
    - Shaders
    - Meshes
    - Command lists
    - OpenGL setup
  - Async
    - Atomics
    - Thread pool
    - Futures
- Game
  - World generation
  - Voxel meshing
  - Movement
  - Threading
- Meta
  - Metaprogram to generate reflection data
  - Asset bulider to generate asset packs
  - Batch build system for asset builder, metaprogram, engine, tests, game
