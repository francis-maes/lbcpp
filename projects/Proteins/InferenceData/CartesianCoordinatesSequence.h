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

  Vector3 getPosition(size_t index) const
    {jassert(index < elements.size()); return elements[index];}

  void setPosition(size_t index, const Vector3& position)
    {jassert(index < elements.size()); elements[index] = position;}
};

typedef ReferenceCountedObjectPtr<CartesianCoordinatesSequence> CartesianCoordinatesSequencePtr;

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_DATA_CARTESIAN_COORDINATES_SEQUENCE_H_
