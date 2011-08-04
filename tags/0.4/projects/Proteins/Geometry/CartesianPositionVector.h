/*-----------------------------------------.---------------------------------.
| Filename: CartesianPositionVector.h      | A sequence of (x,y,z) vectors   |
| Author  : Francis Maes                   |                                 |
| Started : 09/07/2010 13:36               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_GEOMETRY_CARTESIAN_POSITION_VECTOR_H_
# define LBCPP_GEOMETRY_CARTESIAN_POSITION_VECTOR_H_

# include <lbcpp/Core/Vector.h>
# include "Vector3.h"
# include "Matrix4.h"

namespace lbcpp
{

extern ClassPtr cartesianPositionVectorClass;

class CartesianPositionVector : public BuiltinVector<impl::Vector3, Vector3>
{
public:
  typedef BuiltinVector<impl::Vector3, Vector3> BaseClass;

  CartesianPositionVector(size_t length, const impl::Vector3& defaultValue = impl::Vector3())
    : BaseClass(cartesianPositionVectorClass, length, defaultValue) {}
  CartesianPositionVector(const std::vector<impl::Vector3>& positions)
    : BaseClass(cartesianPositionVectorClass, positions) {}
  CartesianPositionVector() {}

  virtual TypePtr getElementsType() const
    {return vector3Class;}

  virtual Variable getElement(size_t index) const
  {
    if (values[index].exists())
      return new Vector3(values[index]);
    else
      return Variable::missingValue(vector3Class);
  }

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

  virtual void saveToXml(XmlExporter& exporter) const;
  virtual bool loadFromXml(XmlImporter& importer);
};

typedef ReferenceCountedObjectPtr<CartesianPositionVector> CartesianPositionVectorPtr;

}; /* namespace lbcpp */

#endif // !LBCPP_GEOMETRY_CARTESIAN_POSITION_VECTOR_H_