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

// FIXME: move in a better place
class DoubleVectorEntropyFunction : public Function
{
public:
  virtual size_t getNumRequiredInputs() const
    {return 1;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return doubleVectorClass(enumValueType, probabilityType);}

  virtual String getOutputPostFix() const
    {return T("Entropy");}

  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
    {return negativeLogProbabilityType;}

  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
  {
    const DoubleVectorPtr& distribution = inputs[0].getObjectAndCast<DoubleVector>();
    if (distribution)
      return Variable(distribution->entropy(), negativeLogProbabilityType);
    else
      return Variable::missingValue(negativeLogProbabilityType);
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_CORE_FUNCTION_SIGNED_SCALAR_TO_PROBABILITY_H_
