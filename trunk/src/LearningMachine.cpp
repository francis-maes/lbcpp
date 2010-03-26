/*-----------------------------------------.---------------------------------.
| Filename: LearningMachine.cpp            | Learning machines               |
| Author  : Francis Maes                   |                                 |
| Started : 08/03/2009 22:59               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/LearningMachine.h>
#include <lbcpp/ObjectStream.h>
#include <lbcpp/ObjectContainer.h>
#include <cfloat>
using namespace lbcpp;

/*
** LearningMachine
*/
void LearningMachine::trainStochastic(ObjectStreamPtr examples, ProgressCallbackPtr progress)
{
  trainStochasticBegin(FeatureDictionaryPtr());
  if (progress)
    progress->progressStart("LearningMachine::trainStochastic");
  size_t count = 0;
  while (true)
  {
    ObjectPtr example = examples->next();
    if (example)
    {
      trainStochasticExample(example);
      ++count;
      if (progress && (count % 50) == 0 && !progress->progressStep("LearningMachine::trainStochastic", (double)count))
        break;
    }
    else
      break;
  }
  if (progress)
    progress->progressEnd();
  trainStochasticEnd();
}
  
void LearningMachine::trainStochastic(ObjectContainerPtr examples, ProgressCallbackPtr progress)
{
  trainStochasticBegin(FeatureDictionaryPtr());
  if (progress)
    progress->progressStart("LearningMachine::trainStochastic");
  for (size_t i = 0; i < examples->size(); ++i)
  {
    trainStochasticExample(examples->get(i));
    if (progress && (i % 50) == 0 && 
      !progress->progressStep("LearningMachine::trainStochastic", (double)i, (double)examples->size()))
      break;
  }
  if (progress)
    progress->progressEnd();
  trainStochasticEnd();
}

bool LearningMachine::trainBatch(ObjectStreamPtr examples, ProgressCallbackPtr progress)
{
  return trainBatch(examples->load(), progress);
}
/*
class BatchToStochasticLearningMachine : public LearningMachine
{
public:
  BatchToStochasticLearningMachine(LearningMachinePtr target)
    : target(target) {}
  
  virtual void trainStochastic(ObjectStreamPtr examples, ProgressCallbackPtr progress = ProgressCallbackPtr())
    {target->trainBatch(examples, progress);}
    
  virtual void trainStochastic(ObjectContainerPtr examples, ProgressCallbackPtr progress = ProgressCallbackPtr())
    {target->trainBatch(examples, progress);}

  virtual void trainStochasticBegin(FeatureDictionaryPtr inputDictionary)
    {examples = new VectorObjectContainer();}
  virtual void trainStochasticExample(ObjectPtr example)
    {examples->append(example);}
  virtual void trainStochasticEnd()
    {target->trainBatch(examples);}
  
  virtual bool trainBatch(ObjectContainerPtr examples, ProgressCallbackPtr progress = ProgressCallbackPtr())
    {return target->trainBatch(examples, progress);}
  
  virtual bool trainBatch(ObjectStreamPtr examples, ProgressCallbackPtr progress = ProgressCallbackPtr())
    {return target->trainBatch(examples, progress);}

  virtual FeatureDictionaryPtr getInputDictionary() const
    {return target->getInputDictionary();}
  
private:
  LearningMachinePtr target;
  VectorObjectContainerPtr examples;
};

LearningMachinePtr LearningMachine::batchToStochasticMachine() const
{
  return new BatchToStochasticLearningMachine(this);
}*/

/*
** VerboseLearningMachine
*/
template<class BaseClass>
class VerboseLearningMachine : public BaseClass
{
public:
  VerboseLearningMachine(std::ostream& ostr) : ostr(ostr) {}
  
  virtual FeatureDictionaryPtr getInputDictionary() const
    {return FeatureDictionaryPtr();}

  virtual bool trainBatch(ObjectContainerPtr examples, ProgressCallbackPtr progress = ProgressCallbackPtr())
  {
    ostr << "trainBatch() with " << examples->size() << " examples:" << std::endl;
    for (size_t i = 0; i < examples->size(); ++i)
      ostr << "  " << i << ": " << examples->get(i)->toString() << std::endl;
    return true;
  }

  virtual void trainStochasticBegin(FeatureDictionaryPtr inputDictionary)
    {ostr << "trainStochasticBegin()" << std::endl;}
    
  virtual void trainStochasticExample(ObjectPtr example)
    {ostr << "trainStochasticExample(" << example->toString() << ")" << std::endl;}
    
  virtual void trainStochasticEnd()
    {ostr << "trainStochasticEnd()" << std::endl;}
    
protected:
  std::ostream& ostr;
};

/*
** Regression
*/
double Regressor::evaluateMeanAbsoluteError(ObjectStreamPtr examples) const
{
  if (!examples->checkContentClassName("RegressionExample"))
    return 0.0;
  double res = 0;
  size_t count = 0;
  while (true)
  {
    RegressionExamplePtr example = examples->nextAndCast<RegressionExample>();
    if (example)
    {
      ++count;
      res += fabs(example->getOutput() - predict(example->getInput()));
    }
    else
      break;
  }
  return count ? res / (double)count : 0.0;
}

class VerboseRegressor : public VerboseLearningMachine<Regressor>
{
public:
  VerboseRegressor(std::ostream& ostr)
    : VerboseLearningMachine<Regressor>(ostr) {}
    
  virtual double predict(const FeatureGeneratorPtr input) const
  { 
    ostr << "predict(" << input->toString() << ")" << std::endl;
    return 0.0;
  }
};

RegressorPtr lbcpp::verboseRegressor(std::ostream& ostr)
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
  return RandomGenerator::getInstance().sampleWithNormalizedProbabilities(probs->getValues());
}

double Classifier::evaluateAccuracy(ObjectStreamPtr examples) const
{
  if (!examples->checkContentClassName("ClassificationExample"))
    return 0.0;

  size_t correct = 0;
  size_t count = 0;
  while (true)
  {
    ClassificationExamplePtr example = examples->nextAndCast<ClassificationExample>();
    if (!example)
      break;
    ++count;
    if (predict(example->getInput()) == example->getOutput())
      ++correct;
  }
  return correct / (double)count;
}

double Classifier::evaluateWeightedAccuracy(ObjectStreamPtr examples) const
{
  if (!examples->checkContentClassName("ClassificationExample"))
    return 0.0;
  double correctWeight = 0.0, totalWeight = 0.0;
  while (true)
  {
    ClassificationExamplePtr example = examples->nextAndCast<ClassificationExample>();
    if (!example)
      break;
    totalWeight += example->getWeight();
    if (predict(example->getInput()) == example->getOutput())
      correctWeight += example->getWeight();
  }
  jassert(totalWeight);
  return correctWeight / totalWeight;
}

void Classifier::save(std::ostream& ostr) const
{
  jassert(labels);
  write(ostr, labels);
}

bool Classifier::load(std::istream& istr)
{
  StringDictionaryPtr labels;
  if (!read(istr, labels))
    return false;
  setLabels(labels);
  return true;
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
  return RandomGenerator::getInstance().sampleBool(prob1) ? 1 : 0;
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
  jassert(res != (size_t)-1);
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
  return RandomGenerator::getInstance().sampleWithNormalizedProbabilities(probs->getValues());  
}

/*
** Ranker
*/
size_t Ranker::predict(const FeatureGeneratorPtr compositeInput) const
{
  jassert(compositeInput && compositeInput->getNumSubGenerators());
  double bestScore = -DBL_MAX;
  size_t res = (size_t)-1;
  for (size_t i = 0; i < compositeInput->getNumSubGenerators(); ++i)
  {
    double score = predictScore(compositeInput->getSubGenerator(i));
    if (score > bestScore)
      bestScore = score, res = compositeInput->getSubGeneratorIndex(i);
  }
  jassert(res != (size_t)-1);
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

double Ranker::evaluateMeanTopRankCost(ObjectStreamPtr examples) const
{
  size_t count = 0;
  double costSum = 0.0;
  while (true)
  {
    RankingExamplePtr example = examples->nextAndCast<RankingExample>();
    if (!example)
      break;
    ++count;
    costSum += example->getCost(predict(example->getInput()));
  }
  return count ? costSum / count : 0.0;
}
