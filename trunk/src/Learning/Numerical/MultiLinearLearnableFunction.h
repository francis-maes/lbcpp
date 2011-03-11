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
  const CompositeDoubleVectorPtr& getParameters() const
    {return parameters.staticCast<CompositeDoubleVector>();}

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
    EnumerationPtr parametersEnumeration = cartesianProductEnumerationEnumeration(outputsEnumeration, featuresEnumeration);
/*
    DefaultEnumerationPtr parametersEnumeration = new DefaultEnumeration(T("MultiLinearParameters"));
    //size_t numFeatures = featuresEnumeration->getNumElements();
    size_t numOutputs = outputsEnumeration->getNumElements();
    for (size_t i = 0; i < numOutputs; ++i)
    {
      EnumerationElementPtr output = outputsEnumeration->getElement(i);
      parametersEnumeration->addElementsWithPrefix(context, featuresEnumeration, output->getName() + T("."), output->getShortName() + T("."));
    }
*/
    parametersClass = compositeDoubleVectorClass(parametersEnumeration);
    // output and learner
    outputName = T("prediction");
    outputShortName = T("p");
    setBatchLearner(stochasticBatchLearner());
    return denseDoubleVectorClass(outputsEnumeration);
  }

  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
  {
    const CompositeDoubleVectorPtr& parameters = getParameters();
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

  virtual void addGradient(const Variable& lossDerivativeOrGradient, const DoubleVectorPtr& input, DoubleVectorPtr& target, double weight) const
  {
    CompositeDoubleVectorPtr compositeTarget = target.dynamicCast<CompositeDoubleVector>();
    if (!target || !compositeTarget->getNumSubVectors())
    {
      compositeTarget = new CompositeDoubleVector(parametersClass);
      createCompositeSubVectors(compositeTarget);
      target = compositeTarget;
    }

    const DenseDoubleVectorPtr& lossGradient = lossDerivativeOrGradient.getObjectAndCast<DenseDoubleVector>();
    size_t n = lossGradient->getNumElements();
    jassert(compositeTarget && compositeTarget->getNumSubVectors() == n);
    for (size_t i = 0; i < n; ++i)
      input->addWeightedTo(compositeTarget->getSubVector(i), 0, weight * lossGradient->getValue(i));
  }

  virtual bool computeAndAddGradient(const FunctionPtr& lossFunction, const Variable* inputs, const Variable& prediction,
                                      double& exampleLossValue, DoubleVectorPtr& target, double weight) const
  {
    ScalarVectorFunctionPtr scalarVectorFunction = lossFunction.dynamicCast<ScalarVectorFunction>();
    jassert(scalarVectorFunction);
    const DoubleVectorPtr& input = inputs[0].getObjectAndCast<DoubleVector>();
    const Variable& supervision = inputs[1];
    const DenseDoubleVectorPtr& predictedScores = prediction.getObjectAndCast<DenseDoubleVector>(); 

    DenseDoubleVectorPtr lossGradient = new DenseDoubleVector(getOutputType());
    scalarVectorFunction->computeScalarVectorFunction(predictedScores, &supervision, &exampleLossValue, &lossGradient, 1.0);
    if (!isNumberValid(exampleLossValue) || !isNumberValid(lossGradient->l2norm()))
      return false;

    addGradient(lossGradient, input, target, weight);
    return true;
  }

  lbcpp_UseDebuggingNewOperator

protected:
  EnumerationPtr featuresEnumeration;
  EnumerationPtr outputsEnumeration;

  void createCompositeSubVectors(const CompositeDoubleVectorPtr& composite) const
  {
    size_t n = outputsEnumeration->getNumElements();
    size_t numFeatures = featuresEnumeration->getNumElements();
    jassert(numFeatures);
    ClassPtr subVectorsClass = denseDoubleVectorClass(featuresEnumeration);
    for (size_t i = 0; i < n; ++i)
      composite->appendSubVector(i * numFeatures, new DenseDoubleVector(subVectorsClass));
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_LEARNING_NUMERICAL_MULTI_LINEAR_LEARNABLE_FUNCTION_H_
