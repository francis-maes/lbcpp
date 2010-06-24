/*-----------------------------------------.---------------------------------.
| Filename: IterativeClassificationAlgo...h| Iterative Classification        |
| Author  : Francis Maes                   |   Algorithm                     |
| Started : 26/03/2009 17:01               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef GRAPH_LABELING_ALGORITHM_ITERATIVE_CLASSIFICATION_H_
# define GRAPH_LABELING_ALGORITHM_ITERATIVE_CLASSIFICATION_H_

# include "ClassifierBasedGraphLabelingAlgorithm.h"

namespace lbcpp
{

class IterativeClassificationGraphLabelingAlgorithm : public ClassifierBasedGraphLabelingAlgorithm
{
public:
  virtual FeatureGeneratorPtr getNodeFeatures(LabeledContentGraphPtr graph, size_t nodeIndex)
    {return graph->nodeAndNeighborhoodLabelsFrequencyFeatures(nodeIndex);}

  virtual void reset(FeatureDictionaryPtr labels)
    {initialClassifier = createClassifier(labels); classifier = createClassifier(labels);}

  virtual void train(LabeledContentGraphPtr graph)
  {
    VectorObjectContainerPtr examples = new VectorObjectContainer("ClassificationExample");
    VectorObjectContainerPtr examplesWithContext = new VectorObjectContainer("ClassificationExample");

    examples->reserve(graph->getNumNodes());
    examplesWithContext->reserve(graph->getNumNodes());
    for (size_t i = 0; i < graph->getNumNodes(); ++i)
    {
      examples->append(new ClassificationExample(graph->getNode(i), graph->getLabel(i)));
      examplesWithContext->append(new ClassificationExample(getNodeFeatures(graph, i), graph->getLabel(i)));
    }
    trainClassifier(initialClassifier, examples);
    trainClassifier(classifier, examplesWithContext);
  }
  
  enum {maxInferencePasses = 100};

  virtual double evaluate(LabeledContentGraphPtr graph, size_t begin, size_t end, LabeledContentGraphPtr res = LabeledContentGraphPtr())
  {
    jassert(end > begin);
    if (!res)
      res = new LabeledContentGraph(graph->getContentGraph(), new LabelSequence(*graph->getLabels()));
    makeInitialPredictions(res, begin, end);
    return iterativeClassification(graph, res, begin, end, maxInferencePasses);
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
    jassert(end > begin);
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
      RandomGenerator::getInstance().sampleOrder(begin, end, order);
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

}; /* namespace lbcpp */

#endif // !GRAPH_LABELING_ALGORITHM_ITERATIVE_CLASSIFICATION_H_
