/*-----------------------------------------.---------------------------------.
| Filename: VariableGenerator.cpp          | Variable Generators             |
| Author  : Francis Maes                   |                                 |
| Started : 02/02/2011 17:06               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include <lbcpp/Operator/VariableGenerator.h>
using namespace lbcpp;

class SetInObjectVariableGeneratorCallback : public VariableGeneratorCallback
{
public:
  SetInObjectVariableGeneratorCallback(const ObjectPtr& target)
    : target(target) {}

  virtual void sense(size_t index, bool value)
    {target->setVariable(index, value);}

  virtual void sense(size_t index, int value)
    {target->setVariable(index, value);}

  virtual void sense(size_t index, double value)
    {target->setVariable(index, value);}

  virtual void sense(size_t index, const String& value)
    {target->setVariable(index, value);}

  virtual void sense(size_t index, const ObjectPtr& value)
    {target->setVariable(index, value);}

  virtual void sense(size_t index, const Variable& value)
    {target->setVariable(index, value);}

private:
  ObjectPtr target;
};

Variable VariableGenerator::computeFunction(ExecutionContext& context, const Variable* inputs) const
{
  ObjectPtr res = Object::create(getOutputType());
  SetInObjectVariableGeneratorCallback callback(res);
  computeVariables(inputs, callback);
  return res;
}
