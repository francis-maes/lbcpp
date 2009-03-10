/*-----------------------------------------.---------------------------------.
| Filename: LearningMachine.cpp            | Learning machines               |
| Author  : Francis Maes                   |                                 |
| Started : 08/03/2009 22:59               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <cralgo/LearningMachine.h>
#include <cralgo/GradientBasedLearningMachine.h>
#include <cralgo/impl/impl.h>
using namespace cralgo;


template<class ExactType, class BaseClass>
class StaticToDynamicGradientBasedLearningMachine : public BaseClass
{
public:
  // abstract: static functions for architecture(), loss() and regularizer()
  
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


/*
** Classifier
*/
size_t Classifier::predict(const FeatureGeneratorPtr input) const
{
  return predictScores(input)->findIndexOfMaximumValue();
}

double Classifier::predictScore(const FeatureGeneratorPtr input, size_t output) const
{
  return predictScores(input)->get(output);
}

DenseVectorPtr Classifier::predictProbabilities(const FeatureGeneratorPtr input) const
{
  // default: Gibbs distribution, P[y|x] = exp(score(y)) / (sum_i exp(score(y_i)))
  DenseVectorPtr scores = predictScores(input);
  double logZ = scores->computeLogSumOfExponentials();
  DenseVectorPtr res = new DenseVector(scores->getDictionary(), scores->getNumValues());
  for (size_t i = 0; i < scores->getNumValues(); ++i)
    res->set(i, exp(scores->get(i) - logZ));
  return res;
}

size_t Classifier::sample(const FeatureGeneratorPtr input) const
{
  assert(false);
  // FIXME
  return 0; 
}

class MaximumEntropyClassifier
  : public StaticToDynamicGradientBasedLearningMachine<MaximumEntropyClassifier, GradientBasedClassifier>
{
public:
  virtual VectorArchitecturePtr getPredictionArchitecture() const
    {return impl::instantiate(architecture());}

  inline impl::MultiLinearArchitecture architecture() const
    {return impl::multiLinearArchitecture(getLabels());}

  inline impl::MultiClassLogBinomialLoss<ClassificationExample> loss() const
    {return impl::multiClassLogBinomialLoss<ClassificationExample>();}
    
  inline impl::ScalarVectorFunctionScalarConstantPair<impl::SumOfSquaresScalarVectorFunction, void>::Multiplication regularizer() const
    {return impl::multiply(impl::sumOfSquares(), impl::constant(0));}
};

GradientBasedClassifierPtr GradientBasedClassifier::createMaximumEntropy(GradientBasedLearnerPtr learner, FeatureDictionary& labels)
{
  GradientBasedClassifierPtr res = new MaximumEntropyClassifier();
  res->setLearner(learner);
  res->setLabels(labels);
  return res;
}

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

class LogisticRegressionClassifier
  : public StaticToDynamicGradientBasedLearningMachine<LogisticRegressionClassifier, GradientBasedBinaryClassifier>
{
public:
  virtual ScalarArchitecturePtr getPredictionArchitecture() const
    {return impl::instantiate(architecture());}

  inline impl::LinearArchitecture architecture() const
    {return impl::linearArchitecture();}

  inline impl::LogBinomialLoss<ClassificationExample> loss() const
    {return impl::logBinomialLoss<ClassificationExample>();}
    
  inline impl::ScalarVectorFunctionScalarConstantPair<impl::SumOfSquaresScalarVectorFunction, void>::Multiplication regularizer() const
    {return impl::multiply(impl::sumOfSquares(), impl::constant(0.001));}
};

GradientBasedBinaryClassifierPtr GradientBasedBinaryClassifier::createLogisticRegression(GradientBasedLearnerPtr learner, FeatureDictionary& labels)
{
  GradientBasedBinaryClassifierPtr res = new LogisticRegressionClassifier();
  res->setLearner(learner);
  res->setLabels(labels);
  return res;
}
