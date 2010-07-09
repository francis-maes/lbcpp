/*-----------------------------------------.---------------------------------.
| Filename: Matrix4.cpp                    | 4x4 Matrix                      |
| Author  : Francis Maes                   | Most of this code comes from    |
| Started : 25/04/2010 14:12               | the Ogre open source 3D engine  |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "Matrix4.h"
#include "Matrix3.h"

namespace lbcpp {
namespace impl {

const Matrix4 Matrix4::zero = Matrix4(
      0.0, 0.0, 0.0, 0.0,
      0.0, 0.0, 0.0, 0.0,
      0.0, 0.0, 0.0, 0.0,
      0.0, 0.0, 0.0, 0.0);

const Matrix4 Matrix4::identity = Matrix4(
      1.0, 0.0, 0.0, 0.0,
      0.0, 1.0, 0.0, 0.0,
      0.0, 0.0, 1.0, 0.0,
      0.0, 0.0, 0.0, 1.0);

String Matrix4::toString() const
{
  String res = T("[[");
  for (size_t i = 0; i < 4; ++i)
    for (size_t j = 0; j < 4; ++j)
    {
      res += String(m[i][j]);
      if (j < 3)
        res += T(",");
      else
      {
        res += T("]");
        if (i < 3)
          res += T("\n");
        else
          res += T("]\n");
      }
    }
  return res;
}

Matrix4 Matrix4::findAffineTransformToSuperposePoints(const std::vector< std::pair<Vector3, Vector3> >& pointPairs, bool* succeeded)
{
  size_t n = pointPairs.size();
  if (!n)
  {
    if (succeeded) *succeeded = false;
    return Matrix4::identity;
  }

  double invN = 1.0 / (double)n;

  // compute centroids
  Vector3 centroid1(0.0), centroid2(0.0);
  for (size_t i = 0; i < n; ++i)
  {
    centroid1 += pointPairs[i].first;
    centroid2 += pointPairs[i].second;
  }
  centroid1 *= invN;
  centroid2 *= invN;

  // compute correlation matrix
  Matrix3 correlationMatrix = Matrix3::zero;
  for (size_t i = 0; i < n; ++i)
    correlationMatrix.addCorrelation(pointPairs[i].first - centroid1, pointPairs[i].second - centroid2);

  // make SVD decomposition
  Matrix3 u, v;
  Vector3 diag;
  if (!correlationMatrix.makeSVDDecomposition(u, diag, v))
  {
    if (succeeded) *succeeded = false;
    return Matrix4::identity;
  }

  // compute optimal rotation matrix
  Matrix3 rotation = v * u.transposed();

  // compute optimal translation
  Vector3 translation(0.0);
  for (size_t i = 0; i < n; ++i)
  {
    translation += pointPairs[i].second;
    Vector3 p = rotation.transformAffine(pointPairs[i].first);
    translation -= p;
  }
  translation *= invN;

  if (succeeded) *succeeded = true;
  return Matrix4(rotation, translation);
}

}; /* namespace impl */
}; /* namespace lbcpp */
