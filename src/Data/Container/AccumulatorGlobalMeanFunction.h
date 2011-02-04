/*-----------------------------------------.---------------------------------.
| Filename: AccumulatorGlobalMeanFunction.h| Global Mean                     |
| Author  : Francis Maes                   |                                 |
| Started : 04/02/2011 15:25               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_DATA_CONTAINER_FUNCTION_ACCUMULATOR_GLOBAL_MEAN_FUNCTION_H_
# define LBCPP_DATA_CONTAINER_FUNCTION_ACCUMULATOR_GLOBAL_MEAN_FUNCTION_H_

# include <lbcpp/Function/Function.h>
# include <lbcpp/Data/DoubleVector.h>

namespace lbcpp
{

  // Accumulator<T> => DenseDoubleVector<T>
class AccumulatorGlobalMeanFunction : public Function
{
public:
  virtual VariableSignaturePtr initializeFunction(ExecutionContext& context)
  {
    TypePtr doubleVectorType;
    if (!checkNumInputs(context, 1) ||
        !Container::getTemplateParameter(context, getInputType(0), doubleVectorType))
      return VariableSignaturePtr();
    VariableSignaturePtr inputVariable = getInputVariable(0);
    return new VariableSignature(doubleVectorType, inputVariable->getName() + T("GlobalMean"), inputVariable->getShortName() + T("gm"));
  }
 
  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
  {
    const ObjectVectorPtr& accumulator = input.getObjectAndCast<ObjectVector>();
    size_t n = accumulator->getNumElements();
    if (!n)
      return Variable::missingValue(getOutputType());

    DenseDoubleVectorPtr res = new DenseDoubleVector(getOutputType());
    accumulator->get(n - 1).staticCast<DenseDoubleVector>()->addWeightedTo(res, 0, 1.0 / (double)n);
    return res;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_DATA_CONTAINER_FUNCTION_ACCUMULATOR_GLOBAL_MEAN_FUNCTION_H_
