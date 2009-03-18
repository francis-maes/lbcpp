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
 //   std::cout << "BestScore: " << bestScore << " y = " << y << std::endl;
    return ClassificationExample(x, getClass(x));
  }
  
  size_t getClass(FeatureGeneratorPtr x) const
  {
    double bestScore = -DBL_MAX;
    size_t y = 0;
    //std::cout << "X dictionary: " << x->getDictionary().getName() << std::endl;
    for (size_t i = 0; i < parameters.size(); ++i)
    {
      //std::cout << "params i dictionary: " << parameters[i]->getDictionary().getName() << std::endl;
      double score = parameters[i]->dotProduct(x);
      if (score > bestScore)
        bestScore = score, y = i;
    }
    return y;
  }
  
  static FeatureDictionaryPtr getDictionary()
  {
    static FeatureDictionaryPtr dictionary = new FeatureDictionary("SyntheticLinearMultiClassGenerator");
    return dictionary;
  }
  
private:
  std::vector<DenseVectorPtr> parameters;
  size_t numFeatures;
  
  static DenseVectorPtr sampleVectorGaussian(size_t numFeatures)
  {
    DenseVectorPtr res = new DenseVector(getDictionary(), numFeatures);
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
    DenseVectorPtr x = new DenseVector(getDictionary(), 0, 1 + foldGenerators.size());
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

  static FeatureDictionaryPtr getDictionary()
  {
    static FeatureDictionaryPtr dictionary = new FeatureDictionary("SyntheticDataGenerator");
    return dictionary;
  }

  SyntheticLinearMultiClassGenerator branchGenerator;
  std::vector<SyntheticLinearMultiClassGenerator> foldGenerators;
};

void testClassifier(const std::vector<ClassificationExample>& train, const std::vector<ClassificationExample>& test, size_t numClasses)
{
  FeatureDictionaryPtr classes = new FeatureDictionary("__classes__");
  for (size_t i = 0; i < numClasses; ++i)
    classes->getFeatures().add("class " + cralgo::toString(i));
  
  IterationFunctionPtr learningRate = IterationFunction::createConstant(0.01);
  GradientBasedClassifierPtr classifier = GradientBasedClassifier::createMaximumEntropy(
    GradientBasedLearner::createGradientDescent(learningRate), classes);
  
  for (size_t i = 0; i < 15; ++i)
  {
    classifier->trainStochastic(train);
    double acc = classifier->evaluateAccuracy(test);
    std::cout << "ITERATION " << i+1 << " accuracy = " << acc << std::endl;
  }
  DenseVectorPtr parameters = classifier->getParameters();
  std::cout << "Params: size = " << parameters->size() << " l2norm = " << parameters->l2norm() << " l1norm = " << parameters->l1norm() << std::endl;
  std::cout << "TRAIN SCORE = " << classifier->evaluateAccuracy(train) << std::endl;
  std::cout << "TEST SCORE = " << classifier->evaluateAccuracy(test) << std::endl;
}

void trainAndTest(const std::vector<ClassificationExample>& train, const std::vector<ClassificationExample>& test, size_t numClasses,
      PolicyPtr learnedPolicy, PolicyPtr learnerPolicy, DenseVectorPtr parameters = DenseVectorPtr(), PolicyPtr learnerPolicy2 = PolicyPtr())
{
  for (size_t iteration = 0; iteration < 100; ++iteration)
  {
    PolicyPtr policy = learnerPolicy->addComputeStatistics();//->verbose(std::cout, 4);
    for (size_t i = 0; i < train.size(); ++i)
    {
      ClassificationExample ex = train[Random::getInstance().sampleSize(train.size())];
  //    std::cout << "SAMPLE " << i << ": " << std::endl << ex << std::endl;
      SparseVectorPtr input = ex.getInput()->toSparseVector();
      
      std::vector<FeatureGeneratorPtr> x;
//      x.push_back(input);
      for (size_t j = 0; j < input->getNumSubVectors(); ++j)
        x.push_back(input->getSubVector(j));
      size_t y = ex.getOutput();
      sequentialClassification(((i % 2) && learnerPolicy2) ? learnerPolicy2 : policy, x, 0.1, numClasses, &y);
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
    if (parameters)
      std::cout << "Params: size = " << parameters->size() << " l2norm = " << parameters->l2norm() << " l1norm = " << parameters->l1norm() << std::endl;
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
  PolicyPtr learnerPolicy = Policy::createGPOMDP(classifier, 0.4, 1.3);

  trainAndTest(train, test, numClasses, learnedPolicy, learnerPolicy);
}

void testCRank(const std::vector<ClassificationExample>& train, const std::vector<ClassificationExample>& test, size_t numClasses,
              PolicyPtr exploration = PolicyPtr(), ActionValueFunctionPtr supervision = ActionValueFunctionPtr())
{
  IterationFunctionPtr learningRate = IterationFunction::createInvLinear(1, 10000);
  IterationFunctionPtr learningRate2 = IterationFunction::createInvLinear(0.01, 10000);

  GradientBasedRankerPtr ranker = GradientBasedRanker::createLargeMarginBestAgainstAllLinear(
    GradientBasedLearner::createGradientDescent(learningRate));
  
  PolicyPtr learnedPolicy = Policy::createGreedy(ActionValueFunction::createPredictions(ranker));  
  PolicyPtr learnerPolicy = Policy::createRankingExampleCreator(exploration ? exploration : learnedPolicy, ranker, supervision);

/*  GradientBasedGeneralizedClassifierPtr classifier = GradientBasedGeneralizedClassifier::createLinear(
    GradientBasedLearner::createGradientDescent(learningRate2));  
  classifier->setParameters(ranker->getParameters());
  PolicyPtr learnerPolicy2 = Policy::createGPOMDP(classifier, 0.8, 1.1);
*/
  trainAndTest(train, test, numClasses, learnedPolicy, learnerPolicy/*->verbose(std::cout, 2)*/, ranker->getParameters()/*, learnerPolicy2*/);
}

class SequenceClassificationSyntheticOptimalPolicy : public Policy
{
public:
 SequenceClassificationSyntheticOptimalPolicy(SyntheticDataGenerator& generator)
   : generator(generator) {}

 virtual VariablePtr policyChoose(ChoosePtr choose)
 {
   CRAlgorithmPtr state = choose->getCRAlgorithm();
   size_t step = state->getVariableReference<size_t>("step");
   std::vector<SparseVectorPtr> currentRepresentation = state->getVariableReference<std::vector<SparseVectorPtr> >("usedFeatureGenerators");
   
   if (step == 0)
     return Variable::create(std::pair<bool, size_t>(false, 0)); // first step: request the first features
   else if (step == 1)
   {
     size_t goodFeatures = generator.branchGenerator.getClass(currentRepresentation[0]);
     return Variable::create(std::pair<bool, size_t>(false, goodFeatures + 1));
   }
   else
   {
     assert(step == 2);
     size_t goodFeatures = generator.branchGenerator.getClass(currentRepresentation[0]);
//     size_t goodFeaturesIndex = generator.branchGenerator.getClass(currentRepresentation[0]);
     SparseVectorPtr features = currentRepresentation[1];
//     assert(goodFeatures);
     return Variable::create(std::pair<bool, size_t>(true, generator.foldGenerators[goodFeatures].getClass(features)));
   }
 }

private:
 SyntheticDataGenerator& generator;
};

class ZeroOneActionValueFunction : public ActionValueFunction
{
public:
  ZeroOneActionValueFunction(PolicyPtr policy) : policy(policy) {}

  virtual void setChoose(ChoosePtr choose)
    {policyChoice = policy->policyChoose(choose);}
  
  virtual double compute(VariablePtr choice) const
    {return policyChoice->equals(choice) ? 1.0 : 0.0;}
  
private:
  PolicyPtr policy;
  VariablePtr policyChoice;
};



int main(int argc, char* argv[])
{
  static const size_t numFeaturesToDecideFold = 2;
  static const size_t numFolds = 2;
  static const size_t numClasses = 10;
  static const size_t numFeatures = 2;
  SyntheticDataGenerator generator(numFeaturesToDecideFold, numFolds, numFeatures, numClasses);
 
 // SyntheticLinearMultiClassGenerator generator(numFeatures, numClasses);
  
  std::vector<ClassificationExample> train(1000), test(1000);
  
  for (size_t i = 0; i < train.size(); ++i)
    train[i] = generator.sample();
  for (size_t i = 0; i < test.size(); ++i)
    test[i] = generator.sample();
  testClassifier(train, test, numClasses);  
  
 // testOLPOMDP(train, test, numClasses);

//  testCRank(train, test, numClasses);
  
  PolicyPtr optimalPolicy = new SequenceClassificationSyntheticOptimalPolicy(generator);
  ActionValueFunctionPtr optimalActionValues = new ZeroOneActionValueFunction(optimalPolicy);
//  testCRank(train, test, numClasses, optimalPolicy, optimalActionValues);*/

  
  PolicyPtr policy = optimalPolicy->addComputeStatistics();
  size_t correct = 0;
  for (size_t i = 0; i < train.size(); ++i)
  {
      ClassificationExample ex = train[i];
  //    std::cout << "SAMPLE " << i << ": " << std::endl << ex << std::endl;
      SparseVectorPtr input = ex.getInput()->toSparseVector();
      
    std::vector<FeatureGeneratorPtr> x;
//      x.push_back(input);
    for (size_t i = 0; i < input->getNumSubVectors(); ++i)
      x.push_back(input->getSubVector(i));
    size_t y = ex.getOutput();
    size_t ypredicted = sequentialClassification(policy, x, 0.1, numClasses, &y);
    if (ypredicted == y)
      ++correct;
  }
  std::cout << "EVALUATION: " << correct << " / 1000" << std::endl << std::endl;
  std::cout << "EVALUATION OF OPTIMAL POLICY: " << std::endl << policy->toString() << std::endl;

  return 0;
}
