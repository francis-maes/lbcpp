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

// DoubleVector<T>, optional Supervision -> Double
class LinearLearnableFunction : public NumericalLearnableFunction
{
public:
  const DenseDoubleVectorPtr& getParameters() const
    {return parameters.staticCast<DenseDoubleVector>();}

  virtual size_t getNumRequiredInputs() const
    {return 2;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return index ? anyType : (TypePtr)doubleVectorClass();}

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

  virtual bool computeAndAddGradient(const FunctionPtr& lossFunction, const Variable* inputs, const Variable& prediction,
                                      double& exampleLossValue, DoubleVectorPtr& target, double weight) const
  {
    ScalarFunctionPtr scalarFunction = lossFunction.dynamicCast<ScalarFunction>();
    jassert(scalarFunction);
    const DoubleVectorPtr& input = inputs[0].getObjectAndCast<DoubleVector>();
    const Variable& supervision = inputs[1];

    double lossDerivative = 0.0;
    scalarFunction->computeScalarFunction(prediction.getDouble(), &supervision, &exampleLossValue, &lossDerivative);
    if (!isNumberValid(exampleLossValue) || !isNumberValid(lossDerivative))
      return false;

    if (!target)
      target = new DenseDoubleVector(parametersClass);
    input->addWeightedTo(target, 0, weight * lossDerivative);
    return true;
  }

  lbcpp_UseDebuggingNewOperator
};

}; /* namespace lbcpp */

#endif // !LBCPP_LEARNING_NUMERICAL_LINEAR_LEARNABLE_FUNCTION_H_
