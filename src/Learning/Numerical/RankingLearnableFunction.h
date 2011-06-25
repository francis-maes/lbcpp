/*-----------------------------------------.---------------------------------.
| Filename: RankingLearnableFunction.h     | Ranking Learnable Function      |
| Author  : Francis Maes                   |                                 |
| Started : 15/03/2011 22:56               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LEARNING_NUMERICAL_RANKING_LEARNABLE_FUNCTION_H_
# define LBCPP_LEARNING_NUMERICAL_RANKING_LEARNABLE_FUNCTION_H_

# include <lbcpp/Function/ScalarFunction.h>
# include <lbcpp/Learning/BatchLearner.h>
# include <lbcpp/Learning/Numerical.h>

namespace lbcpp
{

// Container[DoubleVector], DoubleVector -> DenseDoubleVector
class RankingLearnableFunction : public NumericalLearnableFunction
{
public:
  RankingLearnableFunction(NumericalLearnableFunctionPtr scoringFunction = NumericalLearnableFunctionPtr())
    : scoringFunction(scoringFunction) {}

  virtual size_t getNumRequiredInputs() const
    {return 2;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return index ? anyType : containerClass(doubleVectorClass());} // supervision may be of any type here

  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
  {
    TypePtr featuresType = Container::getTemplateParameter(inputVariables[0]->getType());

    if (!scoringFunction->initialize(context, featuresType, nilType))
      return TypePtr();

    parametersEnumeration = scoringFunction->getParametersEnumeration();
    
    outputName = T("prediction");
    outputShortName = T("p");
    setBatchLearner(stochasticBatchLearner());
    return denseDoubleVectorClass(positiveIntegerEnumerationEnumeration, scoringFunction->getOutputType());
  }

  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
  {
    const ObjectVectorPtr& features = inputs[0].getObjectAndCast<ObjectVector>();
    if (!scoringFunction->getParameters())
      return Variable::missingValue(getOutputType());
    
    const std::vector<ObjectPtr>& featureObjects = features->getObjects();
    size_t n = featureObjects.size();
    DenseDoubleVectorPtr res = new DenseDoubleVector(getOutputType(), n);
    for (size_t i = 0; i < n; ++i)
    {
      Variable score = scoringFunction->compute(context, featureObjects[i], Variable());
      if (!score.exists())
        return Variable::missingValue(getOutputType());
      jassert(score.isDouble());
      res->setValue(i, score.getDouble());
    }
    return res;
  }

  virtual DoubleVectorPtr getParameters() const
    {return scoringFunction->getParameters();}

  virtual void setParameters(const DoubleVectorPtr& parameters)
    {scoringFunction->setParameters(parameters);}

  virtual void addGradient(const Variable& lossDerivativeOrGradient, const Variable* inputs, const DoubleVectorPtr& target, double weight) const
  {
    const ObjectVectorPtr& features = inputs[0].getObjectAndCast<ObjectVector>();
    const DenseDoubleVectorPtr& lossGradient = lossDerivativeOrGradient.getObjectAndCast<DenseDoubleVector>();
    size_t n = features->getNumElements();
    for (size_t i = 0; i < n; ++i)
    {
      Variable alternativeFeatures = features->getElement(i);
      scoringFunction->addGradient(lossGradient->getValue(i), &alternativeFeatures, target, weight);
    }
  }

  virtual bool computeLoss(const FunctionPtr& lossFunction, const Variable* inputs, const Variable& prediction, double& lossValue, Variable& lossDerivativeOrGradient) const
  {
    const ScalarVectorFunctionPtr& loss = lossFunction.staticCast<ScalarVectorFunction>();
    const ObjectVectorPtr& features = inputs[0].getObjectAndCast<ObjectVector>();
    DenseDoubleVectorPtr scores = prediction.getObjectAndCast<DenseDoubleVector>();

    if (!scores)
      scores = new DenseDoubleVector(getOutputType(), features->getNumElements(), 0.0);
    size_t n = scores->getNumElements();

    jassert(features->getNumElements() == n);
    
    lossValue = 0.0;
    DenseDoubleVectorPtr lossGradient = new DenseDoubleVector(denseDoubleVectorClass(positiveIntegerEnumerationEnumeration), n, 0.0);
    loss->computeScalarVectorFunction(scores, inputs + 1, &lossValue, &lossGradient, 1.0);
    if (!isNumberValid(lossValue))
      return false;
    
    lossDerivativeOrGradient = lossGradient;
    return true;
#if 0 // old version with RankingLossFunction
    const RankingLossFunctionPtr& rankingLoss = lossFunction.staticCast<RankingLossFunction>();
    const ObjectVectorPtr& features = inputs[0].getObjectAndCast<ObjectVector>();
    DenseDoubleVectorPtr scores = prediction.getObjectAndCast<DenseDoubleVector>();
    const DenseDoubleVectorPtr& costs = inputs[1].getObjectAndCast<DenseDoubleVector>();

    if (!scores)
      scores = new DenseDoubleVector(getOutputType(), features->getNumElements(), 0.0);

    jassert(costs && costs->getNumElements() == features->getNumElements() && costs->getNumElements() == scores->getNumElements());
    size_t n = scores->getNumElements();
    
    lossValue = 0.0;
    DenseDoubleVectorPtr lossGradient = new DenseDoubleVector(denseDoubleVectorClass(positiveIntegerEnumerationEnumeration), n, 0.0);
    rankingLoss->computeRankingLoss(scores->getValues(), costs->getValues(), &lossValue, &lossGradient->getValues());
    if (!isNumberValid(lossValue))
      return false;
    
    lossDerivativeOrGradient = lossGradient;
#endif // 0
    return true;
  }

  virtual double getInputsSize(const Variable* inputs) const
  {
    const ObjectVectorPtr& features = inputs[0].getObjectAndCast<ObjectVector>();
    size_t n = features->getNumElements();
    double res = 0.0;
    for (size_t i = 0; i < n; ++i)
    {
      Variable subInput = features->getElement(i);
      res += scoringFunction->getInputsSize(&subInput);
    }
    if (n)
      res /= (double)n;
    return res;
  }

  virtual void clone(ExecutionContext& context, const ObjectPtr& target) const
  {
    NumericalLearnableFunction::clone(context, target);
    target.staticCast<RankingLearnableFunction>()->scoringFunction = scoringFunction->cloneAndCast<NumericalLearnableFunction>(context);
  }

protected:
  friend class RankingLearnableFunctionClass;

  NumericalLearnableFunctionPtr scoringFunction;
};

}; /* namespace lbcpp */

#endif // !LBCPP_LEARNING_NUMERICAL_RANKING_LEARNABLE_FUNCTION_H_
