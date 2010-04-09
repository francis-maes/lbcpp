#include "GeneratedCode/LabeledContentGraph.lh"
#include "GeneratedCode/crIterativeClassification.lh"
#include "GeneratedCode/crOrderFreeGraphLabeling.lh"

#include "GraphLabelingAlgorithm/ClassifierBasedGraphLabelingAlgorithm.h"
#include "GraphLabelingAlgorithm/IterativeClassificationAlgorithm.h"
#include "GraphLabelingAlgorithm/GibbsSamplingAlgorithm.h"
#include "GraphLabelingAlgorithm/StackedGraphLabelingAlgorithm.h"
#include "GraphLabelingAlgorithm/CRAlgorithmGraphLabelingAlgorithm.h"
  
#include <fstream>
using namespace lbcpp;

class OnePassOrderFreeGraphLabelingAlgorithm : public CRAlgorithmGraphLabelingAlgorithm
{
public:
  virtual std::pair<PolicyPtr, PolicyPtr> createInitialPolicies(StringDictionaryPtr labels)
  {
    // CRANK - PREDICTED 
    IterationFunctionPtr learningRate = invLinearIterationFunction(10, 10000);
    RankerPtr ranker = largeMarginAllPairsLinearRanker(stochasticDescentLearner(learningRate));
      //largeMarginBestAgainstAllLinearRanker
     // largeMarginMostViolatedPairLinearRanker
    
    PolicyPtr learnedPolicy = greedyPolicy(predictedActionValues(ranker));
    PolicyPtr learnerPolicy = rankingExamplesCreatorPolicy(learnedPolicy, ranker);
    return std::make_pair(learnedPolicy, learnerPolicy);
  }

  virtual CRAlgorithmPtr createCRAlgorithm(LabeledContentGraphPtr graph, size_t begin, size_t end)
    {return onePassOrderFreeGraphLabeling(graph, begin, end);}
};

class CRIterativeClassificationGraphLabelingAlgorithm : public CRAlgorithmGraphLabelingAlgorithm
{
public:
  CRIterativeClassificationGraphLabelingAlgorithm() : epsilon(0.0), temperature(0.0), predicted(true), probabilistic(false), oneClassifierPerPass(false) {}
  
  double epsilon;
  double temperature;
  bool predicted;
  bool probabilistic;
  
  bool oneClassifierPerPass;
  
  virtual std::pair<PolicyPtr, PolicyPtr> createInitialPolicies(FeatureDictionaryPtr labels)
  {
    // Classifier Maxent
    IterationFunctionPtr learningRate = constantIterationFunction(1.0);//InvLinear(26, 10000);
    GradientBasedLearnerPtr learner = stochasticDescentLearner(learningRate);
    GradientBasedClassifierPtr classifier = maximumEntropyClassifier(/*learner->stochasticToBatch(100)*/learner, labels);
    classifier->setL2Regularizer(l2regularizer);

    ActionValueFunctionPtr learnedScores = predictedActionValues(classifier);
    PolicyPtr learnedPolicy = greedyPolicy(learnedScores);
    PolicyPtr explorationPolicy = learnedPolicy;
    if (probabilistic)
      explorationPolicy = stochasticPolicy(probabilitiesActionValues(classifier));
    else if (temperature)
      explorationPolicy = gibbsGreedyPolicy(learnedScores, constantIterationFunction(temperature));
    else
      {
        explorationPolicy = predicted ? learnedPolicy : greedyPolicy(chooseActionValues());
        if (epsilon)
          explorationPolicy = epsilonGreedyPolicy(explorationPolicy, constantIterationFunction(epsilon));
      }
      
    PolicyPtr learnerPolicy = classificationExamplesCreatorPolicy(explorationPolicy, classifier);
    return std::make_pair(learnerPolicy, learnedPolicy);
  }
  
  virtual CRAlgorithmPtr createCRAlgorithm(LabeledContentGraphPtr graph, size_t begin, size_t end)
    {return crIterativeClassification(graph, begin, end, 5, oneClassifierPerPass);}
};

static std::ostream* resultsOutputFile = NULL;
static String allResults;

#ifdef JUCE_WIN32
inline double getTimeInSeconds() {return 0;} // FIXME
#else
#include <sys/time.h>
#include <unistd.h>
inline double getTimeInSeconds()
{
  timeval t;
  if (gettimeofday(&t, 0))
      return 0.0;
  return (double)t.tv_sec + t.tv_usec / 1000000.0;
}
#endif

double testAlgorithm(GraphLabelingAlgorithm& algorithm, const String& name, const std::vector<LabeledContentGraphPtr>& trainGraph,
                     const std::vector<LabeledContentGraph::LabelsFold>& testGraph, bool profile)
{
  FeatureDictionaryPtr labels = FeatureDictionaryManager::getInstance().getFlatVectorDictionary(trainGraph[0]->getLabelDictionary());
  std::cout << "Testing Algorithm " << name << std::endl;
  if (profile)
  {
    algorithm.setL2Regularizer(0.0);
    algorithm.reset(labels);
    double t0 = getTimeInSeconds();
    algorithm.train(trainGraph[0]);
    double t1 = getTimeInSeconds();
    algorithm.evaluate(testGraph[0].graph, testGraph[0].foldBegin, testGraph[0].foldEnd);
    double t2 = getTimeInSeconds();
    (*resultsOutputFile) << name << " " << (t1 - t0) << " " << (t2 - t1) << std::endl;
    return t2 - t1;
  }
  else
  {
    double bestTestAccuracy = 0.0;
    int iterationsWithoutImprovement = 0;
    for (int i = 0; i < 16; i += 16) // tmp (was i < 16; ++i)
    {
      double regularizer = (double)i;
      ScalarVariableStatisticsPtr trainAccuracy = new ScalarVariableStatistics("trainAccuracy");
      ScalarVariableStatisticsPtr testAccuracy = new ScalarVariableStatistics("testAccuracy");
      algorithm.setL2Regularizer(regularizer);
      algorithm.crossValidate(trainGraph, testGraph, trainAccuracy, testAccuracy);
      double score = testAccuracy->getMean();
      String results = name + T(" reg = ") + lbcpp::toString(regularizer) +
          T(" => Train Accuracy: ") + lbcpp::toString(trainAccuracy->getMean() * 100) +
          T(" Test Accuracy: ") + lbcpp::toString(score * 100);
      if (score > bestTestAccuracy)
      {
        iterationsWithoutImprovement = 0;
        bestTestAccuracy = score;
      }
      else
      {
        ++iterationsWithoutImprovement;
        if (iterationsWithoutImprovement == 5)
          break;
      }
      std::cout << results << std::endl;
      allResults += results + T("\n");
      (*resultsOutputFile) << "+" << results << std::endl;
    }
    (*resultsOutputFile) << "==> " << name << " " << bestTestAccuracy << std::endl;
    return bestTestAccuracy;
  }
}

void testAllAlgorithms( const std::vector<LabeledContentGraphPtr>& trainGraphs,
                        const std::vector<LabeledContentGraph::LabelsFold>& testGraphs,
                        bool profile)
{
  ContentOnlyGraphLabelingAlgorithm contentOnly;
  testAlgorithm(contentOnly, "CO", trainGraphs, testGraphs, profile);

  PerfectContextAndContentGraphLabelingAlgorithm perfectContext;
  testAlgorithm(perfectContext, "OPT", trainGraphs, testGraphs, profile);

  IterativeClassificationGraphLabelingAlgorithm iterativeClassification;
  testAlgorithm(iterativeClassification, "ICA", trainGraphs, testGraphs, profile);

//  GibbsSamplingGraphLabelingAlgorithm gibbsProb;
//  testAlgorithm(gibbsProb, "GS", trainGraphs, testGraphs, profile);

  CRIterativeClassificationGraphLabelingAlgorithm crIterative;
  testAlgorithm(crIterative, "CRICA", trainGraphs, testGraphs, profile);

  crIterative.probabilistic = true;
  testAlgorithm(crIterative, "CRICA-PROB", trainGraphs, testGraphs, profile);

  crIterative.oneClassifierPerPass = true;
  testAlgorithm(crIterative, "CRICA-PROB-CPP", trainGraphs, testGraphs, profile);

  crIterative.probabilistic = false;
  crIterative.oneClassifierPerPass = true;
  testAlgorithm(crIterative, "CRICA-CPP", trainGraphs, testGraphs, profile);

  StackedGraphLabelingAlgorithm stacked2(&contentOnly);
  testAlgorithm(stacked2, "STACK2", trainGraphs, testGraphs, profile);
  jassert(&stacked2.getBaseAlgorithm() == &contentOnly);

  StackedGraphLabelingAlgorithm stacked3(&stacked2);
  jassert(&stacked3.getBaseAlgorithm() == &stacked2);
  testAlgorithm(stacked3, "STACK3", trainGraphs, testGraphs, profile);

/*  StackedGraphLabelingAlgorithm stacked4(&stacked3);
  jassert(&stacked4.getBaseAlgorithm() == &stacked3);
  testAlgorithm(stacked4, "STACK4", trainGraphs, testGraphs, profile);

  StackedGraphLabelingAlgorithm stacked5(&stacked4);
  jassert(&stacked5.getBaseAlgorithm() == &stacked4);
  testAlgorithm(stacked5, "STACK5", trainGraphs, testGraphs, profile);*/
}

void displayFolds(const std::vector<LabeledContentGraphPtr>& trainGraphs,
                  const std::vector<LabeledContentGraph::LabelsFold>& testGraphs)
{
  jassert(trainGraphs.size() == testGraphs.size());
  for (size_t i = 0; i < trainGraphs.size(); ++i)
  {
    std::cout << "Fold " << i << ": "
      << trainGraphs[i]->getNumNodes() << " train nodes, "
      << trainGraphs[i]->getNumLinks() << " train links, "
      << testGraphs[i].foldEnd - testGraphs[i].foldBegin << " test nodes."
      << std::endl;
  }
}

void displayGraphInfo(std::ostream& ostr, LabeledContentGraphPtr graph, FeatureDictionaryPtr featuresDictionary, StringDictionaryPtr labelsDictionary)
{
  ostr << graph->getNumNodes() << " nodes, " << graph->getNumLinks() << " links, "
    << featuresDictionary->getNumFeatures() << " features, " << labelsDictionary->getNumElements() << " classes." << std::endl;
}

////////// Sequences ////////////

class HierarchicalToFlatFeatureMap : public Object
{
public:
  HierarchicalToFlatFeatureMap(const String& name = "HierarchicalToFlatFeatureMap")
    {dictionary = new FeatureDictionary(name, new StringDictionary(), StringDictionaryPtr());}
  
  int getIndex(const std::vector<size_t>& path) const
  {
    FeatureMap::const_iterator it = m.find(path);
    return it == m.end() ? -1 : (int)it->second;
  }
  
  size_t add(const std::vector<size_t>& path, const String& flatName)
  {
    FeatureMap::const_iterator it = m.find(path);
    if (it == m.end())
    {
      jassert(dictionary->getFeatures()->getIndex(flatName) < 0);
      return (m[path] = dictionary->getFeatures()->add(flatName));
    }
    else
      return it->second;
  }

  SparseVectorPtr toFlatVector(FeatureGeneratorPtr featureGenerator)
  {
    lbcpp::ReferenceCountedObjectPtr<ConversionVisitor> visitor = new ConversionVisitor(*this);
    visitor->res = new SparseVector(dictionary);
    featureGenerator->accept(visitor);
    return visitor->res;
  }
  
  struct FeatureGeneratorToFlatSparseVectorFunction : public ObjectFunction
  {
    FeatureGeneratorToFlatSparseVectorFunction(HierarchicalToFlatFeatureMap& map) 
      : map(map) {}
    
    HierarchicalToFlatFeatureMap& map;
    
    virtual String getOutputClassName(const String& inputClassName) const
      {return T("SparseVector");}

    virtual ObjectPtr function(ObjectPtr object) const
    {
      FeatureGeneratorPtr featureGenerator = object.dynamicCast<FeatureGenerator>();
      jassert(featureGenerator);
      return map.toFlatVector(featureGenerator);
    }
  };
  
  ObjectFunctionPtr createFeatureGeneratorToFlatSparseVectorFunction()
    {return new FeatureGeneratorToFlatSparseVectorFunction(*this);}

private:
  typedef std::map< std::vector<size_t>, size_t > FeatureMap;
  FeatureMap m;
  FeatureDictionaryPtr dictionary;
  
  struct ConversionVisitor : public PathBasedFeatureVisitor
  {
    ConversionVisitor(HierarchicalToFlatFeatureMap& map) 
      : map(map) {}
    
    HierarchicalToFlatFeatureMap& map;
    SparseVectorPtr res;

    virtual void featureSense(const std::vector<size_t>& path, const String& name, double value)
      {res->set(map.add(path, lbcpp::toString(map.m.size()) /* name */), value);}
  };
};


class SequenceExample : public LearningExample
{
public:
  SequenceExample(const std::vector<FeatureGeneratorPtr>& content, const std::vector<size_t>& labels)
    : content(content), labels(labels) {}
    
  size_t getLength() const
    {return content.size();}
  
  void addToGraph(LabeledContentGraphPtr graph, size_t inputHalfWindowSize = 0, int markovOrder = 1)
  {
    jassert(content.size() == labels.size());
    ContentGraphPtr contentGraph = graph->getContentGraph();
    LabelSequencePtr graphLabels = graph->getLabels();
    size_t startNodeIndex = contentGraph->getNumNodes();
    for (size_t i = 0; i < content.size(); ++i)
    {
      contentGraph->addNode(sparseVectorWindowFeatures(content, i, inputHalfWindowSize));
      graphLabels->append(labels[i]);
      for (int j = 1; j <= markovOrder; ++j)
      {
        int d = (int)i - j;
        if (d >= 0)
          contentGraph->addLink(startNodeIndex + d, startNodeIndex + i);
      }
    }
  }
  
  std::vector<FeatureGeneratorPtr> content;
  std::vector<size_t> labels;
};

class SequenceExamplesParser : public LearningDataObjectParser
{
public:
  SequenceExamplesParser(const File& file, FeatureDictionaryPtr features, StringDictionaryPtr labels)
    : LearningDataObjectParser(file, features), labels(labels) {}
    
  StringDictionaryPtr labels;

  virtual void parseBegin()
  {
    currentContent.clear();
    currentLabels.clear();
  }

  virtual bool parseDataLine(const std::vector<String>& columns)
  {
    jassert(columns.size());
    String label = columns[0];
    SparseVectorPtr features;
    if (!parseFeatureList(columns, 1, features))
      return false;
    currentContent.push_back(features);
    currentLabels.push_back(labels->add(label));
    return true;
  }

  virtual bool parseEmptyLine()
  {
    if (currentContent.size())
    {
      setResult(ObjectPtr(new SequenceExample(currentContent, currentLabels)));
      currentContent.clear();
      currentLabels.clear();
    }
    return true;
  }
  
private:
  std::vector<FeatureGeneratorPtr> currentContent;
  std::vector<size_t> currentLabels;
};

LabeledContentGraphPtr convertSequencesToGraph(ObjectContainerPtr sequences, StringDictionaryPtr labels, size_t inputHalfWindowSize = 0, int markovOrder = 1)
{
  LabeledContentGraphPtr res = new LabeledContentGraph(new ContentGraph(), new LabelSequence(labels));
  for (size_t i = 0; i < sequences->size(); ++i)
    sequences->getAndCast<SequenceExample>(i)->addToGraph(res, inputHalfWindowSize, markovOrder);
  return res;
}

void sequencesToClassificationDataSet(ObjectContainerPtr sequences, StringDictionaryPtr labels, size_t inputHalfWindowSize, const File& outputFile)
{
  static HierarchicalToFlatFeatureMap hierarchicalToFlatMap;
  
  ObjectConsumerPtr printer = classificationExamplesPrinter(outputFile, labels);

  for (size_t i = 0; i < sequences->size(); ++i)
  {
    lbcpp::ReferenceCountedObjectPtr<SequenceExample> example = sequences->getAndCast<SequenceExample>(i);
    for (size_t j = 0; j < example->getLength(); ++j)
      printer->consume(new ClassificationExample(
        hierarchicalToFlatMap.toFlatVector(sparseVectorWindowFeatures(example->content, j, inputHalfWindowSize)),
        example->labels[j]));
  }
}

int crossValidateAllOnSequences(int argc, char* argv[])
{
  if (argc < 4)
  {
    std::cerr << "Usage: " << argv[0] << " sequences.data numFolds resultsFile.txt" << std::endl;
    return 1;
  }
  String dataFileName = String(argv[1]);
  int numFolds = atoi(argv[2]);
  std::ofstream resultsFile(argv[3]);
  if (!resultsFile.is_open())
  {
    std::cerr << "Error: could not open file " << argv[3] << std::endl;
    return 1;
  }
  resultsOutputFile = &resultsFile;
  
  FeatureDictionaryPtr features = new FeatureDictionary("features");
  StringDictionaryPtr labels = new StringDictionary();

  // loading sequences
  std::cout << "Loading sequences..." << std::endl;
  ObjectStreamPtr sequencesParser = new SequenceExamplesParser(File::getCurrentWorkingDirectory().getChildFile(dataFileName), features, labels);
  ObjectContainerPtr allSequences = sequencesParser->load();
  
  size_t numElements = 0;
  for (size_t i = 0; i < allSequences->size(); ++i)
    numElements += allSequences->getAndCast<SequenceExample>(i)->getLength();
  std::cout << "Found " << allSequences->size() << " sequences with a total of " << numElements << " elements" << std::endl;

  std::cout << "Labels: " << labels->toString() << std::endl;
  std::cout << "Features: " << features->toString() << std::endl;

  for (size_t inputHalfWindowSize = 7; inputHalfWindowSize < 8; ++inputHalfWindowSize)
  {
    // make folds and convert to graphs
    std::vector<LabeledContentGraphPtr> trainGraphs;
    std::vector<LabeledContentGraph::LabelsFold> testGraphs;
    
    allSequences = allSequences->randomize();
    for (int i = 0; i < numFolds; ++i)
    {
      static const int markovOrder = 1;
      LabeledContentGraphPtr trainGraph = convertSequencesToGraph(allSequences->invFold(i, numFolds), labels, inputHalfWindowSize, markovOrder);
      LabeledContentGraphPtr testGraph = convertSequencesToGraph(allSequences->fold(i, numFolds), labels, inputHalfWindowSize, markovOrder);
      
      trainGraphs.push_back(trainGraph);
      LabeledContentGraph::LabelsFold test;
      test.graph = testGraph;
      test.foldBegin = 0;
      test.foldEnd = testGraph->getNumNodes();
      testGraphs.push_back(test);
    }

    // display folds
    displayFolds(trainGraphs, testGraphs);
    
    // test content only
    //ContentOnlyGraphLabelingAlgorithm contentOnly;
    //testAlgorithm(contentOnly, "CO ihws = " + lbcpp::toString(inputHalfWindowSize), trainGraphs, testGraphs, false);

    // run all algorithms
    testAllAlgorithms(trainGraphs, testGraphs, false);
  }
  
  std::cout << std::endl << std::endl << std::endl;
  std::cout << allResults << std::endl;
  return 0;
}

////////// Sequences ////////////

int crossValidateAll(int argc, char* argv[])
{
  if (argc < 6)
  {
    std::cerr << "Usage: " << argv[0] << " data.content data.links numFolds removeTrainTestLinks resultsFile.txt" << std::endl;
    return 1;
  }
  File contentFile = File::getCurrentWorkingDirectory().getChildFile(argv[1]);
  File linkFile = File::getCurrentWorkingDirectory().getChildFile(argv[2]);
  int numFolds = atoi(argv[3]);
  bool removeTrainTestLinks = String(argv[4]) == T("true");
  std::ofstream resultsFile(argv[5]);
  if (!resultsFile.is_open())
  {
    std::cerr << "Error: could not open file " << argv[5] << std::endl;
    return 1;
  }
  resultsOutputFile = &resultsFile;

  FeatureDictionaryPtr featuresDictionary = new FeatureDictionary("features");
  StringDictionaryPtr labelsDictionary = new StringDictionary();

  std::cout << "Parsing graph..." << std::endl;
  LabeledContentGraphPtr graph = LabeledContentGraph::parseGetoorGraph(contentFile, linkFile, featuresDictionary, labelsDictionary);
  if (!graph)
    return 1;
  displayGraphInfo(std::cout, graph, featuresDictionary, labelsDictionary);  
  std::cout << *labelsDictionary << std::endl;
  displayGraphInfo(resultsFile, graph, featuresDictionary, labelsDictionary);

  std::cout << "Splitting graph..." << std::endl;
  std::vector<LabeledContentGraphPtr> trainGraphs;
  std::vector<LabeledContentGraph::LabelsFold> testGraphs;
  while (trainGraphs.size() < 10)
    graph->randomizeOrder()->makeFolds(numFolds, removeTrainTestLinks, trainGraphs, testGraphs);
  
  displayFolds(trainGraphs, testGraphs);
  testAllAlgorithms(trainGraphs, testGraphs, false);

  std::cout << std::endl << std::endl << std::endl;
  std::cout << allResults << std::endl;
  return 0;
}

int trainTestFixedTrainSize(int argc, char* argv[], bool profile)
{
  if (argc < 6)
  {
    std::cerr << "Usage: " << argv[0] << " data.content data.links trainPercentage removeTrainTestLinks resultsFile.txt" << std::endl;
    return 1;
  }
  File contentFile = File::getCurrentWorkingDirectory().getChildFile(argv[1]);
  File linkFile = File::getCurrentWorkingDirectory().getChildFile(argv[2]);
  int trainPercentage = atoi(argv[3]);
  if (trainPercentage <= 0 || trainPercentage >= 100)
  {
    std::cerr << "Train percentage must be in interval ]0, 100[" << std::endl;
    return 1;
  }
  bool removeTrainTestLinks = String(argv[4]) == T("true");
  std::ofstream resultsFile(argv[5]);
  if (!resultsFile.is_open())
  {
    std::cerr << "Error: could not open file " << argv[5] << std::endl;
    return 1;
  }
  resultsOutputFile = &resultsFile;

  FeatureDictionaryPtr featuresDictionary = new FeatureDictionary("features");
  StringDictionaryPtr labelsDictionary = new StringDictionary();

  std::cout << "Parsing graph..." << std::endl;
  LabeledContentGraphPtr graph = LabeledContentGraph::parseGetoorGraph(contentFile, linkFile, featuresDictionary, labelsDictionary);
  if (!graph)
    return 1;
  displayGraphInfo(std::cout, graph, featuresDictionary, labelsDictionary);  
  std::cout << *labelsDictionary << std::endl;
  displayGraphInfo(resultsFile, graph, featuresDictionary, labelsDictionary);

  std::cout << "Splitting graph..." << std::endl;
  std::vector<LabeledContentGraphPtr> trainGraphs;
  std::vector<LabeledContentGraph::LabelsFold> testGraphs;
  while (trainGraphs.size() < 10)
  {
    std::pair<LabeledContentGraphPtr, LabeledContentGraph::LabelsFold> fold = 
      graph->randomizeOrder()->makeFold((graph->getNumNodes() * trainPercentage) / 100, graph->getNumNodes(), removeTrainTestLinks);
    trainGraphs.push_back(fold.first);
    testGraphs.push_back(fold.second);
  }
  
  displayFolds(trainGraphs, testGraphs);
  testAllAlgorithms(trainGraphs, testGraphs, profile);

  std::cout << std::endl << std::endl << std::endl;
  std::cout << allResults << std::endl;
  return 0;
}

int testUniformNoise(int argc, char* argv[])
{
  if (argc < 6)
  {
    std::cerr << "Usage: " << argv[0] << " data.content data.links trainPercentage removeTrainTestLinks resultsFile.txt" << std::endl;
    return 1;
  }
  File contentFile = File::getCurrentWorkingDirectory().getChildFile(argv[1]);
  File linkFile = File::getCurrentWorkingDirectory().getChildFile(argv[2]);
  //int numFolds = atoi(argv[3]);
  int trainPercentage = atoi(argv[3]);
  bool removeTrainTestLinks = String(argv[4]) == T("true");
  std::ofstream resultsFile(argv[5]);
  if (!resultsFile.is_open())
  {
    std::cerr << "Error: could not open file " << argv[5] << std::endl;
    return 1;
  }
  resultsOutputFile = &resultsFile;

  FeatureDictionaryPtr featuresDictionary = new FeatureDictionary("features");
  StringDictionaryPtr labelsDictionary = new StringDictionary();

  std::cout << "Parsing graph..." << std::endl;
  LabeledContentGraphPtr graph = LabeledContentGraph::parseGetoorGraph(contentFile, linkFile, featuresDictionary, labelsDictionary);
  if (!graph)
    return 1;
  displayGraphInfo(std::cout, graph, featuresDictionary, labelsDictionary);  
  std::cout << *labelsDictionary << std::endl;
  displayGraphInfo(resultsFile, graph, featuresDictionary, labelsDictionary);

/*  std::cout << "Splitting graph..." << std::endl;
  std::vector<LabeledContentGraphPtr> trainGraphs;
  std::vector<LabeledContentGraph::LabelsFold> testGraphs;
  while (trainGraphs.size() < 10)
    graph->randomizeOrder()->makeFolds(numFolds, removeTrainTestLinks, trainGraphs, testGraphs);*/
    
  std::cout << "Splitting graph..." << std::endl;
  std::vector<LabeledContentGraphPtr> trainGraphs;
  std::vector<LabeledContentGraph::LabelsFold> testGraphs;
  while (trainGraphs.size() < 10)
  {
    std::pair<LabeledContentGraphPtr, LabeledContentGraph::LabelsFold> fold = 
      graph->randomizeOrder()->makeFold((graph->getNumNodes() * trainPercentage) / 100, graph->getNumNodes(), removeTrainTestLinks);
    trainGraphs.push_back(fold.first);
    testGraphs.push_back(fold.second);
  }
  
  displayFolds(trainGraphs, testGraphs);

  for (int percentNoise = 0; percentNoise <= 100; percentNoise += 10)
  {
    CRIterativeClassificationGraphLabelingAlgorithm crIterative;
    crIterative.epsilon = percentNoise / 100.0;
    crIterative.predicted = false;
    testAlgorithm(crIterative, "SICA opt+uniform " + lbcpp::toString(percentNoise), trainGraphs, testGraphs, false);
    crIterative.predicted = true;
    testAlgorithm(crIterative, "SICA pred+uniform " + lbcpp::toString(percentNoise), trainGraphs, testGraphs, false);
    crIterative.oneClassifierPerPass = true;
    testAlgorithm(crIterative, "SICA-CPP pred+uniform " + lbcpp::toString(percentNoise), trainGraphs, testGraphs, false);
    crIterative.predicted = false;
    testAlgorithm(crIterative, "SICA-CPP opt+uniform " + lbcpp::toString(percentNoise), trainGraphs, testGraphs, false);
  }

  std::cout << std::endl << std::endl << std::endl;
  std::cout << allResults << std::endl;
  return 0;
}

int main(int argc, char* argv[])
{
  return crossValidateAllOnSequences(argc, argv);
//  return crossValidateAll(argc, argv);
//  return trainTestFixedTrainSize(argc, argv, true);
//  return testUniformNoise(argc, argv);
}
