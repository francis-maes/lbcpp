/*-----------------------------------------.---------------------------------.
| Filename: GraphLabelingAlgorithm.h       | Graph Labeling Algorithm        |
| Author  : Francis Maes                   |         Base Class              |
| Started : 26/03/2009 16:58               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef GRAPH_LABELING_ALGORITHM_H_
# define GRAPH_LABELING_ALGORITHM_H_

# include <cralgo/cralgo.h>

namespace cralgo
{

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

}; /* namespace cralgo */

#endif // !GRAPH_LABELING_ALGORITHM_H_
