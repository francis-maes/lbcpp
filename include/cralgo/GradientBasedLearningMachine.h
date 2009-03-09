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

namespace cralgo
{

template<class BaseClass, class ExampleType>
class GradientBasedLearningMachine : public BaseClass
{
public:
  virtual DenseVectorPtr createInitialParameters() const = 0;
  virtual ScalarVectorFunctionPtr getRegularizer() const = 0;
  virtual ScalarVectorFunctionPtr getLoss(const ExampleType& example) const = 0;
  virtual ScalarVectorFunctionPtr getEmpiricalRisk(const std::vector<ExampleType>& examples) const = 0;
  virtual ScalarVectorFunctionPtr getRegularizedEmpiricalRisk(const std::vector<ExampleType>& examples) const = 0;

  virtual void trainStochasticBegin()
    {assert(learner); ensureParametersAreCreated(); learner->trainStochasticBegin(parameters, getRegularizer());}
    
  virtual void trainStochasticExample(const ExampleType& example)
    {assert(learner); learner->trainStochasticExample(parameters, getLoss(example), getRegularizer());}

  virtual void trainStochasticEnd()
    {assert(learner); learner->trainStochasticBegin(parameters, getRegularizer());}
  
  virtual void trainBatch(const std::vector<ExampleType>& examples)
    {assert(learner); ensureParametersAreCreated(); learner->trainBatch(parameters, getRegularizedEmpiricalRisk(examples), examples.size());}
  
  void setLearner(GradientBasedLearnerPtr learner)
    {this->learner = learner;}
    
protected:
  DenseVectorPtr parameters;
  GradientBasedLearnerPtr learner;
  
  void ensureParametersAreCreated()
  {
    if (!parameters)
      parameters = createInitialParameters();
  }
};

class GradientBasedBinaryClassifier;
typedef ReferenceCountedObjectPtr<GradientBasedBinaryClassifier> GradientBasedBinaryClassifierPtr;

class GradientBasedBinaryClassifier : public GradientBasedLearningMachine<BinaryClassifier, ClassificationExample>
{
public:
  static GradientBasedBinaryClassifierPtr createLogisticRegression(GradientBasedLearnerPtr learner, FeatureDictionary& labels);

  virtual ScalarArchitecturePtr getPredictionArchitecture() const = 0;

  virtual DenseVectorPtr createInitialParameters() const
    {return getPredictionArchitecture()->createInitialParameters();}

  virtual double predictScoreOfPositiveClass(const FeatureGeneratorPtr input) const
    {return getPredictionArchitecture()->compute(parameters, input);}
};

class GradientBasedClassifier;
typedef ReferenceCountedObjectPtr<GradientBasedClassifier> GradientBasedClassifierPtr;

class GradientBasedClassifier : public GradientBasedLearningMachine<Classifier, ClassificationExample>
{
public:
  static GradientBasedClassifierPtr createMaximumEntropy(GradientBasedLearnerPtr learner, FeatureDictionary& labels);

  virtual VectorArchitecturePtr getPredictionArchitecture() const = 0;

  virtual DenseVectorPtr createInitialParameters() const
    {return getPredictionArchitecture()->createInitialParameters();}
  virtual DenseVectorPtr predictScores(const FeatureGeneratorPtr input) const
    {return getPredictionArchitecture()->compute(parameters, input)->toDenseVector();}
  virtual double predictScore(const FeatureGeneratorPtr input, size_t output) const
    {return getPredictionArchitecture()->compute(parameters, input, output);}
};

class GradientBasedRegressor : public GradientBasedLearningMachine<Regressor, RegressionExample>
{
public:
  virtual ScalarArchitecturePtr getPredictionArchitecture() const = 0;

  virtual DenseVectorPtr createInitialParameters() const
    {return getPredictionArchitecture()->createInitialParameters();}
  virtual double predict(const FeatureGeneratorPtr input) const
    {return getPredictionArchitecture()->compute(parameters, input);}
};

}; /* namespace cralgo */

#endif // !CRALGO_GRADIENT_BASED_LEARNING_MACHINE_H_
