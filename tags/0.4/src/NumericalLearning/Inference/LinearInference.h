/*-----------------------------------------.---------------------------------.
| Filename: LinearInference.h              | Linear Scalar Inference         |
| Author  : Francis Maes                   |                                 |
| Started : 05/05/2010 14:57               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_NUMERICAL_LEARNING_INFERENCE_LINEAR_H_
# define LBCPP_NUMERICAL_LEARNING_INFERENCE_LINEAR_H_

# include <lbcpp/NumericalLearning/NumericalLearning.h>
# include <lbcpp/Function/ScalarFunction.h>

namespace lbcpp
{

// Input: Perception's input
// Output: Scalar
// Supervision: ScalarFunction
class LinearInference : public NumericalInference
{
public:
  LinearInference(const String& name, PerceptionPtr perception)
    : NumericalInference(name, perception)
  {
    parameters = new NumericalInferenceParameters(perception, getWeightsType(perception->getOutputType()));  
  }

  LinearInference() {}

  virtual TypePtr getSupervisionType() const
    {return scalarFunctionClass;}

  virtual TypePtr getOutputType(TypePtr ) const
    {return doubleType;}

  virtual TypePtr getWeightsType(TypePtr perceptionOutputType) const
    {return perceptionOutputType;}

  virtual void computeAndAddGradient(ExecutionContext& context, double weight, const Variable& input, const Variable& supervision, const Variable& prediction, double& exampleLossValue, ObjectPtr* target)
  {
    jassert(supervision.exists());
    const PerceptionPtr& perception = getPerception();
    const FunctionPtr& lossFunction = supervision.getObjectAndCast<Function>(context);
    if (lossFunction.isInstanceOf<ScalarFunction>())
    {
      const ScalarFunctionPtr& loss = lossFunction.staticCast<ScalarFunction>();
      double lossDerivative;
      loss->compute(prediction.exists() ? prediction.getDouble() : 0.0, &exampleLossValue, &lossDerivative);
     // std::cout << "computeAndAddGradient: prevL2=" << (target ? l2norm(target) : -1.0)
     //   << " w = " << weight << " loss = " << exampleLossValue << " lossDerivative = " << lossDerivative << " inputL2 =  " << l2norm(perception, input);

      bool isLocked = false;
      if (!target)
      {
        parametersLock.enterWrite();
        target = &getParameters()->getWeights();
        isLocked = true;
      }

      if (input.getType() == perception->getOutputType())
        lbcpp::addWeighted(context, *target, input.getObject(), lossDerivative * weight);
      else
        lbcpp::addWeighted(context, *target, perception, input, lossDerivative * weight);
       
      if (isLocked)
        parametersLock.exitWrite();
     // std::cout << " newL2 = " << l2norm(target) << std::endl;
    }
    else if (lossFunction.isInstanceOf<RankingLossFunction>())
    {
      const ContainerPtr& alternatives = input.getObjectAndCast<Container>(context);
      const ContainerPtr& scores = prediction.getObjectAndCast<Container>(context);
      size_t n = alternatives->getNumElements();
      jassert(!scores || n == scores->getNumElements());
      std::vector<double> lossGradient;
      const RankingLossFunctionPtr& loss = lossFunction.staticCast<RankingLossFunction>();
      loss->compute(context, scores, n, &exampleLossValue, &lossGradient);
      jassert(lossGradient.size() == n);
      
      if (target)
      {
        for (size_t i = 0; i < n; ++i)
          lbcpp::addWeighted(context, *target, perception, alternatives->getElement(i).getObject(), lossGradient[i] * weight);
      }
      else
      {
        ScopedWriteLock _(parametersLock);
        const NumericalInferenceParametersPtr& parameters = getParameters();
        for (size_t i = 0; i < n; ++i)
        {
          Variable alternative = alternatives->getElement(i);
          if (alternative.exists())
            lbcpp::addWeighted(context, parameters->getWeights(), perception, alternative, lossGradient[i] * weight);
        }
      }
    }
    else
      jassert(false); // unrecognized loss function
  }

  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
  {
    ScopedReadLock _(parametersLock);
    const ObjectPtr& weights = getParameters()->getWeights();
    if (!weights)
      return Variable::missingValue(doubleType);
    if (!input.exists())
      return 0.0;
    const PerceptionPtr& perception = getPerception();
    double res;
    if (input.getType() == perception->getOutputType())
      res = lbcpp::dotProduct(context, weights, input.getObject());
    else
      res = lbcpp::dotProduct(context, weights, perception, input);
    return isNumberValid(res) ? Variable(res) : Variable::missingValue(doubleType);
  }
};

typedef ReferenceCountedObjectPtr<LinearInference> LinearInferencePtr;

}; /* namespace lbcpp */

#endif // !LBCPP_NUMERICAL_LEARNING_INFERENCE_LINEAR_H_
