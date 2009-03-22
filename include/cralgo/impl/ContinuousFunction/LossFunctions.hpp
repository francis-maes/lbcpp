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
# include "ScalarFunctions.hpp"

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

  void compute(const FeatureGeneratorPtr input, double* output, const FeatureGeneratorPtr gradientDirection, FeatureGeneratorPtr* gradient) const
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

  void compute(const FeatureGeneratorPtr input, double* output, const FeatureGeneratorPtr gradientDirection, FeatureGeneratorPtr* gradient) const
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
  
  static void sortScores(const std::vector<double>& scores, std::vector<size_t>& res)
  {
    res.resize(scores.size());
    for (size_t i = 0; i < res.size(); ++i)
      res[i] = i;
    std::sort(res.begin(), res.end(), CompareWRTScores(scores));
  }  

private:
  const std::vector<double>* costs;
  
  struct CompareWRTScores
  {
    CompareWRTScores(const std::vector<double>& scores) : scores(scores) {}
    const std::vector<double>& scores;
    bool operator()(size_t first, size_t second) const
      {return scores[first] > scores[second];}
  };
};

template<class ExactType, class DiscriminantLoss>
struct AdditiveRankingLossFunction : public RankingLossFunction<ExactType>
{
  typedef RankingLossFunction<ExactType> BaseClass;

  AdditiveRankingLossFunction(const DiscriminantLoss& discriminantLoss)
    : discriminantLoss(discriminantLoss) {}
  AdditiveRankingLossFunction() {}
  
  enum {isDerivable = DiscriminantLoss::isDerivable};

  // override this:
  void computeRankingLoss(const std::vector<double>& scores, const std::vector<double>& costs,
               double* output, const std::vector<double>* gradientDirection, std::vector<double>* gradient) const
    {assert(false);}
                              
  void compute(const FeatureGeneratorPtr input, double* output, const FeatureGeneratorPtr gradientDirection, FeatureGeneratorPtr* gradient) const
  {
    if (output)
      *output = 0.0;

    const std::vector<double>& costs = BaseClass::getCosts();
    if (!costs.size())
      return;

    DenseVectorPtr scores = input->toDenseVector();
    if (!scores || !scores->getNumValues())
    {
      std::cerr << "Error: no scores, input class = " << input->getClassName() << ", input = " << std::endl << input->toString() << std::endl;
      assert(false);
    }
    assert(scores && scores->getNumValues() == costs.size());
    std::vector<double> g;
    DenseVectorPtr gradientDirectionDense;
    const std::vector<double>* gdir = NULL;
    if (gradient)
      g.resize(costs.size(), 0.0);
    if (gradientDirection)
    {
      gradientDirectionDense = gradientDirection->toDenseVector();
      gdir = &gradientDirectionDense->getValues();
    }
    BaseClass::_this().computeRankingLoss(scores->getValues(), costs, output, gdir, gradient ? &g : NULL);
    if (gradient)
      *gradient = new DenseVector(input->getDictionary(), g);
  }

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

  static void multiplyOutputAndGradient(double* output, std::vector<double>* gradient, double k)
  {
    if (output)
      *output *= k;
    if (gradient)
      for (size_t i = 0; i < gradient->size(); ++i)
        (*gradient)[i] *= k;
  }
  

  static bool areCostsBipartite(const std::vector<double>& costs)
  {
    double positiveCost = 0.0;
    bool positiveCostDefined = false;
    for (size_t i = 0; i < costs.size(); ++i)
      if (costs[i])
      {
        if (positiveCostDefined)
        {
          if (costs[i] != positiveCost)
            return false;
        }
        else
          positiveCost = costs[i], positiveCostDefined = true;
      }
      
    return positiveCostDefined;
  }
  
  // returns a map from costs to (argmin scores, argmax scores) pairs
  static void getScoreRangePerCost(const std::vector<double>& scores, const std::vector<double>& costs, std::map<double, std::pair<size_t, size_t> >& res)
  {
    res.clear();
    for (size_t i = 0; i < costs.size(); ++i)
    {
      double cost = costs[i];
      double score = scores[i];
      std::map<double, std::pair<size_t, size_t> >::iterator it = res.find(cost);
      if (it == res.end())
        res[cost] = std::make_pair(i, i);
      else
      {
        if (score < scores[it->second.first]) it->second.first = i;
        if (score > scores[it->second.second]) it->second.second = i;
      }
    }
  }
  
  static bool hasFewDifferentCosts(size_t numAlternatives, size_t numDifferentCosts)
    {return numAlternatives > 3 && (double)numAlternatives < 2.5 * numDifferentCosts;}  
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
