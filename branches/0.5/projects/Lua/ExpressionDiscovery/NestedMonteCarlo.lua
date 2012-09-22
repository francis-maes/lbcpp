-- Francis Maes, 12/08/2011
-- Nested Monte Carlo algorithm

require '../ExpressionDiscovery/DecisionProblem'

subspecified function DecisionProblem.NestedMonteCarlo(problem, x)

  parameter level = {default=2}

  local function argmax(episodeFunction, x)
    local bestScore = -math.huge
    local bestActionSequence
    local bestFinalState

    local U = problem.U(x)
    assert(U ~= nil)
    for i,u in ipairs(U) do
      local score, actionSequence, finalState = episodeFunction(problem.f(x,u))
      if score > bestScore then
        bestFinalState = finalState
        bestActionSequence = actionSequence
        table.insert(bestActionSequence, 1, u)
        bestScore = score
      end
    end
    --print ("argmax", problem.stateToString(x), actionSequenceToString(bestActionSequence))
    return bestScore, bestActionSequence, bestFinalState
  end

  local bestScore = -math.huge
  local bestActionSequence
  local bestFinalState
  local previousActions = {}

  x = x or problem.x0 

  while not problem.isFinal(x) do
    local finalState, actionSequence, score
    if level == 1 then
      score, actionSequence, finalState = argmax(|x| DecisionProblem.randomEpisode(problem, x), x)
    else
      score, actionSequence, finalState = argmax(|x| DecisionProblem.NestedMonteCarlo{level=level-1}(problem, x), x)
    end
    --print (actionSequenceToString(actionSequence), finalState:print(), score)

    if score > bestScore then
      bestFinalState = finalState
      bestScore = score
      bestActionSequence = {}
      for i,u in ipairs(previousActions) do table.insert(bestActionSequence, u) end
      for i,u in ipairs(actionSequence) do table.insert(bestActionSequence, u) end
    end

    if not bestActionSequence then
      return bestScore, bestActionSequence, bestFinalState
    end

    local u = bestActionSequence[#previousActions + 1]
    --print ("Selected action: " .. problem.actionToString(u))
    x = problem.f(x,u)
    table.insert(previousActions, u)
  end
  return bestScore, bestActionSequence, bestFinalState
end