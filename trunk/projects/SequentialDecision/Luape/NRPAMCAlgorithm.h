/*-----------------------------------------.---------------------------------.
| Filename: NRPAMCAlgorithm.h              | Nested Rollout Policy Adaptation|
| Author  : Francis Maes                   |                                 |
| Started : 30/06/2012 21:32               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUAPE_MC_ALGORITHM_NRPA_H_
# define LBCPP_LUAPE_MC_ALGORITHM_NRPA_H_

# include "OnlineLearningMCAlgorithm.h"

namespace lbcpp
{

class NRPAMCAlgorithm : public OnlineLearningMCAlgorithm
{
public:
  NRPAMCAlgorithm(size_t level = 3, size_t iterations = 100, double learningRate = 1.0)
    : OnlineLearningMCAlgorithm(learningRate), level(level), iterations(iterations) {}

  virtual void search(ExecutionContext& context, MCObjectivePtr objective,
                      const std::vector<Variable>& previousActions, DecisionProblemStatePtr state)
  {
    while (!objective->shouldStop()) // tmp !!!
      searchRecursively(context, objective, previousActions, state, level, DenseDoubleVectorPtr());
  }
  
  std::pair<double, Trajectory> searchRecursively(ExecutionContext& context, MCObjectivePtr objective,
                                                  const std::vector<Variable>& previousActions, DecisionProblemStatePtr state,
                                                  size_t level, DenseDoubleVectorPtr parameters)
  {
    if (objective->shouldStop())
      return std::make_pair(-DBL_MAX, Trajectory());
      
    if (level == 0)
    {
      Trajectory trajectory;
      double score = rollout(context, objective, previousActions, state, parameters, trajectory);
      return std::make_pair(score, trajectory);
    }
    else
    {
      double bestScore = -DBL_MAX;
      Trajectory bestTrajectory;
      if (parameters)
        parameters = parameters->cloneAndCast<DenseDoubleVector>();
        
      size_t numIterations = iterations ? iterations : (size_t)pow(10.0, context.getRandomGenerator()->sampleDouble(1.0, 3.0));
      double learningRate = this->learningRate ? this->learningRate : pow(10.0, context.getRandomGenerator()->sampleDouble(-1.0, 1.0));
        
      for (size_t i = 0; i < numIterations; ++i)
      {
        std::pair<double, Trajectory> subResult = searchRecursively(context, objective, previousActions, state, level - 1, parameters);
        if (subResult.first >= bestScore)
        {
          bestScore = subResult.first;
          bestTrajectory = subResult.second;
        }
        
        // update trajectory activations
        for (size_t i = 0; i < bestTrajectory.size(); ++i)
        {
          Step& step = bestTrajectory[i];
          size_t n = step.actionFeatures->getNumElements();
          for (size_t j = 0; j < n; ++j)
            step.actionActivations->setValue(j, predictActivation(parameters, step.actionFeatures->getAndCast<DoubleVector>(j)));
        }
        makeSGDStep(context, parameters, bestTrajectory, learningRate);
      }
      return std::make_pair(bestScore, bestTrajectory);
    }
  }

protected:
  friend class NRPAMCAlgorithmClass;

  size_t level;
  size_t iterations;
};


}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_MC_ALGORITHM_NRPA_H_
