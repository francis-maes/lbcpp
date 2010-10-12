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
# include <lbcpp/Data/Pair.h>

namespace lbcpp
{

class BoundsProximityPerception : public Perception
{
public:
  BoundsProximityPerception()
    {computeOutputType();}

  virtual String toString() const
    {return T("bounds proximity");}

  virtual TypePtr getInputType() const
    {return pairClass(containerClass(anyType), integerType);}

  virtual void computePerception(const Variable& input, PerceptionCallbackPtr callback) const
  {
    const PairPtr& pair = input.getObjectAndCast<Pair>();
    const Variable& first = pair->getFirst();
    const ContainerPtr& container = first.getObjectAndCast<Container>();
    jassert(container);
    int n = (int)container->getNumElements();
    int index = pair->getSecond().getInteger();
    
    if (index >= 0 && index < n)
    {
      callback->sense(0, index);
      callback->sense(1, n - index);
      callback->sense(2, index / (double)(n - 1));
    }
  }

  virtual void computeOutputType()
  {
    reserveOutputVariables(3);
    addOutputVariable(T("distanceToBegin"), positiveIntegerType);
    addOutputVariable(T("distanceToEnd"), positiveIntegerType);
    addOutputVariable(T("relativePosition"), probabilityType);
    Perception::computeOutputType();
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_FUNCTION_PERCEPTION_BOUNDS_PROXIMITY_H_
