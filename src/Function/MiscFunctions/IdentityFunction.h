/*-----------------------------------------.---------------------------------.
| Filename: IdentityFunction.h             | Identity Function               |
| Author  : Francis Maes                   |                                 |
| Started : 26/11/2010 14:33               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_FUNCTION_IDENTITY_H_
# define LBCPP_FUNCTION_IDENTITY_H_

# include <lbcpp/Function/Function.h>

namespace lbcpp
{

class IdentityFunction : public Function
{
public:
  IdentityFunction(TypePtr type) : type(type) {}
  IdentityFunction() {}

  virtual TypePtr getInputType() const
    {return type;}

  virtual TypePtr getOutputType(TypePtr inputType) const
    {return inputType;}

  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
    {return input;}

protected:
  friend class IdentityFunctionClass;

  TypePtr type;
};

}; /* namespace lbcpp */

#endif // !LBCPP_FUNCTION_IDENTITY_H_
