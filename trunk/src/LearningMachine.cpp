/*-----------------------------------------.---------------------------------.
| Filename: LearningMachine.cpp            | Learning machines               |
| Author  : Francis Maes                   |                                 |
| Started : 08/03/2009 22:59               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <cralgo/LearningMachine.h>
#include <cfloat>
using namespace cralgo;

template<class BaseClass, class ExampleType>
class VerboseLearningMachine : public BaseClass
{
public:
  VerboseLearningMachine(std::ostream& ostr) : ostr(ostr) {}
  
  virtual bool trainBatch(const std::vector<ExampleType>& examples, ProgressCallback* progress = NULL)
  {
    ostr << "trainBatch() with " << examples.size() << " examples:" << std::endl;
    for (size_t i = 0; i < examples.size(); ++i)
      ostr << "  " << i << ": " << examples[i] << std::endl;
    return true;
  }

  virtual void trainStochasticBegin()
    {ostr << "trainStochasticBegin()" << std::endl;}
    
  virtual void trainStochasticExample(const ExampleType& example)
    {ostr << "trainStochasticExample(" << example << ")" << std::endl;}
    
  virtual void trainStochasticEnd()
    {ostr << "trainStochasticEnd()" << std::endl;}
    
protected:
  std::ostream& ostr;
};

/*
** Regression
*/
double Regressor::evaluateMeanAbsoluteError(const std::vector<RegressionExample>& examples) const
{
  assert(examples.size());
  double res = 0;
  for (size_t i = 0; i < examples.size(); ++i)
  {
    const RegressionExample& example = examples[i];
    res += fabs(example.getOutput() - predict(example.getInput()));
  }
  return res / examples.size();
}

class VerboseRegressor : public VerboseLearningMachine<Regressor, RegressionExample>
{
public:
  VerboseRegressor(std::ostream& ostr)
    : VerboseLearningMachine<Regressor, RegressionExample>(ostr) {}
    
  virtual double predict(const FeatureGeneratorPtr input) const
  { 
    ostr << "predict(" << input->toString() << ")" << std::endl;
    return 0.0;
  }
};

RegressorPtr Regressor::createVerbose(std::ostream& ostr)
  {return RegressorPtr(new VerboseRegressor(ostr));}

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
  DenseVectorPtr probs = predictProbabilities(input);
  return Random::getInstance().sampleWithNormalizedProbabilities(probs->getValues());
}

double Classifier::evaluateAccuracy(const std::vector<ClassificationExample>& examples) const
{
  assert(examples.size());
  size_t correct = 0;
  for (size_t i = 0; i < examples.size(); ++i)
  {
    const ClassificationExample& example = examples[i];
    if (predict(example.getInput()) == example.getOutput())
      ++correct;
  }
  return correct / (double)examples.size();
}

double Classifier::evaluateWeightedAccuracy(const std::vector<ClassificationExample>& examples) const
{
  assert(examples.size());
  double correctWeight = 0.0, totalWeight = 0.0;
  for (size_t i = 0; i < examples.size(); ++i)
  {
    const ClassificationExample& example = examples[i];
    if (predict(example.getInput()) == example.getOutput())
      correctWeight += example.getWeight();
    totalWeight += example.getWeight();
  }
  assert(totalWeight);
  return correctWeight / totalWeight;
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
  DenseVectorPtr res = new DenseVector(outputsDictionary);
  res->set(0, -score);
  res->set(1, score);
  return res;
}

DenseVectorPtr BinaryClassifier::predictProbabilities(const FeatureGeneratorPtr input) const
{
  double prob1 = scoreToProbability(predictScoreOfPositiveClass(input));
  double prob0 = 1 - prob1;
  DenseVectorPtr res = new DenseVector(outputsDictionary);
  res->set(0, prob0);
  res->set(1, prob1);
  return res;    
}

size_t BinaryClassifier::sample(const FeatureGeneratorPtr input) const
{
  double prob1 = scoreToProbability(predictScoreOfPositiveClass(input));
  return Random::getInstance().sampleBool(prob1) ? 1 : 0;
}

/*
** Generalized classifier
*/
size_t GeneralizedClassifier::predict(const GeneralizedClassificationExample& example) const
{
  double bestScore = -DBL_MAX;
  size_t res = (size_t)-1;
  for (size_t i = 0; i < example.getNumAlternatives(); ++i)
  {
    double score = predictScore(example.getAlternative(i));
    if (score > bestScore)
      bestScore = score, res = i;
  }
  assert(res != (size_t)-1);
  return res;
}

DenseVectorPtr GeneralizedClassifier::predictScores(const FeatureGeneratorPtr compositeInput) const
{
  size_t n = compositeInput->getNumSubGenerators();
  DenseVectorPtr res = new DenseVector(compositeInput->getDictionary()->getDictionaryWithSubScopesAsFeatures(), n);
  for (size_t i = 0; i < n; ++i)
    res->set(compositeInput->getSubGeneratorIndex(i), predictScore(compositeInput->getSubGenerator(i)));
  return res;
}

DenseVectorPtr GeneralizedClassifier::predictProbabilities(const FeatureGeneratorPtr compositeInput) const
{
  // default: Gibbs distribution, P[y|x] = exp(score(y)) / (sum_i exp(score(y_i)))
  DenseVectorPtr scores = predictScores(compositeInput);
  double logZ = scores->computeLogSumOfExponentials();
  DenseVectorPtr res = new DenseVector(scores->getDictionary(), scores->getNumValues());
  for (size_t i = 0; i < scores->getNumValues(); ++i)
    res->set(i, exp(scores->get(i) - logZ));
  return res;
}

size_t GeneralizedClassifier::sample(const FeatureGeneratorPtr compositeInput) const
{
  DenseVectorPtr probs = predictProbabilities(compositeInput);
  return Random::getInstance().sampleWithNormalizedProbabilities(probs->getValues());  
}

/*
** Ranker
*/
size_t Ranker::predict(const FeatureGeneratorPtr compositeInput) const
{
  assert(compositeInput && compositeInput->getNumSubGenerators());
  double bestScore = -DBL_MAX;
  size_t res = (size_t)-1;
  for (size_t i = 0; i < compositeInput->getNumSubGenerators(); ++i)
  {
    double score = predictScore(compositeInput->getSubGenerator(i));
    if (score > bestScore)
      bestScore = score, res = compositeInput->getSubGeneratorIndex(i);
  }
  assert(res != (size_t)-1);
  return res;
}

DenseVectorPtr Ranker::predictScores(const FeatureGeneratorPtr compositeInput) const
{
  size_t n = compositeInput->getNumSubGenerators();
  DenseVectorPtr res = new DenseVector(compositeInput->getDictionary()->getDictionaryWithSubScopesAsFeatures(), n);
  for (size_t i = 0; i < n; ++i)
    res->set(compositeInput->getSubGeneratorIndex(i), predictScore(compositeInput->getSubGenerator(i)));
  return res;  
}
