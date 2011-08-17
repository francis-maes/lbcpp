-- Search Algorithm module

require '../ExpressionDiscovery/DecisionProblem'

PolicyBasedSearchAlgorithm = subspecified {
  parameter policy = {},

  initialize = function (self, problem)
    self.problem = problem
    self.x0 = problem.x0
  end,

  episode = function (self)
    return DecisionProblem.doEpisode(self.problem, self.x0, policy)
  end,
  
  finalize = function (self) end
}

randomSearchAlgorithm = PolicyBasedSearchAlgorithm{policy = DecisionProblem.randomPolicy}.__get
nested1SearchAlgorithm = PolicyBasedSearchAlgorithm{
  policy = DecisionProblem.ActionValueBasedPolicy{
    actionValues = DecisionProblem.StateToActionValue{
      stateValues = DecisionProblem.SingleRolloutStateValue{
        policy = DecisionProblem.randomPolicy
      }
    }
  }
}.__get

function runSearchAlgorithm(problem, algorithm, numIterations, numEpisodesPerIteration)
  algorithm:initialize(problem)
  local bestScore = -math.huge
  local bestActionSequence, bestFinalState
  for i=1,numIterations do
    context:enter("Iteration " .. i)
    local scoreStats = Statistics.meanVarianceAndBounds()
    local bestIterationFinalState
    for j=1,numEpisodesPerIteration do
      score, actionSequence, finalState = algorithm:episode()
      if score > bestScore then
        bestScore = score
        bestActionSequence = actionSequence
        bestFinalState = finalState
      end
      if score > scoreStats:getMaximum() then
        bestIterationFinalState = finalState
      end
      scoreStats:observe(score)
    end
    context:result("iteration", i)
    context:result("best ever score", bestScore)
    context:result("best ever final state", problem.stateToString(bestFinalState))
    context:result("best iteration score", scoreStats:getMaximum())
    context:result("mean iteration score", scoreStats:getMean())
    context:result("worst iteration score", scoreStats:getMinimum())
    context:result("best iteration final state", problem.stateToString(bestIterationFinalState))
    if algorithm.finalizeIteration then
      algorithm:finalizeIteration()
    end
    context:leave(scoreStats:getMaximum())
  end
  algorithm:finalize()
end
