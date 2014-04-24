//#####################################################################
// Class Field
//#####################################################################
//
// An array indexed by a handle type to distinguish between different kinds of fields
//
//#####################################################################
#pragma once

#include <geode/array/Array.h>
namespace geode {

template<class T,class Id> struct HasCheapCopy<Field<T,Id> >:public mpl::true_{};

template<class T,class Id> static inline PyObject* to_python(const Field<T,Id>& field) {
  return to_python(field.flat);
}

template<class T,class Id> struct FromPython<Field<T,Id> >{static inline Field<T,Id> convert(PyObject* object) {
  return Field<T,Id>(from_python<Array<T>>(object));
}};

template<class T,class Id>
class Field {
private:
  struct Unusable {};
public:
  typedef typename remove_const<T>::type Element;
  static const bool is_const = geode::is_const<T>::value;
  typedef T& result_type;

  Array<T> flat;

  Field() {}

  Field(const Field& source)
    : flat(source.flat) {}

  Field(typename mpl::if_c<is_const,const Field<Element,Id>&,Unusable>::type source)
    : flat(source.flat) {}

  explicit Field(int n, bool initialize=true)
    : flat(n,initialize) {}

  explicit Field(const Array<T>& source)
    : flat(source) {}

  Field(const Hashtable<Id,T>& source, const int size, const T def = T()) {
    flat.resize(size);
    for (auto& s : flat)
      s = def;
    for (const auto& p : source)
      flat[p.key().idx()] = p.data();
  }

  Field& operator=(const Field<Element,Id>& source) {
    flat = source.flat;
    return *this;
  }

  Field& operator=(const Field<const Element,Id>& source) {
    flat = source.flat;
    return *this;
  }

  int size() const {
    return flat.size();
  }

  bool empty() const {
    return flat.empty();
  }

  T& operator[](Id i) const {
    return flat[i.idx()];
  }

  T& operator()(Id i) const { // Allow use as a function
    return flat[i.idx()];
  }

  bool valid(Id i) const {
    return flat.valid(i.idx());
  }

  // Type safe conversion to go from positions in the field back to an Id
  Id ptr_to_id(const T* x) const {
    Id result = Id(x - flat.begin());
    assert(valid(result));
    return result;
  }

  Id append(const T& x) GEODE_ALWAYS_INLINE {
    return Id(flat.append(x));
  }

  template<class TField> void extend(const TField& append_field) {
    flat.extend(append_field.flat);
  }

  void preallocate(const int m_new, const bool copy_existing_elements=true) {
    flat.preallocate(m_new, copy_existing_elements);
  }

  Field<Element,Id> copy() const {
    Field<Element,Id> copy;
    copy.flat.copy(flat);
    return copy;
  }

  Field<Element,Id>& const_cast_() {
    return *(Field<Element,Id>*)this;
  }

  const Field<Element,Id>& const_cast_() const {
    return *(const Field<Element,Id>*)this;
  }
};

}
