/*-----------------------------------------.---------------------------------.
| Filename: CartesianPositionVector.h      | A sequence of (x,y,z) vectors   |
| Author  : Francis Maes                   |                                 |
| Started : 09/07/2010 13:36               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEINS_DATA_CARTESIAN_POSITION_VECTOR_H_
# define LBCPP_PROTEINS_DATA_CARTESIAN_POSITION_VECTOR_H_

# include <lbcpp/Object/Vector.h>
# include "../../Geometry/Vector3.h"
# include "../../Geometry/Matrix4.h"

namespace lbcpp
{

class CartesianPositionVector : public BuiltinVector<impl::Vector3, Vector3>
{
public:
  typedef BuiltinVector<impl::Vector3, Vector3> BaseClass;

  CartesianPositionVector(size_t length, const impl::Vector3& defaultValue = impl::Vector3())
    : BaseClass(length, defaultValue) {}
  CartesianPositionVector(const std::vector<impl::Vector3>& positions)
    : BaseClass(positions) {}
  CartesianPositionVector() {}

  virtual TypePtr getStaticType() const
    {return vector3Class();}

  bool hasPosition(size_t index) const
    {return values[index].exists();}

  impl::Vector3 getPosition(size_t index) const
    {return values[index];}

  impl::Vector3 getPositionChecked(int index) const
    {return index >= 0 && index < (int)values.size() ? values[index] : impl::Vector3();}

  void setPosition(size_t index, Vector3Ptr position)
    {values[index] = position ? position->getValue() : impl::Vector3();}

  void setPosition(size_t index, const impl::Vector3& position)
    {values[index] = position;}

  void clearPosition(size_t index)
    {values[index] = impl::Vector3();}

  void movePosition(size_t index, const impl::Vector3& delta);
  void applyAffineTransform(const impl::Matrix4& affineTransform);

  impl::Vector3 getGravityCenter() const;

  std::vector<impl::Vector3>& getVectorOfPositions()
    {return values;}

  virtual void saveToXml(XmlElement* xml) const;
  virtual bool loadFromXml(XmlElement* xml, ErrorHandler& callback);
};

typedef ReferenceCountedObjectPtr<CartesianPositionVector> CartesianPositionVectorPtr;

extern ClassPtr cartesianPositionVectorClass();

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEINS_DATA_CARTESIAN_POSITION_VECTOR_H_
