#include "GeneratedCode/LabeledContentGraph.lh"
#include "GeneratedCode/crIterativeClassification.lh"
#include "GeneratedCode/crOrderFreeGraphLabeling.lh"

#include "GraphLabelingAlgorithm/ClassifierBasedGraphLabelingAlgorithm.h"
#include "GraphLabelingAlgorithm/IterativeClassificationAlgorithm.h"
#include "GraphLabelingAlgorithm/GibbsSamplingAlgorithm.h"
#include "GraphLabelingAlgorithm/CRAlgorithmGraphLabelingAlgorithm.h"
  
#include <fstream>
using namespace cralgo;

class OnePassOrderFreeGraphLabelingAlgorithm : public CRAlgorithmGraphLabelingAlgorithm
{
public:
  virtual std::pair<PolicyPtr, PolicyPtr> createInitialPolicies(StringDictionaryPtr labels)
  {
    // CRANK - PREDICTED 
    IterationFunctionPtr learningRate = IterationFunction::createInvLinear(10, 10000);
    RankerPtr ranker = GradientBasedRanker::
      //createLargeMarginBestAgainstAllLinear
     // createLargeMarginMostViolatedPairLinear
      createLargeMarginAllPairsLinear
      (GradientBasedLearner::createStochasticDescent(learningRate));
    
    PolicyPtr learnedPolicy = Policy::createGreedy(ActionValueFunction::createPredictions(ranker));
    PolicyPtr learnerPolicy = Policy::createRankingExampleCreator(learnedPolicy, ranker);
    return std::make_pair(learnedPolicy, learnerPolicy);
  }

  virtual CRAlgorithmPtr createCRAlgorithm(LabeledContentGraphPtr graph, size_t begin, size_t end)
    {return onePassOrderFreeGraphLabeling(graph, begin, end);}
};

class CRIterativeClassificationGraphLabelingAlgorithm : public CRAlgorithmGraphLabelingAlgorithm
{
public:
  CRIterativeClassificationGraphLabelingAlgorithm() : l2regularizer(0.0), epsilon(0.0), temperature(0.0), probabilistic(false), oneClassifierPerPass(false) {}
  
  double l2regularizer;
  double epsilon;
  double temperature;
  bool probabilistic;
  
  bool oneClassifierPerPass;
  
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

    ActionValueFunctionPtr learnedScores = ActionValueFunction::createScores(classifier);
    PolicyPtr learnedPolicy = Policy::createGreedy(learnedScores);
    PolicyPtr explorationPolicy = learnedPolicy;
    if (probabilistic)
      explorationPolicy = Policy::createNonDeterministic(ActionValueFunction::createProbabilities(classifier));
    else if (temperature)
      explorationPolicy = Policy::createGibbsGreedy(learnedScores, IterationFunction::createConstant(temperature));
    else if (epsilon)
      explorationPolicy = learnedPolicy->epsilonGreedy(IterationFunction::createConstant(epsilon));
    else
      explorationPolicy = learnedPolicy;
      
    PolicyPtr learnerPolicy = Policy::createClassificationExampleCreator(explorationPolicy, classifier);
    return std::make_pair(learnerPolicy, learnedPolicy);
  }
  
  virtual CRAlgorithmPtr createCRAlgorithm(LabeledContentGraphPtr graph, size_t begin, size_t end)
    {return crIterativeClassification(graph, begin, end, 5, oneClassifierPerPass);}
};

static std::ostream* resultsOutputFile = NULL;
static std::string allResults;

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
  (*resultsOutputFile) << results << std::endl;
}

int main(int argc, char* argv[])
{
  // contentFile citeFile numFolds resultsFile
  if (argc < 5)
  {
    std::cerr << "Usage: " << argv[0] << " data.content data.links numFolds resultsFile.txt" << std::endl;
    return 1;
  }
  std::string contentFile = argv[1];
  std::string linkFile = argv[2];
  int numFolds = atoi(argv[3]);
  std::ofstream resultsFile(argv[4], std::ios::app);
  if (!resultsFile.is_open())
  {
    std::cerr << "Error: could not open file " << argv[4] << std::endl;
    return 1;
  }
  resultsOutputFile = &resultsFile;

//  static const std::string basename = "/Users/francis/Projets/CRAlgo/trunk/test/GraphLabeling/data/cora/cora";
  FeatureDictionaryPtr featuresDictionary = new FeatureDictionary("features");
  StringDictionaryPtr labelsDictionary = new StringDictionary();

  std::cout << "Parsing graph..." << std::endl;
  LabeledContentGraphPtr graph = LabeledContentGraph::parseGetoorGraph(contentFile, linkFile, featuresDictionary, labelsDictionary);
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
  graph->splitRandomly(numFolds, trainGraphs, testGraphs);
  
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
/*
  OnePassOrderFreeGraphLabelingAlgorithm onePassOrderFree;
  testAlgorithm(onePassOrderFree, "One pass order-free", trainGraphs, testGraphs);
  return 0;
  */
  
  for (int i = 0; i < 16; ++i)
  {
    double regularizer = (double)i;

    ContentOnlyGraphLabelingAlgorithm contentOnly;
    contentOnly.l2regularizer = regularizer;
    testAlgorithm(contentOnly, "CO" + cralgo::toString(regularizer), trainGraphs, testGraphs);

    PerfectContextAndContentGraphLabelingAlgorithm perfectContext;
    perfectContext.l2regularizer = regularizer;
    testAlgorithm(perfectContext, "OPT" + cralgo::toString(regularizer), trainGraphs, testGraphs);
   
    IterativeClassificationGraphLabelingAlgorithm iterativeClassification;
    iterativeClassification.l2regularizer = regularizer;
    testAlgorithm(iterativeClassification, "ICA " + cralgo::toString(regularizer), trainGraphs, testGraphs);
    
    GibbsSamplingGraphLabelingAlgorithm gibbsProb;
    gibbsProb.l2regularizer = regularizer;
    testAlgorithm(gibbsProb, "GS" + cralgo::toString(regularizer), trainGraphs, testGraphs);
  
      CRIterativeClassificationGraphLabelingAlgorithm crIterative;
      crIterative.l2regularizer = regularizer;
//      testAlgorithm(crIterative, "CR-Iterative Classification with Maxent reg " + cralgo::toString(regularizer) + " deterministic", trainGraphs, testGraphs);
      testAlgorithm(crIterative, "CRICA " + cralgo::toString(regularizer), trainGraphs, testGraphs);

      crIterative.probabilistic = true;
//      testAlgorithm(crIterative, "CR-Iterative Classification with Maxent reg " + cralgo::toString(regularizer) + " probabilistic", trainGraphs, testGraphs);
//      testAlgorithm(crIterative, "CR-Iterative Classification with Maxent reg " + cralgo::toString(regularizer) + " probabilistic", trainGraphs, testGraphs);
      testAlgorithm(crIterative, "CRICA-PROB" + cralgo::toString(regularizer), trainGraphs, testGraphs);

      crIterative.oneClassifierPerPass = true;
//      testAlgorithm(crIterative, "CR-Iterative Classification with Maxent reg " + cralgo::toString(regularizer) + " probabilistic oneClassifierPerPass", trainGraphs, testGraphs);
//      testAlgorithm(crIterative, "CR-Iterative Classification with Maxent reg " + cralgo::toString(regularizer) + " probabilistic oneClassifierPerPass", trainGraphs, testGraphs);
      testAlgorithm(crIterative, "CRICA-PROB-CPP" + cralgo::toString(regularizer), trainGraphs, testGraphs);

      crIterative.probabilistic = false;
      crIterative.oneClassifierPerPass = true;
//      testAlgorithm(crIterative, "CR-Iterative Classification with Maxent reg " + cralgo::toString(regularizer) + " probabilistic oneClassifierPerPass", trainGraphs, testGraphs);
//      testAlgorithm(crIterative, "CR-Iterative Classification with Maxent reg " + cralgo::toString(regularizer) + " probabilistic oneClassifierPerPass", trainGraphs, testGraphs);
      testAlgorithm(crIterative, "CRICA-CPP" + cralgo::toString(regularizer), trainGraphs, testGraphs);

/*
    double t = -1.5;//for (double t = -3; t <= 3; t += 0.5)
    {
      double temperature = pow(2.0, (double)t);
      CRIterativeClassificationGraphLabelingAlgorithm crIterative;
      crIterative.l2regularizer = regularizer;
      crIterative.temperature = temperature;
      testAlgorithm(crIterative, "CR-Iterative Classification with Maxent reg " + cralgo::toString(regularizer) + " temp " + cralgo::toString(temperature), trainGraphs, testGraphs);
    }*/
  }


  std::cout << std::endl << std::endl << std::endl;
  std::cout << allResults << std::endl;
  return 0;
}

