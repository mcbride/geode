#include <geode/config.h>
#ifdef GEODE_OPENMESH
#include <geode/openmesh/decimate.h>
#include <OpenMesh/Tools/Decimater/ModQuadricT.hh>
#include <OpenMesh/Tools/Decimater/ModNormalFlippingT.hh>
#include <OpenMesh/Tools/Decimater/ModBaseT.hh>
#include <geode/geometry/Triangle3d.h>
#include <geode/python/wrap.h>
namespace geode {

typedef real T;
typedef Vector<T,3> TV;

template class FaceQualityModule<TriMesh>;
template class BoundaryPreservationModule<TriMesh>;

int decimate(TriMesh &mesh, int max_collapses, double maxangleerror, double maxquadricerror, double min_face_quality) {

  // need normals for this
  mesh.request_face_normals();
  mesh.update_face_normals();

  DecimaterT decimater(mesh);

  // module types
  typedef OpenMesh::Decimater::ModNormalFlippingT<TriMesh>::Handle HModNormalFlipping;
  typedef OpenMesh::Decimater::ModQuadricT<TriMesh>::Handle HModQuadric;

  // Quadric
  HModQuadric hModQuadric;
  decimater.add(hModQuadric);

  if (maxquadricerror == std::numeric_limits<double>::infinity())
    decimater.module(hModQuadric).unset_max_err(); // use only as priority
  else
    decimater.module(hModQuadric).set_max_err(sqr(maxquadricerror));

  // prevent ruining the boundary
  BoundaryPreservationModule<TriMesh>::Handle hModBoundary;
  decimater.add(hModBoundary);
  decimater.module(hModBoundary).set_max_error(maxquadricerror);

  // prevent creation of crappy triangles
  if (min_face_quality > 0.) {
    FaceQualityModule<TriMesh>::Handle hModFaceQuality;
    decimater.add(hModFaceQuality);
    decimater.module(hModFaceQuality).min_quality(min_face_quality);
  }

  // normal change termination criterion
  if (maxangleerror > 0.) {
    HModNormalFlipping hModNormalFlipping;
    decimater.add(hModNormalFlipping);
    decimater.module(hModNormalFlipping).set_max_normal_deviation((float)maxangleerror);
  }

  if (!decimater.initialize()) {
    std::cerr << "ERROR: could not initialize decimation." << std::endl;
    return 0;
  }

  const int count = (int) decimater.decimate(max_collapses);

  mesh.release_face_normals();

  mesh.garbage_collection();
  return count;
}

}
using namespace geode;

void wrap_openmesh_decimate() {
  GEODE_FUNCTION_2(decimate_openmesh,decimate)
}
#endif // GEODE_OPENMESH
