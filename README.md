# exile
Handmade-style voxel engine &amp; game.

Objective is to create a featureful low-level programming environment & voxel game engine with zero or minimal libraries, to dig into and perfect critical and noncritical systems, and to create a modded-minecraft-ish game on top of the engine.

Gameplay goals:
   - High quality graphics (lighting, render distance, fast movement, large worlds)
   - Interesting and variable procedural terrain & feature generation
   - Powerful &amp; complex player-managed systems (e.g. modded minecraft/factorio)
   - Smooth multiplayer and efficient servers
   - Flexible &amp; powerful modding support

### Tools
- Engine & Game:
  - C-like C++: C with member functions, function/operator overloading, and some templates (no RAII, no exceptions, no polymorphism, no STL, extremely minimal inheritance)
  - Personal standard library (data structures, type info, platform layer, etc.)
  - Extremely minimal headers (total: stdint, float, stdarg, new, typeinfo, intrinsics, cmath [will remove])
  - Win32/Unix APIs
- Metaprogram:
  - libclang for C++ parsing
  - STL/C++11
- Assets:
  - STB libraries for asset processing
  - STL/C++11

### Current Features
Tons more is planned!
- Lib
   - Data structures
     - Checked array
     - Hash map (robin-hood open addressing)
     - Queue/Concurrent Queue (circular buffer)
     - Stack
     - Vector (stretchy buffer)
     - Strings
       - Utility
       - Reflection-based string formatting
     - Allocator & allocation context system
       - Platform abstraction
       - Pool allocator (used for scratch & joint allocations)
       - Free list (TODO)
   - Async
     - Thread pool
     - Async job/event system (integrated with platform events)
   - Platform
     - Platform abstraction layer
       - File IO
       - Concurency
       - Keyboard/mouse/window input
       - Memory
       - Windowing/OpenGL
     - Win32 platform layer
     - Linux platform layer (TODO)
     - Dynamic compilation/reload system
   - Reflection (using metaprogram)
 - Engine
   - Assets
     - Packed asset dynamic loading
   - Debug/Profiling
     - Debug options for optimizations/safety checks, release-mode
     - Whole-program call stack
     - Timing & allocation event collation
     - RAD Telemetry style profiler (TODO)
   - Input
     - Context sensitive control registry (TODO)
   - IMGUI
     - ImGui inspired immediate-mode GUI system
     - Render-agnostic
     - Powerful window formatting (TODO)
   - Logging 
     - Thread-safe &amp; flexible logging system
     - Context &amp; info formatting
   - Math
     - Vector &amp; matrix math
     - SIMD and intrinsic trig/vector/matrix math (TODO)
   - Rendering
     - Meshing
     - Render command list abstraction
     - Text rendering (using asset system)
   - oGL
     - Function loading
     - Context management
     - Shader dynamic loading
     - Texture asset loading
 - Metaprogram
   - C++ parsing &amp; type info extraction
   - Reflection info output
 - Asset builder
   - Image processor
   - Font processor
   - Font atlasing/kerning
 - Game
   - Engine testing
   - LOTs to add here
