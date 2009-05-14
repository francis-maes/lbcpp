#include "GeneratedCode/SemanticRoleLabeling.lh"
#include <fstream>
using namespace lbcpp;

class SentencesParser : public TextFileParser
{
public:
  SentencesParser(std::vector<SentencePtr>& sentences)
    : sentences(sentences) {}

  virtual bool parseDataLine(const std::vector<std::string>& columns)
  {
    assert(columns.size() > 0);
    sentences.push_back(new Sentence(columns));
    return true;
  }
  
private:
  std::vector<SentencePtr>& sentences;
};

class SRLLabelsParser : public TextFileParser
{
public:
  SRLLabelsParser(StringDictionaryPtr relations, StringDictionaryPtr arguments, std::vector<SRLLabelPtr>& labels)
    : relations(relations), arguments(arguments), labels(labels) {}
    
  virtual bool parseDataLine(const std::vector<std::string>& columns)
  {
    assert(columns.size() >= 1);
    SRLLabelPtr res = new SRLLabel();
    res->rel = relations->add(toLowerCase(columns[0]));
    if (columns.size() > 1)
    {
      assert(columns[1].size() > 1);
      res->arg0 = arguments->add(toLowerCase(columns[1]));
    }
    if (columns.size() > 2)
    {
      assert(columns[2].size() > 1);
      res->arg1 = arguments->add(toLowerCase(columns[2]));
    }
    labels.push_back(res);
    return true;
  }
  
private:
  StringDictionaryPtr relations;
  StringDictionaryPtr arguments;
  
  std::vector<SRLLabelPtr>& labels;

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
    SentencesParser sentencesParser(sentences);
    SRLLabelsParser labelsParser(relations, arguments, labels);
    return sentencesParser.parseFile(sentencesFile) && labelsParser.parseFile(labelsFile);
  }

  void convertToCRAlgorithms(std::vector<CRAlgorithmPtr>& res)
  {
    assert(sentences.size() == labels.size());
    res.resize(sentences.size());
    for (size_t i = 0; i < res.size(); ++i)
      res[i] = crSemanticRoleLabeling(sentences[i], relations, arguments, labels[i]);
  }

private:
  StringDictionaryPtr relations;
  StringDictionaryPtr arguments;
  std::vector<SentencePtr> sentences;
  std::vector<SRLLabelPtr> labels;
};

PolicyPtr learn(const std::vector<CRAlgorithmPtr>& instances, size_t maxLearningIterations = 100, size_t maxLearningIterationsWithoutImprovement = 5)
{
  IterationFunctionPtr learningRate = IterationFunction::createInvLinear(10, 10000);
  GradientBasedRankerPtr ranker = GradientBasedRanker::
    //createLargeMarginBestAgainstAllLinear
    createLargeMarginMostViolatedPairLinear
    //createLargeMarginAllPairsLinear
    (GradientBasedLearner::createStochasticDescent(learningRate));
  
  PolicyPtr learnedPolicy = Policy::createGreedy(ActionValueFunction::createPredictions(ranker));
  PolicyPtr learnerPolicy = Policy::createRankingExampleCreator(learnedPolicy, ranker);

  double bestTotalReward = 0.0;
  size_t numIterationsWithoutImprovement = 0;
  
  for (size_t i = 0; i < maxLearningIterations; ++i)
  {
    PolicyPtr policy = learnerPolicy->addComputeStatistics();
    std::vector<size_t> order;
    Random::getInstance().sampleOrder(0, instances.size(), order);
    for (size_t j = 0; j < order.size(); ++j)
      instances[order[j]]->clone()->run(policy->verbose(1));
      
    std::cout << "[" << numIterationsWithoutImprovement << "] Learning Iteration " << i;// << " => " << policy->toString() << std::endl;
    double totalReward = policy->getResultWithName("rewardPerEpisode").dynamicCast<ScalarRandomVariableStatistics>()->getMean();
    std::cout << " REWARD PER EPISODE = " << totalReward << std::endl;
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
  
  return learnedPolicy;
}

double evaluate(PolicyPtr policy, const std::vector<CRAlgorithmPtr>& instances)
{
  PolicyPtr p = policy->addComputeStatistics();
  for (size_t j = 0; j < instances.size(); ++j)
    instances[j]->clone()->run(p);
  return p->getResultWithName("rewardPerEpisode").dynamicCast<ScalarRandomVariableStatistics>()->getMean();
}

int main(int argc, char* argv[])
{
  const char* trainInputFile = "/Users/francis/Projets/LBC++/trunk/test/SemanticRoleLabeling/data/train-2004-txt";
  const char* trainLabelFile = "/Users/francis/Projets/LBC++/trunk/test/SemanticRoleLabeling/data/label-2004-txt";
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
  
  PolicyPtr policy = learn(trainingCR, 2);
  std::cout << "TRAINING SCORE = " << evaluate(policy, trainingCR) << std::endl;
  std::cout << "TESTING SCORE = " << evaluate(policy, testingCR) << std::endl;
  return 0;
}
