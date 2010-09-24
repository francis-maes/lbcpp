/*-----------------------------------------.---------------------------------.
| Filename: BiContainerPerception.h        | Bi Container Perception         |
| Author  : Julien Becker                  |                                 |
| Started : 23/09/2010 10:53               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_DATA_PERCEPTION_BI_CONTAINER_H_
# define LBCPP_DATA_PERCEPTION_BI_CONTAINER_H_

# include <lbcpp/Function/Perception.h>

namespace lbcpp
{

class BiContainerPerception : public DecoratorPerception
{
public:
  BiContainerPerception(size_t windowSize, PerceptionPtr subPerception)
    : DecoratorPerception(subPerception), windowSize(windowSize) {}
  BiContainerPerception() : windowSize(0) {}
  
  virtual TypePtr getInputType() const
    {return pairType(pairType(containerClass(anyType()), containerClass(anyType())), integerType());}
  
  virtual TypePtr getOutputType() const
    {return Perception::getOutputType();}
  
  virtual size_t getNumOutputVariables() const
    {return windowSize;}

  virtual TypePtr getOutputVariableType(size_t index) const
    {return decorated->getOutputType();}

  virtual String getOutputVariableName(size_t index) const
    {return T("[") + String((int)index - (int)(windowSize / 2)) + ("]");}

  virtual void computePerception(const Variable& input, PerceptionCallbackPtr callback) const
  {
    ContainerPtr first = input[0][0].getObjectAndCast<Container>();
    ContainerPtr second = input[0][1].getObjectAndCast<Container>();
    
    if (!first || !second)
      return;
    
    size_t firstPosition = (size_t)input[1].getInteger();
    int startPosition = (int)firstPosition - (int)(windowSize / 2);
    for (size_t i = 0; i < windowSize; ++i)
    {
      int position = startPosition + (int)i;
      if (position >= 0 && position < (int)second->getNumElements())
      {
        Variable variable = Variable::pair(first->getElement(firstPosition), second->getElement(position));
        callback->sense(i, decorated, variable);
      }
    }
  }

protected:
  friend class BiContainerPerceptionClass;

  size_t windowSize;
};

}; /* namespace lbcpp */

#endif // !LBCPP_DATA_PERCEPTION_BI_CONTAINER_H_
