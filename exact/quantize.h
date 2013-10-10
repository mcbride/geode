// Warp and quantize a set of points in R^d in preparation for exact arithmetic
// See other/core/exact/config.h for discussion.
#pragma once

#include <other/core/exact/config.h>
#include <other/core/geometry/Box.h>
#include <limits>
namespace other {

template<class TS,int d> struct Quantizer {
  typedef Vector<Quantized,d> QV;  // quantized vector type
  typedef Vector<TS,d> TVS; // unquantized vector type

  struct Inverse {
    TVS center;
    TS inv_scale; 

    Inverse(TVS center, TS inv_scale)
      : center(center), inv_scale(inv_scale) {}

    TVS operator()(const QV& p) const {
      return center+(inv_scale*TVS(p));
    }
  };

  TVS center;
  TS scale;
  TVS shifted_center;
  Inverse inverse;

  Quantizer(const Box<TVS>& box)
    : center(box.center())
    , scale(TS(exact::bound)/max(TS(1.01)*box.sizes().max(),TS(1e-6)))
    , shifted_center(center-TS(.5)/scale)
    , inverse(center,1/scale) {}

  QV operator()(const TVS& p) const {
    return QV(floor(scale*(p-shifted_center))); // Transform to -bound <= q <= bound (see config.h)
  }
};

template<class TS,int d> static inline Quantizer<TS, d> quantizer(const Box<Vector<TS,d>>& box) {
  return Quantizer<TS,d>(box);
}

}