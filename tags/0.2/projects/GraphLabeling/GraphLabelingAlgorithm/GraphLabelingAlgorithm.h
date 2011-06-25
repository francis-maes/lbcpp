/*-----------------------------------------.---------------------------------.
| Filename: GraphLabelingAlgorithm.h       | Graph Labeling Algorithm        |
| Author  : Francis Maes                   |         Base Class              |
| Started : 26/03/2009 16:58               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef GRAPH_LABELING_ALGORITHM_H_
# define GRAPH_LABELING_ALGORITHM_H_

# include <lbcpp/lbcpp.h>

namespace lbcpp
{

class GraphLabelingAlgorithm
{
public:
  GraphLabelingAlgorithm() : l2regularizer(0.0) {}
  
  virtual ~GraphLabelingAlgorithm() {}

  virtual void setL2Regularizer(double reg)
    {l2regularizer = reg;}
  
  virtual void reset(FeatureDictionaryPtr labels) = 0;
  virtual void train(LabeledContentGraphPtr graph) = 0;
  
  // predictedGraph must be initialized as a copy of correctGraph
  // evaluate() fills the predictions into predictedGraph
  virtual double evaluate(LabeledContentGraphPtr correctGraph, size_t begin, size_t end, LabeledContentGraphPtr res = LabeledContentGraphPtr()) = 0;
  
  void crossValidate(const std::vector<LabeledContentGraphPtr>& trainGraphs,
                     const std::vector<LabeledContentGraph::LabelsFold>& testGraphs, 
                     ScalarVariableStatisticsPtr trainAccuracy,
                     ScalarVariableStatisticsPtr testAccuracy)
  {
    jassert(trainGraphs.size() == testGraphs.size());
    for (size_t i = 0; i < trainGraphs.size(); ++i)
    {
      std::cout << "CROSS-VALIDATION FOLD " << (i+1) << " / " << trainGraphs.size() << std::endl;
      FeatureDictionaryPtr labels = FeatureDictionaryManager::getInstance().getFlatVectorDictionary(trainGraphs[i]->getLabelDictionary());
      reset(labels);
      train(trainGraphs[i]);
      trainAccuracy->push(evaluate(trainGraphs[i], 0, trainGraphs[i]->getNumNodes()));
      testAccuracy->push(evaluate(testGraphs[i].graph, testGraphs[i].foldBegin, testGraphs[i].foldEnd));
    }
  }
  
protected:
  double l2regularizer;
};

}; /* namespace lbcpp */

#endif // !GRAPH_LABELING_ALGORITHM_H_