/*-----------------------------------------.---------------------------------.
| Filename: BondCoordinatesSequence.h      | A sequence of                   |
| Author  : Francis Maes                   |(bond length, angle, torsion angle)|
| Started : 22/04/2010 15:17               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_DATA_BOND_COORDINATES_SEQUENCE_H_
# define LBCPP_INFERENCE_DATA_BOND_COORDINATES_SEQUENCE_H_

# include "CartesianCoordinatesSequence.h"
# include "../Geometry/DihedralAngle.h"
# include "../Geometry/Matrix4.h"

namespace lbcpp
{

class BondCoordinates : public Object
{
public:
  BondCoordinates(double length, Angle theta, DihedralAngle phi);
  BondCoordinates() {}

  void multiplyMatrix(Matrix4& matrix, bool applyAngle, bool applyDihedralAngle);

  virtual String toString() const;

private:
  double length;
  Angle theta;
  DihedralAngle phi;
};

typedef ReferenceCountedObjectPtr<BondCoordinates> BondCoordinatesPtr;

class BondCoordinatesSequence : public TypedObjectVectorBasedSequence<BondCoordinates>
{
public:
  typedef TypedObjectVectorBasedSequence<BondCoordinates> BaseClass;

  BondCoordinatesSequence(const String& name, CartesianCoordinatesSequencePtr cartesianCoordinates);
  BondCoordinatesSequence(const String& name, size_t length = 0);
  BondCoordinatesSequence() {}  

  CartesianCoordinatesSequencePtr createCartesianCoordinates(const String& name, const Matrix4& initialMatrix = Matrix4::identity);

  void setCoordinates(size_t position, BondCoordinatesPtr coordinates);
  BondCoordinatesPtr getCoordinates(size_t position) const;
};

typedef ReferenceCountedObjectPtr<BondCoordinatesSequence> BondCoordinatesSequencePtr;

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_DATA_BOND_COORDINATES_SEQUENCE_H_
