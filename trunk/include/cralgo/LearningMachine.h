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
  virtual void trainBatch(const std::vector<ExampleType>& examples) = 0;

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
  
  size_t getNumLabels() const
    {assert(labels); return labels->getFeatures().count();}
  
  FeatureDictionary& getLabels() const
    {assert(labels); return *labels;}
  
  void setLabels(FeatureDictionary& labels)
    {this->labels = &labels;}
  
  virtual DenseVectorPtr predictScores(const FeatureGeneratorPtr input) const = 0;

  virtual size_t predict(const FeatureGeneratorPtr input) const;
  virtual double predictScore(const FeatureGeneratorPtr input, size_t output) const;
  virtual DenseVectorPtr predictProbabilities(const FeatureGeneratorPtr input) const;
  virtual size_t sample(const FeatureGeneratorPtr input) const;
  
protected:
  FeatureDictionary* labels;
};

typedef ReferenceCountedObjectPtr<Classifier> ClassifierPtr;

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

typedef ReferenceCountedObjectPtr<BinaryClassifier> BinaryClassifierPtr;

class GeneralizedClassifier : public LearningMachine_<GeneralizedClassificationExample>
{
public:
  virtual size_t predict(const GeneralizedClassificationExample& example) = 0;
  virtual double predictScore(const FeatureGeneratorPtr input) const = 0;
  virtual DenseVectorPtr predictProbabilities(const std::vector<FeatureGeneratorPtr>& inputs) = 0;
  virtual size_t sample(const std::vector<FeatureGeneratorPtr>& inputs) const = 0;
};

typedef ReferenceCountedObjectPtr<GeneralizedClassifier> GeneralizedClassifierPtr;

class Regressor : public LearningMachine_<RegressionExample>
{
public:
  virtual double predict(const FeatureGeneratorPtr input) const = 0;
};

typedef ReferenceCountedObjectPtr<Regressor> RegressorPtr;

class Ranker : public LearningMachine_<RankingExample>
{
public:
  virtual double predict(const FeatureGeneratorPtr input) const = 0;
};

typedef ReferenceCountedObjectPtr<Ranker> RankerPtr;

}; /* namespace cralgo */

#endif // !CRALGO_LEARNING_MACHINE_H_
