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
  BondCoordinates(const Vector3* a, const Vector3* b, const Vector3* c, const Vector3* d);
  BondCoordinates(double length, Angle theta, DihedralAngle phi);
  BondCoordinates() : length(-1.0), theta(M_2_TIMES_PI), phi(M_2_TIMES_PI) {}

  bool exists() const
    {return hasLength() || hasThetaAngle() || hasPhiDihedralAngle();}

  String toString() const;

  /*
  ** Length
  */
  bool hasLength() const
    {return length >= 0.0;}

  double getLength() const
    {return length;}

  double& getLength()
    {return length;}

  void setLength(double length)
    {this->length = length;}

  /*
  ** Theta angle
  */
  bool hasThetaAngle() const
    {return (double)theta != M_2_TIMES_PI;}

  Angle getThetaAngle() const
    {return theta;}

  Angle& getThetaAngle()
    {return theta;}

  void setThetaAngle(Angle angle)
    {theta = angle;}

  /*
  ** Phi Dihedral angle
  */
  bool hasPhiDihedralAngle() const
    {return (double)phi != M_2_TIMES_PI;}

  DihedralAngle getPhiDihedralAngle() const
    {return phi;}
  
  DihedralAngle& getPhiDihedralAngle()
    {return phi;}

  void setPhiDihedralAngle(DihedralAngle angle)
    {phi = angle;}

  /*
  ** Utilities
  */
  void multiplyMatrix(Matrix4& matrix);

private:
  double length;
  Angle theta;
  DihedralAngle phi;
};

class BondCoordinatesSequence : public BuiltinVectorBasedSequence<BondCoordinates>
{
public:
  typedef BuiltinVectorBasedSequence<BondCoordinates> BaseClass;

  BondCoordinatesSequence(const String& name, CartesianCoordinatesSequencePtr cartesianCoordinates);
  BondCoordinatesSequence(const String& name, size_t length = 0);
  BondCoordinatesSequence() {}  

  CartesianCoordinatesSequencePtr createCartesianCoordinates(const String& name, const Matrix4& initialMatrix = Matrix4::identity);

  virtual bool hasObject(size_t position) const
    {return BaseClass::getElement(position).exists();}

  bool hasCoordinates(size_t position) const
    {return BaseClass::hasObject(position);}

  BondCoordinates getCoordinates(size_t position) const
    {return BaseClass::getElement(position);}

  void setCoordinates(size_t position, const BondCoordinates& coordinates)
    {BaseClass::setElement(position, coordinates);}

  void clearCoordinates(size_t position)
    {BaseClass::setElement(position, BondCoordinates());}
};

typedef ReferenceCountedObjectPtr<BondCoordinatesSequence> BondCoordinatesSequencePtr;

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_DATA_BOND_COORDINATES_SEQUENCE_H_
