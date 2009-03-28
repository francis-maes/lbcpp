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

namespace cralgo
{

class StackedGraphLabelingAlgorithm : public ClassifierBasedGraphLabelingAlgorithm
{
public:
  StackedGraphLabelingAlgorithm(GraphLabelingAlgorithm* baseAlgorithm)
    : baseAlgorithm(baseAlgorithm) {}

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

      LabeledContentGraphPtr testGraph =
        new LabeledContentGraph(graph->getContentGraph(), new LabelSequence(*graph->getLabels()));
      baseAlgorithm->evaluate(testGraph, foldBegin, foldEnd);
      for (size_t j = foldBegin; j < foldEnd; ++j)
        intermediaryLabels->set(j, testGraph->getLabel(j));
    }
    
    baseAlgorithm->reset(graph->getLabelDictionary());
    baseAlgorithm->train(graph);
    ClassifierBasedGraphLabelingAlgorithm::train(intermediaryGraph, graph);
  }
  
  virtual double evaluate(LabeledContentGraphPtr graph, size_t begin, size_t end)
  {
    LabelSequencePtr intermediaryLabels = new LabelSequence(*graph->getLabels());
    LabeledContentGraphPtr intermediaryGraph = new LabeledContentGraph(graph->getContentGraph(), intermediaryLabels);
    baseAlgorithm->evaluate(intermediaryGraph, begin, end);
    return evaluateClassifier(intermediaryGraph, graph, begin, end);
  }

  GraphLabelingAlgorithm& getBaseAlgorithm()
    {assert(baseAlgorithm); return *baseAlgorithm;}

private:
  GraphLabelingAlgorithm* baseAlgorithm;
};

}; /* namespace cralgo */

#endif // !GRAPH_LABELING_ALGORITHM_STACKED_H_

