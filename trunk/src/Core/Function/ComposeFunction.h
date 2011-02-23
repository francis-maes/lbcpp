/*-----------------------------------------.---------------------------------.
| Filename: ComposeFunction.h              | Compose Function                |
| Author  : Francis Maes                   |                                 |
| Started : 01/12/2010 20:18               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_CORE_FUNCTION_COMPOSE_H_
# define LBCPP_CORE_FUNCTION_COMPOSE_H_

# include <lbcpp/Core/Function.h>

namespace lbcpp
{

class ComposeFunction : public Function
{
public:
  ComposeFunction(const FunctionPtr& f, const FunctionPtr& g) : f(f), g(g) {}
  ComposeFunction() {}

  virtual size_t getMinimumNumRequiredInputs() const
    {return f->getMinimumNumRequiredInputs();}

  virtual size_t getMaximumNumRequiredInputs() const
    {return f->getMaximumNumRequiredInputs();}
  
  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return f->getRequiredInputType(index, numInputs);}

  virtual String toString() const
    {return f->toString() + T(" -> ") + g->toString();}

  virtual String getDescription(const Variable& input) const
    {return f->getDescription(input);}

  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
  {
    if (!f->initialize(context, inputVariables) || !g->initialize(context, f->getOutputType()))
      return TypePtr();

    outputName = g->getOutputVariable()->getName();
    outputShortName = g->getOutputVariable()->getShortName();
    return g->getOutputType();
  }

  // return g(f(x))
  virtual Variable computeFunction(ExecutionContext& context, const Variable* input) const
    {return g->compute(context, f->compute(context, input, getNumInputs()));}

protected:
  friend class ComposeFunctionClass;

  FunctionPtr f;
  FunctionPtr g;
};

}; /* namespace lbcpp */

#endif // !LBCPP_CORE_FUNCTION_COMPOSE_H_
