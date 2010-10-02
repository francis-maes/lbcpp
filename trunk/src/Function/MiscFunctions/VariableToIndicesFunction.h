/*-----------------------------------------.---------------------------------.
| Filename: VariableToIndicesFunction.h    | Variable To Indices Function    |
| Author  : Julien Becker                  |                                 |
| Started : 06/10/2010 20:17               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_DATA_FUNCTION_VARIABLE_TO_INDICES_H_
# define LBCPP_DATA_FUNCTION_VARIABLE_TO_INDICES_H_

# include <lbcpp/Data/Vector.h>
# include <lbcpp/Function/Function.h>

namespace lbcpp
{

class VariableToIndicesFunction : public Function
{
public:
  virtual TypePtr getInputType() const
    {return containerClass(anyType());}

  virtual TypePtr getOutputType(TypePtr inputType) const
    {jassert(inputType != nilType()); return pairType(inputType, pairType(integerType(), integerType()));}

  // Container => (Container, (StartPosition, EndPosition))
  virtual Variable computeFunction(const Variable& input, MessageCallback& callback) const
  {
    ContainerPtr container = input.getObjectAndCast<Container>();
    size_t n = container ? container->getNumElements() : 0;
    return Variable::pair(input, Variable::pair(0, n));
  }

private:
  friend class VariableToIndicesFunctionClass;
};

}; /* namespace lbcpp */

#endif // !LBCPP_DATA_FUNCTION_VARIABLE_TO_INDICES_H_
