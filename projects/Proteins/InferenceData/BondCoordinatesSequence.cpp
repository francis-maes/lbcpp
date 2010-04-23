/*-----------------------------------------.---------------------------------.
| Filename: BondCoordinatesSequence.cpp    | A sequence of                   |
| Author  : Francis Maes                   |(bond length, angle, torsion angle)|
| Started : 22/04/2010 15:18               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include "BondCoordinatesSequence.h"
using namespace lbcpp;

/*
** BondCoordinates
*/
BondCoordinates::BondCoordinates(double length, Angle theta, DihedralAngle phi)
  : length(length), theta(theta), phi(phi) {}

BondCoordinates::BondCoordinates(const Vector3* a, const Vector3* b, const Vector3* c, const Vector3* d)
  : length(-1.0), theta(M_2_TIMES_PI), phi(M_2_TIMES_PI)
{
  if (b && c)
  {
    Vector3 cb = (*b - *c);
    length = cb.l2norm();
    if (d)
    {
      Vector3 cd = (*d - *c);
      theta = cb.angle(cd);
      if (a)
        phi = DihedralAngle(*a, *b, *c, *d);
    }
  }
}

void BondCoordinates::multiplyMatrix(Matrix4& matrix)
{
  // avant: length, puis phi (around x), puis theta (around z)
  if (phi.exists())
    matrix.rotateAroundXAxis(phi);
  if (length >= 0.0)
    matrix.translate(Vector3(length, 0.0, 0.0));
  if (theta.exists())
    matrix.rotateAroundZAxis(M_PI - theta);
}

String BondCoordinates::toString() const
{
  return T("(") + String(length, 2) + T(", ") +
    lbcpp::toString(theta) + T(", ") +
    lbcpp::toString(phi) + T(")");
}

/*
** BondCoordinatesSequence
*/
BondCoordinatesSequence::BondCoordinatesSequence(const String& name, CartesianCoordinatesSequencePtr cartesianCoordinates)
  : BaseClass(name)
{
  size_t n = cartesianCoordinates->size();
  jassert(n);
  if (n == 1)
    return;
  
  resize(n - 1);

  Vector3 A;
  Vector3 B = cartesianCoordinates->getPositionChecked(0);
  Vector3 C = cartesianCoordinates->getPositionChecked(1);
  for (size_t i = 0; i < n - 1; ++i)
  {
    Vector3 D = cartesianCoordinates->getPositionChecked(i + 2);
    setCoordinates(i, BondCoordinates(A.exists() ? &A : NULL, 
                                B.exists() ? &B : NULL, 
                                C.exists() ? &C : NULL, 
                                D.exists() ? &D : NULL));
    A = B;
    B = C;
    C = D;
  }
}

BondCoordinatesSequence::BondCoordinatesSequence(const String& name, size_t length)
  : BaseClass(name, length)
{
}

CartesianCoordinatesSequencePtr BondCoordinatesSequence::createCartesianCoordinates(const String& name, const Matrix4& initialMatrix)
{
  size_t n = size() + 1;

  CartesianCoordinatesSequencePtr res = new CartesianCoordinatesSequence(name, n);
  Matrix4 matrix(initialMatrix);
  res->setPosition(0, matrix.getTranslation());
  for (size_t i = 1; i < n; ++i)
  {
    getCoordinates(i - 1).multiplyMatrix(matrix);
    res->setPosition(i, matrix.getTranslation());
  }
  return res;
}
