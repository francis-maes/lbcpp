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

class GradientBasedLearningMachine
{
public:
  GradientBasedLearningMachine() : initializeParametersRandomly(false)
    {}
  virtual ~GradientBasedLearningMachine() {}
    
  /*
  ** Abstract
  */
  virtual FeatureDictionaryPtr getInputDictionaryFromExample(ObjectPtr example) = 0;
  virtual DenseVectorPtr createInitialParameters(FeatureDictionaryPtr inputDictionary, bool initializeRandomly) const = 0;
  virtual ScalarVectorFunctionPtr getLoss(ObjectPtr example) const = 0;
  virtual ScalarVectorFunctionPtr getEmpiricalRisk(ObjectContainerPtr examples) const = 0;
  virtual ScalarVectorFunctionPtr getRegularizedEmpiricalRisk(ObjectContainerPtr examples) const = 0;

  /*
  ** Input Dictionary
  */
  FeatureDictionaryPtr getInputDictionary() const
    {return inputDictionary;}
    
  bool ensureInputDictionary(FeatureDictionaryPtr dictionary)
  {
    if (inputDictionary)
      return inputDictionary->checkEquals(dictionary);
    inputDictionary = dictionary;
    return true;
  }

  /*
  ** Parameters
  */
  DenseVectorPtr getParameters() const
    {return parameters;}
    
  void createParameters(FeatureDictionaryPtr inputDictionary, bool initializeRandomly)
  {
    ensureInputDictionary(inputDictionary);
    assert(!parameters);
    parameters = createInitialParameters(inputDictionary, initializeRandomly);
  }
    
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
    
protected:
  FeatureDictionaryPtr inputDictionary;
  DenseVectorPtr parameters;
  ScalarVectorFunctionPtr regularizer;
  GradientBasedLearnerPtr learner;
  bool initializeParametersRandomly;
  
  /*
  ** Serialization
  */
  void saveImpl(std::ostream& ostr) const;
  bool loadImpl(std::istream& istr);  

  /*
  ** Training
  */
  void trainStochasticBeginImpl(FeatureDictionaryPtr inputDictionary);
  void trainStochasticExampleImpl(FeatureGeneratorPtr gradient, double weight);
  void trainStochasticExampleImpl(ObjectPtr example);
  void trainStochasticEndImpl();
  bool trainBatchImpl(ObjectContainerPtr examples, ProgressCallbackPtr progress);
};

template<class BaseClass, class ExampleType_>
class GradientBasedLearningMachine_ : public BaseClass, public GradientBasedLearningMachine
{
public:
  typedef ExampleType_ ExampleType;
  typedef ReferenceCountedObjectPtr<ExampleType> ExampleTypePtr;

  virtual FeatureDictionaryPtr getInputDictionaryFromExample(ObjectPtr example)
    {return example.staticCast<ExampleType>()->getInput()->getDictionary();}

  virtual FeatureDictionaryPtr getInputDictionary() const
    {return GradientBasedLearningMachine::getInputDictionary();}
  
  /*
  ** LearningMachine
  */
  virtual void trainStochasticBegin(FeatureDictionaryPtr inputDictionary)
    {trainStochasticBeginImpl(inputDictionary);}

  void trainStochasticExample(FeatureGeneratorPtr gradient, double weight)
    {trainStochasticExampleImpl(gradient, weight);}

  virtual void trainStochasticExample(ObjectPtr example)
    {trainStochasticExampleImpl(example);}
  
  virtual void trainStochasticEnd()
    {trainStochasticEndImpl();}
  
  virtual bool trainBatch(ObjectContainerPtr examples, ProgressCallbackPtr progress = ProgressCallbackPtr())
    {return trainBatchImpl(examples, progress);}
  
  /*
  ** Serialization
  */
  virtual void save(std::ostream& ostr) const
    {BaseClass::save(ostr); saveImpl(ostr);}
  
  virtual bool load(std::istream& istr)
    {return BaseClass::load(istr) && loadImpl(istr);}
};

/*
** Regression
*/
class GradientBasedRegressor : public GradientBasedLearningMachine_<Regressor, RegressionExample>
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
class GradientBasedBinaryClassifier : public GradientBasedLearningMachine_<BinaryClassifier, ClassificationExample>
{
public:
  virtual ScalarArchitecturePtr getPredictionArchitecture() const = 0;

  virtual DenseVectorPtr createInitialParameters(FeatureDictionaryPtr inputDictionary, bool initializeRandomly) const
    {return getPredictionArchitecture()->createInitialParameters(inputDictionary, initializeRandomly);}

  virtual double predictScoreOfPositiveClass(const FeatureGeneratorPtr input) const
    {return getPredictionArchitecture()->compute(parameters, input);}
};

extern GradientBasedBinaryClassifierPtr linearSVMBinaryClassifier(GradientBasedLearnerPtr learner, StringDictionaryPtr labels);
extern GradientBasedBinaryClassifierPtr logisticRegressionBinaryClassifier(GradientBasedLearnerPtr learner, StringDictionaryPtr labels, double l2regularizer = 0.0);

/*
** Classification
*/
class GradientBasedClassifier : public GradientBasedLearningMachine_<Classifier, ClassificationExample>
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
  : public GradientBasedLearningMachine_<GeneralizedClassifier, GeneralizedClassificationExample>
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
  : public GradientBasedLearningMachine_<Ranker, RankingExample>
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
