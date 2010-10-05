/*-----------------------------------------.---------------------------------.
| Filename: WindowPerception.h             | Window Perception               |
| Author  : Francis Maes                   |                                 |
| Started : 12/07/2010 16:56               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_FUNCTION_PERCEPTION_WINDOW_H_
# define LBCPP_FUNCTION_PERCEPTION_WINDOW_H_

# include <lbcpp/Function/Perception.h>

namespace lbcpp
{

class WindowPerception : public Perception
{
public:
  WindowPerception(TypePtr elementsType, size_t windowSize, PerceptionPtr subPerception)
    : elementsType(elementsType), windowSize(windowSize), subPerception(subPerception) {}
  WindowPerception() : windowSize(0) {}

  virtual String getPreferedOutputClassName() const
    {return T("window of ") + String((int)windowSize) + T(" ") + elementsType->getName() + T("s");}

  virtual TypePtr getInputType() const
    {return pairType(containerClass(elementsType), integerType());}

  virtual size_t getNumOutputVariables() const
    {return windowSize;}

  virtual TypePtr getOutputVariableType(size_t index) const
    {return subPerception ? subPerception->getOutputType() : elementsType;}

  virtual String getOutputVariableName(size_t index) const
    {return T("[") + String((int)index - (int)(windowSize / 2)) + T("]");}

  virtual PerceptionPtr getOutputVariableGenerator(size_t index) const
    {return subPerception;}

  virtual void computePerception(const Variable& input, PerceptionCallbackPtr callback) const
  {
    ContainerPtr container = input[0].getObjectAndCast<Container>();
    if (container)
    {
      int startPosition = input[1].getInteger() - (int)(windowSize / 2);
      
      for (size_t i = 0; i < windowSize; ++i)
      {
        int position = startPosition + (int)i;
        if (position >= 0 && position < (int)container->getNumElements())
        {
          Variable variable = container->getElement(position);
          if (subPerception)
            callback->sense(i, subPerception, variable);
          else if (variable)
            callback->sense(i, variable);
        }
      }
    }
  }

protected:
  friend class WindowPerceptionClass;

  TypePtr elementsType;
  size_t windowSize;
  PerceptionPtr subPerception;
};

}; /* namespace lbcpp */

#endif // !LBCPP_FUNCTION_PERCEPTION_WINDOW_H_
