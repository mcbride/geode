//#####################################################################
// Module Python
//#####################################################################
#include <geode/utility/config.h>
#define GEODE_IMPORT_NUMPY
#include <geode/python/module.h>
#include <geode/python/Buffer.h>
#include <geode/python/enum.h>
#include <geode/python/numpy.h>
#include <geode/python/stl.h>
#include <geode/python/wrap.h>
#ifndef _WIN32
#include <dlfcn.h>
#endif
namespace geode {

#ifdef GEODE_PYTHON

// Verify that Python macros agree with sizeof operator
static_assert(SIZEOF_SIZE_T == sizeof(size_t), "Error: Bad value for SIZEOF_SIZE_T");
static_assert(SIZEOF_INT == sizeof(int), "Error: Bad value for SIZEOF_INT");

extern bool numpy_imported;

#define ASSERT_NUMPY_TYPE_CONSISTENT(type)\
  static_assert((int)::type == (int)type, "Numpy's and our definition of " #type " doesn't match. Look at numpy-types.h for the problem.")

ASSERT_NUMPY_TYPE_CONSISTENT(NPY_BOOL);
ASSERT_NUMPY_TYPE_CONSISTENT(NPY_BYTE);
ASSERT_NUMPY_TYPE_CONSISTENT(NPY_UBYTE);
ASSERT_NUMPY_TYPE_CONSISTENT(NPY_SHORT);
ASSERT_NUMPY_TYPE_CONSISTENT(NPY_USHORT);
ASSERT_NUMPY_TYPE_CONSISTENT(NPY_INT);
ASSERT_NUMPY_TYPE_CONSISTENT(NPY_UINT);
ASSERT_NUMPY_TYPE_CONSISTENT(NPY_LONG);
ASSERT_NUMPY_TYPE_CONSISTENT(NPY_ULONG);
ASSERT_NUMPY_TYPE_CONSISTENT(NPY_LONGLONG);
ASSERT_NUMPY_TYPE_CONSISTENT(NPY_ULONGLONG);
ASSERT_NUMPY_TYPE_CONSISTENT(NPY_FLOAT);
ASSERT_NUMPY_TYPE_CONSISTENT(NPY_DOUBLE);
ASSERT_NUMPY_TYPE_CONSISTENT(NPY_LONGDOUBLE);
ASSERT_NUMPY_TYPE_CONSISTENT(NPY_CFLOAT);
ASSERT_NUMPY_TYPE_CONSISTENT(NPY_CDOUBLE);
ASSERT_NUMPY_TYPE_CONSISTENT(NPY_CLONGDOUBLE);
ASSERT_NUMPY_TYPE_CONSISTENT(NPY_OBJECT);
ASSERT_NUMPY_TYPE_CONSISTENT(NPY_STRING);
ASSERT_NUMPY_TYPE_CONSISTENT(NPY_UNICODE);
ASSERT_NUMPY_TYPE_CONSISTENT(NPY_VOID);

static std::vector<PyObject*> modules;

static PyObject* module() {
  if (modules.empty())
    throw RuntimeError("No current module");
  return modules.back();
}

static void import_geode() {
#ifdef _WIN32
  // On windows, all code is compiled into a single python module, so there's nothing else to import
  return;
#else
  const char* name = PyModule_GetName(module());
  if (!name) throw_python_error();
  if (strcmp(name,"geode_wrap")) {
    PyObject* python_str = PyUnicode_FromString("geode");
    if (!python_str) throw_python_error();
    PyObject* python = PyImport_Import(python_str);
    Py_DECREF(python_str);
    if (!python) throw_python_error();
  }
#endif
}

// Prevent libgeode from being dlclose'd during interpreter shutdown.
// Static PyTypeObject instances live in the library's data segment;
// if the library is unmapped while Python still holds references to
// those types, _Py_Dealloc will segfault accessing invalid memory.
static void prevent_unload() {
#ifndef _WIN32
  static bool done = false;
  if (done) return;
  done = true;
  Dl_info info;
  if (dladdr((void*)&prevent_unload, &info))
    dlopen(info.dli_fname, RTLD_LAZY | RTLD_NODELETE);
#endif
}

void module_push(const char* name) {
  prevent_unload();
  static PyMethodDef no_methods[] = {{NULL, NULL, 0, NULL}};
  // Each module needs its own PyModuleDef -- Python holds a pointer to it.
  // Intentionally leaked (one per submodule, ~15 total).
  auto* def = new PyModuleDef{
    PyModuleDef_HEAD_INIT, name, NULL, -1, no_methods
  };
  auto module = PyModule_Create(def);
  if (!module)
    throw_python_error();
  // Register in sys.modules so other code can find this module
  PyObject* sys_modules = PyImport_GetModuleDict();
  PyDict_SetItemString(sys_modules, name, module);
  modules.push_back(module);
  import_geode();
}

void module_pop() {
  modules.pop_back();
}

PyObject* module_peek() {
  if (modules.empty())
    throw RuntimeError("No current module");
  return modules.back();
}

template<class TC> static TC convert_test(const TC& c) {
  return c;
}

string str_repr_test(const string& s) {
  return repr(s);
}

namespace python {

void add_object(const char* name, PyObject* object) {
  if (!object) throw PythonError();
  PyModule_AddObject(module(),name,object);
}

}

#else // non-python stubs

namespace python {
void add_object(const char* name, PyObject* object) {}
}

#endif

enum EnumTest { EnumTestA, EnumTestB };
GEODE_DEFINE_ENUM(EnumTest,GEODE_CORE_EXPORT)

}
using namespace geode;
using namespace geode::python;

void wrap_python() {
#ifdef GEODE_PYTHON
  if(strncmp(PY_VERSION,Py_GetVersion(),3)) {
    PyErr_Format(PyExc_ImportError,"python version mismatch: compiled again %s, linked against %s",PY_VERSION,Py_GetVersion());
    throw_python_error();
  }

  // Initialize Buffer type (used for array ownership)
  if (PyType_Ready(&Buffer::pytype) < 0)
    throw_python_error();
  Py_INCREF(&Buffer::pytype);

  GEODE_WRAP(object)
  GEODE_WRAP(python_function)
  GEODE_WRAP(exceptions)
  GEODE_WRAP(test_class)
  GEODE_WRAP(numpy)

  python::function("list_convert_test",convert_test<vector<int> >);
  python::function("set_convert_test",convert_test<unordered_set<int> >);
  python::function("dict_convert_test",convert_test<unordered_map<int,string> >);
  python::function("enum_convert_test",convert_test<EnumTest>);
  GEODE_FUNCTION(str_repr_test)

  GEODE_ENUM(EnumTest)
  GEODE_ENUM_VALUE(EnumTestA)
  GEODE_ENUM_VALUE(EnumTestB)

  // import numpy
  if (_import_array() >= 0)
    numpy_imported = true;
  else {
    PyErr_Print();
    PyErr_SetString(PyExc_ImportError,"numpy.core.multiarray failed to import");
    throw_python_error();
  }

  python::add_object("real",(PyObject*)PyArray_DescrFromType(NumpyScalar<real>::value));
#endif
}
