/*-----------------------------------------.---------------------------------.
| Filename: DistributionEntropyFunction.h  | Computes the entropy of a       |
| Author  : Francis Maes                   |  distribution                   |
| Started : 28/01/2011 15:55               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_DISTRIBUTION_FUNCTION_ENTROPY_H_
# define LBCPP_DISTRIBUTION_FUNCTION_ENTROPY_H_

# include <lbcpp/Distribution/Distribution.h>
# include <lbcpp/Function/Function.h>

namespace lbcpp
{

class DistributionEntropyFunction : public Function
{
public:
  virtual VariableSignaturePtr initializeFunction(ExecutionContext& context)
  {
     if (!checkNumInputs(context, 1) || !checkInputType(context, 0, distributionClass(anyType)))
      return VariableSignaturePtr();
    VariableSignaturePtr inputVariable = getInputVariable(0);
    return new VariableSignature(negativeLogProbabilityType, inputVariable->getName() + T("Entropy"), inputVariable->getShortName() + T("e"));
  }

  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
    {return inputs[0].getObjectAndCast<Distribution>()->computeEntropy();}
};

}; /* namespace lbcpp */

#endif // !LBCPP_DISTRIBUTION_FUNCTION_ENTROPY_H_
