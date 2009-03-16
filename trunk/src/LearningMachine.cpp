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
  typedef typename BaseClass::ExampleType ExampleType;
  
  // abstract: static functions for architecture(), loss() and regularizer()
  
  virtual ScalarVectorFunctionPtr getRegularizer() const
    {return impl::staticToDynamic(_this().regularizer());}
  
  virtual ScalarVectorFunctionPtr getLoss(const ExampleType& example) const
    {return impl::staticToDynamic(impl::exampleRisk(_this().architecture(), _this().loss(), example));}
    
  virtual ScalarVectorFunctionPtr getEmpiricalRisk(const std::vector<ExampleType>& examples) const
    {return impl::staticToDynamic(impl::empiricalRisk(_this().architecture(), _this().loss(), examples));}
    
  virtual ScalarVectorFunctionPtr getRegularizedEmpiricalRisk(const std::vector<ExampleType>& examples) const
    {return impl::staticToDynamic(impl::add(impl::empiricalRisk(_this().architecture(), _this().loss(), examples), _this().regularizer()));}

protected:
  const ExactType& _this() const  {return *(const ExactType* )this;}
};


template<class ExactType>
class StaticToDynamicGradientBasedBinaryClassifier
  : public StaticToDynamicGradientBasedLearningMachine<ExactType, GradientBasedBinaryClassifier>
{
public:
  typedef StaticToDynamicGradientBasedLearningMachine<ExactType, GradientBasedBinaryClassifier> BaseClass;
  
  virtual ScalarArchitecturePtr getPredictionArchitecture() const
    {return impl::staticToDynamic(BaseClass::_this().architecture());}
};


template<class ExactType>
class StaticToDynamicGradientBasedGeneralizedClassifier
  : public StaticToDynamicGradientBasedLearningMachine<ExactType, GradientBasedGeneralizedClassifier>
{
public:
  typedef StaticToDynamicGradientBasedLearningMachine<ExactType, GradientBasedGeneralizedClassifier> BaseClass;
  
  virtual ScalarArchitecturePtr getPredictionArchitecture() const
    {return impl::staticToDynamic(BaseClass::_this().baseArchitecture());}
};



template<class BaseClass, class ExampleType>
class VerboseLearningMachine : public BaseClass
{
public:
  VerboseLearningMachine(std::ostream& ostr) : ostr(ostr) {}
  
  virtual void trainBatch(const std::vector<ExampleType>& examples)
  {
    ostr << "trainBatch() with " << examples.size() << " examples:" << std::endl;
    for (size_t i = 0; i < examples.size(); ++i)
      ostr << "  " << i << ": " << examples[i] << std::endl;
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

class LeastSquaresLinearRegressor
  : public StaticToDynamicGradientBasedLearningMachine<LeastSquaresLinearRegressor, GradientBasedRegressor>
{
public:
  virtual ScalarArchitecturePtr getPredictionArchitecture() const
    {return impl::staticToDynamic(architecture());}

  inline impl::LinearArchitecture architecture() const
    {return impl::linearArchitecture();}

  inline impl::SquareLoss<RegressionExample> loss() const
    {return impl::squareLoss<RegressionExample>();}
    
  inline impl::ScalarVectorFunctionScalarConstantPair<impl::SumOfSquaresScalarVectorFunction, void>::Multiplication regularizer() const
    {return impl::multiply(impl::sumOfSquares(), impl::constant(0.001));}
};

GradientBasedRegressorPtr GradientBasedRegressor::createLeastSquaresLinear(GradientBasedLearnerPtr learner)
{
  GradientBasedRegressorPtr res = new LeastSquaresLinearRegressor();
  res->setLearner(learner);
  res->createParameters();
  return res;
}

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


class MaximumEntropyClassifier
  : public StaticToDynamicGradientBasedLearningMachine<MaximumEntropyClassifier, GradientBasedClassifier>
{
public:
  virtual VectorArchitecturePtr getPredictionArchitecture() const
    {return impl::staticToDynamic(architecture());}

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
  res->createParameters();
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
  return Random::getInstance().sampleBool(prob1) ? 1 : 0;
}

class LogisticRegressionClassifier : public StaticToDynamicGradientBasedBinaryClassifier<LogisticRegressionClassifier>
{
public:
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
  res->createParameters();
  return res;
}

class LinearSupportVectorMachine : public StaticToDynamicGradientBasedBinaryClassifier<LinearSupportVectorMachine>
{
public:
  virtual ScalarArchitecturePtr getPredictionArchitecture() const
    {return impl::staticToDynamic(architecture());}

  inline impl::LinearArchitecture architecture() const
    {return impl::linearArchitecture();}

  inline impl::HingeLoss<ClassificationExample> loss() const
    {return impl::hingeLoss<ClassificationExample>();}
    
  inline impl::ScalarVectorFunctionScalarConstantPair<impl::SumOfSquaresScalarVectorFunction, void>::Multiplication regularizer() const
    {return impl::multiply(impl::sumOfSquares(), impl::constant(0.001));}
};

GradientBasedBinaryClassifierPtr GradientBasedBinaryClassifier::createLinearSVM(GradientBasedLearnerPtr learner, FeatureDictionary& labels)
{
  GradientBasedBinaryClassifierPtr res = new LinearSupportVectorMachine();
  res->setLearner(learner);
  res->setLabels(labels);
  res->createParameters();
  return res;
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

DenseVectorPtr GeneralizedClassifier::predictScores(const std::vector<FeatureGeneratorPtr>& inputs) const
{
  DenseVectorPtr res = new DenseVector(inputs.size());
  for (size_t i = 0; i < inputs.size(); ++i)
    res->set(i, predictScore(inputs[i]));
  return res;
}

DenseVectorPtr GeneralizedClassifier::predictProbabilities(const std::vector<FeatureGeneratorPtr>& inputs) const
{
  // default: Gibbs distribution, P[y|x] = exp(score(y)) / (sum_i exp(score(y_i)))
  DenseVectorPtr scores = predictScores(inputs);
  double logZ = scores->computeLogSumOfExponentials();
  DenseVectorPtr res = scores->hasDictionary() ? new DenseVector(scores->getDictionary(), scores->getNumValues()) : new DenseVector(scores->getNumValues());
  for (size_t i = 0; i < scores->getNumValues(); ++i)
    res->set(i, exp(scores->get(i) - logZ));
  return res;
}

size_t GeneralizedClassifier::sample(const std::vector<FeatureGeneratorPtr>& inputs) const
{
  DenseVectorPtr probs = predictProbabilities(inputs);
  return Random::getInstance().sampleWithNormalizedProbabilities(probs->getValues());  
}

class LinearGeneralizedClassifier
  : public StaticToDynamicGradientBasedGeneralizedClassifier<LinearGeneralizedClassifier>
{
public:
  inline impl::LinearArchitecture baseArchitecture() const
    {return impl::linearArchitecture();}
  
  inline impl::ScalarToVectorArchitecture<impl::LinearArchitecture> architecture() const
    {return impl::parallelArchitecture(baseArchitecture());}

  inline impl::MultiClassLogBinomialLoss<GeneralizedClassificationExample> loss() const
    {return impl::multiClassLogBinomialLoss<GeneralizedClassificationExample>();}
    
  inline impl::ScalarVectorFunctionScalarConstantPair<impl::SumOfSquaresScalarVectorFunction, void>::Multiplication regularizer() const
    {return impl::multiply(impl::sumOfSquares(), impl::constant(0.0));}
};

GradientBasedGeneralizedClassifierPtr GradientBasedGeneralizedClassifier::createLinear(GradientBasedLearnerPtr learner)
{
  GradientBasedGeneralizedClassifierPtr res = new LinearGeneralizedClassifier();
  res->setLearner(learner);
  res->createParameters();
  return res;
}
