/*-----------------------------------------.---------------------------------.
| Filename: GoActionScoringEvaluator.h     | Go Action Scoring Evaluator     |
| Author  : Francis Maes                   |                                 |
| Started : 13/03/2011 21:44               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_SEQUENTIAL_DECISION_GO_ACTION_SCORING_EVALUATOR_H_
# define LBCPP_SEQUENTIAL_DECISION_GO_ACTION_SCORING_EVALUATOR_H_

# include "GoProblem.h"
# include "GoSupervisedEpisode.h"
# include <lbcpp/Function/Evaluator.h>

namespace lbcpp
{

class GoActionScoringScoreObject : public ScoreObject
{
public:
  GoActionScoringScoreObject() 
    : predictionRate(new ScalarVariableMean(T("predictionRate"))), 
      rankOfAction(new ScalarVariableStatistics(T("rankOfAction"))),
      unsupervisedRate(new ScalarVariableMean(T("unsupervisedRate"))) {}

  static int getRank(const std::multimap<double, size_t>& sortedScores, size_t index)
  {
    int res = 0;
    for (std::multimap<double, size_t>::const_iterator it = sortedScores.begin(); it != sortedScores.end(); ++it, ++res)
      if (it->second == index)
        return res;
    return -1;
  }

  bool add(ExecutionContext& context, const DenseDoubleVectorPtr& scores, const DenseDoubleVectorPtr& costs)
  {
    std::multimap<double, size_t> sortedScores;
    for (size_t i = 0; i < scores->getNumElements(); ++i)
      sortedScores.insert(std::make_pair(-(scores->getValue(i)), i));
    
    if (sortedScores.empty())
    {
      context.errorCallback(T("No scores"));
      return false;
    }

    // prediction rate
    size_t selectedAction = sortedScores.begin()->second;
    predictionRate->push(costs->getValue(selectedAction) < 0 ? 1.0 : 0.0);

    // rank of selected action
    size_t index = costs->getIndexOfMinimumValue();
    if (index >= 0 && costs->getValue(index) < 0)
    {
      int rank = getRank(sortedScores, index);
      if (rank >= 0)
      {
        rankOfAction->push((double)rank);
        unsupervisedRate->push(0.0);
      }
      else
        unsupervisedRate->push(1.0);
    }
    else
      unsupervisedRate->push(1.0);
    return true;
  }

  virtual double getScoreToMinimize() const
    //{return 1.0 - predictionRate->getMean();} // prediction error
    {return rankOfAction->getMean();} // mean rank of best action

private:
  friend class GoActionScoringScoreObjectClass;

  ScalarVariableMeanPtr predictionRate;
  ScalarVariableStatisticsPtr rankOfAction;
  ScalarVariableMeanPtr unsupervisedRate;
};

class GoActionScoringEvaluator : public SupervisedEvaluator
{
public:
  virtual TypePtr getRequiredPredictionType() const
    {return denseDoubleVectorClass(positiveIntegerEnumerationEnumeration);}

  virtual TypePtr getRequiredSupervisionType() const
    {return denseDoubleVectorClass(positiveIntegerEnumerationEnumeration);}

  virtual ScoreObjectPtr createEmptyScoreObject(ExecutionContext& context, const FunctionPtr& function) const
    {return new GoActionScoringScoreObject();}

  virtual void addPrediction(ExecutionContext& context, const Variable& prediction, const Variable& supervision, const ScoreObjectPtr& result) const
    {result.staticCast<GoActionScoringScoreObject>()->add(context, prediction.getObjectAndCast<DenseDoubleVector>(), supervision.getObjectAndCast<DenseDoubleVector>());}
};

class GoSupervisedEpisodeEvaluator : public CallbackBasedEvaluator
{
public:
  GoSupervisedEpisodeEvaluator() : CallbackBasedEvaluator(new GoActionScoringEvaluator()) {}

  virtual FunctionPtr getFunctionToListen(const FunctionPtr& evaluatedFunction) const
  {
    const DecisionProblemSupervisedEpisodePtr& episodeFunction = evaluatedFunction.staticCast<DecisionProblemSupervisedEpisode>();
    return episodeFunction->getSupervisedDecisionMaker().staticCast<SupervisedLinearRankingBasedDecisionMaker>()->getRankingMachine();
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_SEQUENTIAL_DECISION_GO_ACTION_SCORING_EVALUATOR_H_
