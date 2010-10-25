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

  virtual void computeAndAddGradient(double weight, const Variable& input, const Variable& supervision, const Variable& prediction, double& exampleLossValue, ObjectPtr* target)
  {
    jassert(supervision.exists());
    const PerceptionPtr& perception = getPerception();
    const FunctionPtr& lossFunction = supervision.getObjectAndCast<Function>();
    if (lossFunction.isInstanceOf<ScalarFunction>())
    {
      const ScalarFunctionPtr& loss = lossFunction.staticCast<ScalarFunction>();
      double lossDerivative;
      loss->compute(prediction.exists() ? prediction.getDouble() : 0.0, &exampleLossValue, &lossDerivative);
     // std::cout << "computeAndAddGradient: prevL2=" << (target ? l2norm(target) : -1.0)
     //   << " w = " << weight << " loss = " << exampleLossValue << " lossDerivative = " << lossDerivative << " inputL2 =  " << l2norm(perception, input);

      if (target)
        lbcpp::addWeighted(*target, perception, input, lossDerivative * weight);
      else
        addWeightedToParameters(perception, input, lossDerivative * weight);
     // std::cout << " newL2 = " << l2norm(target) << std::endl;
    }
    else if (lossFunction.isInstanceOf<RankingLossFunction>())
    {
      const ContainerPtr& alternatives = input.getObjectAndCast<Container>();
      const ContainerPtr& scores = prediction.getObjectAndCast<Container>();
      size_t n = alternatives->getNumElements();
      jassert(!scores || n == scores->getNumElements());
      std::vector<double> lossGradient;
      const RankingLossFunctionPtr& loss = lossFunction.staticCast<RankingLossFunction>();
      loss->compute(scores, n, &exampleLossValue, &lossGradient);
      jassert(lossGradient.size() == n);
      
      if (target)
      {
        for (size_t i = 0; i < n; ++i)
          lbcpp::addWeighted(*target, perception, alternatives->getElement(i).getObject(), lossGradient[i] * weight);
      }
      else
      {
        ScopedWriteLock _(parametersLock);
        const NumericalInferenceParametersPtr& parameters = getParameters();
        for (size_t i = 0; i < n; ++i)
        {
          Variable alternative = alternatives->getElement(i);
          if (alternative.exists())
            lbcpp::addWeighted(parameters->getWeights(), perception, alternative, lossGradient[i] * weight);
        }
      }
    }
    else
      jassert(false); // unrecognized loss function
  }

  virtual Variable predict(const Variable& input) const
  {
    ScopedReadLock _(parametersLock);
    const ObjectPtr& weights = getParameters()->getWeights();
    if (!weights)
      return Variable::missingValue(doubleType);
    if (!input.exists())
      return 0.0;
    return lbcpp::dotProduct(weights, getPerception(), input);
  }
};

typedef ReferenceCountedObjectPtr<LinearInference> LinearInferencePtr;

}; /* namespace lbcpp */

#endif // !LBCPP_NUMERICAL_LEARNING_INFERENCE_LINEAR_H_
