/*-----------------------------------------.---------------------------------.
| Filename: RankingEvaluator.h             | Ranking Error Evaluator         |
| Author  : Francis Maes                   |                                 |
| Started : 16/03/2011 21:41               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_FUNCTION_EVALUATOR_RANKING_ERROR_H_
# define LBCPP_FUNCTION_EVALUATOR_RANKING_ERROR_H_

# include <lbcpp/Function/Evaluator.h>
# include <lbcpp/Data/RandomVariable.h>

namespace lbcpp
{

class RankingScoreObject : public ScoreObject
{
public:
  RankingScoreObject()
    : topRankCost(new ScalarVariableStatistics()),
      misOrderingCost(new ScalarVariableStatistics()) {}
  
  virtual double getScoreToMinimize() const
    {return misOrderingCost->getMean();}

  void finalize()
    {}

  void addExample(const DenseDoubleVectorPtr& scores, const DenseDoubleVectorPtr& costs)
  {
    size_t n = scores->getNumElements();
    jassert(n == costs->getNumElements());
    double bestScore = -DBL_MAX;
    double topRankCost = 0.0;
    double misOrderingCost = 0.0;
    for (size_t i = 0; i < n; ++i)
    {
      double score = scores->getValue(i);
      if (score > bestScore)
        bestScore = score, topRankCost = costs->getValue(i);

      for (size_t j = 0; j < n; ++j)
        if (costs->getValue(i) < costs->getValue(j)) // i is better than j
        {
          if (scores->getValue(i) < scores->getValue(j)) // misordering
            misOrderingCost += (costs->getValue(j) - costs->getValue(i));
        }
    }

    this->topRankCost->push(topRankCost);
    this->misOrderingCost->push(misOrderingCost);
  }

protected:
  friend class RankingScoreObjectClass;
 
  ScalarVariableStatisticsPtr topRankCost;
  ScalarVariableStatisticsPtr misOrderingCost;
};

class RankingEvaluator : public SupervisedEvaluator
{
public:
  virtual TypePtr getRequiredPredictionType() const
    {return denseDoubleVectorClass(positiveIntegerEnumerationEnumeration);}
  
  virtual TypePtr getRequiredSupervisionType() const
    {return denseDoubleVectorClass(positiveIntegerEnumerationEnumeration);}
  
protected:  
  virtual ScoreObjectPtr createEmptyScoreObject(ExecutionContext& context, const FunctionPtr& function) const
    {return new RankingScoreObject();}
  
  virtual void finalizeScoreObject(const ScoreObjectPtr& score, const FunctionPtr& function) const
    {score.staticCast<RankingScoreObject>()->finalize();}

  virtual void addPrediction(ExecutionContext& context, const Variable& predicted, const Variable& correct, const ScoreObjectPtr& result) const
    {result.staticCast<RankingScoreObject>()->addExample(predicted.getObjectAndCast<DenseDoubleVector>(), correct.getObjectAndCast<DenseDoubleVector>());}
};

}; /* namespace lbcpp */

#endif // !LBCPP_FUNCTION_EVALUATOR_RANKING_ERROR_H_
