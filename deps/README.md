
# Dependencies

One of the goals of this project is to create a system without external libraries, but a few are used for non-essential tasks:

	- STB media libraries for processing assets in the asset builder tool
	- libclang for parsing C++ in the reflection-info metaprogram
	- Simple DirectMedia Layer 2 (SDL2) for implementing a separate non-windows compatible platform layer. The Win32 layer is implemented with only the Win32 APIs.
