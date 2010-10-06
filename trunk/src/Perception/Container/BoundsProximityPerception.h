/*-----------------------------------------.---------------------------------.
| Filename: BoundsProximityPerception.h    | Bounds Proximity Perception     |
| Author  : Francis Maes                   |                                 |
| Started : 05/10/2010 14:42               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_FUNCTION_PERCEPTION_BOUNDS_PROXIMITY_H_
# define LBCPP_FUNCTION_PERCEPTION_BOUNDS_PROXIMITY_H_

# include <lbcpp/Perception/Perception.h>

namespace lbcpp
{

class BoundsProximityPerception : public Perception
{
public:
  BoundsProximityPerception()
    {computeOutputVariables();}

  virtual String toString() const
    {return T("bounds proximity");}

  virtual TypePtr getInputType() const
    {return pairClass(containerClass(anyType()), integerType());}

  virtual void computePerception(const Variable& input, PerceptionCallbackPtr callback) const
  {
    ContainerPtr container = input[0].getObjectAndCast<Container>();
    jassert(container);
    int n = (int)container->getNumElements();
    int index = input[1].getInteger();
    
    if (index >= 0 && index < n)
    {
      callback->sense(0, Variable(index, positiveIntegerType()));
      callback->sense(1, Variable(n - index, positiveIntegerType()));
      callback->sense(2, Variable(index / (double)(n - 1), probabilityType()));
    }
  }

  virtual void computeOutputVariables()
  {
    reserveOutputVariables(3);
    addOutputVariable(T("distanceToBegin"), positiveIntegerType());
    addOutputVariable(T("distanceToEnd"), positiveIntegerType());
    addOutputVariable(T("relativePosition"), probabilityType());
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_FUNCTION_PERCEPTION_BOUNDS_PROXIMITY_H_
