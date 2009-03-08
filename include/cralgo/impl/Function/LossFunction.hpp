/*-----------------------------------------.---------------------------------.
| Filename: LossFunction.hpp               |                                 |
| Author  : Francis Maes                   |                                 |
| Started : 07/03/2009 20:09               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef CRALGO_IMPL_FUNCTION_LOSS_H_
# define CRALGO_IMPL_FUNCTION_LOSS_H_

# include "ContinuousFunction.hpp"
# include "ScalarContinuousFunction.hpp"
# include "ScalarDerivableFunction.hpp"
# include "FunctionPairTraits.hpp"

namespace cralgo
{
namespace impl
{

template<class ExactType, class ScalarFunctionType, class LearningExampleType>
struct DiscriminantLossFunction : public ScalarLossFunction<ExactType>
{
  DiscriminantLossFunction(const ScalarFunctionType& scalarFunction) 
    : scalarFunction(scalarFunction), marginMultiplier(0.0) {}
  DiscriminantLossFunction() : marginMultiplier(0.0) {}
  
  enum
  {
    isDerivable = ScalarFunctionType::isDerivable,
  };
  
  void setLearningExample(const LearningExampleType& learningExample)
    {marginMultiplier = learningExample.getMarginMultiplier() * learningExample.getWeight();}

  // f(x) = scalarFunction(x * marginMultiplier)
  void compute(double input, double* output, const double* derivativeDirection, double* derivative) const
  {
    compose(multiplyScalarByConstant(marginMultiplier), scalarFunction)
      .compute(input, output, derivativeDirection, derivative);
  }
  
private:
  ScalarFunctionType scalarFunction;
  double marginMultiplier;
};

#define STATIC_DISCRIMINANT_LOSS_FUNCTION(FunctionName, LossName, LossFunctionName) \
template<class LearningExampleType> \
struct LossName : public DiscriminantLossFunction< LossName <LearningExampleType> , LossFunctionName, LearningExampleType> { \
   typedef DiscriminantLossFunction< LossName <LearningExampleType> , LossFunctionName, LearningExampleType> BaseClassType; \
   LossName(const LearningExampleType& learningExample) {BaseClassType::setLearningExample(learningExample);} \
   LossName() {} }; \
template<class LearningExampleType> \
inline LossName <LearningExampleType> FunctionName () {return LossName <LearningExampleType>();} \
template<class LearningExampleType> \
inline LossName <LearningExampleType> FunctionName (const LearningExampleType& example) {return LossName <LearningExampleType>(example);}

STATIC_DISCRIMINANT_LOSS_FUNCTION(perceptronLoss, PerceptronLoss, PerceptronLossFunction);
STATIC_DISCRIMINANT_LOSS_FUNCTION(hingeLoss, HingeLoss, HingeLossFunction);
STATIC_DISCRIMINANT_LOSS_FUNCTION(exponentialLoss, ExponentialLoss, ExponentialLossFunction);
STATIC_DISCRIMINANT_LOSS_FUNCTION(logBinomialLoss, LogBinomialLoss, LogBinomialLossFunction);

}; /* namespace impl */
}; /* namespace cralgo */

#endif // !CRALGO_IMPL_FUNCTION_LOSS_H_
