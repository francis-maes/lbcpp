-- Francis Maes, 12/08/2011
-- Decision Problem

DecisionProblem = {}

function DecisionProblem.randomEpisode(problem, x)
  local actionSequence = {}
  x = x or problem.x0
  local score = 0.0
  local d = 1.0
  while not problem.isFinal(x) do
    local U = problem.U(x)
    assert(#U > 0) 
    local u = U[Stochastic.uniformInteger(1, #U)]
    table.insert(actionSequence, u)
    score = score + d * problem.g(x, u)
    x = problem.f(x, u)
    d = d * problem.discount
  end
  return x, actionSequence, score
end

return DecisionProblem
