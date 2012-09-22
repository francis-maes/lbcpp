/*-----------------------------------------.---------------------------------.
| Filename: ConcatenateContainerFunction.h | Concatenate Container Function  |
| Author  : Julien Becker                  |                                 |
| Started : 23/02/2011 15:21               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_CORE_FUNCTION_CONCATENATE_CONTAINER_H_
# define LBCPP_CORE_FUNCTION_CONCATENATE_CONTAINER_H_

# include <lbcpp/Core/Function.h>

namespace lbcpp
{

class ConcatenateContainerFunction : public Function
{
public:
  virtual size_t getNumRequiredInputs() const
    {return 1;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return containerClass(containerClass(anyType));}

  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
    {return inputVariables[0]->getType()->getTemplateArgument(0);}
  
protected:
  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
  {
    ContainerPtr inputContainer = input.getObjectAndCast<Container>();
    VectorPtr res = Vector::create(getOutputType());
    size_t n = inputContainer->getNumElements();
    for (size_t i = 0; i < n; ++i)
    {
      ContainerPtr values = inputContainer->getElement(i).getObjectAndCast<Container>();
      const size_t numElements = values->getNumElements();
      const size_t totalNumElements = res->getNumElements();
      res->resize(totalNumElements + numElements);
      for (size_t i = 0; i < numElements; ++i)
        res->setElement(totalNumElements + i, values->getElement(i));
    }
    return res;
  }
};

};

#endif //!LBCPP_CORE_FUNCTION_CONCATENATE_CONTAINER_H_
