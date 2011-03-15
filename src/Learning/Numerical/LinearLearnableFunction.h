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
# include <lbcpp/Learning/BatchLearner.h>
# include <lbcpp/Learning/Numerical.h>

namespace lbcpp
{

// DoubleVector<T>, optional Supervision -> Double
class LinearLearnableFunction : public NumericalLearnableFunction
{
public:
  virtual size_t getNumRequiredInputs() const
    {return 2;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return index ? anyType : (TypePtr)doubleVectorClass();}

  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
  {
    parametersEnumeration = DoubleVector::getElementsEnumeration(inputVariables[0]->getType());
    outputName = T("prediction");
    outputShortName = T("p");
    setBatchLearner(stochasticBatchLearner());
    return doubleType;
  }

  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
  {
    const DoubleVectorPtr& inputVector = inputs[0].getObjectAndCast<DoubleVector>();
    if (!parameters || !inputVector)
      return Variable::missingValue(doubleType);

    double res = inputVector->dotProduct(parameters);
    return isNumberValid(res) ? Variable(res) : Variable::missingValue(doubleType);
  }

  virtual DoubleVectorPtr getParameters() const
    {return parameters;}

  virtual void setParameters(const DoubleVectorPtr& parameters)
    {this->parameters = parameters;}

  virtual void addGradient(const Variable& lossDerivativeOrGradient, const Variable* inputs, const DoubleVectorPtr& target, double weight) const
    {inputs[0].getObjectAndCast<DoubleVector>()->addWeightedTo(target, 0, weight * (lossDerivativeOrGradient.exists() ? lossDerivativeOrGradient.getDouble() : 1.0));}

  virtual bool computeLoss(const FunctionPtr& lossFunction, const Variable* inputs, const Variable& prediction, double& lossValue, Variable& lossDerivativeOrGradient) const
  {
    ScalarFunctionPtr scalarFunction = lossFunction.dynamicCast<ScalarFunction>();
    jassert(scalarFunction);
    const DoubleVectorPtr& input = inputs[0].getObjectAndCast<DoubleVector>();
    const Variable& supervision = inputs[1];

    double lossDerivative = 0.0;
    lossValue = 0.0;
    scalarFunction->computeScalarFunction(prediction.getDouble(), &supervision, &lossValue, &lossDerivative);
    if (!isNumberValid(lossValue) || !isNumberValid(lossDerivative))
      return false;

    lossDerivativeOrGradient = lossDerivative;
    return true;
  }

  virtual double getInputsSize(const Variable* inputs) const
    {return (double)inputs[0].getObjectAndCast<DoubleVector>()->l0norm();}

  virtual void clone(ExecutionContext& context, const ObjectPtr& target) const
  {
    NumericalLearnableFunction::clone(context, target);
    target.staticCast<LinearLearnableFunction>()->parameters = parameters->cloneAndCast<DenseDoubleVector>(context);
  }

  lbcpp_UseDebuggingNewOperator

protected:
  friend class LinearLearnableFunctionClass;

  DenseDoubleVectorPtr parameters;
};

}; /* namespace lbcpp */

#endif // !LBCPP_LEARNING_NUMERICAL_LINEAR_LEARNABLE_FUNCTION_H_
