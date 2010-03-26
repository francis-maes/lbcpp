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

namespace lbcpp
{

class ClassifierBasedGraphLabelingAlgorithm : public GraphLabelingAlgorithm
{
public:  
  virtual FeatureGeneratorPtr getNodeFeatures(LabeledContentGraphPtr graph, size_t nodeIndex) = 0;

  virtual ClassifierPtr createClassifier(StringDictionaryPtr labels)
  {
    IterationFunctionPtr learningRate = constantIterationFunction(1.0);//InvLinear(26, 10000);
    GradientBasedLearnerPtr learner = stochasticDescentLearner(learningRate);
    GradientBasedClassifierPtr res = maximumEntropyClassifier(/*learner->stochasticToBatch(100)*/learner, labels);
    res->setL2Regularizer(l2regularizer);
    return res;
  }

  virtual void reset(StringDictionaryPtr labels)
    {classifier = createClassifier(labels);}

  virtual void train(LabeledContentGraphPtr graph)
    {train(graph, graph);}

  virtual double evaluate(LabeledContentGraphPtr correctGraph, size_t begin, size_t end, LabeledContentGraphPtr res = LabeledContentGraphPtr())
    {return evaluateClassifier(correctGraph, correctGraph, begin, end, res);}

  enum
  {
    maxLearningIterations = 500,
    maxLearningIterationsWithoutImprovement = 5,
  };
  
protected:
  ClassifierPtr classifier;
  
  void train(LabeledContentGraphPtr predictedGraph, LabeledContentGraphPtr correctGraph)
  {
    jassert(predictedGraph->getNumNodes() == correctGraph->getNumNodes());
    VectorObjectContainerPtr examples = new VectorObjectContainer("ClassificationExample");
    
    examples->reserve(predictedGraph->getNumNodes());
    for (size_t i = 0; i < predictedGraph->getNumNodes(); ++i)
      examples->append(new ClassificationExample(getNodeFeatures(predictedGraph, i), correctGraph->getLabel(i)));
    trainClassifier(classifier, examples);
  }
    
  void trainClassifier(ClassifierPtr classifier, ObjectContainerPtr examples)
  {
    //std::cout << "Training classifier with " << examples->size() << " examples" << std::endl;
    size_t numIterationsWithoutImprovement = 0;
    double bestAccuracy = 0.0;
    for (size_t i = 0; i < maxLearningIterations; ++i)
    {    
      classifier->trainStochastic(examples);
      double accuracy = classifier->evaluateAccuracy(examples);
      std::cout << accuracy << " " << std::flush;
      if (accuracy > bestAccuracy)
      {
        bestAccuracy = accuracy;
        numIterationsWithoutImprovement = 0;
      }
      else
      {
        ++numIterationsWithoutImprovement;
        if (numIterationsWithoutImprovement >= maxLearningIterationsWithoutImprovement)
          break;
      }
    }
    
    GradientBasedClassifierPtr gradientBasedClassifier = classifier.dynamicCast<GradientBasedClassifier>();
    if (gradientBasedClassifier)
    {
      DenseVectorPtr parameters = gradientBasedClassifier->getParameters();
      if (parameters)
        std::cout << " => paramsL0 = " << parameters->l0norm() << " paramsL1 = " << parameters->l1norm() << " paramsL2 = " << parameters->l2norm() << std::endl;
    }  
    std::cout << std::endl;
  }
  
  double evaluateClassifier(LabeledContentGraphPtr featuresGraphs, LabeledContentGraphPtr correctGraph, size_t begin, size_t end, LabeledContentGraphPtr res = LabeledContentGraphPtr())
  {
    jassert(end > begin);
    size_t correct = 0;
    for (size_t i = begin; i < end; ++i)
    {
      size_t prediction = classifier->predict(getNodeFeatures(featuresGraphs, i));
      if (res)
        res->setLabel(i, prediction);
      if (prediction == correctGraph->getLabel(i))
        ++correct;
    }
    return correct / (double)(end - begin);
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
    {return graph->nodeAndNeighborhoodLabelsFrequencyFeatures(nodeIndex);}
//    {return graph->nodeFeatures(nodeIndex, 1, 1, false);}
};

}; /* namespace lbcpp */

#endif // !GRAPH_LABELING_ALGORITHM_CLASSIFIER_BASED_H_
