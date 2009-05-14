/*-----------------------------------------.---------------------------------.
| Filename: GradientBasedLearningMachine.h | Gradient based learning machines|
| Author  : Francis Maes                   |                                 |
| Started : 09/03/2009 20:44               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_GRADIENT_BASED_LEARNING_MACHINE_H_
# define LBCPP_GRADIENT_BASED_LEARNING_MACHINE_H_

# include "LearningMachine.h"
# include "GradientBasedLearner.h"
# include "RandomVariable.h"

namespace lbcpp
{

template<class BaseClass, class ExampleType_>
class GradientBasedLearningMachine : public BaseClass
{
public:
  GradientBasedLearningMachine() : initializeParametersRandomly(false)
    {}
    
  typedef ExampleType_ ExampleType;

  virtual DenseVectorPtr createInitialParameters(FeatureDictionaryPtr inputDictionary, bool initializeRandomly) const = 0;
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
    if (!parameters)
      parameters = createInitialParameters(gradient->getDictionary(), initializeParametersRandomly);
    learner->setParameters(parameters);
    learner->trainStochasticExample(gradient, weight);
  }

  virtual void trainStochasticExample(const ExampleType& example)
  {
    assert(learner);
    if (!parameters)
      learner->setParameters(parameters = createInitialParameters(example.getInput()->getDictionary(), initializeParametersRandomly));
    inputSize.push((double)(example.getInput()->l0norm()));
    learner->setMeanInputSize(inputSize.getMean());
    learner->trainStochasticExample(getLoss(example));
  }
  
  virtual void trainStochasticEnd()
    {assert(learner); learner->trainStochasticEnd();}
  
  virtual bool trainBatch(const std::vector<ExampleType>& examples, ProgressCallback* progress = NULL)
  {
    assert(learner && examples.size());

    if (!parameters)
      parameters = createInitialParameters(examples[0].getInput()->getDictionary(), initializeParametersRandomly);

    // sample mean input size
    for (size_t i = 0; i < 20; ++i)
      inputSize.push((double)(examples[Random::getInstance().sampleSize(examples.size())].getInput()->l0norm()));

    // delegate to learner
    learner->setMeanInputSize(inputSize.getMean());
    learner->setParameters(parameters);
    learner->setRegularizer(getRegularizer());
    if (!learner->trainBatch(getRegularizedEmpiricalRisk(examples), examples.size(), progress))
      return false;
    parameters = learner->getParameters();
    return true;
  }
  
  /*
  ** Parameters
  */
  DenseVectorPtr getParameters() const
    {return parameters;}
    
  void createParameters(FeatureDictionaryPtr inputDictionary, bool initializeRandomly)
    {assert(!parameters); parameters = createInitialParameters(inputDictionary, initializeRandomly);}
    
  void setParameters(DenseVectorPtr parameters)
    {this->parameters = parameters;}
    
  void setInitializeParametersRandomly()
  {
    assert(!parameters); // This function should be called before parameters creation.
    initializeParametersRandomly = true;
  }
      
  /*
  ** Regularizer
  */
  ScalarVectorFunctionPtr getRegularizer() const
    {return regularizer;}

  void setRegularizer(ScalarVectorFunctionPtr regularizer)
    {this->regularizer = regularizer;}

  void setL2Regularizer(double weight)
    {regularizer = ScalarVectorFunction::createSumOfSquares(weight);}
  
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
    {assert(parameters); return regularizer ? regularizer->compute(parameters) : 0.0;}

  double computeEmpiricalRisk(const std::vector<ExampleType>& examples) const
    {assert(parameters); return getEmpiricalRisk(examples)->compute(parameters);}
  
  double computeRegularizedEmpiricalRisk(const std::vector<ExampleType>& examples) const
    {assert(parameters); return getRegularizedEmpiricalRisk(examples)->compute(parameters);}
  
protected:
  DenseVectorPtr parameters;
  ScalarVectorFunctionPtr regularizer;
  GradientBasedLearnerPtr learner;
  ScalarRandomVariableMean inputSize;
  bool initializeParametersRandomly;
};

/*
** Regression
*/
class GradientBasedRegressor : public GradientBasedLearningMachine<Regressor, RegressionExample>
{
public:
  static GradientBasedRegressorPtr createLeastSquaresLinear(GradientBasedLearnerPtr learner);

  virtual ScalarArchitecturePtr getPredictionArchitecture() const = 0;

  virtual DenseVectorPtr createInitialParameters(FeatureDictionaryPtr inputDictionary, bool initializeRandomly) const
    {return getPredictionArchitecture()->createInitialParameters(inputDictionary, initializeRandomly);}
    
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

  virtual DenseVectorPtr createInitialParameters(FeatureDictionaryPtr inputDictionary, bool initializeRandomly) const
    {return getPredictionArchitecture()->createInitialParameters(inputDictionary, initializeRandomly);}

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

  virtual DenseVectorPtr createInitialParameters(FeatureDictionaryPtr inputDictionary, bool initializeRandomly) const
    {return getPredictionArchitecture()->createInitialParameters(inputDictionary, initializeRandomly);}

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
  
  virtual DenseVectorPtr createInitialParameters(FeatureDictionaryPtr inputDictionary, bool initializeRandomly) const
    {return getPredictionArchitecture()->createInitialParameters(inputDictionary, initializeRandomly);}
       
  virtual double predictScore(const FeatureGeneratorPtr input) const
    {return parameters ? getPredictionArchitecture()->compute(parameters, input) : 0.0;}
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
  
  virtual DenseVectorPtr createInitialParameters(FeatureDictionaryPtr inputDictionary, bool initializeRandomly) const
    {return getPredictionArchitecture()->createInitialParameters(inputDictionary->getSubDictionary(0), initializeRandomly);}
       
  virtual double predictScore(const FeatureGeneratorPtr input) const
    {return getPredictionArchitecture()->compute(parameters, input);}
};

}; /* namespace lbcpp */

#endif // !LBCPP_GRADIENT_BASED_LEARNING_MACHINE_H_
