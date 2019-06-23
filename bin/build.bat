@echo off

meson build
meson configure build -D buildtype=debugoptimized
meson configure build -D warning_level=3
ninja -C build
