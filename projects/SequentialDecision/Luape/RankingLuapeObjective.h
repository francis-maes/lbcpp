/*-----------------------------------------.---------------------------------.
| Filename: RankingLuapeObjective.h        | Luape Ranking Objective         |
| Author  : Francis Maes                   |                                 |
| Started : 19/11/2011 13:13               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUAPE_OBJECTIVE_RANKING_H_
# define LBCPP_LUAPE_OBJECTIVE_RANKING_H_

# include "LuapeProblem.h"

namespace lbcpp
{

class RankingLuapeObjective : public LuapeObjective
{
public:
  RankingLuapeObjective(RankingLossFunctionPtr rankingLoss = RankingLossFunctionPtr())
    : rankingLoss(rankingLoss) {}

  virtual bool initialize(ExecutionContext& context, const LuapeProblemPtr& problem, const LuapeInferencePtr& function)
  {
    this->problem = problem;
    this->function = function;
    this->graph = function->getGraph();
    return true;
  }

  virtual void setExamples(bool isTrainingData, const std::vector<ObjectPtr>& data)
  {
    graph->clearSamples(isTrainingData, !isTrainingData);
    for (size_t i = 0; i < data.size(); ++i)
      addRankingExampleToGraph(isTrainingData, data[i].staticCast<Pair>());
    if (isTrainingData)
      trainingData = *(std::vector<PairPtr>* )&data;
    else
      validationData = *(std::vector<PairPtr>* )&data;
  }

  void addRankingExampleToGraph(bool isTrainingData, const PairPtr& rankingExample) const
  {
    const ContainerPtr& alternatives = rankingExample->getFirst().getObjectAndCast<Container>();
    size_t n = alternatives->getNumElements();

    LuapeNodeCachePtr inputNodeCache = graph->getNode(0)->getCache();
    size_t firstIndex = inputNodeCache->getNumSamples(isTrainingData);
    inputNodeCache->resizeSamples(isTrainingData, firstIndex + n);
    for (size_t i = 0; i < n; ++i)
      inputNodeCache->setSample(true, firstIndex + 1, alternatives->getElement(i));
  }

  virtual void computeLoss(const DenseDoubleVectorPtr& predictions, double* lossValue, DenseDoubleVectorPtr* lossGradient) const
  {
    if (lossValue)
      *lossValue = 0.0;
    if (lossGradient)
      *lossGradient = new DenseDoubleVector(predictions->getNumValues(), 0.0);
  
    size_t index = 0;
    for (size_t i = 0; i < trainingData.size(); ++i)
    {
      size_t n = trainingData[i]->getFirst().getObjectAndCast<Container>()->getNumElements();
      DenseDoubleVectorPtr costs = trainingData[i]->getSecond().getObjectAndCast<DenseDoubleVector>();

      DenseDoubleVectorPtr scores = new DenseDoubleVector(n, 0.0);
      memcpy(scores->getValuePointer(0), predictions->getValuePointer(index), sizeof (double) * n);

      double v = 0.0;
      DenseDoubleVectorPtr g = lossGradient ? new DenseDoubleVector(n, 0.0) : DenseDoubleVectorPtr();
      rankingLoss->computeRankingLoss(scores, costs, lossValue ? &v : NULL, lossGradient ? &g : NULL, 1.0);
      if (lossValue)
        *lossValue += v;
      if (g)      
        memcpy((*lossGradient)->getValuePointer(index), g->getValuePointer(0), sizeof (double) * n);
      index += n;
    }
    if (lossValue)
      *lossValue /= trainingData.size();
    if (lossGradient)
      (*lossGradient)->multiplyByScalar(-1.0);
  }

protected:
  RankingLossFunctionPtr rankingLoss;

  LuapeProblemPtr problem;
  LuapeInferencePtr function;
  LuapeGraphPtr graph;

  std::vector<PairPtr> trainingData;
  std::vector<PairPtr> validationData;
};

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_OBJECTIVE_RANKING_H_
