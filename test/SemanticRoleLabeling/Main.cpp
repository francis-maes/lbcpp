#include "GeneratedCode/SemanticRoleLabeling.lh"
#include <fstream>
using namespace lbcpp;

class SentencesParser : public LearningDataObjectParser
{
public:
  SentencesParser(const std::string& filename)
    : LearningDataObjectParser(filename) {}
  
  virtual std::string getContentClassName() const
    {return "Sentence";}

  virtual bool parseDataLine(const std::vector<std::string>& columns)
  {
    assert(columns.size() > 0);
    setResult(new Sentence(columns));
    return true;
  }
};

class SRLLabelsParser : public LearningDataObjectParser
{
public:
  SRLLabelsParser(const std::string& filename, StringDictionaryPtr relations, StringDictionaryPtr arguments)
    : LearningDataObjectParser(filename), relations(relations), arguments(arguments) {}
    
  virtual std::string getContentClassName() const
    {return "SRLLabelChoice";}
    
  virtual bool parseDataLine(const std::vector<std::string>& columns)
  {
    assert(columns.size() >= 1);
    SRLLabelChoicePtr res = new SRLLabelChoice();
    size_t i = 0;
    while (i < columns.size())
    {
      SRLLabelPtr label = new SRLLabel();
      label->rel = relations->add(toLowerCase(columns[i]));
      ++i;
      if (i < columns.size() && columns[i] != ";")
      {
        label->arg0 = arguments->add(toLowerCase(columns[i]));
        ++i;
        if (i < columns.size() && columns[i] != ";")
        {
          label->arg1 = arguments->add(toLowerCase(columns[i]));
          ++i;
        }
      }
      res->addLabel(label);
      while (i < columns.size() && columns[i] == ";")
        ++i;
    }
    setResult(res);
    //std::cout << "Parsed: " << *res << std::endl;
    return true;
  }
  
private:
  StringDictionaryPtr relations;
  StringDictionaryPtr arguments;
  
  static std::string toLowerCase(const std::string& str)
  {
    std::string res;
    for (size_t i = 0; i < str.size(); ++i)
      if (str[i] >= 'A' && str[i] <= 'Z')
        res += str[i] + 'a' - 'A';
      else
        res += str[i];
    return res;
  }
};

class SRLTrainingSet : public Object
{
public:
  SRLTrainingSet(StringDictionaryPtr relations, StringDictionaryPtr arguments)
    : relations(relations), arguments(arguments) {}

  bool parse(const std::string& sentencesFile, const std::string& labelsFile)
  {
    sentences = (new SentencesParser(sentencesFile))->load();
    labels = (new SRLLabelsParser(labelsFile, relations, arguments))->load();
    return sentences && labels;
  }

  void convertToCRAlgorithms(std::vector<CRAlgorithmPtr>& res)
  {
    assert(sentences->size() == labels->size());
    res.resize(sentences->size());
    for (size_t i = 0; i < res.size(); ++i)
      res[i] = crSemanticRoleLabeling(sentences->getAndCast<Sentence>(i), relations, arguments, labels->getAndCast<SRLLabelChoice>(i));
  }

private:
  StringDictionaryPtr relations;
  StringDictionaryPtr arguments;
  ObjectContainerPtr sentences; // Sentence
  ObjectContainerPtr labels;    // SRLLabelChoice
};

double evaluate(PolicyPtr policy, const std::vector<CRAlgorithmPtr>& instances)
{
  PolicyStatisticsPtr statistics = new PolicyStatistics();
  for (size_t j = 0; j < instances.size(); ++j)
    policy->run(instances[j], statistics);
  return statistics->getRewardPerEpisodeMean();
}

/*
void testOLPOMDP(const std::vector<SequenceExample>& learningExamples, StringDictionaryPtr labels)
{
  IterationFunctionPtr learningRate = invLinearIterationFunction(0.1, 10000);

  GeneralizedClassifierPtr classifier = linearGeneralizedClassifier(stochasticDescentLearner(learningRate));
  
  PolicyPtr learnedPolicy = greedyPolicy(predictedActionValues(classifier));  
  PolicyPtr learnerPolicy = gpomdpPolicy(classifier, 0, 2.0);

  for (size_t i = 0; i < 10; ++i)
  {
    std::cout << "ITERATION: " << (i+1) << std::endl;
    PolicyPtr p = learnerPolicy->addComputeStatistics()->verbose(0);
  //  testTrivial(p);
//    std::cout << p->toString() << std::endl;
    runPolicy(p, learningExamples, labels->getNumElements());
 //   std::cout << "Params: " << regressor->getParameters()->toString() << std::endl;
  }
}
*/
std::pair<PolicyPtr, PolicyPtr> createOLPOMDPPolicies(GradientBasedGeneralizedClassifierPtr& classifier, DenseVectorPtr initialParameters = DenseVectorPtr())
{
  IterationFunctionPtr learningRate = invLinearIterationFunction(0.1, 10000);

  classifier = linearGeneralizedClassifier(stochasticDescentLearner(learningRate));
  if (initialParameters)
    classifier->setParameters(initialParameters);
  
  PolicyPtr learnedPolicy = greedyPolicy(predictedActionValues(classifier));  
  PolicyPtr learnerPolicy = gpomdpPolicy(classifier, 0.999, 1.2);
  return std::make_pair(learnedPolicy, learnerPolicy);
}

std::pair<PolicyPtr, PolicyPtr> createCRankPolicies(GradientBasedRankerPtr& ranker, DenseVectorPtr initialParameters = DenseVectorPtr())
{
  IterationFunctionPtr learningRate = constantIterationFunction(10);
  ranker = largeMarginMostViolatedPairLinearRanker(stochasticDescentLearner(learningRate));
//    largeMarginBestAgainstAllLinearRanker
//    largeMarginAllPairsLinearRanker
    
  if (initialParameters)
    ranker->setParameters(initialParameters);
  
  PolicyPtr learnedPolicy = greedyPolicy(predictedActionValues(ranker));
  PolicyPtr learnerPolicy = rankingExampleCreatorPolicy(learnedPolicy, ranker);
  return std::make_pair(learnedPolicy, learnerPolicy);
}

std::pair<PolicyPtr, DenseVectorPtr> learn(const std::vector<CRAlgorithmPtr>& instances, const std::vector<CRAlgorithmPtr>& testingInstances, 
                size_t maxLearningIterations = 100, size_t maxLearningIterationsWithoutImprovement = 5, DenseVectorPtr initialParameters = DenseVectorPtr())
{
  GradientBasedGeneralizedClassifierPtr classifier;
  GradientBasedRankerPtr ranker;
  
  
  std::pair<PolicyPtr, PolicyPtr> policies = initialParameters ? createOLPOMDPPolicies(classifier, initialParameters) : createCRankPolicies(ranker);
  PolicyPtr learnedPolicy = policies.first, learnerPolicy = policies.second;

  double bestTotalReward = 0.0;
  size_t numIterationsWithoutImprovement = 0;
  
  for (size_t i = 0; i < maxLearningIterations; ++i)
  {
    PolicyStatisticsPtr statistics = new PolicyStatistics();
    std::vector<size_t> order;
    Random::getInstance().sampleOrder(0, instances.size(), order);
    for (size_t j = 0; j < order.size(); ++j)
      learnerPolicy->verbose(0)->run(instances[order[j]], statistics);
      
    std::cout << "[" << numIterationsWithoutImprovement << "] Learning Iteration " << i;// << " => " << policy->toString() << std::endl;
    double totalReward = statistics->getRewardPerEpisodeMean();
    std::cout << " REWARD PER EPISODE = " << totalReward << std::endl;
    std::cout << "TRAINING SCORE = " << (1.0 - evaluate(learnedPolicy, instances)) * 100 << "%" << std::endl;
    std::cout << "TESTING SCORE = " << (1.0 - evaluate(learnedPolicy, testingInstances)) * 100 << "%" << std::endl;

//    std::cout << " parameters => " << std::endl << ranker->getParameters()->toString() << std::endl;
    if (totalReward > bestTotalReward)
    {
      bestTotalReward = totalReward; // clone best policy ?
      numIterationsWithoutImprovement = 0;
    }
    else
    {
      ++numIterationsWithoutImprovement;
      if (numIterationsWithoutImprovement >= maxLearningIterationsWithoutImprovement)
        break;
    }
  }
  
  return std::make_pair(learnedPolicy, initialParameters ? classifier->getParameters() : ranker->getParameters());
}
int main(int argc, char* argv[])
{
//  const char* trainInputFile = "/Users/francis/Projets/LBC++/trunk/test/SemanticRoleLabeling/data/train-2001-txt";
//  const char* trainLabelFile = "/Users/francis/Projets/LBC++/trunk/test/SemanticRoleLabeling/data/label-2001-txt";

  const char* trainInputFile = "/Users/francis/Projets/LBC++/trunk/test/SemanticRoleLabeling/data/ambiguous/train-2004_ambig-txt";
  const char* trainLabelFile = "/Users/francis/Projets/LBC++/trunk/test/SemanticRoleLabeling/data/ambiguous/train-2004_ambig-label";

  const char* testInputFile = "/Users/francis/Projets/LBC++/trunk/test/SemanticRoleLabeling/data/gold-2004-txt";
  const char* testLabelFile = "/Users/francis/Projets/LBC++/trunk/test/SemanticRoleLabeling/data/gold-2004-label";

  StringDictionaryPtr relations = new StringDictionary();
  StringDictionaryPtr arguments = new StringDictionary();
  arguments->add("__none__");
  
  SRLTrainingSet trainingSet(relations, arguments);
  if (!trainingSet.parse(trainInputFile, trainLabelFile))
    return 1;
  SRLTrainingSet testingSet(relations, arguments);
  if (!testingSet.parse(testInputFile, testLabelFile))
    return 2;
  
  std::cout << "Relations: " << *relations << std::endl;
  std::cout << "Arguments: " << *arguments << std::endl;

  std::vector<CRAlgorithmPtr> trainingCR, testingCR;
  trainingSet.convertToCRAlgorithms(trainingCR);
  testingSet.convertToCRAlgorithms(testingCR);
  
  std::pair<PolicyPtr, DenseVectorPtr> policyAndParameters = learn(trainingCR, testingCR, 100, 10);
//  std::pair<PolicyPtr, DenseVectorPtr> policyAndParameters2 = learn(trainingCR, testingCR, 100, 5, policyAndParameters.second);
  return 0;
}
