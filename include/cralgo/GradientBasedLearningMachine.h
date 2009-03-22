/*-----------------------------------------.---------------------------------.
| Filename: GradientBasedLearningMachine.h | Gradient based learning machines|
| Author  : Francis Maes                   |                                 |
| Started : 09/03/2009 20:44               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef CRALGO_GRADIENT_BASED_LEARNING_MACHINE_H_
# define CRALGO_GRADIENT_BASED_LEARNING_MACHINE_H_

# include "LearningMachine.h"
# include "GradientBasedLearner.h"
# include "RandomVariable.h"

namespace cralgo
{

template<class BaseClass, class ExampleType_>
class GradientBasedLearningMachine : public BaseClass
{
public:
  typedef ExampleType_ ExampleType;

  virtual DenseVectorPtr createInitialParameters() const = 0;
  virtual ScalarVectorFunctionPtr getRegularizer() const = 0;
  virtual ScalarVectorFunctionPtr getLoss(const ExampleType& example) const = 0;
  virtual ScalarVectorFunctionPtr getEmpiricalRisk(const std::vector<ExampleType>& examples) const = 0;
  virtual ScalarVectorFunctionPtr getRegularizedEmpiricalRisk(const std::vector<ExampleType>& examples) const = 0;

  /*
  ** LearningMachine
  */
  virtual void trainStochasticBegin()
  {
    assert(learner);
    learner->setParameters(parameters);
    learner->setRegularizer(getRegularizer());
    learner->trainStochasticBegin();
  }
    
  void trainStochasticExample(FeatureGeneratorPtr gradient, double weight)
  {
    assert(learner);
    learner->trainStochasticExample(gradient, weight);
  }

  virtual void trainStochasticExample(const ExampleType& example)
  {
    assert(learner);
    inputSize.push((double)(example.getInput()->l0norm()));
    learner->setMeanInputSize(inputSize.getMean());
    learner->trainStochasticExample(getLoss(example));
  }
  
  virtual void trainStochasticEnd()
    {assert(learner); learner->trainStochasticEnd();}
  
  virtual void trainBatch(const std::vector<ExampleType>& examples)
  {
    assert(learner && examples.size());
    // sample mean input size
    for (size_t i = 0; i < 20; ++i)
      inputSize.push((double)(examples[Random::getInstance().sampleSize(examples.size())].getInput()->l0norm()));

    // delegate to learner
    learner->setMeanInputSize(inputSize.getMean());
    learner->setParameters(parameters);
    learner->setRegularizer(getRegularizer());
    learner->trainBatch(getRegularizedEmpiricalRisk(examples), examples.size());
  }
  
  /*
  ** Parameters
  */
  DenseVectorPtr getParameters() const
    {return parameters;}
    
  void createParameters()
    {assert(!parameters); parameters = createInitialParameters();}
    
  void setParameters(DenseVectorPtr parameters)
    {this->parameters = parameters;}

  /*
  ** Learner
  */
  GradientBasedLearnerPtr getLearner() const
    {return learner;}
    
  void setLearner(GradientBasedLearnerPtr learner)
    {this->learner = learner;}

  void pushInputSize(double inputSize)
    {this->inputSize.push(inputSize);}


  /*
  ** Shortcuts for functions computation
  */
  double computeRegularizer() const
    {assert(parameters); return getRegularizer()->compute(parameters);}

  double computeEmpiricalRisk(const std::vector<ExampleType>& examples) const
    {assert(parameters); return getEmpiricalRisk(examples)->compute(parameters);}
  
  double computeRegularizedEmpiricalRisk(const std::vector<ExampleType>& examples) const
    {assert(parameters); return getRegularizedEmpiricalRisk(examples)->compute(parameters);}
  
protected:
  DenseVectorPtr parameters;
  GradientBasedLearnerPtr learner;
  ScalarRandomVariableMean inputSize;
};

/*
** Regression
*/
class GradientBasedRegressor : public GradientBasedLearningMachine<Regressor, RegressionExample>
{
public:
  static GradientBasedRegressorPtr createLeastSquaresLinear(GradientBasedLearnerPtr learner);

  virtual ScalarArchitecturePtr getPredictionArchitecture() const = 0;

  virtual DenseVectorPtr createInitialParameters() const
    {return getPredictionArchitecture()->createInitialParameters();}
    
  virtual double predict(const FeatureGeneratorPtr input) const
    {return getPredictionArchitecture()->compute(parameters, input);}
};

/*
** Binary Classification
*/
class GradientBasedBinaryClassifier : public GradientBasedLearningMachine<BinaryClassifier, ClassificationExample>
{
public:
  static GradientBasedBinaryClassifierPtr createLinearSVM(GradientBasedLearnerPtr learner, StringDictionaryPtr labels);
  static GradientBasedBinaryClassifierPtr createLogisticRegression(GradientBasedLearnerPtr learner, StringDictionaryPtr labels);

  virtual ScalarArchitecturePtr getPredictionArchitecture() const = 0;

  virtual DenseVectorPtr createInitialParameters() const
    {return getPredictionArchitecture()->createInitialParameters();}

  virtual double predictScoreOfPositiveClass(const FeatureGeneratorPtr input) const
    {return getPredictionArchitecture()->compute(parameters, input);}
};

/*
** Classification
*/
class GradientBasedClassifier : public GradientBasedLearningMachine<Classifier, ClassificationExample>
{
public:
  static GradientBasedClassifierPtr createMaximumEntropy(GradientBasedLearnerPtr learner, StringDictionaryPtr labels);

  virtual VectorArchitecturePtr getPredictionArchitecture() const = 0;

  virtual DenseVectorPtr createInitialParameters() const
    {return getPredictionArchitecture()->createInitialParameters();}
  virtual DenseVectorPtr predictScores(const FeatureGeneratorPtr input) const
    {return getPredictionArchitecture()->compute(parameters, input)->toDenseVector();}
  virtual double predictScore(const FeatureGeneratorPtr input, size_t output) const
    {return getPredictionArchitecture()->compute(parameters, input, output);}
};

/*
** Generalized Classification
*/
class GradientBasedGeneralizedClassifier
  : public GradientBasedLearningMachine<GeneralizedClassifier, GeneralizedClassificationExample>
{
public:
  static GradientBasedGeneralizedClassifierPtr createLinear(GradientBasedLearnerPtr learner);

  virtual ScalarArchitecturePtr getPredictionArchitecture() const = 0;
  
  virtual DenseVectorPtr createInitialParameters() const
    {return getPredictionArchitecture()->createInitialParameters();}
       
  virtual double predictScore(const FeatureGeneratorPtr input) const
    {return getPredictionArchitecture()->compute(parameters, input);}
};


/*
** Ranker
*/
class GradientBasedRanker
  : public GradientBasedLearningMachine<Ranker, RankingExample>
{
public:
  static GradientBasedRankerPtr createLargeMarginAllPairsLinear(GradientBasedLearnerPtr learner);
  static GradientBasedRankerPtr createLargeMarginMostViolatedPairLinear(GradientBasedLearnerPtr learner);
  static GradientBasedRankerPtr createLargeMarginBestAgainstAllLinear(GradientBasedLearnerPtr learner);

  virtual ScalarArchitecturePtr getPredictionArchitecture() const = 0;
  
  virtual DenseVectorPtr createInitialParameters() const
    {return getPredictionArchitecture()->createInitialParameters();}
       
  virtual double predictScore(const FeatureGeneratorPtr input) const
    {return getPredictionArchitecture()->compute(parameters, input);}
};

}; /* namespace cralgo */

#endif // !CRALGO_GRADIENT_BASED_LEARNING_MACHINE_H_
