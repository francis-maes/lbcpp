/*-----------------------------------------.---------------------------------.
| Filename: CartesianCoordinatesSequence.h | A sequence of (x,y,z) vectors   |
| Author  : Francis Maes                   |                                 |
| Started : 22/04/2010 15:15               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_DATA_CARTESIAN_COORDINATES_SEQUENCE_H_
# define LBCPP_INFERENCE_DATA_CARTESIAN_COORDINATES_SEQUENCE_H_

# include "Sequence.h"
# include "../Geometry/Vector3.h"
# include "../Geometry/Matrix4.h"

namespace lbcpp
{

class CartesianCoordinatesSequence : public BuiltinVectorBasedSequence<impl::Vector3>
{
public:
  typedef BuiltinVectorBasedSequence<impl::Vector3> BaseClass;

  CartesianCoordinatesSequence(const String& name, size_t length, const impl::Vector3& defaultValue = impl::Vector3())
    : BaseClass(name, length, defaultValue) {}
  CartesianCoordinatesSequence(const String& name, const std::vector<impl::Vector3>& positions)
    : BaseClass(name, positions) {}
  CartesianCoordinatesSequence() {}

  virtual bool hasObject(size_t index) const
    {return getPosition(index).exists();}

  virtual ObjectPtr get(size_t index) const
  {
    impl::Vector3 p = getPosition(index);
    return p.exists() ? new Vector3(p) : ObjectPtr();
  }

  virtual void set(size_t index, ObjectPtr object)
  {
    Vector3Ptr v3o = object.dynamicCast<Vector3>();
    jassert(v3o);
    setPosition(index, v3o->getValue());
  }

  bool hasPosition(size_t index) const
    {return hasObject(index);}

  impl::Vector3 getPosition(size_t index) const
    {return getElement(index);}

  impl::Vector3 getPositionChecked(int index) const
    {return index >= 0 && index < (int)elements.size() ? elements[index] : impl::Vector3();}

  void setPosition(size_t index, const impl::Vector3& position)
    {setElement(index, position);}

  void clearPosition(size_t index)
    {setElement(index, impl::Vector3());}

  void movePosition(size_t index, const impl::Vector3& delta)
    {impl::Vector3 v = getPosition(index); jassert(v.exists()); v += delta; setPosition(index, v);}

  void applyAffineTransform(const impl::Matrix4& affineTransform)
  {
    for (size_t i = 0; i < elements.size(); ++i)
      if (elements[i].exists())
        setElement(i, affineTransform.transformAffine(getElement(i)));
  }

  impl::Vector3 getGravityCenter() const
  {
    // todo: cache
    impl::Vector3 sum = 0;
    size_t count = 0;
    for (size_t i = 0; i < elements.size(); ++i)
      if (elements[i].exists())
      {
        sum += elements[i];
        ++count;
      }
    return count ? sum / (double)count : sum;
  }

  std::vector<impl::Vector3>& getVectorOfPositions()
    {return elements;}
};

typedef ReferenceCountedObjectPtr<CartesianCoordinatesSequence> CartesianCoordinatesSequencePtr;

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_DATA_CARTESIAN_COORDINATES_SEQUENCE_H_
