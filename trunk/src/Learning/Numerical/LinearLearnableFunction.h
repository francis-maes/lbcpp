/*-----------------------------------------.---------------------------------.
| Filename: LinearLearnableFunction.h      | Linear Learnable Function       |
| Author  : Francis Maes                   |                                 |
| Started : 15/02/2011 20:05               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LEARNING_NUMERICAL_LINEAR_LEARNABLE_FUNCTION_H_
# define LBCPP_LEARNING_NUMERICAL_LINEAR_LEARNABLE_FUNCTION_H_

# include <lbcpp/Function/ScalarFunction.h>
# include <lbcpp/Function/Evaluator.h>
# include <lbcpp/Learning/BatchLearner.h>
# include <lbcpp/Learning/Numerical.h>

namespace lbcpp
{

// DoubleVector<T>, optional ScalarFunction -> Double
class LinearLearnableFunction : public NumericalLearnableFunction
{
public:
  const DenseDoubleVectorPtr& getParameters() const
    {return parameters.staticCast<DenseDoubleVector>();}

  virtual size_t getNumRequiredInputs() const
    {return 2;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return index ? scalarFunctionClass : doubleVectorClass();}

  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
  {
    EnumerationPtr featuresEnumeration = DoubleVector::getElementsEnumeration(inputVariables[0]->getType());
    parametersClass = denseDoubleVectorClass(featuresEnumeration);
    outputName = T("prediction");
    outputShortName = T("p");
    setBatchLearner(stochasticBatchLearner());
    return doubleType;
  }

  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
  {
    const DoubleVectorPtr& inputVector = inputs[0].getObjectAndCast<DoubleVector>();
    const DenseDoubleVectorPtr& parameters = getParameters();
    if (!parameters || !inputVector)
      return Variable::missingValue(doubleType);

    double res = inputVector->dotProduct(parameters);
    return isNumberValid(res) ? Variable(res) : Variable::missingValue(doubleType);
  }

  virtual bool computeAndAddGradient(const Variable* inputs, const Variable& prediction, double& exampleLossValue, DoubleVectorPtr& target, double weight) const
  {
    const DoubleVectorPtr& inputVector = inputs[0].getObjectAndCast<DoubleVector>();
    const FunctionPtr& supervision = inputs[1].getObjectAndCast<Function>();

    if (!supervision)
      return false;

    if (supervision.isInstanceOf<ScalarFunction>())
    {
      const ScalarFunctionPtr& loss = supervision.staticCast<ScalarFunction>();
      double lossDerivative;
      loss->compute(prediction.exists() ? prediction.getDouble() : 0.0, &exampleLossValue, &lossDerivative);
      if (!target)
        target = new DenseDoubleVector(parametersClass);
      inputVector->addWeightedTo(target, 0, weight * lossDerivative);
    }
    else
      jassert(false);

    return true;
  }

  lbcpp_UseDebuggingNewOperator
};

}; /* namespace lbcpp */

#endif // !LBCPP_LEARNING_NUMERICAL_LINEAR_LEARNABLE_FUNCTION_H_
