/*-----------------------------------------.---------------------------------.
| Filename: MultiLinearLearnableFunction.h | MultiLinear Learnable Function  |
| Author  : Francis Maes                   |                                 |
| Started : 17/02/2011 01:12               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LEARNING_NUMERICAL_MULTI_LINEAR_LEARNABLE_FUNCTION_H_
# define LBCPP_LEARNING_NUMERICAL_MULTI_LINEAR_LEARNABLE_FUNCTION_H_

# include <lbcpp/Function/ScalarFunction.h>
# include <lbcpp/Function/Evaluator.h>
# include <lbcpp/Learning/BatchLearner.h>
# include <lbcpp/Learning/Numerical.h>

namespace lbcpp
{

// DoubleVector<T>, optional ScalarVectorFunction -> DenseDoubleVector<>
class MultiLinearLearnableFunction : public NumericalLearnableFunction
{
public:
  const CompositeDoubleVectorPtr& getParameters() const
    {return parameters.staticCast<CompositeDoubleVector>();}

  virtual size_t getNumRequiredInputs() const
    {return 2;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return index ? scalarVectorFunctionClass : doubleVectorClass();}

  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
  {
    EnumerationPtr featuresEnumeration = DoubleVector::getElementsEnumeration(inputVariables[0]->getType());     
    parametersClass = compositeDoubleVectorClass();
    outputName = T("prediction");
    outputShortName = T("p");
    setBatchLearner(stochasticBatchLearner());
    return doubleType;
  }

  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
  {
    const DoubleVectorPtr& inputVector = inputs[0].getObjectAndCast<DoubleVector>();
    const CompositeDoubleVectorPtr& parameters = getParameters();

    if (!parameters || !inputVector)
      return Variable::missingValue(doubleType);

    size_t n = parameters->getNumSubVectors();
    DenseDoubleVectorPtr res = new DenseDoubleVector(denseDoubleVectorClass(), n);
    for (size_t i = 0; i < n; ++i)
    {
      double prediction = inputVector->dotProduct(parameters->getSubVector(i));
      if (!isNumberValid(prediction))
        return Variable::missingValue(denseDoubleVectorClass());
      res->setValue(i, prediction);
    }
    return res;
  }

  virtual bool computeAndAddGradient(const FunctionPtr& lossFunction, const Variable* inputs, const Variable& prediction,
                                      double& exampleLossValue, DoubleVectorPtr& target, double weight) const
  {
    // FIXME
    return false;
  }
#if 0
  virtual bool computeAndAddGradient(const Variable* inputs, const Variable& prediction, double& exampleLossValue, DoubleVectorPtr& target, double weight) const
  {
    const DoubleVectorPtr& inputVector = inputs[0].getObjectAndCast<DoubleVector>();
    const ScalarVectorFunctionPtr& supervision = inputs[1].getObjectAndCast<ScalarVectorFunction>();

    if (!supervision)
      return false;

    // FIXME: compute all inputs

    jassert(false); // not implemented
    exampleLossValue = 0.0;
    DenseDoubleVectorPtr lossGradient;// = new DenseDoubleVector(denseDoubleVectorClass(inputVector->getElementsEnumeration()));
    //supervision->computeScalarVectorFunction(inputVector, &exampleLossValue, (DoubleVectorPtr* )&lossGradient, 1.0);

    if (!target)
      target = new CompositeDoubleVector(compositeDoubleVectorClass()); // FIXME: create sub vectors 

    CompositeDoubleVectorPtr compositeTarget = target.dynamicCast<CompositeDoubleVector>();
    jassert(compositeTarget || !target);
    
    for (size_t i = 0; i < lossGradient->getNumElements(); ++i)
      inputVector->addWeightedTo(compositeTarget->getSubVector(i), 0, weight * lossGradient->getValue(i));
    return true;
  }
#endif // 0

  lbcpp_UseDebuggingNewOperator
};

}; /* namespace lbcpp */

#endif // !LBCPP_LEARNING_NUMERICAL_MULTI_LINEAR_LEARNABLE_FUNCTION_H_
