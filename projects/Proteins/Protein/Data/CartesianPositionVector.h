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

template<class ImplementationType, class ObjectType>
class BuiltinVector : public VariableContainer
{
public:
  BuiltinVector(size_t initialSize, const ImplementationType& defaultValue)
    : values(initialSize, defaultValue) {}
  BuiltinVector(const std::vector<ImplementationType>& values)
    : values(values) {}
  BuiltinVector() {}

  virtual TypePtr getStaticType() const
    {return thisClass->getTemplateArgument(0);}

  virtual size_t getNumVariables() const
    {return values.size();}

  virtual Variable getVariable(size_t index) const
  {
    if (values[index].exists())
      return ObjectPtr(new ObjectType(values[index]));
    else
      return Variable::missingValue(getStaticType());
  }

  virtual void setVariable(size_t index, const Variable& value)
  {
    ReferenceCountedObjectPtr<ObjectType> v = value.getObjectAndCast<ObjectType>();
    if (v)
      values[index] = ImplementationType();
    else
      values[index] = v->getValue();
  }

  virtual void saveToXml(XmlElement* xml) const
    {jassert(false);}
  virtual bool loadFromXml(XmlElement* xml, ErrorHandler& callback)
    {jassert(false); return false;}

  void reserve(size_t size)
    {values.reserve(size);}

  void clear()
    {values.clear();}

  void append(const ImplementationType& value)
    {values.push_back(value);}

protected:
  std::vector<ImplementationType> values;
};

class CartesianPositionVector : public BuiltinVector<Vector3, Vector3Object>
{
public:
  typedef BuiltinVector<Vector3, Vector3Object> BaseClass;

  CartesianPositionVector(size_t length, const Vector3& defaultValue = Vector3())
    : BaseClass(length, defaultValue) {}
  CartesianPositionVector(const std::vector<Vector3>& positions)
    : BaseClass(positions) {}
  CartesianPositionVector() {}

  virtual TypePtr getStaticType() const
    {return vector3Class();}

  bool hasPosition(size_t index) const
    {return values[index].exists();}

  Vector3 getPosition(size_t index) const
    {return values[index];}

  Vector3 getPositionChecked(int index) const
    {return index >= 0 && index < (int)values.size() ? values[index] : Vector3();}

  void setPosition(size_t index, const Vector3& position)
    {values[index] = position;}

  void clearPosition(size_t index)
    {values[index] = Vector3();}

  void movePosition(size_t index, const Vector3& delta);
  void applyAffineTransform(const Matrix4& affineTransform);

  Vector3 getGravityCenter() const;

  std::vector<Vector3>& getVectorOfPositions()
    {return values;}

  virtual void saveToXml(XmlElement* xml) const;
  virtual bool loadFromXml(XmlElement* xml, ErrorHandler& callback);
};

typedef ReferenceCountedObjectPtr<CartesianPositionVector> CartesianPositionVectorPtr;

extern ClassPtr cartesianPositionVectorClass();

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEINS_DATA_CARTESIAN_POSITION_VECTOR_H_
