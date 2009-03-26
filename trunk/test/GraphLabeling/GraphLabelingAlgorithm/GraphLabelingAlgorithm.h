#include "GeneratedCode/LabeledContentGraph.lh"
#include "GeneratedCode/GraphLabelingCRAlgorithms.lh"
#include <fstream>
using namespace cralgo;

class GraphLabelingAlgorithm
{
public:
  virtual ~GraphLabelingAlgorithm() {}
  
  virtual void reset(StringDictionaryPtr labels) = 0;
  virtual void train(LabeledContentGraphPtr graph) = 0;
  virtual double evaluate(LabeledContentGraphPtr graph, size_t begin, size_t end) = 0;
  
  void crossValidate(const std::vector<LabeledContentGraphPtr>& trainGraphs,
                     const std::vector<LabeledContentGraph::LabelsFold>& testGraphs, 
                     ScalarRandomVariableStatisticsPtr trainAccuracy,
                     ScalarRandomVariableStatisticsPtr testAccuracy)
  {
    assert(trainGraphs.size() == testGraphs.size());
    for (size_t i = 0; i < trainGraphs.size(); ++i)
    {
      std::cout << "CROSS-VALIDATION FOLD " << (i+1) << " / " << trainGraphs.size() << std::endl;
      reset(trainGraphs[i]->getLabelDictionary());
      train(trainGraphs[i]);
      trainAccuracy->push(evaluate(trainGraphs[i], 0, trainGraphs[i]->getNumNodes()));
      testAccuracy->push(evaluate(testGraphs[i].graph, testGraphs[i].foldBegin, testGraphs[i].foldEnd));
    }
  }
};

class ClassifierBasedGraphLabelingAlgorithm : public GraphLabelingAlgorithm
{
public:  
  ClassifierBasedGraphLabelingAlgorithm() : l2regularizer(0.0) {}
  
  virtual FeatureGeneratorPtr getNodeFeatures(LabeledContentGraphPtr graph, size_t nodeIndex) = 0;

  double l2regularizer; // a la rache
  
  virtual ClassifierPtr createClassifier(StringDictionaryPtr labels)
  {
    IterationFunctionPtr learningRate = IterationFunction::createConstant(1.0);//InvLinear(26, 10000);
    GradientBasedLearnerPtr learner = GradientBasedLearner::createStochasticDescent(learningRate);
    GradientBasedClassifierPtr res = GradientBasedClassifier::createMaximumEntropy(/*learner->stochasticToBatch(100)*/learner, labels);
    res->setL2Regularizer(l2regularizer);
    return res;
  }

  virtual void reset(StringDictionaryPtr labels)
    {classifier = createClassifier(labels);}

  virtual void train(LabeledContentGraphPtr graph)
  {
    std::vector<ClassificationExample> examples;
    examples.reserve(graph->getNumNodes());
    for (size_t i = 0; i < graph->getNumNodes(); ++i)
      examples.push_back(ClassificationExample(getNodeFeatures(graph, i), graph->getLabel(i)));
    trainClassifier(classifier, examples);
  }
    
  virtual double evaluate(LabeledContentGraphPtr graph, size_t begin, size_t end)
  {
    assert(end > begin);
    size_t correct = 0;
    for (size_t i = begin; i < end; ++i)
      if (classifier->predict(getNodeFeatures(graph, i)) == graph->getLabel(i))
        ++correct;
    return correct / (double)(end - begin);
  }

protected:
  ClassifierPtr classifier;
  
  void trainClassifier(ClassifierPtr classifier, const std::vector<ClassificationExample>& examples)
  {
    for (size_t i = 0; i < 10; ++i)
    {
      std::cout << classifier->evaluateAccuracy(examples) << " " << std::flush;
      classifier->trainStochastic(examples);
    }
    std::cout << std::endl;
  }
};

class ContentOnlyGraphLabelingAlgorithm : public ClassifierBasedGraphLabelingAlgorithm
{
public:
  virtual FeatureGeneratorPtr getNodeFeatures(LabeledContentGraphPtr graph, size_t nodeIndex)
    {return graph->getNode(nodeIndex);}
};

class PerfectContextAndContentGraphLabelingAlgorithm : public ClassifierBasedGraphLabelingAlgorithm
{
public:
  virtual FeatureGeneratorPtr getNodeFeatures(LabeledContentGraphPtr graph, size_t nodeIndex)
//    {return graph->nodeAndNeighborhoodLabelsFrequencyFeatures(nodeIndex);}
    {return graph->nodeFeatures(nodeIndex, 1, 1, false);}
};

class IterativeClassificationGraphLabelingAlgorithm : public ClassifierBasedGraphLabelingAlgorithm
{
public:
  virtual FeatureGeneratorPtr getNodeFeatures(LabeledContentGraphPtr graph, size_t nodeIndex)
    {return graph->nodeAndNeighborhoodLabelsFrequencyFeatures(nodeIndex);}

  virtual void reset(StringDictionaryPtr labels)
    {initialClassifier = createClassifier(labels); classifier = createClassifier(labels);}

  virtual void train(LabeledContentGraphPtr graph)
  {
    std::vector<ClassificationExample> examples, examplesWithContext;
    examples.reserve(graph->getNumNodes());
    examplesWithContext.reserve(graph->getNumNodes());
    for (size_t i = 0; i < graph->getNumNodes(); ++i)
    {
      examples.push_back(ClassificationExample(graph->getNode(i), graph->getLabel(i)));
      examplesWithContext.push_back(ClassificationExample(getNodeFeatures(graph, i), graph->getLabel(i)));
    }
    trainClassifier(initialClassifier, examples);
    trainClassifier(classifier, examplesWithContext);
  }
  
  enum {maxInferencePasses = 100};

  virtual double evaluate(LabeledContentGraphPtr graph, size_t begin, size_t end)
  {
    assert(end > begin);
    LabeledContentGraphPtr predictedGraph 
      = new LabeledContentGraph(graph->getContentGraph(), new LabelSequence(*graph->getLabels()));
    makeInitialPredictions(predictedGraph, begin, end);
    return iterativeClassification(graph, predictedGraph, begin, end, maxInferencePasses);
  }

  
protected:
  ClassifierPtr initialClassifier;
    
  void makeInitialPredictions(LabeledContentGraphPtr graph, size_t begin, size_t end)
  {
    for (size_t i = begin; i < end; ++i)
      graph->setLabel(i, initialClassifier->predict(graph->getNode(i)));
  }
  
  double computeAccuracy(LabelSequencePtr correctLabels, LabeledContentGraphPtr predictedGraph, size_t begin, size_t end)
  {
    assert(end > begin);
    return predictedGraph->getLabels()->numberOfLabelsInCommonWith(correctLabels, begin, end) / (double)(end - begin);
  }
  
  double iterativeClassification(LabeledContentGraphPtr graph, LabeledContentGraphPtr predictedGraph, size_t begin, size_t end, size_t maxNumPasses)
  {
    LabelSequencePtr previousLabels = new LabelSequence(*predictedGraph->getLabels());
    double accuracy = 0.0;
    for (size_t i = 0; i < maxInferencePasses; ++i)
    {
      // label each node in a randomly sampled order
      std::vector<size_t> order;
      Random::getInstance().sampleOrder(begin, end, order);
      for (size_t j = 0; j < order.size(); ++j)
      {
        size_t nodeIndex = order[j];
        predictedGraph->setLabel(nodeIndex, classifier->predict(getNodeFeatures(predictedGraph, nodeIndex)));
      }
      accuracy = computeAccuracy(graph->getLabels(), predictedGraph, begin, end);
       
       // stopping criterion
      size_t numCommon = previousLabels->numberOfLabelsInCommonWith(predictedGraph->getLabels(), begin, end);
      double changeRatio = 1.0 - numCommon / (double)(end - begin);
      std::cout << "ITERATION " << i << " NumCommon = " << numCommon << " changeRatio = " << changeRatio
          << " accuracy = " << accuracy << std::endl;
      if (changeRatio < 0.0001)
        break;
      previousLabels = new LabelSequence(*predictedGraph->getLabels());
    }
    return accuracy;
  }
};

class GibbsSamplingGraphLabelingAlgorithm : public IterativeClassificationGraphLabelingAlgorithm
{
public:
  enum
  {
    maxBurnInPasses = 200,
    numGibbsSamples = 1000,
  };

  virtual double evaluate(LabeledContentGraphPtr graph, size_t begin, size_t end)
  {
    assert(end > begin);
    LabeledContentGraphPtr predictedGraph 
      = new LabeledContentGraph(graph->getContentGraph(), new LabelSequence(*graph->getLabels()));
    
    // initial predictions
    makeInitialPredictions(predictedGraph, begin, end);
    
    // burn-in 
    double iterativeClassificationAccuracy = iterativeClassification(graph, predictedGraph, begin, end, maxInferencePasses);
    
    // initialize sample counts
    size_t numLabels = graph->getLabelDictionary()->getNumElements();
    std::vector< std::vector< size_t > > labelFrequencies(end - begin, std::vector<size_t>(numLabels, 0));
    // labelFrequencies : (nodeIndex - begin) -> label -> count
    
    // collect samples
    for (size_t i = 0; i < numGibbsSamples; ++i)
    {
      std::vector<size_t> order;
      Random::getInstance().sampleOrder(begin, end, order);
      for (size_t j = 0; j < order.size(); ++j)
      {
        size_t nodeIndex = order[j];
        size_t label = classifier->sample(getNodeFeatures(predictedGraph, nodeIndex));
        labelFrequencies[nodeIndex - begin][label]++;
        predictedGraph->setLabel(nodeIndex, label);
      }
    }
    
    // compute final labels
    for (size_t i = begin; i < end; ++i)
    {
      std::vector<size_t>& labelCounts = labelFrequencies[i - begin];
      size_t max = 0;
      size_t label = (size_t)-1;
      for (size_t j = 0; j < labelCounts.size(); ++j)
        if (labelCounts[j] > max)
          max = labelCounts[j], label = j;
      assert(label != (size_t)-1);
      predictedGraph->setLabel(i, label);
    }
    
    double finalAccuracy = computeAccuracy(graph->getLabels(), predictedGraph, begin, end);
    std::cout << "Gibbs gain: " << (finalAccuracy - iterativeClassificationAccuracy) * 100 << "%" << std::endl; 
    return finalAccuracy;
  }
};

class CRAlgorithmGraphLabelingAlgorithm : public GraphLabelingAlgorithm
{
public:
  virtual std::pair<PolicyPtr, PolicyPtr> createInitialPolicies(StringDictionaryPtr labels) = 0; // returns a pair (learnerPolicy, learnedPolicy)
  virtual CRAlgorithmPtr createCRAlgorithm(LabeledContentGraphPtr graph, size_t begin, size_t end) = 0;

  virtual void reset(StringDictionaryPtr labels)
  {
    std::pair<PolicyPtr, PolicyPtr> p = createInitialPolicies(labels);
    learnerPolicy = p.first;
    learnedPolicy = p.second;
  }
  
  enum
  {
    maxLearningIterations = 5,
    maxLearningIterationsWithoutImprovement = 5,
  };
  
  virtual void train(LabeledContentGraphPtr graph)
  {    
    double bestTotalReward = 0.0;
    size_t numIterationsWithoutImprovement = 0;
    for (size_t i = 0; i < maxLearningIterations; ++i)
    {
      PolicyPtr policy = learnerPolicy->addComputeStatistics();
      CRAlgorithmPtr crAlgorithm = createCRAlgorithm(graph, 0, graph->getNumNodes());
      crAlgorithm->run(policy);
      std::cout << "[" << numIterationsWithoutImprovement << "] Learning Iteration " << i;// << " => " << policy->toString() << std::endl;
      double totalReward = policy->getResultWithName("rewardPerEpisode").dynamicCast<ScalarRandomVariableStatistics>()->getMean();
      std::cout << " TOTAL REWARD = " << totalReward << " => online accuracy = " << totalReward / (double)graph->getNumNodes() << std::endl;
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
  }
  
  virtual double evaluate(LabeledContentGraphPtr graph, size_t begin, size_t end)
  {
    assert(end > begin);
    CRAlgorithmPtr crAlgorithm = createCRAlgorithm(graph, begin, end);
    PolicyPtr policy = learnedPolicy->addComputeStatistics();
    crAlgorithm->run(policy);
    assert(crAlgorithm->hasReturn());
    LabelSequencePtr predictedLabels = crAlgorithm->getReturn()->getConstReference<LabelSequencePtr>();
    assert(predictedLabels);
    return predictedLabels->numberOfLabelsInCommonWith(graph->getLabels(), begin, end) / (double)(end - begin);
  }
  
protected:
  PolicyPtr learnerPolicy;
  PolicyPtr learnedPolicy;
};

class CRIterativeClassificationGraphLabelingAlgorithm : public CRAlgorithmGraphLabelingAlgorithm
{
public:
  CRIterativeClassificationGraphLabelingAlgorithm() : l2regularizer(0.0) {}
  
  double l2regularizer;
  
  virtual std::pair<PolicyPtr, PolicyPtr> createInitialPolicies(StringDictionaryPtr labels)
  {
/* CRANK - PREDICTED 
    IterationFunctionPtr learningRate = IterationFunction::createInvLinear(10, 10000);
    RankerPtr ranker = GradientBasedRanker::
      //createLargeMarginBestAgainstAllLinear
      //createLargeMarginMostViolatedPairLinear
      createLargeMarginAllPairsLinear
      (GradientBasedLearner::createStochasticDescent(learningRate));
    
    PolicyPtr learnedPolicy = Policy::createGreedy(ActionValueFunction::createPredictions(ranker));  
    PolicyPtr learnerPolicy = Policy::createRankingExampleCreator(learnedPolicy, ranker);
    */

    // Classifier Maxent
    IterationFunctionPtr learningRate = IterationFunction::createConstant(1.0);//InvLinear(26, 10000);
    GradientBasedLearnerPtr learner = GradientBasedLearner::createStochasticDescent(learningRate);
    GradientBasedClassifierPtr classifier = GradientBasedClassifier::createMaximumEntropy(/*learner->stochasticToBatch(100)*/learner, labels);
    classifier->setL2Regularizer(l2regularizer);

    PolicyPtr learnedPolicy = Policy::createGreedy(ActionValueFunction::createScores(classifier));
    PolicyPtr learnerPolicy = Policy::createClassificationExampleCreator(learnedPolicy, classifier);
    
    return std::make_pair(learnerPolicy, learnedPolicy);
  }
  
  virtual CRAlgorithmPtr createCRAlgorithm(LabeledContentGraphPtr graph, size_t begin, size_t end)
    {return iterativeClassificationCRAlgorithm(graph, begin, end);}
};

static std::string allResults;

static std::ofstream resultsFile("results.txt", std::ios::app);

void testAlgorithm(GraphLabelingAlgorithm& algorithm, const std::string& name, const std::vector<LabeledContentGraphPtr>& trainGraph,
                     const std::vector<LabeledContentGraph::LabelsFold>& testGraph)
{
  std::cout << "Testing Algorithm " << name << std::endl;
  ScalarRandomVariableStatisticsPtr trainAccuracy = new ScalarRandomVariableStatistics("trainAccuracy");
  ScalarRandomVariableStatisticsPtr testAccuracy = new ScalarRandomVariableStatistics("testAccuracy");
  algorithm.crossValidate(trainGraph, testGraph, trainAccuracy, testAccuracy);
  std::string results = name + " => Train Accuracy: " + cralgo::toString(trainAccuracy->getMean() * 100) + " Test Accuracy: " + cralgo::toString(testAccuracy->getMean() * 100);
  
  std::cout << results << std::endl;
  allResults += results + "\n";
  resultsFile << results << std::endl;
}

int main(int argc, char* argv[])
{
  std::cout << "HO4 ! " << std::endl;

  static const std::string basename = "/Users/francis/Projets/CRAlgo/trunk/test/GraphLabeling/data/cora/cora";
  FeatureDictionaryPtr featuresDictionary = new FeatureDictionary("features");
  StringDictionaryPtr labelsDictionary = new StringDictionary();

  std::cout << "Parsing graph..." << std::endl;
  LabeledContentGraphPtr graph = LabeledContentGraph::parseGetoorGraph(basename + ".content", basename + ".cites", featuresDictionary, labelsDictionary);
  if (!graph)
    return 1;

  std::cout << graph->getNumNodes() << " nodes, " << graph->getNumLinks() << " links, "
    << featuresDictionary->getNumFeatures() << " features, " << labelsDictionary->getNumElements() << " classes." << std::endl;
  std::cout << *labelsDictionary << std::endl;
  
  resultsFile << std::endl << graph->getNumNodes() << " nodes, " << graph->getNumLinks() << " links, "
    << featuresDictionary->getNumFeatures() << " features, " << labelsDictionary->getNumElements() << " classes." << std::endl << std::endl;
  
  std::cout << "Splitting graph..." << std::endl;
  std::vector<LabeledContentGraphPtr> trainGraphs;
  std::vector<LabeledContentGraph::LabelsFold> testGraphs;
  graph->splitRandomly(10, trainGraphs, testGraphs);
  
  for (size_t i = 0; i < trainGraphs.size(); ++i)
  {
    std::cout << "Fold " << i << ": "
      << trainGraphs[i]->getNumNodes() << " train nodes, "
      << trainGraphs[i]->getNumLinks() << " train links, "
      << testGraphs[i].foldEnd - testGraphs[i].foldBegin << " test nodes."
      << std::endl;
  }
  
//  ContentOnlyGraphLabelingAlgorithm contentOnly;
//  testAlgorithm(contentOnly, "Content Only", trainGraphs, testGraphs);

  for (int i = 6; i <= 10; ++i)
  {
    double regularizer = (double)i;

/*    ContentOnlyGraphLabelingAlgorithm contentOnly;
    contentOnly.l2regularizer = regularizer;
    testAlgorithm(contentOnly, "Content Only with reg = " + cralgo::toString(regularizer), trainGraphs, testGraphs);

    PerfectContextAndContentGraphLabelingAlgorithm perfectContext;
    perfectContext.l2regularizer = regularizer;
    testAlgorithm(perfectContext, "Perfect Context with reg " + cralgo::toString(regularizer), trainGraphs, testGraphs);

    IterativeClassificationGraphLabelingAlgorithm iterativeClassification;
    iterativeClassification.l2regularizer = regularizer;
    testAlgorithm(iterativeClassification, "Iterative Classification with reg " + cralgo::toString(regularizer), trainGraphs, testGraphs);*/
    
/*    GibbsSamplingGraphLabelingAlgorithm gibbsProb;
    gibbsProb.l2regularizer = regularizer;
    testAlgorithm(gibbsProb, "Gibbs" + cralgo::toString(regularizer), trainGraphs, testGraphs);*/
  
    CRIterativeClassificationGraphLabelingAlgorithm crIterative;
    crIterative.l2regularizer = regularizer;
    testAlgorithm(crIterative, "CR-Iterative Classification with Maxent reg = " + cralgo::toString(regularizer), trainGraphs, testGraphs);
  }


  std::cout << std::endl << std::endl << std::endl;
  std::cout << allResults << std::endl;
  return 0;
}

