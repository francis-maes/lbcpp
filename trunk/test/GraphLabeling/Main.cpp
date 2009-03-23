#include "GeneratedCode/LabeledContentGraph.lh"
using namespace cralgo;

class GraphLabelingAlgorithm
{
public:
  virtual ~GraphLabelingAlgorithm() {}
  
  virtual void reset(StringDictionaryPtr labels) = 0;
  virtual void train(LabeledContentGraphPtr graph) = 0;
  virtual double evaluate(LabeledContentGraphPtr graph) = 0;
  
  void crossValidate(const std::vector<LabeledContentGraphPtr>& trainGraphs,
                     const std::vector<LabeledContentGraphPtr>& testGraphs, 
                     ScalarRandomVariableStatisticsPtr trainAccuracy,
                     ScalarRandomVariableStatisticsPtr testAccuracy)
  {
    assert(trainGraphs.size() == testGraphs.size());
    for (size_t i = 0; i < trainGraphs.size(); ++i)
    {
      std::cout << "CROSS-VALIDATION FOLD " << (i+1) << " / " << trainGraphs.size() << std::endl;
      reset(trainGraphs[i]->getLabelDictionary());
      train(trainGraphs[i]);
      trainAccuracy->push(evaluate(trainGraphs[i]));
      testAccuracy->push(evaluate(testGraphs[i]));
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
    
  virtual double evaluate(LabeledContentGraphPtr graph)
  {
    size_t correct = 0;
    for (size_t i = 0; i < graph->getNumNodes(); ++i)
      if (classifier->predict(getNodeFeatures(graph, i)) == graph->getLabel(i))
        ++correct;
    return correct / (double)graph->getNumNodes();
  }

protected:
  ClassifierPtr classifier;
  
  void trainClassifier(ClassifierPtr classifier, const std::vector<ClassificationExample>& examples)
  {
    for (size_t i = 0; i < 10; ++i)
    {
      std::cout << classifier->evaluateAccuracy(examples) << " ";
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
//    {return graph->nodeAndNeighborhoodLabelsFrequencyFeatures(nodeIndex);}
    {return graph->nodeFeatures(nodeIndex, 1, 1, false);}

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
  
  enum {maxIterations = 100};
    
  virtual double evaluate(LabeledContentGraphPtr graph)
  {
    size_t numNodes = graph->getNumNodes();
    
    LabeledContentGraphPtr predictedGraph 
      = new LabeledContentGraph(graph->getContentGraph(), new LabelSequence(graph->getLabelDictionary(), numNodes));
  
    // initial predictions
    for (size_t i = 0; i < numNodes; ++i)
      predictedGraph->setLabel(i, initialClassifier->predict(graph->getNode(i)));
 
    // iterative classification
    LabelSequencePtr previousLabels = new LabelSequence(*predictedGraph->getLabels());
    for (size_t i = 0; i < maxIterations; ++i)
    {
      // label each node in a randomly sampled order
      std::vector<size_t> order;
      Random::getInstance().sampleOrder(numNodes, order);
      for (size_t j = 0; j < order.size(); ++j)
      {
        size_t nodeIndex = order[j];
        predictedGraph->setLabel(nodeIndex, classifier->predict(getNodeFeatures(predictedGraph, nodeIndex)));
      }
       
       // stopping criterion
      size_t numCommon = previousLabels->numberOfLabelsInCommonWith(predictedGraph->getLabels());
      double changeRatio = 1.0 - numCommon / (double)numNodes;
      std::cout << "ITERATION " << i << " NumCommon = " << numCommon << " changeRatio = " << changeRatio
          << " accuracy = " << predictedGraph->getLabels()->numberOfLabelsInCommonWith(graph->getLabels()) / (double)numNodes << std::endl;
      if (changeRatio < 0.0001)
        break;
      previousLabels = new LabelSequence(*predictedGraph->getLabels());
    }
    
    return predictedGraph->getLabels()->numberOfLabelsInCommonWith(graph->getLabels()) / (double)numNodes;
  }
  
private:
  ClassifierPtr initialClassifier;  
};


static std::string allResults;

void testAlgorithm(GraphLabelingAlgorithm& algorithm, const std::string& name, const std::vector<LabeledContentGraphPtr>& trainGraphs,
                     const std::vector<LabeledContentGraphPtr>& testGraphs)
{
  std::cout << "Testing Algorithm " << name << std::endl;
  ScalarRandomVariableStatisticsPtr trainAccuracy = new ScalarRandomVariableStatistics("trainAccuracy");
  ScalarRandomVariableStatisticsPtr testAccuracy = new ScalarRandomVariableStatistics("testAccuracy");
  algorithm.crossValidate(trainGraphs, testGraphs, trainAccuracy, testAccuracy);
  std::string results = name + " => Train Accuracy: " + cralgo::toString(trainAccuracy->getMean() * 100) + " Test Accuracy: " + cralgo::toString(testAccuracy->getMean() * 100);
  
  std::cout << results << std::endl;
  allResults += results + "\n";
}

int main(int argc, char* argv[])
{
  std::cout << "HO2 ! " << std::endl;

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
  
  std::cout << "Splitting graph..." << std::endl;
  std::vector<LabeledContentGraphPtr> trainGraphs, testGraphs;
  graph->splitRandomly(10, trainGraphs, testGraphs);
  
  for (size_t i = 0; i < trainGraphs.size(); ++i)
  {
    std::cout << "Fold " << i << ": "
      << trainGraphs[i]->getNumNodes() << " train nodes, "
      << trainGraphs[i]->getNumLinks() << " train links, "
      << testGraphs[i]->getNumNodes() << " test nodes, "
      << testGraphs[i]->getNumLinks() << " test links."
      << std::endl;
  }
  
//  ContentOnlyGraphLabelingAlgorithm contentOnly;
//  testAlgorithm(contentOnly, "Content Only", trainGraphs, testGraphs);

  for (int i = 0; i < 10; i += 2)
  {
    double regularizer = (double)i;

  /*  ContentOnlyGraphLabelingAlgorithm contentOnly;
    contentOnly.l2regularizer = regularizer;
    testAlgorithm(contentOnly, "Content Only with reg = " + cralgo::toString(regularizer), trainGraphs, testGraphs);
*/
    PerfectContextAndContentGraphLabelingAlgorithm perfectContext;
    perfectContext.l2regularizer = regularizer;
    testAlgorithm(perfectContext, "Perfect Context with reg " + cralgo::toString(regularizer), trainGraphs, testGraphs);

    IterativeClassificationGraphLabelingAlgorithm iterativeClassification;
    iterativeClassification.l2regularizer = regularizer;
    testAlgorithm(iterativeClassification, "Iterative Classification with reg " + cralgo::toString(regularizer), trainGraphs, testGraphs);
  }

  std::cout << std::endl << std::endl << std::endl;
  std::cout << allResults << std::endl;
  return 0;
}

