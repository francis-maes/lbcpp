
-- Main experiment script

require '../ExpressionDiscovery/SearchAlgorithm'
require '../ExpressionDiscovery/ReversePolishNotationProblem'
require '../ExpressionDiscovery/NestedMonteCarlo'

require 'DiscreteBandit'
require 'Sampler'
require 'Random'

local minK = 2
local maxK = 10
local numBanditProblems = 10
local horizon = 10


local function cacheObjectiveFunction(objective, cache)
  return function (candidate, candidateDescription, ast)
    candidateDescription = ast:canonize():print()
    local score = cache[candidateDescription]
    if score == nil then
      score = objective(candidate)
      print ("Evaluate " .. candidateDescription .. " ==> " .. score)
      cache[candidateDescription] = score
    end
    return score
  end
end


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


local banditsScoreCache = {}

local formulaDiscoveryProblem = DecisionProblem.ReversePolishNotation{
  variables = {"rk", "sk", "tk", "t"},
  constants = {1,2,3,5,7},
  objective = cacheObjectiveFunction(makeBanditObjective(minK, maxK, numBanditProblems, 1, horizon), banditsScoreCache),
  maxSize = 10
}.__get

local nestedMC1 = DecisionProblem.NestedMonteCarlo{level=1}
local nestedMC2 = DecisionProblem.NestedMonteCarlo{level=2}
local nestedMC3 = DecisionProblem.NestedMonteCarlo{level=3}

bestScore, bestActionSequence, bestFinalState = nestedMC1(formulaDiscoveryProblem)
context:result("bestScore", bestScore)
context:result("bestFormula", bestFinalState:print())
context:result("bestCanonizedFormula", bestFinalState:canonize():print())
