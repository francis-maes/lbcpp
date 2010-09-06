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
    {return pairType(vectorClass(anyType()), integerType());}

  virtual TypePtr getOutputType(TypePtr inputType) const
    {return pairType(vectorClass(inputType->getTemplateArgument(0)), pairType(integerType(), integerType()));}

  virtual Variable computeFunction(const Variable& input, MessageCallback& callback) const
  {
    VectorPtr vector = input[0].getObjectAndCast<Vector>();
    size_t n = vector ? vector->getNumElements() : 0;
      
    return Variable::pair(input[0], Variable::pair(0, n));
  }

private:
  friend class VariableToIndicesFunctionClass;
};

}; /* namespace lbcpp */

#endif // !LBCPP_DATA_FUNCTION_VARIABLE_TO_INDICES_H_
