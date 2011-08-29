-- Optimize Formula Constants

require 'AST'
require 'DiscreteBandit'
require 'Stochastic'
require 'Sampler'
require 'Random'

--[[
function math.inverse(x)
  return 1 / x
end

_deflog = math.log
function math.log(x)
  if x < 0 then
    return -math.huge
  else
    return _deflog(x)
  end
end

]]

function makeBanditObjective(minArms, maxArms, numProblems, numEstimationsPerProblem, numTimeSteps)

  local banditProblems = {}
  for i=1,numProblems do
    local K = Stochastic.uniformInteger(minArms, maxArms)
    local problem = {}
    for j=1,K do
      table.insert(problem, Sampler.Bernoulli{p=Stochastic.standardUniform()})
    end
    table.insert(banditProblems, problem)  
  end

  return function (f)
    local policy = DiscreteBandit.indexBasedPolicy{indexFunction = f}.__get
    local prevRandom = context.randomGenerator
    context.randomGenerator = Random.new()
    local res = DiscreteBandit.estimatePolicyRegretOnProblems(policy, banditProblems, numEstimationsPerProblem, numTimeSteps)
    context.randomGenerator = prevRandom
    return res
  end

end

banditObjective = makeBanditObjective(2, 10, 1000, 1, 1000)

for C=0,3,0.1 do
  context:enter("C=" .. C)
  context:result("C", C)
  score = banditObjective(|rk,sk,tk,t| math.max(rk, C))
  context:result("score", score)
  context:leave()
end


context:call("KL-UCB(0)", banditObjective, DiscreteBandit.klUcb{c=0})
context:call("KL-UCB(3)", banditObjective, DiscreteBandit.klUcb{c=3})

context:call("Greedy", banditObjective, DiscreteBandit.greedy)
context:call("UCB1Tuned", banditObjective, DiscreteBandit.ucb1Tuned)
context:call("UCB1Tuned", banditObjective, DiscreteBandit.ucb1Tuned)
context:call("UCB1(2)", banditObjective, DiscreteBandit.ucb1C{C=2.0})
context:call("UCB1(1)", banditObjective, DiscreteBandit.ucb1C{C=1.0})


problem = {objective=|C| banditObjective(|rk,sk,tk,t| rk + C / tk), sampler=Sampler.Gaussian{}}
optimizer = Optimizer.EDA{numIterations=100, populationSize=100, numBests=10}
score,solution = optimizer(problem)