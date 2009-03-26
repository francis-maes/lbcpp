/*-----------------------------------------.---------------------------------.
| Filename: ClasifierBasedGraphLabeling...h| Classifier based                |
| Author  : Francis Maes                   |   Graph Labeling Algorithms     |
| Started : 26/03/2009 17:00               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef GRAPH_LABELING_ALGORITHM_CLASSIFIER_BASED_H_
# define GRAPH_LABELING_ALGORITHM_CLASSIFIER_BASED_H_

# include "GraphLabelingAlgorithm.h"

namespace cralgo
{

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

}; /* namespace cralgo */

#endif // !GRAPH_LABELING_ALGORITHM_CLASSIFIER_BASED_H_
