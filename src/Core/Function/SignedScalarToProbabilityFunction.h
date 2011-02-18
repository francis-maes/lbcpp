/*-----------------------------------------.---------------------------------.
| Filename: SignedScalarToProbabilityFun...h| Signed Scalar -> Probability   |
| Author  : Francis Maes                   |                                 |
| Started : 17/02/2010 12:42               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_CORE_FUNCTION_SIGNED_SCALAR_TO_PROBABILITY_H_
# define LBCPP_CORE_FUNCTION_SIGNED_SCALAR_TO_PROBABILITY_H_

# include <lbcpp/Core/Function.h>

namespace lbcpp
{

class SignedScalarToProbabilityFunction : public SimpleUnaryFunction
{
public:
  SignedScalarToProbabilityFunction() : SimpleUnaryFunction(doubleType, probabilityType, T("Prob")) {}

  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
  {
    if (!input.exists())
      return Variable::missingValue(probabilityType);

    double score = input.getDouble();
    static const double temperature = 1.0;
    return Variable(1.0 / (1.0 + exp(-score * temperature)), probabilityType);
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_CORE_FUNCTION_SIGNED_SCALAR_TO_PROBABILITY_H_
