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
pip3 install --break-system-packages numpy pytest pytest-forked scipy
```

## Running tests

```bash
cd build
python3 -m pytest --forked --tb=short
```

- Must run from `build/` so `geode/math/__init__.py` doesn't shadow stdlib `math`
- `--forked` required: 7 tests crash with C++ assertion aborts (SIGABRT) that kill the process without isolation
- `setup.cfg` uses `[tool:pytest]` section
- Expected results: 158 passed, 7 failed (C++ crashes), 2 skipped

### Known test failures (C++ crashes, tracked in GitHub issues)
- **Circle quantization** (issue #1): test_circle_quantize, test_circles, test_positive_offsets, test_negative_offsets -- assertion in `rational_approximation()` at `circle_quantization.cpp:37`
- **Mesh CSG triangulation** (issue #2): test_csg, test_depth_weight, test_offset -- assertion in `add_constraint_edge()` at `simple_triangulate.h:196`
- Both may be related to x86_64 Rosetta 2 floating-point behavior affecting exact arithmetic quantization

### Skipped tests
- `test_refcount`: Deferred reference counting in Python 3.12+
- OpenMesh tests: `openmesh_enabled()` returns False when C++ OpenMesh module not compiled

## Architecture
- `geode/` contains modules: array, exact, force, geometry, image, math, mesh, openmesh, python, random, solver, structure, svg, utility, value, vector
- Each module has a `CMakeLists.txt` using `ADD_GEODE_MODULE` macro (defined in `GeodeSupport.cmake`)
- Python bindings use a custom C API layer (not pybind11/boost.python). Key files: `geode/python/Class.h`, `module.h`, `module.cpp`
- `GEODE_DEFINE_TYPE` macro creates PyTypeObject structs; `GEODE_PYTHON_MODULE` macro creates module init functions
- `PyTypeObject` initializers use C99 designated initializers (ported from positional for Python 3.12+ compat)

## C++ / Python string binding
- `from_python<string>` accepts both `PyBytes` and `PyUnicode` (Latin-1 decode for str)
- `to_python<string>` uses `PyUnicode_DecodeLatin1` for byte-transparent round-trip
- This means C++ `std::string` with arbitrary bytes survives the Python round-trip via Latin-1 encoding

## Porting history (2026)
Originally built on x86 Linux/macOS with Python 2. Ported to:
- macOS Apple Silicon (ARM64): conditional `GEODE_SSE`, aarch64 in Random123, Accelerate.framework gate
- Python 3.14: PyInt->PyLong, PyString->PyUnicode, PyModule_Create, PyInit_ entry points, numpy 2.x API
- Python 2->3 runtime: xrange, except syntax, map/filter iterators, cPickle/cStringIO, builtin shadowing by numpy, tryfile str->bytes, cache_method name mangling, uint128 unsigned conversion
- C++17, modern CMake (3.5+), Homebrew `/usr/local/` paths (x86_64 Rosetta)
