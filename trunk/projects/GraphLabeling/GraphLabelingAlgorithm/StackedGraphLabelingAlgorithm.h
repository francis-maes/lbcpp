/*-----------------------------------------.---------------------------------.
| Filename: StackedGraphLabelingAlgorithm.h| Stacked Learning Graph Labeling |
| Author  : Francis Maes                   |   Algorithm                     |
| Started : 28/03/2009 18:43               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef GRAPH_LABELING_ALGORITHM_STACKED_H_
# define GRAPH_LABELING_ALGORITHM_STACKED_H_

# include "ClassifierBasedGraphLabelingAlgorithm.h"

namespace lbcpp
{

class StackedGraphLabelingAlgorithm : public ClassifierBasedGraphLabelingAlgorithm
{
public:
  StackedGraphLabelingAlgorithm(GraphLabelingAlgorithm* baseAlgorithm)
    : baseAlgorithm(baseAlgorithm) {}

  virtual void setL2Regularizer(double reg)
  {
    baseAlgorithm->setL2Regularizer(reg);
    ClassifierBasedGraphLabelingAlgorithm::setL2Regularizer(reg);
  }

  enum
  {
    numFolds = 5
  };

  virtual FeatureGeneratorPtr getNodeFeatures(LabeledContentGraphPtr graph, size_t nodeIndex)
    {return graph->nodeAndNeighborhoodLabelsFrequencyFeatures(nodeIndex);}
  
  virtual void reset(StringDictionaryPtr labels)
  {
    baseAlgorithm->reset(labels);
    ClassifierBasedGraphLabelingAlgorithm::reset(labels);
  }
  
  virtual void train(LabeledContentGraphPtr graph)
  {
    std::vector<LabeledContentGraphPtr> trainGraphs;
    std::vector<LabeledContentGraph::LabelsFold> testGraphs;
    graph->makeFolds(numFolds, false, trainGraphs, testGraphs);
    assert(trainGraphs.size() == numFolds && testGraphs.size() == numFolds);

    LabelSequencePtr intermediaryLabels = new LabelSequence(graph->getLabelDictionary(), graph->getNumNodes());
    LabeledContentGraphPtr intermediaryGraph = new LabeledContentGraph(graph->getContentGraph(), intermediaryLabels);
    for (size_t i = 0; i < numFolds; ++i)
    {
      assert(testGraphs[i].graph == graph);
      size_t foldBegin = testGraphs[i].foldBegin;
      size_t foldEnd = testGraphs[i].foldEnd;
      
      baseAlgorithm->reset(graph->getLabelDictionary());
      std::cout << "FOLD " << (i+1) << " / " << numFolds << std::endl;
      baseAlgorithm->train(trainGraphs[i]);
      std::cout << "FOLD " << (i+1) << " / " << numFolds << " OK." << std::endl;

      LabeledContentGraphPtr tmpGraph =
        new LabeledContentGraph(graph->getContentGraph(), new LabelSequence(*graph->getLabels()));
      baseAlgorithm->evaluate(graph, foldBegin, foldEnd, tmpGraph);
      for (size_t j = foldBegin; j < foldEnd; ++j)
        intermediaryLabels->set(j, tmpGraph->getLabel(j));
    }
    
    baseAlgorithm->reset(graph->getLabelDictionary());
    baseAlgorithm->train(graph);
    ClassifierBasedGraphLabelingAlgorithm::train(intermediaryGraph, graph);
  }
  
  virtual double evaluate(LabeledContentGraphPtr graph, size_t begin, size_t end, LabeledContentGraphPtr res = LabeledContentGraphPtr())
  {
    LabeledContentGraphPtr intermediaryGraph = 
      new LabeledContentGraph(graph->getContentGraph(), new LabelSequence(*graph->getLabels()));
    baseAlgorithm->evaluate(graph, begin, end, intermediaryGraph);
    return evaluateClassifier(intermediaryGraph, graph, begin, end, res);
  }

  GraphLabelingAlgorithm& getBaseAlgorithm()
    {assert(baseAlgorithm); return *baseAlgorithm;}

private:
  GraphLabelingAlgorithm* baseAlgorithm;
};

}; /* namespace lbcpp */

#endif // !GRAPH_LABELING_ALGORITHM_STACKED_H_

