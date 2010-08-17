/*-----------------------------------------.---------------------------------.
| Filename: WindowPerception.h             | Window Perception               |
| Author  : Francis Maes                   |                                 |
| Started : 12/07/2010 16:56               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_DATA_PERCEPTION_WINDOW_H_
# define LBCPP_DATA_PERCEPTION_WINDOW_H_

# include <lbcpp/Data/Perception.h>

namespace lbcpp
{

class WindowPerception : public DecoratorPerception
{
public:
  WindowPerception(TypePtr elementsType, size_t windowSize, PerceptionPtr subPerception)
    : DecoratorPerception(subPerception), elementsType(elementsType), windowSize(windowSize) {}
  WindowPerception() : windowSize(0) {}

  virtual TypePtr getInputType() const
    {return pairType(vectorClass(elementsType), integerType());}

  virtual TypePtr getOutputType() const
    {return Perception::getOutputType();}

  virtual size_t getNumOutputVariables() const
    {return windowSize;}

  virtual TypePtr getOutputVariableType(size_t index) const
    {return decorated ? decorated->getOutputType() : elementsType;}

  virtual String getOutputVariableName(size_t index) const
    {return T("[") + String((int)index - (int)(windowSize / 2)) + T("]");}

  virtual PerceptionPtr getOutputVariableGenerator(size_t index) const
    {return decorated;}

  virtual void computePerception(const Variable& input, PerceptionCallbackPtr callback) const
  {
    ContainerPtr container = input[0].getObjectAndCast<Container>();
    if (container)
    {
      int startPosition = input[1].getInteger() - (int)(windowSize / 2);
      
      for (size_t i = 0; i < windowSize; ++i)
      {
        int position = startPosition + (int)i;
        Variable variable = position >= 0 && position < (int)container->size()
            ? container->getVariable(position)
            : Variable::missingValue(elementsType);

        if (decorated)
          callback->sense(i, decorated, variable);
        else
          callback->sense(i, variable);
      }
    }
  }

protected:
  TypePtr elementsType;
  size_t windowSize;
};

}; /* namespace lbcpp */

#endif // !LBCPP_DATA_PERCEPTION_WINDOW_H_
