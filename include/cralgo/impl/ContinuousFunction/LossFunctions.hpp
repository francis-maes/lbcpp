/*-----------------------------------------.---------------------------------.
| Filename: LossFunctions.hpp              |                                 |
| Author  : Francis Maes                   |                                 |
| Started : 07/03/2009 20:09               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef CRALGO_IMPL_FUNCTION_LOSS_H_
# define CRALGO_IMPL_FUNCTION_LOSS_H_

# include "FunctionStatic.hpp"
# include "FunctionPairTraits.hpp"

namespace cralgo {
namespace impl {

/*
** Regression
*/
template<class ExactType, class ScalarFunctionType, class LearningExampleType>
struct RegressionLoss : public ScalarLossFunction<ExactType>
{
  RegressionLoss(const ScalarFunctionType& scalarFunction) 
    : scalarFunction(scalarFunction), correctValue(0.0) {}
  RegressionLoss() : correctValue(0.0) {}
  
  enum  {isDerivable = ScalarFunctionType::isDerivable};
  
  void setLearningExample(const LearningExampleType& learningExample)
    {correctValue = learningExample.getOutput();}

  // f(x) = scalarFunction(x - correctValue)
  void compute(double input, double* output, const double* derivativeDirection, double* derivative) const
  {
    compose(addConstant(-correctValue), scalarFunction)
      .compute(input, output, derivativeDirection, derivative);
  }
  
private:
  ScalarFunctionType scalarFunction;
  double correctValue;
};

/*
** Discriminant: binary classification and base losses for ranking
*/
template<class ExactType, class ScalarFunctionType, class LearningExampleType>
struct DiscriminantLoss : public ScalarLossFunction<ExactType>
{
  DiscriminantLoss(const ScalarFunctionType& scalarFunction) 
    : scalarFunction(scalarFunction), marginMultiplier(0.0) {}
  DiscriminantLoss() : marginMultiplier(0.0) {}
  
  enum  {isDerivable = ScalarFunctionType::isDerivable};
  
  void setLearningExample(const LearningExampleType& learningExample)
    {marginMultiplier = learningExample.getMarginMultiplier();}

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

/*
** Multi-class classification
*/
template<class ExactType, class VectorFunctionType, class LearningExampleType>
struct MulticlassLoss : public VectorLossFunction<ExactType>
{
  MulticlassLoss(const VectorFunctionType& vectorToScalarFunction)
    : vectorToScalarFunction(vectorToScalarFunction) {}
  MulticlassLoss() {}
    
  enum  {isDerivable = VectorFunctionType::isDerivable};
  
  void setLearningExample(const LearningExampleType& learningExample)
    {vectorToScalarFunction.setCorrectClass(learningExample.getOutput());}

  void compute(const FeatureGeneratorPtr input, double* output, const FeatureGeneratorPtr gradientDirection, LazyVectorPtr gradient) const
    {vectorToScalarFunction.compute(input, output, gradientDirection, gradient);}

private:
  VectorFunctionType vectorToScalarFunction;
};

/*
** Ranking
*/
template<class ExactType, class VectorFunctionType, class LearningExampleType>
struct RankingLoss : public VectorLossFunction<ExactType>
{
  RankingLoss(const VectorFunctionType& vectorToScalarFunction)
    : vectorToScalarFunction(vectorToScalarFunction) {}
  RankingLoss() {}

  enum  {isDerivable = VectorFunctionType::isDerivable};
  
  void setLearningExample(const LearningExampleType& example)
    {vectorToScalarFunction.setCosts(example.getCosts());}

  void compute(const FeatureGeneratorPtr input, double* output, const FeatureGeneratorPtr gradientDirection, LazyVectorPtr gradient) const
    {vectorToScalarFunction.compute(input, output, gradientDirection, gradient);}

private:
  VectorFunctionType vectorToScalarFunction;
};

template<class ExactType>
struct RankingLossFunction : public ScalarVectorFunction<ExactType>
{
  RankingLossFunction() : costs(NULL) {}
  
  void setCosts(const std::vector<double>& costs)
    {this->costs = &costs;}
  
  const std::vector<double>& getCosts() const
    {assert(costs); return *costs;}
  
protected:
  const std::vector<double>* costs;
};

template<class ExactType, class DiscriminantLoss>
struct AdditiveRankingLossFunction : public RankingLossFunction<ExactType>
{
  typedef RankingLossFunction<ExactType> BaseClass;

  AdditiveRankingLossFunction(const DiscriminantLoss& discriminantLoss)
    : discriminantLoss(discriminantLoss) {}
  AdditiveRankingLossFunction() {}
  
  enum {isDerivable = DiscriminantLoss::isDerivable};
  
protected:
  DiscriminantLoss discriminantLoss;
  
  void addRankingPair(double deltaCost, double deltaScore, size_t positiveAlternative, size_t negativeAlternative,
            double* output, const std::vector<double>* gradientDirection, std::vector<double>* gradient) const
  {
    assert(deltaCost > 0);
    // deltaScore = scores[positiveAlternative] - scores[negativeAlternative]
    // deltaScore should be positive
    
    double discriminantValue, discriminantDerivative;
    double discriminantDerivativeDirection;
    if (gradientDirection)
      discriminantDerivativeDirection = (*gradientDirection)[positiveAlternative] - (*gradientDirection)[negativeAlternative];
    discriminantLoss.compute(deltaScore, output ? &discriminantValue : NULL,
      gradientDirection ? &discriminantDerivativeDirection : NULL, gradient ? &discriminantDerivative : NULL);
    if (gradient)
    {
      double delta = deltaCost * discriminantDerivative;
      (*gradient)[positiveAlternative] += delta;
      (*gradient)[negativeAlternative] -= delta;
    }
    if (output)
      *output += deltaCost * discriminantValue;
  }
};

/*
** Macros to declare loss adaptators
*/
#define STATIC_LOSS_FUNCTION(LossClass, FunctionName, LossName, LossFunctionName) \
template<class LearningExampleType> \
struct LossName : public LossClass< LossName <LearningExampleType> , LossFunctionName, LearningExampleType> { \
   typedef LossClass< LossName <LearningExampleType> , LossFunctionName, LearningExampleType> BaseClassType; \
   LossName(const LearningExampleType& learningExample) {BaseClassType::setLearningExample(learningExample);} \
   LossName() {} }; \
template<class LearningExampleType> \
inline LossName <LearningExampleType> FunctionName () {return LossName <LearningExampleType>();} \
template<class LearningExampleType> \
inline LossName <LearningExampleType> FunctionName (const LearningExampleType& example) {return LossName <LearningExampleType>(example);}

#define STATIC_REGRESSION_LOSS_FUNCTION(FunctionName, LossName, LossFunctionName) \
  STATIC_LOSS_FUNCTION(RegressionLoss, FunctionName, LossName, LossFunctionName)

#define STATIC_DISCRIMINANT_LOSS_FUNCTION(FunctionName, LossName, LossFunctionName) \
  STATIC_LOSS_FUNCTION(DiscriminantLoss, FunctionName, LossName, LossFunctionName)

#define STATIC_MULTICLASS_LOSS_FUNCTION(FunctionName, LossName, LossFunctionName) \
  STATIC_LOSS_FUNCTION(MulticlassLoss, FunctionName, LossName, LossFunctionName)

#define STATIC_RANKING_LOSS_FUNCTION(FunctionName, LossName, LossFunctionName) \
template<class DiscriminantLoss, class LearningExampleType> \
struct LossName : public RankingLoss< LossName <DiscriminantLoss, LearningExampleType> , LossFunctionName<DiscriminantLoss>, LearningExampleType> { \
   typedef RankingLoss< LossName <DiscriminantLoss, LearningExampleType> , LossFunctionName<DiscriminantLoss>, LearningExampleType> BaseClassType; \
   LossName(const LearningExampleType& learningExample) {BaseClassType::setLearningExample(learningExample);} \
   LossName() {} }; \
template<class DiscriminantLoss, class LearningExampleType> \
inline LossName <DiscriminantLoss, LearningExampleType> FunctionName () {return LossName <DiscriminantLoss, LearningExampleType>();} \
template<class DiscriminantLoss, class LearningExampleType> \
inline LossName <DiscriminantLoss, LearningExampleType> FunctionName (const LearningExampleType& example) {return LossName <DiscriminantLoss, LearningExampleType>(example);}

}; /* namespace impl */
}; /* namespace cralgo */

#endif // !CRALGO_IMPL_FUNCTION_LOSS_H_
