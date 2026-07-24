cmake_minimum_required(VERSION 3.15)
project(BlueShard LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

find_package(SDL3 REQUIRED)
find_package(Python3 COMPONENTS Interpreter Development REQUIRED)

# find_package(pybind11 CONFIG) only searches standard install prefixes
# and CMAKE_PREFIX_PATH - it has no built-in awareness of Python virtual
# environments, so a `pip install pybind11` inside a venv is invisible to
# it by default. pybind11 ships a `python -m pybind11 --cmakedir` helper
# specifically for this: it prints where *that* Python's pybind11 CMake
# files live, so we ask Python3_EXECUTABLE (the venv interpreter CMake
# just found above) directly, then point find_package at the answer.
execute_process(
    COMMAND "${Python3_EXECUTABLE}" -m pybind11 --cmakedir
    OUTPUT_VARIABLE pybind11_DIR
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
find_package(pybind11 CONFIG REQUIRED)

include_directories(include)

# --- Core rendering library ---
# All the math/rasterizing/windowing logic, with zero Python dependency.
# The pybind11 module below is a thin shell around this; it's also what
# you'd link a future non-Python front end (or a plain C++ test/demo)
# against.
add_library(gfx_core STATIC
    src/mesh.cpp
    src/scene.cpp
    src/window.cpp
)
target_include_directories(gfx_core PUBLIC include)
target_link_libraries(gfx_core PUBLIC SDL3::SDL3)

# gfx_core gets linked into _blueshard.so (a shared object) below, so its
# object code has to be position-independent - a static library doesn't
# get that by default, only executables/shared libs do. Without this,
# the linker can produce relocations that are illegal inside a .so
# (exactly the "recompile with -fPIC" error this fixes).
set_target_properties(gfx_core PROPERTIES POSITION_INDEPENDENT_CODE ON)

# --- Python extension module ---
# Produces _blueshard<abi-tag>.so - imported by python/blueshard/__init__.py
# as `from ._blueshard import ...`.
pybind11_add_module(_blueshard src/bindings.cpp)
target_link_libraries(_blueshard PRIVATE gfx_core)

# Drop the compiled extension straight into the python package folder so
# `import blueshard` works from a source checkout (e.g. via
# `PYTHONPATH=python python3 examples/spinning_boxes.py`) without a
# separate install step.
set_target_properties(_blueshard PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/python/blueshard"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/python/blueshard"
)
