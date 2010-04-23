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

namespace lbcpp
{

class CartesianCoordinatesSequence : public BuiltinVectorBasedSequence<Vector3>
{
public:
  typedef BuiltinVectorBasedSequence<Vector3> BaseClass;

  CartesianCoordinatesSequence(const String& name, size_t length)
    : BaseClass(name, length) {}
  CartesianCoordinatesSequence() {}

  virtual bool hasObject(size_t index) const
    {return getPosition(index).exists();}

  bool hasPosition(size_t index) const
    {return hasObject(index);}

  Vector3 getPosition(size_t index) const
    {return getElement(index);}

  Vector3 getPositionChecked(int index) const
    {return index >= 0 && index < (int)elements.size() ? elements[index] : Vector3();}

  void setPosition(size_t index, const Vector3& position)
    {setElement(index, position);}

  void clearPosition(size_t index)
    {setElement(index, Vector3());}
};

typedef ReferenceCountedObjectPtr<CartesianCoordinatesSequence> CartesianCoordinatesSequencePtr;

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_DATA_CARTESIAN_COORDINATES_SEQUENCE_H_
