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
# include <lbcpp/Core/Function.h>
# include <lbcpp/Data/DoubleVector.h>

namespace lbcpp
{

class DistributionEntropyFunction : public Function
{
public:
  virtual size_t getNumRequiredInputs() const
    {return 1;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return distributionClass();}

  virtual String getOutputPostFix() const
    {return T("Entropy");}

  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
    {return negativeLogProbabilityType;}

  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
  {
    const DistributionPtr& distribution = inputs[0].getObjectAndCast<Distribution>();
    if (distribution)
      return distribution->computeEntropy();
    else
      return Variable::missingValue(getOutputType());
  }
};

// todo: move ?, ComputeEntropyFunction(ProxyFunction) ?
class DenseDoubleVectorEntropyFunction : public Function
{
public:
  virtual size_t getNumRequiredInputs() const
    {return 1;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return denseDoubleVectorClass(enumValueType, probabilityType);}

  virtual String getOutputPostFix() const
    {return T("Entropy");}

  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
    {return negativeLogProbabilityType;}

  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
  {
    const DenseDoubleVectorPtr& distribution = inputs[0].getObjectAndCast<DenseDoubleVector>();
    if (distribution)
      return Variable(distribution->computeEntropy(), negativeLogProbabilityType);
    else
      return Variable::missingValue(negativeLogProbabilityType);
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_DISTRIBUTION_FUNCTION_ENTROPY_H_
