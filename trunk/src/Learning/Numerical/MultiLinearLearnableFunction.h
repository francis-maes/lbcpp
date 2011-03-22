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
# include <lbcpp/Learning/BatchLearner.h>
# include <lbcpp/Learning/Numerical.h>

namespace lbcpp
{

// DoubleVector<T>, optional ScalarVectorFunction -> DenseDoubleVector<>
class MultiLinearLearnableFunction : public NumericalLearnableFunction
{
public:
  virtual size_t getNumRequiredInputs() const
    {return 2;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return index ? anyType : (TypePtr)doubleVectorClass();}

  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
  {
    // retrieve features and outputs
    featuresEnumeration = DoubleVector::getElementsEnumeration(inputVariables[0]->getType());
    jassert(featuresEnumeration);
    outputsEnumeration = inputVariables[1]->getType().dynamicCast<Enumeration>();
    if (!outputsEnumeration)
      outputsEnumeration = DoubleVector::getElementsEnumeration(inputVariables[1]->getType());
    if (!outputsEnumeration)
    {
      context.errorCallback(T("Could not identify elements enumeration"));
      return TypePtr();
    }

    // make parameters class
    parametersEnumeration = cartesianProductEnumerationEnumeration(outputsEnumeration, featuresEnumeration);
    // output and learner
    outputName = T("prediction");
    outputShortName = T("p");
    setBatchLearner(stochasticBatchLearner());
    return denseDoubleVectorClass(outputsEnumeration);
  }

  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
  {
    const DoubleVectorPtr& inputVector = inputs[0].getObjectAndCast<DoubleVector>();

    if (!parameters || !parameters->getNumSubVectors() || !inputVector)
      return Variable::missingValue(getOutputType());
    
    size_t n = parameters->getNumSubVectors();
    jassert(n == outputsEnumeration->getNumElements());
    DenseDoubleVectorPtr res = new DenseDoubleVector(getOutputType());
    for (size_t i = 0; i < n; ++i)
    {
      double prediction = inputVector->dotProduct(parameters->getSubVector(i));
      if (!isNumberValid(prediction))
        return Variable::missingValue(getOutputType());
      res->setValue(i, prediction);
    }
    return res;
  }

  virtual DoubleVectorPtr createParameters() const
  {
    CompositeDoubleVectorPtr res = new CompositeDoubleVector(compositeDoubleVectorClass(parametersEnumeration));
  
    size_t n = outputsEnumeration->getNumElements();
    size_t numFeatures = featuresEnumeration->getNumElements();
    jassert(numFeatures);
    ClassPtr subVectorsClass = denseDoubleVectorClass(featuresEnumeration);
    for (size_t i = 0; i < n; ++i)
      res->appendSubVector(i * numFeatures, new DenseDoubleVector(subVectorsClass));
    return res;
  }

  virtual DoubleVectorPtr getParameters() const
    {return parameters;}

  virtual void setParameters(const DoubleVectorPtr& parameters)
  {
    CompositeDoubleVectorPtr params = parameters.dynamicCast<CompositeDoubleVector>();
    jassert(params);
    this->parameters = params;
  }

  virtual void addGradient(const Variable& lossDerivativeOrGradient, const Variable* inputs, const DoubleVectorPtr& target, double weight) const
  {
    const DenseDoubleVectorPtr& lossGradient = lossDerivativeOrGradient.getObjectAndCast<DenseDoubleVector>();
    size_t n = lossGradient->getNumElements();
    const DoubleVectorPtr& input = inputs[0].getObjectAndCast<DoubleVector>();

    DenseDoubleVectorPtr denseTarget = target.dynamicCast<DenseDoubleVector>();
    if (denseTarget)
    {
      for (size_t i = 0; i < n; ++i)
        input->addWeightedTo(denseTarget, i * featuresEnumeration->getNumElements(), weight * lossGradient->getValue(i));
      return;
    }

    CompositeDoubleVectorPtr compositeTarget = target.dynamicCast<CompositeDoubleVector>();
    if (compositeTarget)
    {
      for (size_t i = 0; i < n; ++i)
      {
        DenseDoubleVectorPtr subTarget = compositeTarget->getSubVector(i).dynamicCast<DenseDoubleVector>();
        jassert(subTarget);
        input->addWeightedTo(subTarget, 0, weight * lossGradient->getValue(i));
      }
      return;
    }

    jassert(false);
  }
 
  virtual bool computeLoss(const FunctionPtr& lossFunction, const Variable* inputs, const Variable& prediction, double& lossValue, Variable& lossDerivativeOrGradient) const
  {
    ScalarVectorFunctionPtr scalarVectorFunction = lossFunction.dynamicCast<ScalarVectorFunction>();
    jassert(scalarVectorFunction);
    //const DoubleVectorPtr& input = inputs[0].getObjectAndCast<DoubleVector>();
    const Variable& supervision = inputs[1];
    const DenseDoubleVectorPtr& predictedScores = prediction.getObjectAndCast<DenseDoubleVector>(); 

    DenseDoubleVectorPtr lossGradient = new DenseDoubleVector(getOutputType());
    scalarVectorFunction->computeScalarVectorFunction(predictedScores, &supervision, &lossValue, &lossGradient, 1.0);
    if (!isNumberValid(lossValue) || !isNumberValid(lossGradient->l2norm()))
      return false;

    lossDerivativeOrGradient = lossGradient;
    return true;
  }

  virtual double getInputsSize(const Variable* inputs) const
    {return (double)inputs[0].getObjectAndCast<DoubleVector>()->l0norm();}

  virtual void clone(ExecutionContext& context, const ObjectPtr& target) const
  {
    //NumericalLearnableFunction::clone(context, target);
    target.staticCast<MultiLinearLearnableFunction>()->parameters = parameters->cloneAndCast<CompositeDoubleVector>(context);
  }

  lbcpp_UseDebuggingNewOperator

protected:
  friend class MultiLinearLearnableFunctionClass;

  CompositeDoubleVectorPtr parameters;
  EnumerationPtr featuresEnumeration;
  EnumerationPtr outputsEnumeration;
};

}; /* namespace lbcpp */

#endif // !LBCPP_LEARNING_NUMERICAL_MULTI_LINEAR_LEARNABLE_FUNCTION_H_
