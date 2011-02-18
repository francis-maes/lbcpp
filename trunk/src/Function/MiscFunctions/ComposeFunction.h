/*-----------------------------------------.---------------------------------.
| Filename: ComposeFunction.h              | Compose Function                |
| Author  : Francis Maes                   |                                 |
| Started : 01/12/2010 20:18               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_FUNCTION_COMPOSE_H_
# define LBCPP_FUNCTION_COMPOSE_H_

# include <lbcpp/Core/Function.h>

namespace lbcpp
{

class ComposeFunction : public Function
{
public:
  ComposeFunction(const FunctionPtr& f, const FunctionPtr& g) : f(f), g(g) {numInputs = 1;}
  ComposeFunction() {}

  virtual String toString() const
    {return f->toString() + T(" -> ") + g->toString();}

  virtual TypePtr getInputType() const
    {return f->getInputType();}

  virtual TypePtr getOutputType(TypePtr inputType) const
    {return g->getOutputType(f->getOutputType(inputType));}

  virtual String getDescription(const Variable& input) const
    {return f->getDescription(input);}

  // return g(f(x))
  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
    {return g->compute(context, f->compute(context, input));}

protected:
  friend class ComposeFunctionClass;

  FunctionPtr f;
  FunctionPtr g;
};

}; /* namespace lbcpp */

#endif // !LBCPP_FUNCTION_COMPOSE_H_
