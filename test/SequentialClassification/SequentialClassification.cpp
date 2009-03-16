/*-----------------------------------------.---------------------------------.
| Filename: SequentialClassification.cpp   | Sequential Classification Main  |
| Author  : Francis Maes                   |                                 |
| Started : 15/03/2009 19:57               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include "GeneratedCode/SequentialClassification.h"
#include <cralgo/cralgo.h>
using namespace cralgo;

class SyntheticLinearMultiClassGenerator
{
public:
  SyntheticLinearMultiClassGenerator(size_t numFeatures, size_t numClasses)
    : parameters(numClasses), numFeatures(numFeatures)
  {
    for (size_t i = 0; i < parameters.size(); ++i)
      parameters[i] = sampleVectorGaussian(numFeatures);
  }
  
  ClassificationExample sample() const
  {
    DenseVectorPtr x = sampleVectorGaussian(numFeatures);
    double bestScore = -DBL_MAX;
    size_t y = 0;
    for (size_t i = 0; i < parameters.size(); ++i)
    {
      double score = parameters[i]->dotProduct(x);
      if (score > bestScore)
        bestScore = score, y = i;
    }
 //   std::cout << "BestScore: " << bestScore << " y = " << y << std::endl;
    return ClassificationExample(x, y);
  }
  
private:
  std::vector<DenseVectorPtr> parameters;
  size_t numFeatures;
  
  static DenseVectorPtr sampleVectorGaussian(size_t numFeatures)
  {
    DenseVectorPtr res = new DenseVector(numFeatures);
    for (size_t i = 0; i < numFeatures; ++i)
      res->set(i, Random::getInstance().sampleDoubleFromGaussian());
    return res;
  }
};

class SyntheticDataGenerator
{
public:
  SyntheticDataGenerator(size_t numFeaturesInBranch, size_t numFolds, size_t numFeaturesPerFold, size_t numClasses)
    : branchGenerator(numFeaturesInBranch, numFolds), foldGenerators(numFolds, SyntheticLinearMultiClassGenerator(numFeaturesPerFold, numClasses))
    {}

  ClassificationExample sample() const
  {
    DenseVectorPtr x = new DenseVector(0, 1 + foldGenerators.size());
    ClassificationExample branch = branchGenerator.sample();
    x->setSubVector(0, branch.getInput());
  //  std::cout << "BRANCH output: " << branch.getOutput() << std::endl;
    size_t y = 0;
    for (size_t i = 0; i < foldGenerators.size(); ++i)
    {
      ClassificationExample fold = foldGenerators[i].sample();
      x->setSubVector(i + 1, fold.getInput());
      if (i == branch.getOutput())
        y = fold.getOutput();
    }
    return ClassificationExample(x, y);
  }

private:
  SyntheticLinearMultiClassGenerator branchGenerator;
  std::vector<SyntheticLinearMultiClassGenerator> foldGenerators;
};

void testClassifier(const std::vector<ClassificationExample>& train, const std::vector<ClassificationExample>& test, size_t numClasses)
{
  FeatureDictionary classes;
  for (size_t i = 0; i < numClasses; ++i)
    classes.getFeatures().add("class " + cralgo::toString(i));
  
  IterationFunctionPtr learningRate = IterationFunction::createConstant(0.01);
  GradientBasedClassifierPtr classifier = GradientBasedClassifier::createMaximumEntropy(
    GradientBasedLearner::createGradientDescent(learningRate), classes);
  
  for (size_t i = 0; i < 15; ++i)
  {
    classifier->trainStochastic(train);
    double acc = classifier->evaluateAccuracy(test);
    std::cout << "ITERATION " << i+1 << " accuracy = " << acc << std::endl;
  }
}

void trainAndTest(const std::vector<ClassificationExample>& train, const std::vector<ClassificationExample>& test, size_t numClasses, PolicyPtr learnedPolicy, PolicyPtr learnerPolicy)
{
  for (size_t iteration = 0; iteration < 500; ++iteration)
  {
    PolicyPtr policy = learnerPolicy->addComputeStatistics();//->verbose(std::cout, 4);
    for (size_t i = 0; i < train.size(); ++i)
    {
      ClassificationExample ex = train[Random::getInstance().sampleSize(train.size())];
  //    std::cout << "SAMPLE " << i << ": " << std::endl << ex << std::endl;
      SparseVectorPtr input = ex.getInput()->toSparseVector();
      
      std::vector<FeatureGeneratorPtr> x;
//      x.push_back(input);
      for (size_t i = 0; i < input->getNumSubVectors(); ++i)
        x.push_back(input->getSubVector(i));
      size_t y = ex.getOutput();
      sequentialClassification(policy, x, 0.1, numClasses, &y);
    }
    
    size_t correct = 0;
    for (size_t i = 0; i < test.size(); ++i)
    {
      ClassificationExample ex = test[i];
  //    std::cout << "SAMPLE " << i << ": " << std::endl << ex << std::endl;
      SparseVectorPtr input = ex.getInput()->toSparseVector();
      
      std::vector<FeatureGeneratorPtr> x;
//      x.push_back(input);
      for (size_t i = 0; i < input->getNumSubVectors(); ++i)
        x.push_back(input->getSubVector(i));
      size_t y = ex.getOutput();
      size_t ypredicted = sequentialClassification(learnedPolicy, x, 0.1, numClasses, &y);
      if (ypredicted == y)
        ++correct;
    }
    std::cout << "EVALUATION: " << correct << " / 1000" << std::endl << std::endl;
    std::cout << "END OF ITERATION " << iteration+1 << ": " << policy->toString() << std::endl;
  }
}

void testMonteCarloControl(const std::vector<ClassificationExample>& train, const std::vector<ClassificationExample>& test, size_t numClasses)
{
  IterationFunctionPtr learningRate = IterationFunction::createConstant(0.001);
  IterationFunctionPtr epsilon = IterationFunction::createConstant(0.01);
  
// Regressor::createVerbose(std::cout);
  RegressorPtr regressor = GradientBasedRegressor::createLeastSquaresLinear(
    GradientBasedLearner::createGradientDescent(learningRate));
  
  PolicyPtr learnedPolicy = Policy::createGreedy(ActionValueFunction::createPredictions(regressor));
  
  PolicyPtr learnerPolicy = Policy::createMonteCarloControl(learnedPolicy->epsilonGreedy(epsilon), regressor, 0.8);
  
  trainAndTest(train, test, numClasses, learnedPolicy, learnerPolicy);
}


void testOLPOMDP(const std::vector<ClassificationExample>& train, const std::vector<ClassificationExample>& test, size_t numClasses)
{
  IterationFunctionPtr learningRate = IterationFunction::createInvLinear(0.001, 10000);

  GeneralizedClassifierPtr classifier = GradientBasedGeneralizedClassifier::createLinear(
    GradientBasedLearner::createGradientDescent(learningRate));
  
  PolicyPtr learnedPolicy = Policy::createGreedy(ActionValueFunction::createScores(classifier));  
  PolicyPtr learnerPolicy = Policy::createGPOMDP(classifier, 0.8, 1.1);

  trainAndTest(train, test, numClasses, learnedPolicy, learnerPolicy);
}

int main(int argc, char* argv[])
{
  static const size_t numFeaturesToDecideFold = 4;
  static const size_t numFolds = 4;
  static const size_t numClasses = 2;
  static const size_t numFeatures = 2;
  SyntheticDataGenerator generator(numFeaturesToDecideFold, numFolds, numFeatures, numClasses);
 
 // SyntheticLinearMultiClassGenerator generator(numFeatures, numClasses);
  
  std::vector<ClassificationExample> train(1000), test(1000);
  
  for (size_t i = 0; i < train.size(); ++i)
    train[i] = generator.sample();
  for (size_t i = 0; i < test.size(); ++i)
    test[i] = generator.sample();
  testClassifier(train, test, numClasses);  
  
  testOLPOMDP(train, test, numClasses);
  
  return 0;
}
