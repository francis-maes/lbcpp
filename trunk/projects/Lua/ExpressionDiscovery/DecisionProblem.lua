-- Francis Maes, 12/08/2011
-- Decision Problem

DecisionProblem = {}

function math.argmax(vector, f)
  local bestScore = -math.huge
  local bestSolutions = {}

  for key,value in ipairs(vector) do
    local score = f(key, value)
    if score > bestScore then
      bestScore = score
      bestSolutions = {key}
    elseif score == bestScore then
      table.insert(bestSolutions, key)
    end
  end

  if #bestSolutions == 0 then
    return nil
  elseif #bestSolutions == 1 then
    return bestSolutions[1]
  else
    return bestSolutions[Stochastic.uniformInteger(1,#bestSolutions)]
  end
end

function DecisionProblem.randomPolicy(problem, x)
  local U = problem.U(x)
  assert(#U > 0) 
  return U[Stochastic.uniformInteger(1, #U)]
end

subspecified function DecisionProblem.ActionValueBasedPolicy(problem, x)
  parameter actionValues = {}
  local U = problem.U(x)
  return U[math.argmax(U, |i,u| actionValues(problem, x, u))]
end

function DecisionProblem.doEpisode(problem, x, policy)
  local actionSequence = {}
  x = x or problem.x0
  local score = 0.0
  local d = 1.0
  while not problem.isFinal(x) do
    local u = policy(problem, x)
    table.insert(actionSequence, u)
    score = score + d * problem.g(x, u)
    x = problem.f(x, u)
    d = d * problem.discount
  end
  return x, actionSequence, score
end

function DecisionProblem.randomEpisode(problem, x)
  return DecisionProblem.doEpisode(problem, x, DecisionProblem.randomPolicy)
end

return DecisionProblem