# Geode

Computational geometry library (PhysBAM lineage). C++ core with Python bindings.

## Building

```bash
mkdir build && cd build
cmake .. -DGEODE_DISABLE_PYTHON=ON   # C++ only
cmake .. -DGEODE_DISABLE_PYTHON=OFF  # with Python 3 bindings
make -j$(sysctl -n hw.ncpu)
```

### Dependencies (macOS)
```
brew install gmp open-mesh libpng libjpeg
```

### Known issues
- Build copies `__init__.py` files into `build/geode/<module>/` subdirectories. The `math/__init__.py` shadows Python's stdlib `math` module, breaking numpy import. Must be fixed before tests can run.
- `geode_wrap.so` segfaults on Python interpreter exit (module finalization/refcount issue in module_push/module_pop lifecycle).
- Test suite (`python -m pytest` from build dir, per Jenkinsfile) is all Python-based and needs Python 2->3 syntax updates.

## Architecture
- `geode/` contains modules: array, exact, force, geometry, image, math, mesh, openmesh, python, random, solver, structure, svg, utility, value, vector
- Each module has a `CMakeLists.txt` using `ADD_GEODE_MODULE` macro (defined in `GeodeSupport.cmake`)
- Python bindings use a custom C API layer (not pybind11/boost.python). Key files: `geode/python/Class.h`, `module.h`, `module.cpp`
- `GEODE_DEFINE_TYPE` macro creates PyTypeObject structs; `GEODE_PYTHON_MODULE` macro creates module init functions
- `PyTypeObject` initializers use C99 designated initializers (ported from positional for Python 3.12+ compat)

## Porting history (2026)
Originally built on x86 Linux/macOS with Python 2. Ported to:
- macOS Apple Silicon (ARM64): conditional `GEODE_SSE`, aarch64 in Random123, Accelerate.framework gate
- Python 3.14: PyInt->PyLong, PyString->PyUnicode, Py_InitModule3->PyModule_Create, PyInit_ entry points, numpy 2.x API
- C++17, modern CMake (3.5+), Homebrew `/opt/homebrew/` paths
