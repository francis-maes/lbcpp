/*-----------------------------------------.---------------------------------.
| Filename: SequentialClassification.cpp   | Sequential Classification Main  |
| Author  : Francis Maes                   |                                 |
| Started : 15/03/2009 19:57               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include "GeneratedCode/SequentialClassification.h"
#include "SyntheticDataGenerator.h"
#include <lbcpp/lbcpp.h>
using namespace lbcpp;

void testClassifier(ObjectContainerPtr train, ObjectContainerPtr test, size_t numClasses)
{
  StringDictionaryPtr classes = new StringDictionary();
  for (size_t i = 0; i < numClasses; ++i)
    classes->add("class " + lbcpp::toString(i));
  
  IterationFunctionPtr learningRate = constantIterationFunction(0.01);
  GradientBasedClassifierPtr classifier = maximumEntropyClassifier(stochasticDescentLearner(learningRate), classes);
  
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

void trainAndTest(ObjectContainerPtr train, ObjectContainerPtr test, size_t numClasses,
      PolicyPtr learnedPolicy, PolicyPtr learnerPolicy, DenseVectorPtr parameters = DenseVectorPtr(), PolicyPtr learnerPolicy2 = PolicyPtr())
{
  for (size_t iteration = 0; iteration < 100; ++iteration)
  {
    PolicyPtr policy = learnerPolicy;//->verbose(4);
    for (size_t i = 0; i < train->size(); ++i)
    {
      ClassificationExample ex = *train->getAndCast<ClassificationExample>(Random::getInstance().sampleSize(train->size()));
  //    std::cout << "SAMPLE " << i << ": " << std::endl << ex << std::endl;
      SparseVectorPtr input = ex.getInput()->toSparseVector();
      
      std::vector<FeatureGeneratorPtr> x;
//      x.push_back(input);
      for (size_t j = 0; j < input->getNumSubVectors(); ++j)
        x.push_back(input->getSubVector(j));
      size_t y = ex.getOutput();
      sequentialClassification(((i % 2) && learnerPolicy2) ? learnerPolicy2 : policy, x, 0.1, numClasses, y);
    }
    
    size_t correct = 0;
    for (size_t i = 0; i < test->size(); ++i)
    {
      ClassificationExample ex = *test->getAndCast<ClassificationExample>(i);
  //    std::cout << "SAMPLE " << i << ": " << std::endl << ex << std::endl;
      SparseVectorPtr input = ex.getInput()->toSparseVector();
      
      std::vector<FeatureGeneratorPtr> x;
//      x.push_back(input);
      for (size_t i = 0; i < input->getNumSubVectors(); ++i)
        x.push_back(input->getSubVector(i));
      size_t y = ex.getOutput();
      size_t ypredicted = sequentialClassification(learnedPolicy, x, 0.1, numClasses, y);
      if (ypredicted == y)
        ++correct;
    }
    std::cout << "EVALUATION: " << correct << " / 1000" << std::endl << std::endl;
    std::cout << "END OF ITERATION " << iteration+1 << ": " << policy->toString() << std::endl;
    if (parameters)
      std::cout << "Params: size = " << parameters->size() << " l2norm = " << parameters->l2norm() << " l1norm = " << parameters->l1norm() << std::endl;
  }
}

void testMonteCarloControl(ObjectContainerPtr train, ObjectContainerPtr test, size_t numClasses)
{
  IterationFunctionPtr learningRate = constantIterationFunction(0.001);
  IterationFunctionPtr epsilon = constantIterationFunction(0.01);
  
// Regressor::createVerbose(std::cout);
  RegressorPtr regressor = leastSquaresLinearRegressor(stochasticDescentLearner(learningRate));
  
  PolicyPtr learnedPolicy = greedyPolicy(predictedActionValues(regressor));
  
  PolicyPtr learnerPolicy = monteCarloControlPolicy(epsilonGreedyPolicy(learnedPolicy, epsilon), regressor, 0.8);
  
  trainAndTest(train, test, numClasses, learnedPolicy, learnerPolicy);
}


void testCRank(ObjectContainerPtr train, ObjectContainerPtr test, size_t numClasses,
              PolicyPtr exploration = PolicyPtr(), ActionValueFunctionPtr supervision = ActionValueFunctionPtr())
{
  IterationFunctionPtr learningRate = invLinearIterationFunction(1, 10000);
//  IterationFunctionPtr learningRate2 = invLinearIterationFunction(0.01, 10000);

  GradientBasedRankerPtr ranker = largeMarginBestAgainstAllLinearRanker(stochasticDescentLearner(learningRate));
  
  PolicyPtr learnedPolicy = greedyPolicy(predictedActionValues(ranker));  
  PolicyPtr learnerPolicy = rankingExamplesCreatorPolicy(exploration ? exploration : learnedPolicy, ranker, supervision);

/*  GradientBasedGeneralizedClassifierPtr classifier = linearGeneralizedClassifier(
    stochasticDescentLearner(learningRate2));  
  classifier->setParameters(ranker->getParameters());
  PolicyPtr learnerPolicy2 = gpomdpPolicy(classifier, 0.8, 1.1);
*/
  trainAndTest(train, test, numClasses, learnedPolicy, learnerPolicy/*->verbose(2)*/, ranker->getParameters()/*, learnerPolicy2*/);
}
/*
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
     size_t goodFeatures = generator.branchGenerator->getClass(currentRepresentation[0]);
     return Variable::create(std::pair<bool, size_t>(false, goodFeatures + 1));
   }
   else
   {
     assert(step == 2);
     size_t goodFeatures = generator.branchGenerator->getClass(currentRepresentation[0]);
//     size_t goodFeaturesIndex = generator.branchGenerator->getClass(currentRepresentation[0]);
     SparseVectorPtr features = currentRepresentation[1];
//     assert(goodFeatures);
     return Variable::create(std::pair<bool, size_t>(true, generator.foldGenerators[goodFeatures]->getClass(features)));
   }
 }

private:
 SyntheticDataGenerator& generator;
};*/

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


void runPolicy(const std::vector<CRAlgorithmPtr>& crAlgorithms, PolicyPtr policy)
{
  for (size_t i = 0; i < crAlgorithms.size(); ++i)
    crAlgorithms[Random::getInstance().sampleSize(crAlgorithms.size())]->cloneAndCast<CRAlgorithm>()->run(policy);
//  std::cout << policy->toString() << std::endl;
}

void convertExamplesToCRAlgorithms(ObjectContainerPtr examples, size_t numClasses, std::vector<CRAlgorithmPtr>& res)
{
  res.clear();
  res.resize(examples->size());
  for (size_t i = 0; i < examples->size(); ++i)
  {
    ClassificationExamplePtr ex = examples->getAndCast<ClassificationExample>(Random::getInstance().sampleSize(examples->size()));
//    std::cout << "SAMPLE " << i << ": " << std::endl << ex << std::endl;
    std::vector<FeatureGeneratorPtr> x;
//      x.push_back(input);
    SparseVectorPtr input = ex->getInput()->toSparseVector();
    if (input->getNumSubVectors())
      for (size_t j = 0; j < input->getNumSubVectors(); ++j)
        x.push_back(input->getSubVector(j));
    else
      x.push_back(input);
    size_t y = ex->getOutput();
    res[i] = sequentialClassification(x, 0, numClasses, y);
  }
}

double evaluatePolicy(const std::vector<CRAlgorithmPtr>& examples, PolicyPtr policy)
{
  size_t correct = 0;
  PolicyPtr p = policy;
  for (size_t i = 0; i < examples.size(); ++i)
  {
    CRAlgorithmPtr crAlgorithm = examples[i]->cloneAndCast<CRAlgorithm>();
    crAlgorithm->run(p);
    size_t ypredicted = crAlgorithm->getReturn()->getCopy<size_t>();
    if (ypredicted == crAlgorithm->getVariableReference<size_t>("ycorrect"))
      ++correct;
  }
 // std::cout << "Reward per choose: " << (p->getResultWithName("rewardPerChoose").dynamicCast<ScalarVariableStatistics>()->getMean()) << std::endl;
  return correct / (double)examples.size();
}

class GPOMDPAverageRewardFunction : public ScalarVectorFunction
{
public:
  GPOMDPAverageRewardFunction(GradientBasedGeneralizedClassifierPtr classifier, double beta, const std::vector<CRAlgorithmPtr>& instances)
    : classifier(classifier), beta(beta), instances(instances) {}
  
  virtual bool isDerivable() const
    {return false;}
    
  struct ComputeRiskGradientLearner : public GradientBasedLearner
  {
    ComputeRiskGradientLearner() : numTerms(0), sumOfGradients(new DenseVector()) {}
    
    virtual void trainStochasticExample(FeatureGeneratorPtr gradient, double weight)
      {++numTerms; sumOfGradients->addWeighted(gradient, weight);}

    DenseVectorPtr getGradient()
    {
      if (!sumOfGradients->hasDictionary())
        sumOfGradients->setDictionary(parameters->getDictionary());
      if (numTerms > 1)
        sumOfGradients->multiplyByScalar(1.0 / numTerms);
      if (regularizer)
        sumOfGradients->add(regularizer->computeGradient(parameters));
      return sumOfGradients;
    }
    
  private:
    size_t numTerms;
    DenseVectorPtr sumOfGradients;
  };

  typedef ReferenceCountedObjectPtr<ComputeRiskGradientLearner> ComputeRiskGradientLearnerPtr;

  virtual void compute(const FeatureGeneratorPtr input, double* output, const FeatureGeneratorPtr gradientDirection, FeatureGeneratorPtr* gradient) const
  {
    ComputeRiskGradientLearnerPtr learner = new ComputeRiskGradientLearner();
    
    GradientBasedLearnerPtr previousLearner = classifier->getLearner();
    classifier->setLearner(learner);
    classifier->setParameters(input->toDenseVector());

    PolicyStatisticsPtr statistics = new PolicyStatistics();
    PolicyPtr policy = gpomdpPolicy(classifier, beta);
    for (size_t i = 0; i < instances.size(); ++i)
      policy->run(instances[i], statistics);
    if (output)
      *output = -statistics->getRewardPerChooseMean();
    if (gradient)
      *gradient = learner->getGradient();
    classifier->setLearner(previousLearner);
  }
        
private:
  GradientBasedGeneralizedClassifierPtr classifier;
  double beta;
  std::vector<CRAlgorithmPtr> instances;
};


void testBatchGPOMDP(const std::vector<CRAlgorithmPtr>& train, const std::vector<CRAlgorithmPtr>& test)
{
  for (size_t i = 0; i <= 10; ++i)
  {
    double beta = i / 10.0; //pow(10.0, (double)(i - 5.0));
    //IterationFunctionPtr stepSize = constantIterationFunction(10.0);
    double reg = 0.0; // (double)pow(10.0, (double)(i - 5.0));
    
    VectorOptimizerPtr optimizer = rpropOptimizer(); //gradientDescentOptimizer(stepSize);
    GradientBasedGeneralizedClassifierPtr classifier = linearGeneralizedClassifier(batchLearner(optimizer));
    classifier->setL2Regularizer(reg);
    //classifier->setInitializeParametersRandomly();
    PolicyPtr learnedPolicy = greedyPolicy(predictedActionValues(classifier));  

    ScalarVectorFunctionPtr objective = new GPOMDPAverageRewardFunction(classifier, beta, train);
    FeatureGeneratorPtr parameters 
      = optimizer->optimize(objective, maxIterationsStoppingCriterion(50), consoleProgressCallback());
    double trainAccuracy = evaluatePolicy(train, learnedPolicy);
    double testAccuracy = evaluatePolicy(test, learnedPolicy);
    std::cout << "REG = " << reg 
      << " train = " << trainAccuracy << " test = " << testAccuracy << std::endl;
  }
};

class OverrideDecideStepsPolicy : public Policy
{
public:
  OverrideDecideStepsPolicy(PolicyPtr explorationPolicy)
    : explorationPolicy(explorationPolicy) {}

  virtual void policyEnter(CRAlgorithmPtr crAlgorithm)
    {explorationPolicy->policyEnter(crAlgorithm);}

  virtual VariablePtr policyChoose(ChoosePtr choose)
  {
    VariablePtr res = explorationPolicy->policyChoose(choose);
    std::pair<bool, size_t>& c = res->getReference< std::pair<bool, size_t> >();
   // if (c.first)
   //   c.second = choose->getCRAlgorithm()->getVariableReference<size_t>("ycorrect");
    return res;
  }
  
  virtual void policyReward(double reward)
    {explorationPolicy->policyReward(reward);}

  virtual void policyLeave()
    {explorationPolicy->policyLeave();}
  
private:
  PolicyPtr explorationPolicy;
};

void testOLPOMDP(const std::vector<CRAlgorithmPtr>& train, const std::vector<CRAlgorithmPtr>& test)
{
 // for (size_t i = 0; i <= 10; ++i)
  {
    double beta = 1.0; // i / 10.0; //pow(10.0, (double)(i - 5.0));
    
    IterationFunctionPtr learningRate = invLinearIterationFunction(0.1, 10000);

    GeneralizedClassifierPtr classifier = linearGeneralizedClassifier(stochasticDescentLearner(learningRate));
    
    PolicyPtr learnedPolicy = greedyPolicy(predictedActionValues(classifier));  
    PolicyPtr learnerPolicy = gpomdpPolicy(classifier, beta, new OverrideDecideStepsPolicy(
      stochasticPolicy(probabilitiesActionValues(classifier))));

    for (size_t iteration = 0; iteration < 1000; ++iteration)
    {
      PolicyPtr p = learnerPolicy;
      runPolicy(train, p);
      //std::cout << "Learner: " << p->toString() << std::endl;
      PolicyStatisticsPtr statistics = new PolicyStatistics();
      runPolicy(test, computeStatisticsPolicy(learnedPolicy, statistics));
      double trainAccuracy = evaluatePolicy(train, learnedPolicy);
      double testAccuracy = evaluatePolicy(test, learnedPolicy);
//      if ((iteration % 10) == 9)
        std::cout << "Beta = " << beta << " Iteration " << iteration
          << " train = " << trainAccuracy << " test = " << testAccuracy
          << " test mean steps = " << statistics->getChoosesPerEpisode()->getMean() << std::endl;
    }
  }
}
int main(int argc, char* argv[])
{
  static const size_t numFeaturesToDecideFold = 20;
  static const size_t numFolds = 10;
  static const size_t numClasses = 3;
  static const size_t numFeatures = 4;
  ObjectStreamPtr generator = new SyntheticDataGenerator(numFeaturesToDecideFold, numFolds, numFeatures, numClasses);
  ObjectContainerPtr train = generator->load(50000);
  ObjectContainerPtr test = generator->load(1000);
  
//  SyntheticLinearMultiClassGenerator generator(numFeatures, numClasses);
  
/*  std::vector<double> classFrequencies(numClasses, 0.0);
  for (size_t i = 0; i < test.size(); ++i)
  {
    test[i] = generator.sample();
    classFrequencies[test[i].getOutput()]++;
  }
  std::cout << "Test Class Frequencies: " << lbcpp::toString(classFrequencies) << std::endl;
  for (size_t i = 0; i < classFrequencies.size(); ++i)
    classFrequencies[i] /= (double)test.size();
  std::cout << "Normalized Test Class Frequencies: " << lbcpp::toString(classFrequencies) << std::endl;
  */
  testClassifier(train, test, numClasses);  
  
  std::vector<CRAlgorithmPtr> crTrain, crTest;
  convertExamplesToCRAlgorithms(train, numClasses, crTrain);
  convertExamplesToCRAlgorithms(test, numClasses, crTest);
  
//  testBatchGPOMDP(crTrain, crTest);
  testOLPOMDP(crTrain, crTest);

//  testCRank(train, test, numClasses);
 return 0; 
/*  PolicyPtr optimalPolicy = new SequenceClassificationSyntheticOptimalPolicy(generator);
  ActionValueFunctionPtr optimalActionValues = new ZeroOneActionValueFunction(optimalPolicy);
  testCRank(train, test, numClasses, optimalPolicy, optimalActionValues);

  double accuracy = evaluatePolicy(crTrain, optimalPolicy);
  std::cout << "EVALUATION: " << accuracy * 100 << "%" << std::endl << std::endl;
  return 0;*/
}
