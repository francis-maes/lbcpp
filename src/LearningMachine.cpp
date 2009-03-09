/*-----------------------------------------.---------------------------------.
| Filename: LearningMachine.cpp            | Learning machines               |
| Author  : Francis Maes                   |                                 |
| Started : 08/03/2009 22:59               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <cralgo/LearningMachine.h>
#include <cralgo/impl/impl.h>
using namespace cralgo;

/*
** BinaryClassifier
*/
size_t BinaryClassifier::predict(const FeatureGeneratorPtr input) const
{
  return predictScoreOfPositiveClass(input) > 0 ? 1 : 0;
}

double BinaryClassifier::predictScore(const FeatureGeneratorPtr input, size_t output) const
{
  double score = predictScoreOfPositiveClass(input);
  return output ? score : -score;
}

DenseVectorPtr BinaryClassifier::predictScores(const FeatureGeneratorPtr input) const
{
  double score = predictScoreOfPositiveClass(input);
  DenseVectorPtr res = new DenseVector(getLabels());
  res->set(0, -score);
  res->set(1, score);
  return res;
}

DenseVectorPtr BinaryClassifier::predictProbabilities(const FeatureGeneratorPtr input) const
{
  double prob1 = scoreToProbability(predictScoreOfPositiveClass(input));
  double prob0 = 1 - prob1;
  DenseVectorPtr res = new DenseVector(getLabels());
  res->set(0, prob0);
  res->set(1, prob1);
  return res;    
}

size_t BinaryClassifier::sample(const FeatureGeneratorPtr input) const
{
  double prob1 = scoreToProbability(predictScoreOfPositiveClass(input));
  return rand() / (double)RAND_MAX < prob1 ? 1 : 0;
}

template<class ExactType, class BaseClass>
class StaticToDynamicGradientBasedLearningMachine : public BaseClass
{
public:
  // abstract: static functions for architecture(), loss() and regularizer()
  
  virtual ScalarArchitecturePtr getPredictionArchitecture() const
    {return impl::instantiate(_this().architecture());}

  virtual ScalarVectorFunctionPtr getRegularizer() const
    {return impl::instantiate(_this().regularizer());}
  
  virtual ScalarVectorFunctionPtr getLoss(const ClassificationExample& example) const
    {return impl::instantiate(impl::exampleRisk(_this().architecture(), _this().loss(), example));}
    
  virtual ScalarVectorFunctionPtr getEmpiricalRisk(const std::vector<ClassificationExample>& examples) const
    {return impl::instantiate(impl::empiricalRisk(_this().architecture(), _this().loss(), examples));}
    
  virtual ScalarVectorFunctionPtr getRegularizedEmpiricalRisk(const std::vector<ClassificationExample>& examples) const
    {return impl::instantiate(impl::add(impl::empiricalRisk(_this().architecture(), _this().loss(), examples), _this().regularizer()));}

protected:
  const ExactType& _this() const  {return *(const ExactType* )this;}
};

class LogisticRegressionClassifier
  : public StaticToDynamicGradientBasedLearningMachine<LogisticRegressionClassifier, GradientBasedBinaryClassifier>
{
public:
  inline impl::LinearArchitecture architecture() const
    {return impl::linearArchitecture();}

  inline impl::LogBinomialLoss<ClassificationExample> loss() const
    {return impl::logBinomialLoss<ClassificationExample>();}
    
  inline impl::ScalarVectorFunctionScalarConstantPair<impl::SumOfSquaresScalarVectorFunction, void>::Multiplication regularizer() const
    {return impl::multiply(impl::sumOfSquares(), impl::constant(0.001));}
};

BinaryClassifierPtr cralgo::createLogisticRegressionClassifier(GradientBasedLearnerPtr learner, FeatureDictionary& labels)
{
  LogisticRegressionClassifier* res = new LogisticRegressionClassifier();
  res->setLearner(learner);
  res->setLabels(labels);
  return res;
}
