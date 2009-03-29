/*-----------------------------------------.---------------------------------.
| Filename: LearningMachine.h              | Learning machines               |
| Author  : Francis Maes                   |                                 |
| Started : 08/03/2009 22:26               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef CRALGO_LEARNING_MACHINE_H_
# define CRALGO_LEARNING_MACHINE_H_

# include "ContinuousFunction.h"

# include "GradientBasedLearner.h" // only for GradientBasedMachines

namespace cralgo
{

class LearningMachine : public Object {};
typedef ReferenceCountedObjectPtr<LearningMachine> LearningMachinePtr;

template<class ExampleType>
class LearningMachine_ : public LearningMachine
{
public:
  virtual bool trainBatch(const std::vector<ExampleType>& examples, ProgressCallback* progress = NULL) = 0;

  virtual void trainStochastic(const std::vector<ExampleType>& examples)
  {
    trainStochasticBegin();
    for (size_t i = 0; i < examples.size(); ++i)
      trainStochasticExample(examples[i]);
    trainStochasticEnd();
  }

  virtual void trainStochasticBegin() = 0;
  virtual void trainStochasticExample(const ExampleType& example) = 0;
  virtual void trainStochasticEnd() = 0;
};

class Classifier : public LearningMachine_<ClassificationExample>
{
public:
  Classifier() : labels(NULL) {}
  
  /*
  ** Abstract
  */
  virtual DenseVectorPtr predictScores(const FeatureGeneratorPtr input) const = 0;
  virtual size_t predict(const FeatureGeneratorPtr input) const;
  virtual double predictScore(const FeatureGeneratorPtr input, size_t output) const;
  virtual DenseVectorPtr predictProbabilities(const FeatureGeneratorPtr input) const;
  virtual size_t sample(const FeatureGeneratorPtr input) const;

  
  /*
  ** Labels
  */
  size_t getNumLabels() const
    {assert(labels); return labels->getNumElements();}
  
  StringDictionaryPtr getLabels() const
    {assert(labels); return labels;}
  
  void setLabels(StringDictionaryPtr labels)
    {this->labels = labels; outputsDictionary = new FeatureDictionary("Classifier output", labels, StringDictionaryPtr());}
    
  /*
  ** Evaluation
  */
  double evaluateAccuracy(const std::vector<ClassificationExample>& examples) const;
  double evaluateWeightedAccuracy(const std::vector<ClassificationExample>& examples) const;
  
protected:
  StringDictionaryPtr labels;
  FeatureDictionaryPtr outputsDictionary;
};

class BinaryClassifier : public Classifier
{
public:
  virtual double predictScoreOfPositiveClass(const FeatureGeneratorPtr input) const = 0;

  virtual double scoreToProbability(double score) const
    {return 1.0 / (1.0 + exp(-score));} // default: apply a sigmoid

  virtual size_t predict(const FeatureGeneratorPtr input) const;
  virtual double predictScore(const FeatureGeneratorPtr input, size_t output) const;
  virtual DenseVectorPtr predictScores(const FeatureGeneratorPtr input) const;
  virtual DenseVectorPtr predictProbabilities(const FeatureGeneratorPtr input) const;
  virtual size_t sample(const FeatureGeneratorPtr input) const;
};


class GeneralizedClassifier : public LearningMachine_<GeneralizedClassificationExample>
{
public:
  virtual double predictScore(const FeatureGeneratorPtr input) const = 0;

  virtual size_t predict(const GeneralizedClassificationExample& example) const;
  virtual DenseVectorPtr predictScores(const FeatureGeneratorPtr compositeInput) const;
  virtual DenseVectorPtr predictProbabilities(const FeatureGeneratorPtr compositeInput) const;
  virtual size_t sample(const FeatureGeneratorPtr compositeInput) const;
};

class Regressor : public LearningMachine_<RegressionExample>
{
public:
  static RegressorPtr createVerbose(std::ostream& ostr);

  virtual double predict(const FeatureGeneratorPtr input) const = 0;
  
  double evaluateMeanAbsoluteError(const std::vector<RegressionExample>& examples) const;
};

class Ranker : public LearningMachine_<RankingExample>
{
public:
  virtual double predictScore(const FeatureGeneratorPtr input) const = 0;
  
  virtual size_t predict(const FeatureGeneratorPtr compositeInput) const;
  virtual DenseVectorPtr predictScores(const FeatureGeneratorPtr compositeInput) const;
};


}; /* namespace cralgo */

#endif // !CRALGO_LEARNING_MACHINE_H_
