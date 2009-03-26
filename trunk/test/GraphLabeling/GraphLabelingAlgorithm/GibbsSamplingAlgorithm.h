/*-----------------------------------------.---------------------------------.
| Filename: GibbsSamplingAlgorithm.h       | Gibbs Sampling                  |
| Author  : Francis Maes                   |   Algorithm                     |
| Started : 26/03/2009 17:02               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef GRAPH_LABELING_ALGORITHM_GIBBS_SAMPLING_
# define GRAPH_LABELING_ALGORITHM_GIBBS_SAMPLING_

# include "IterativeClassificationAlgorithm.h"

namespace cralgo
{

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

}; /* namespace cralgo */

#endif // !GRAPH_LABELING_ALGORITHM_GIBBS_SAMPLING_
