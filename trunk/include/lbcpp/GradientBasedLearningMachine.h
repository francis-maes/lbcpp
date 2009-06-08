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
# include "ObjectContainer.h"

namespace lbcpp
{

template<class BaseClass, class ExampleType_>
class GradientBasedLearningMachine : public BaseClass
{
public:
  GradientBasedLearningMachine() : initializeParametersRandomly(false)
    {}
    
  typedef ExampleType_ ExampleType;
  typedef ReferenceCountedObjectPtr<ExampleType> ExampleTypePtr;

  virtual DenseVectorPtr createInitialParameters(FeatureDictionaryPtr inputDictionary, bool initializeRandomly) const = 0;
  virtual ScalarVectorFunctionPtr getLoss(ObjectPtr example) const = 0;
  virtual ScalarVectorFunctionPtr getEmpiricalRisk(ObjectContainerPtr examples) const = 0;
  virtual ScalarVectorFunctionPtr getRegularizedEmpiricalRisk(ObjectContainerPtr examples) const = 0;

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

  virtual void trainStochasticExample(ObjectPtr example)
  {
    assert(learner);
    if (!parameters)
      learner->setParameters(parameters = createInitialParameters(example.staticCast<ExampleType>()->getInput()->getDictionary(), initializeParametersRandomly));
    learner->trainStochasticExample(getLoss(example));
  }
  
  virtual void trainStochasticEnd()
    {assert(learner); learner->trainStochasticEnd();}
  
  virtual bool trainBatch(ObjectContainerPtr examples, ProgressCallback* progress = NULL)
  {
    assert(learner && examples->size());
    if (!parameters)
      parameters = createInitialParameters(examples->getCast<ExampleType>(0)->getInput()->getDictionary(), initializeParametersRandomly);

    // delegate to learner
    learner->setParameters(parameters);
    learner->setRegularizer(getRegularizer());
    if (!learner->trainBatch(getRegularizedEmpiricalRisk(examples), examples->size(), progress))
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
    {regularizer = sumOfSquaresFunction(weight);}
  
  /*
  ** Learner
  */
  GradientBasedLearnerPtr getLearner() const
    {return learner;}
    
  void setLearner(GradientBasedLearnerPtr learner)
    {this->learner = learner;}

  /*
  ** Shortcuts for functions computation
  */
  double computeRegularizer() const
    {assert(parameters); return regularizer ? regularizer->compute(parameters) : 0.0;}

  double computeEmpiricalRisk(ObjectContainerPtr examples) const
    {assert(parameters); return getEmpiricalRisk(examples)->compute(parameters);}
  
  double computeRegularizedEmpiricalRisk(ObjectContainerPtr examples) const
    {assert(parameters); return getRegularizedEmpiricalRisk(examples)->compute(parameters);}
  
  /*
  ** Serialization
  */
  virtual void save(std::ostream& ostr) const
  {
    write(ostr, parameters);
    write(ostr, regularizer);
    write(ostr, learner);
    write(ostr, initializeParametersRandomly);
  }
  
  virtual bool load(std::istream& istr)
    {return read(istr, parameters) && read(istr, regularizer) && read(istr, learner) && read(istr, initializeParametersRandomly);}
    
protected:
  DenseVectorPtr parameters;
  ScalarVectorFunctionPtr regularizer;
  GradientBasedLearnerPtr learner;
  bool initializeParametersRandomly;
};

/*
** Regression
*/
class GradientBasedRegressor : public GradientBasedLearningMachine<Regressor, RegressionExample>
{
public:
  virtual ScalarArchitecturePtr getPredictionArchitecture() const = 0;

  virtual DenseVectorPtr createInitialParameters(FeatureDictionaryPtr inputDictionary, bool initializeRandomly) const
    {return getPredictionArchitecture()->createInitialParameters(inputDictionary, initializeRandomly);}
    
  virtual double predict(const FeatureGeneratorPtr input) const
    {return getPredictionArchitecture()->compute(parameters, input);}
};

extern GradientBasedRegressorPtr leastSquaresLinearRegressor(GradientBasedLearnerPtr learner);


/*
** Binary Classification
*/
class GradientBasedBinaryClassifier : public GradientBasedLearningMachine<BinaryClassifier, ClassificationExample>
{
public:
  virtual ScalarArchitecturePtr getPredictionArchitecture() const = 0;

  virtual DenseVectorPtr createInitialParameters(FeatureDictionaryPtr inputDictionary, bool initializeRandomly) const
    {return getPredictionArchitecture()->createInitialParameters(inputDictionary, initializeRandomly);}

  virtual double predictScoreOfPositiveClass(const FeatureGeneratorPtr input) const
    {return getPredictionArchitecture()->compute(parameters, input);}
};

extern GradientBasedBinaryClassifierPtr linearSVMBinaryClassifier(GradientBasedLearnerPtr learner, StringDictionaryPtr labels);
extern GradientBasedBinaryClassifierPtr logisticRegressionBinaryClassifier(GradientBasedLearnerPtr learner, StringDictionaryPtr labels);

/*
** Classification
*/
class GradientBasedClassifier : public GradientBasedLearningMachine<Classifier, ClassificationExample>
{
public:
  virtual VectorArchitecturePtr getPredictionArchitecture() const = 0;

  virtual DenseVectorPtr createInitialParameters(FeatureDictionaryPtr inputDictionary, bool initializeRandomly) const
    {return getPredictionArchitecture()->createInitialParameters(inputDictionary, initializeRandomly);}

  virtual DenseVectorPtr predictScores(const FeatureGeneratorPtr input) const
    {return getPredictionArchitecture()->compute(parameters, input)->toDenseVector();}
  virtual double predictScore(const FeatureGeneratorPtr input, size_t output) const
    {return getPredictionArchitecture()->compute(parameters, input, output);}
};

extern GradientBasedClassifierPtr maximumEntropyClassifier(GradientBasedLearnerPtr learner, StringDictionaryPtr labels, double l2regularizer = 0.0);

/*
** Generalized Classification
*/
class GradientBasedGeneralizedClassifier
  : public GradientBasedLearningMachine<GeneralizedClassifier, GeneralizedClassificationExample>
{
public:
  virtual ScalarArchitecturePtr getPredictionArchitecture() const = 0;
  
  virtual DenseVectorPtr createInitialParameters(FeatureDictionaryPtr inputDictionary, bool initializeRandomly) const
    {return getPredictionArchitecture()->createInitialParameters(inputDictionary, initializeRandomly);}
       
  virtual double predictScore(const FeatureGeneratorPtr input) const
    {return parameters ? getPredictionArchitecture()->compute(parameters, input) : 0.0;}
};

extern GradientBasedGeneralizedClassifierPtr linearGeneralizedClassifier(GradientBasedLearnerPtr learner);

/*
** Ranker
*/
class GradientBasedRanker
  : public GradientBasedLearningMachine<Ranker, RankingExample>
{
public:
  virtual ScalarArchitecturePtr getPredictionArchitecture() const = 0;
  
  virtual DenseVectorPtr createInitialParameters(FeatureDictionaryPtr inputDictionary, bool initializeRandomly) const
    {return getPredictionArchitecture()->createInitialParameters(inputDictionary->getSubDictionary(0), initializeRandomly);}
       
  virtual double predictScore(const FeatureGeneratorPtr input) const
    {return getPredictionArchitecture()->compute(parameters, input);}
};

extern GradientBasedRankerPtr largeMarginAllPairsLinearRanker(GradientBasedLearnerPtr learner);
extern GradientBasedRankerPtr largeMarginMostViolatedPairLinearRanker(GradientBasedLearnerPtr learner);
extern GradientBasedRankerPtr largeMarginBestAgainstAllLinearRanker(GradientBasedLearnerPtr learner);

}; /* namespace lbcpp */

#endif // !LBCPP_GRADIENT_BASED_LEARNING_MACHINE_H_
