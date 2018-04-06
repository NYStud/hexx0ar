# hexx0ar

hexx0ar is a simple, but powerful hexeditor currently in development.

dependecies
-----------

hexx0ar uses several dependecies:

 - SDL2, SDL2_image
 - epoxy (for OpenGL)
 - Boost (filesystem & program_options)
 - GLM (for maths)
 - Lua (currently unused, in future for plugins)
 - imgui (provided in src/deps)
 - C++ json parser from nlohmann (provided in src/deps)

in Debian you can simply install the following packages:

```
apt install libboost1.63-all-dev libepoxy-dev libglm-dev libsdl2-dev libsdl2-image-dev lua5.3 lua5.3-dev
```

building
--------

The supported build systems are CMake and redo.

CMake
-----

```
mkdir build; cd build
cmake ..
make -j8
```

redo
----

Either use an existing redo installation:

```
redo all
```

or use the simple implementation provided:

```
./do all
```

