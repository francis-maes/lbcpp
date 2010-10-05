/*-----------------------------------------.---------------------------------.
| Filename: BoundsProximityPerception.h    | Bounds Proximity Perception     |
| Author  : Francis Maes                   |                                 |
| Started : 05/10/2010 14:42               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_FUNCTION_PERCEPTION_BOUNDS_PROXIMITY_H_
# define LBCPP_FUNCTION_PERCEPTION_BOUNDS_PROXIMITY_H_

# include <lbcpp/Function/Perception.h>

namespace lbcpp
{

class BoundsProximityPerception : public Perception
{
public:
  virtual String getPreferedOutputClassName() const
    {return T("bounds proximity");}

  virtual TypePtr getInputType() const
    {return pairType(containerClass(anyType()), integerType());}

  virtual size_t getNumOutputVariables() const
    {return 3;}
  
  virtual TypePtr getOutputVariableType(size_t index) const
    {return index < 2 ? positiveIntegerType() : probabilityType();}
  
  virtual String getOutputVariableName(size_t index) const
  {
    if (index == 0)
      return T("distanceToBegin");
    else if (index == 1)
      return T("distanceToEnd");
    else
      return T("relativePosition");
  }

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
};

}; /* namespace lbcpp */

#endif // !LBCPP_FUNCTION_PERCEPTION_BOUNDS_PROXIMITY_H_
