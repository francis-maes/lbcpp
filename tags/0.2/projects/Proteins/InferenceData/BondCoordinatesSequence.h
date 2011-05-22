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

  bool load(InputStream& istr)
    {return lbcpp::read(istr, length) && lbcpp::read(istr, theta) && lbcpp::read(istr, phi);}

  void save(OutputStream& ostr) const
    {lbcpp::write(ostr, length); lbcpp::write(ostr, theta); lbcpp::write(ostr, phi);}

private:
  double length;
  Angle theta;
  DihedralAngle phi;
};

template<>
struct Traits<BondCoordinates>
{
  typedef BondCoordinates Type;

  static inline String toString(const BondCoordinates& value)
    {return value.toString();}

  static inline void write(OutputStream& ostr, const BondCoordinates& value)
  {
    lbcpp::write(ostr, value.getLength());
    lbcpp::write(ostr, value.getThetaAngle());
    lbcpp::write(ostr, value.getPhiDihedralAngle());
  }

  static inline bool read(InputStream& istr, BondCoordinates& res)
  {
    double length;
    Angle theta;
    DihedralAngle phi;
    if (!lbcpp::read(istr, length) || !lbcpp::read(istr, theta) || !lbcpp::read(istr, phi))
      return false;
    res = BondCoordinates(length, theta, phi);
    return true;
  }
};

class BondCoordinatesObject : public Object
{
public:
  BondCoordinatesObject(const BondCoordinates& value) : value(value) {}
  BondCoordinatesObject() {}

  BondCoordinates getValue() const
    {return value;}

  BondCoordinates& getValue()
    {return value;}

  void setValue(const BondCoordinates& coordinates)
    {value = coordinates;}

private:
  BondCoordinates value;
};

typedef ReferenceCountedObjectPtr<BondCoordinatesObject> BondCoordinatesObjectPtr;

class BondCoordinatesSequence : public BuiltinVectorBasedSequence<BondCoordinates>
{
public:
  typedef BuiltinVectorBasedSequence<BondCoordinates> BaseClass;

  BondCoordinatesSequence(const String& name, CartesianCoordinatesSequencePtr cartesianCoordinates);
  BondCoordinatesSequence(const String& name, size_t length = 0);
  BondCoordinatesSequence() {}  

  CartesianCoordinatesSequencePtr makeCartesianCoordinates(const String& name, const Matrix4& initialMatrix = Matrix4::identity);

  virtual bool hasObject(size_t position) const
    {return BaseClass::getElement(position).exists();}
  
  virtual ObjectPtr get(size_t index) const
  {
    BondCoordinates p = getCoordinates(index);
    return p.exists() ? new BondCoordinatesObject(p) : ObjectPtr();
  }

  virtual void set(size_t index, ObjectPtr object)
  {
    BondCoordinatesObjectPtr bond = object.dynamicCast<BondCoordinatesObject>();
    jassert(bond);
    setCoordinates(index, bond->getValue());
  }

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