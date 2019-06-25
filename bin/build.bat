@echo off

meson build
meson configure build -D buildtype=debug
meson configure build -D warning_level=3
ninja -C build
