/*-----------------------------------------.---------------------------------.
| Filename: WindowPerception.h             | Window Perception               |
| Author  : Francis Maes                   |                                 |
| Started : 12/07/2010 16:56               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_FUNCTION_PERCEPTION_WINDOW_H_
# define LBCPP_FUNCTION_PERCEPTION_WINDOW_H_

# include <lbcpp/Perception/Perception.h>

namespace lbcpp
{

class WindowPerception : public Perception
{
public:
  WindowPerception(TypePtr elementsType, size_t windowSize, PerceptionPtr subPerception)
    : elementsType(elementsType), windowSize(windowSize), subPerception(subPerception)
  {
    computeOutputType();
  }

  WindowPerception() : windowSize(0) {}

  virtual String toString() const
    {return T("window of ") + String((int)windowSize) + T(" ") + elementsType->getName() + T("s");}

  virtual TypePtr getInputType() const
    {return pairClass(containerClass(elementsType), integerType);}

  virtual void computePerception(const Variable& input, PerceptionCallbackPtr callback) const
  {
    PairPtr pair = input.getObjectAndCast<Pair>();
    ContainerPtr container = pair->getFirst().getObjectAndCast<Container>();
    if (container)
    {
      int startPosition = pair->getSecond().getInteger() - (int)(windowSize / 2);
      
      for (size_t i = 0; i < windowSize; ++i)
      {
        int position = startPosition + (int)i;
        if (position >= 0 && position < (int)container->getNumElements())
        {
          Variable variable = container->getElement(position);
          if (subPerception)
            callback->sense(i, subPerception, variable);
          else if (variable.exists())
            callback->sense(i, variable);
        }
      }
    }
  }

  virtual void computeOutputType()
  {
    reserveOutputVariables(windowSize);
    for (size_t i = 0; i < windowSize; ++i)
    {
      String name = T("[") + String((int)i - (int)(windowSize / 2)) + T("]");
      if (subPerception)
        addOutputVariable(name, subPerception);
      else
        addOutputVariable(name, elementsType);
    }
    Perception::computeOutputType();
  }

protected:
  friend class WindowPerceptionClass;

  TypePtr elementsType;
  size_t windowSize;
  PerceptionPtr subPerception;
};

}; /* namespace lbcpp */

#endif // !LBCPP_FUNCTION_PERCEPTION_WINDOW_H_
