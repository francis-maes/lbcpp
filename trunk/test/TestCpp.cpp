#include <cralgo/cralgo.h>
#include <cralgo/impl/impl.h>
#include <fstream>
using namespace cralgo;

class LearningMachine : public Object
{
public:
  virtual void trainStochasticBegin() = 0;
  virtual void trainStochasticEnd() = 0;
};

typedef ReferenceCountedObjectPtr<LearningMachine> LearningMachinePtr;

class Classifier : public LearningMachine
{
public:
  virtual void trainStochasticExample(const ClassificationExample& example) = 0;
  virtual void trainStochastic(const std::vector<ClassificationExample>& examples)
  {
    trainStochasticBegin();
    for (size_t i = 0; i < examples.size(); ++i)
      trainStochasticExample(examples[i]);
    trainStochasticEnd();
  }
  
  virtual void trainBatch(const std::vector<ClassificationExample>& examples) = 0;

  virtual size_t predict(const FeatureGeneratorPtr input) const = 0;
  virtual double predictScore(const FeatureGeneratorPtr input, size_t output) const = 0;
  virtual DenseVectorPtr predictProbabilities(const FeatureGeneratorPtr input) const = 0;
  virtual size_t sample(const FeatureGeneratorPtr input) const = 0;
};

typedef ReferenceCountedObjectPtr<Classifier> ClassifierPtr;

class GeneralizedClassifier : public LearningMachine
{
public:
  virtual void trainStochasticExample(const GeneralizedClassificationExample& example) = 0;
  
  virtual void trainStochastic(const std::vector<GeneralizedClassificationExample>& examples)
  {
    trainStochasticBegin();
    for (size_t i = 0; i < examples.size(); ++i)
      trainStochasticExample(examples[i]);
    trainStochasticEnd();
  }

  virtual void trainBatch(const std::vector<GeneralizedClassificationExample>& examples) = 0;
  
  virtual size_t predict(const GeneralizedClassificationExample& example) = 0;
  virtual double predictScore(const FeatureGeneratorPtr input) const = 0;
  virtual DenseVectorPtr predictProbabilities(const std::vector<FeatureGeneratorPtr>& inputs) = 0;
  virtual size_t sample(const std::vector<FeatureGeneratorPtr>& inputs) const = 0;
};

typedef ReferenceCountedObjectPtr<GeneralizedClassifier> GeneralizedClassifierPtr;

class Regressor : public LearningMachine
{
public:
  virtual void trainStochasticExample(const RegressionExample& example) = 0;
  virtual void trainBatch(const std::vector<RegressionExample>& examples) = 0;

  virtual double predict(const FeatureGeneratorPtr input) const = 0;
};

typedef ReferenceCountedObjectPtr<Regressor> RegressorPtr;

class Ranker : public LearningMachine
{
public:
  virtual void trainStochasticExample(const Ranker& example) = 0;
  virtual void trainBatch(const std::vector<Ranker>& examples) = 0;

  virtual double predict(const FeatureGeneratorPtr input) const = 0;
};

typedef ReferenceCountedObjectPtr<Ranker> RankerPtr;

class GradientBasedLearner : public Object
{
public:
  virtual void trainStochasticBegin(ScalarVectorFunctionPtr regularizer) {}
  virtual void trainStochasticExample(const FeatureGeneratorPtr input, ScalarVectorFunctionPtr loss, ScalarVectorFunctionPtr regularizer) = 0;
  virtual void trainStochasticEnd(ScalarVectorFunctionPtr regularizer) {}
  virtual void trainBatch(ScalarVectorFunctionPtr objective) = 0;

protected:
  DenseVectorPtr parameters;
};
typedef ReferenceCountedObjectPtr<GradientBasedLearner> GradientBasedLearnerPtr;

class StochasticDescentLearner : public GradientBasedLearner
{
public:
  virtual void trainStochasticExample(const FeatureGeneratorPtr input, ScalarVectorFunctionPtr loss, ScalarVectorFunctionPtr regularizer)
  {
    double alpha = 0.1;
    parameters->addWeighted(loss->computeGradient(input), -alpha);
  }
  
  virtual void trainStochasticEnd(ScalarVectorFunctionPtr regularizer)
  {
    if (regularizer)
    {
      // apply regularizer
      double alpha = 0.1;
      parameters->addWeighted(regularizer->computeGradient(parameters), -alpha);
    }
  }

  virtual void trainBatch(ScalarVectorFunctionPtr objective)
    {assert(false);} // this is not a batch learner
};

template<class BaseClass, class ExampleType>
class GradientBasedLearningMachine : public BaseClass
{
public:
  virtual ScalarArchitecturePtr getPredictionArchitecture() const = 0;
  virtual ScalarVectorFunctionPtr getRegularizer() const = 0;
  virtual ScalarVectorFunctionPtr getLoss(const ExampleType& example) const = 0;
  virtual ScalarVectorFunctionPtr getEmpiricalRisk(const std::vector<ExampleType>& examples) const = 0;
  virtual ScalarVectorFunctionPtr getRegularizedEmpiricalRisk(const std::vector<ExampleType>& examples) const = 0;

  virtual void trainStochasticBegin()
    {learner->trainStochasticBegin(getRegularizer());}
    
  void trainStochasticExample(const ExampleType& example)
    {learner->trainStochasticExample(example.getInput(), getLoss(example), getRegularizer());}

  virtual void trainStochasticEnd()
    {learner->trainStochasticBegin(getRegularizer());}
  
  virtual void trainBatch(const std::vector<ExampleType>& examples)
    {learner->trainBatch(getRegularizedEmpiricalRisk(examples));}
    
protected:
  DenseVectorPtr parameters;
  GradientBasedLearnerPtr learner;
};

class GradientBasedRegressor : public GradientBasedLearningMachine<Regressor, RegressionExample>
{
public:
  virtual double predict(const FeatureGeneratorPtr input) const
    {return getPredictionArchitecture()->compute(parameters, input);}
};

class GradientBasedBinaryClassifier : public GradientBasedLearningMachine<Classifier, ClassificationExample>
{
public:
  virtual size_t predict(const FeatureGeneratorPtr input) const
    {return getPredictionArchitecture()->compute(parameters, input) > 0 ? 1 : 0;}
  
  virtual double predict(const FeatureGeneratorPtr input, size_t output) const
  {
    double score = getPredictionArchitecture()->compute(parameters, input);
    return output ? score : -score;
  }
};

int main(int argc, char* argv[])
{
  static const char* filename = "/Users/francis/Projets/Nieme/trunk/examples/data/binaryclassif/a1a.train";

  std::ifstream istr(filename);
  if (!istr.is_open())
  {
    std::cerr << "Could not open file " << filename << std::endl;
    return 1;
  }

  FeatureDictionary features("dictionary");
  StringDictionary labels;
  std::vector<ClassificationExample> examples;
  if (!parseClassificationExamples(istr, features, labels, examples))
    return 1;

  std::cout << examples.size() << " Examples." << std::endl << toString(features) << std::endl << "Labels: " << toString(labels) << std::endl;
/*  
  ClassifierPtr classifier = ClassifierPtr(); // FIXME
  classifier->trainBatch(examples);
  // ou
  for (int i = 0; i < 100; ++i)
    classifier->trainStochastic(examples);*/
  
  ScalarArchitecturePtr predictionArchitecture = impl::instantiate(impl::linearArchitecture());
  ScalarLossFunctionPtr lossFunction = impl::instantiate(impl::logBinomialLoss<ClassificationExample>());
  ScalarFunctionPtr lossFunctionWithExample = impl::instantiate(impl::logBinomialLoss(examples[0]));
  ScalarArchitecturePtr lossArchitecture = impl::instantiate(impl::compose(impl::linearArchitecture(), impl::logBinomialLoss(examples[0])));
  
  ScalarVectorFunctionPtr empiricalRisk = impl::instantiate(impl::empiricalRisk(
                            impl::linearArchitecture(),
                            impl::logBinomialLoss<ClassificationExample>(),
                            examples));
  
  ScalarVectorFunctionPtr regularizer = impl::instantiate(impl::sumOfSquares());
  ScalarVectorFunctionPtr weightedRegularizer = impl::instantiate(impl::multiply(impl::sumOfSquares(), impl::constant(0.1)));
  ScalarVectorFunctionPtr regularizedEmpiricalRisk = impl::instantiate(impl::add(
    impl::empiricalRisk(impl::linearArchitecture(), impl::logBinomialLoss<ClassificationExample>(), examples),
    impl::multiply(impl::sumOfSquares(), impl::constant(0.1))));

  DenseVectorPtr parameters(new DenseVector());
  for (int i = 0; i < 100; ++i)
  {
    static const double alpha = 1.0;
    std::cout << "Iteration " << i << " loss = " << empiricalRisk->compute(parameters) << std::endl;
    LazyVectorPtr gradient = empiricalRisk->computeGradient(parameters);
    parameters->addWeighted(gradient, -alpha);
  }
  
  size_t numErrors = 0;
  for (size_t i = 0; i < examples.size(); ++i)
  {
    double activation = predictionArchitecture->compute(parameters, examples[i].getInput());
    bool correct = examples[i].getOutput() > 0;
    bool predicted = activation > 0;
    if (correct != predicted)
      ++numErrors;
  }
  std::cout << numErrors << " Num Errors: " << numErrors << " / " << examples.size() << " => " << (100.0 * numErrors / (double)examples.size()) << "%." << std::endl;


  // ScalarVectorFunctionPtr regularizer = impl::instantiate(impl::multiply(impl::l2norm(), impl::constant(0.1)));
  // ScalarVectorFunctionPtr regularizedRisk = impl::instantiate(impl::add(
  //    impl::empiricalRisk(impl::logBinomialLoss(impl::linearArchitecture()), examples),
  //    impl::multiply(impl::l2norm(), impl::constant(0.1))))
  
  return 0;
}