/*-----------------------------------------.---------------------------------.
| Filename: CRAlgorithmGraphLabelingAlg...h| CR-algorithm based              |
| Author  : Francis Maes                   |   Graph Labeling Algorithms     |
| Started : 26/03/2009 17:03               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef GRAPH_LABELING_ALGORITHM_CRALGORITHM_H_
# define GRAPH_LABELING_ALGORITHM_CRALGORITHM_H_

# include "GraphLabelingAlgorithm.h"

namespace lbcpp
{

class CRAlgorithmGraphLabelingAlgorithm : public GraphLabelingAlgorithm
{
public:
  virtual std::pair<PolicyPtr, PolicyPtr> createInitialPolicies(FeatureDictionaryPtr labels) = 0; // returns a pair (learnerPolicy, learnedPolicy)
  virtual CRAlgorithmPtr createCRAlgorithm(LabeledContentGraphPtr graph, size_t begin, size_t end) = 0;

  virtual void reset(FeatureDictionaryPtr labels)
  {
    std::pair<PolicyPtr, PolicyPtr> p = createInitialPolicies(labels);
    learnerPolicy = p.first;
    learnedPolicy = p.second;
  }
  
  enum
  {
    maxLearningIterations = 100,
    maxLearningIterationsWithoutImprovement = 5,
  };
  
  virtual void train(LabeledContentGraphPtr graph)
  {    
    double bestTotalReward = 0.0;
    size_t numIterationsWithoutImprovement = 0;
    for (size_t i = 0; i < maxLearningIterations; ++i)
    {
      PolicyStatisticsPtr statistics = new PolicyStatistics();
      CRAlgorithmPtr crAlgorithm = createCRAlgorithm(graph, 0, graph->getNumNodes());
      crAlgorithm->run(computeStatisticsPolicy(learnerPolicy, statistics));
      std::cout << "[" << numIterationsWithoutImprovement << "] Learning Iteration " << i;// << " => " << policy->toString() << std::endl;
      double totalReward = statistics->getRewardPerEpisodeMean();
      std::cout << " TOTAL REWARD = " << totalReward << " => online accuracy = " << totalReward / (double)graph->getNumNodes() << std::endl;
      if (totalReward > bestTotalReward)
      {
        bestTotalReward = totalReward; // clone best policy ?
        numIterationsWithoutImprovement = 0;
      }
      else
      {
        ++numIterationsWithoutImprovement;
        if (numIterationsWithoutImprovement >= maxLearningIterationsWithoutImprovement)
          break;
      }
    }
  }
  
  virtual double evaluate(LabeledContentGraphPtr graph, size_t begin, size_t end, LabeledContentGraphPtr res = LabeledContentGraphPtr())
  {
    jassert(end > begin);
    CRAlgorithmPtr crAlgorithm = createCRAlgorithm(graph, begin, end);
    crAlgorithm->run(learnedPolicy);
    jassert(crAlgorithm->hasReturn());
    LabelSequencePtr predictedLabels = crAlgorithm->getReturn()->getConstReference<LabelSequencePtr>();
    jassert(predictedLabels);
    if (res)
      res->setLabels(predictedLabels);
    return predictedLabels->numberOfLabelsInCommonWith(graph->getLabels(), begin, end) / (double)(end - begin);
  }
  
protected:
  PolicyPtr learnerPolicy;
  PolicyPtr learnedPolicy;
};

}; /* namespace lbcpp */

#endif // !GRAPH_LABELING_ALGORITHM_CRALGORITHM_H_
