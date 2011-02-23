/*-----------------------------------------.---------------------------------.
| Filename: CreateObjectFunction.h         | Create Object Function          |
| Author  : Francis Maes                   |                                 |
| Started : 16/02/2011 18:46               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_CORE_FUNCTION_CREATE_OBJECT_H_
# define LBCPP_CORE_FUNCTION_CREATE_OBJECT_H_

# include <lbcpp/Core/Function.h>

namespace lbcpp
{

class CreateObjectFunction : public Function
{
public:
  CreateObjectFunction(ClassPtr objectClass = ClassPtr())
    : objectClass(objectClass) {}

  virtual size_t getMinimumNumRequiredInputs() const
    {return 0;}

  virtual size_t getMaximumNumRequiredInputs() const
    {return objectClass->getNumMemberVariables();}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return objectClass->getMemberVariable(index)->getType();}

  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
  {
    outputName = T("new ") + objectClass->getName();
    outputShortName = objectClass->getName();
    return objectClass;
  }

  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
  {
    ObjectPtr res = Object::create(objectClass);
    jassert(res);
    size_t numInputs = getNumInputs();
    for (size_t i = 0; i < numInputs; ++i)
      res->setVariable(i, inputs[i]);
    return res;
  }

protected:
  friend class CreateObjectFunctionClass;

  ClassPtr objectClass;
};

}; /* namespace lbcpp */

#endif // !LBCPP_CORE_FUNCTION_CREATE_OBJECT_H_
