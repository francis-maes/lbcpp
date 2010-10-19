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
# include <lbcpp/Perception/PerceptionMaths.h>
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
    : NumericalInference(name, perception) {}
  LinearInference() {}

  virtual TypePtr getSupervisionType() const
    {return scalarFunctionClass;}

  virtual TypePtr getOutputType(TypePtr ) const
    {return doubleType;}

  virtual TypePtr getParametersType() const
    {return getPerceptionOutputType();}

  virtual void computeAndAddGradient(double weight, const Variable& input, const Variable& supervision, const Variable& prediction, double& exampleLossValue, ObjectPtr* target)
  {
    const ScalarFunctionPtr& lossFunction = supervision.getObjectAndCast<ScalarFunction>();
    double lossDerivative;
    lossFunction->compute(prediction.exists() ? prediction.getDouble() : 0.0, &exampleLossValue, &lossDerivative);
   // std::cout << "computeAndAddGradient: prevL2=" << (target ? l2norm(target) : -1.0)
   //   << " w = " << weight << " loss = " << exampleLossValue << " lossDerivative = " << lossDerivative << " inputL2 =  " << l2norm(perception, input);

    if (target)
      lbcpp::addWeighted(*target, perception, input, lossDerivative * weight);
    else
    {
      ScopedWriteLock _(parametersLock);
      lbcpp::addWeighted(parameters, perception, input, lossDerivative * weight);
    }
   // std::cout << " newL2 = " << l2norm(target) << std::endl;
  }

  virtual Variable predict(const Variable& input) const
  {
    ScopedReadLock _(parametersLock);
    if (!parameters)
      return Variable::missingValue(doubleType);
    return lbcpp::dotProduct(parameters, perception, input);
  }
};

typedef ReferenceCountedObjectPtr<LinearInference> LinearInferencePtr;

}; /* namespace lbcpp */

#endif // !LBCPP_NUMERICAL_LEARNING_INFERENCE_LINEAR_H_
