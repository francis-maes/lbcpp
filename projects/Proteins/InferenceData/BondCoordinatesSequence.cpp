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

void BondCoordinates::multiplyMatrix(Matrix4& matrix, bool applyAngle, bool applyDihedralAngle)
{
  matrix.translate(Vector3(length, 0.0, 0.0));
  if (applyDihedralAngle)
    matrix.rotateAroundXAxis(phi);
  if (applyAngle)
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
  if (n > 1)
  {
    resize(n - 1);
    Vector3 previousCoordinates = cartesianCoordinates->getPosition(0);
    Vector3 coordinates = cartesianCoordinates->getPosition(1);
    for (size_t i = 1; i < n; ++i)
    {
      Vector3 nextCoordinates;
      if (i < n - 1)
        nextCoordinates = cartesianCoordinates->getPosition(i + 1);

      Vector3 prev = previousCoordinates - coordinates;
      Vector3 next = nextCoordinates - coordinates;

      double length = prev.l2norm();
      Angle theta = i < n - 1 ? prev.angle(next) : 0.0;
      DihedralAngle phi = i > 1 && i < n - 1
        ? DihedralAngle(cartesianCoordinates->getPosition(i - 2), previousCoordinates, coordinates, nextCoordinates)
        : DihedralAngle(2 * M_PI);
      setCoordinates(i - 1, BondCoordinates(length, theta, phi));

      previousCoordinates = coordinates;
      coordinates = nextCoordinates;
    }
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
    getCoordinates(i - 1).multiplyMatrix(matrix, i < n - 1, i < n - 2);
    res->setPosition(i, matrix.getTranslation());
  }
  return res;
}
