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

class BondCoordinates
{
public:
  BondCoordinates(double length, Angle theta, DihedralAngle phi);
  BondCoordinates() {}

  void multiplyMatrix(Matrix4& matrix, bool applyAngle, bool applyDihedralAngle);

  String toString() const;

private:
  double length;
  Angle theta;
  DihedralAngle phi;
};

class BondCoordinatesSequence : public BuiltinVectorBasedSequenceWithEmptyValues<BondCoordinates>
{
public:
  typedef BuiltinVectorBasedSequenceWithEmptyValues<BondCoordinates> BaseClass;

  BondCoordinatesSequence(const String& name, CartesianCoordinatesSequencePtr cartesianCoordinates);
  BondCoordinatesSequence(const String& name, size_t length = 0);
  BondCoordinatesSequence() {}  

  CartesianCoordinatesSequencePtr createCartesianCoordinates(const String& name, const Matrix4& initialMatrix = Matrix4::identity);

  bool hasCoordinates(size_t position) const
    {return BaseClass::hasObject(position);}

  BondCoordinates getCoordinates(size_t position) const
    {return BaseClass::getElement(position);}

  void setCoordinates(size_t position, const BondCoordinates& coordinates)
    {BaseClass::setElement(position, coordinates);}

  void clearCoordinates(size_t position)
    {BaseClass::unsetElement(position);}
};

typedef ReferenceCountedObjectPtr<BondCoordinatesSequence> BondCoordinatesSequencePtr;

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_DATA_BOND_COORDINATES_SEQUENCE_H_
