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

class CartesianCoordinatesSequence : public BuiltinVectorBasedSequenceWithEmptyValues<Vector3>
{
public:
  typedef BuiltinVectorBasedSequenceWithEmptyValues<Vector3> BaseClass;

  CartesianCoordinatesSequence(const String& name, size_t length)
    : BaseClass(name, length) {}
  CartesianCoordinatesSequence() {}

  bool hasPosition(size_t index) const
    {return hasObject(index);}

  Vector3 getPosition(size_t index) const
    {return getElement(index);}

  void setPosition(size_t index, const Vector3& position)
    {setElement(index, position);}

  void clearPosition(size_t index)
    {unsetElement(index);}
};

typedef ReferenceCountedObjectPtr<CartesianCoordinatesSequence> CartesianCoordinatesSequencePtr;

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_DATA_CARTESIAN_COORDINATES_SEQUENCE_H_
