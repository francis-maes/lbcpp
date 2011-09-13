
-- Main experiment script

require 'SearchAlgorithm'
require 'ReversePolishNotationProblem'
require 'NestedMonteCarlo'

require 'DiscreteBandit'
require 'Sampler'
require 'Random'

local minK = 2
local maxK = 10
local maxRewardExpectation = 1
local numBanditProblems = 10
local horizon = 1000

local nestedMCLevel = 1
local maxFormulaSize = 20

local bestFormulaScore = math.huge
local bestFormulaString
local numRequests = 0
local numCacheReuse = 0
local cacheSize = 0


local function cacheObjectiveFunction(objective, cache)

  local function getScore(candidateDescription)
    return cache[candidateDescription] 
  end

  local function setScore(candidateDescription, score)
    --print(candidateDescription, "->", score)
    cache[candidateDescription] = score
    cacheSize = cacheSize + 1
    if score <= bestFormulaScore then
      bestFormulaScore = score
      bestFormulaString = candidateDescription
      print ("Best: " .. bestFormulaString, bestFormulaScore)
    end    
  end

  return function (candidate, candidateDescription, ast)

    numRequests = numRequests + 1
    candidateDescription = ast:canonize():print()
    local score = getScore(candidateDescription)
    if score == nil then
      score = objective(candidate)
      setScore(candidateDescription, score)
    else
      numCacheReuse = numCacheReuse + 1
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
      table.insert(problem, Sampler.Bernoulli{p=Stochastic.standardUniform() * maxRewardExpectation})
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
  maxSize = maxFormulaSize
}.__get

local nestedMC = DecisionProblem.NestedMonteCarlo{level=nestedMCLevel}

for i=1,10000 do
  context:enter("Iteration " .. i)
  numCacheReuse = 0
  context:result("iteration", i)
  local n = numRequests
  bestScore, bestActionSequence, bestFinalState = nestedMC(formulaDiscoveryProblem)

  local iterationNumRequests = numRequests - n
  context:result("iterationNumRequests", iterationNumRequests)
  context:result("iterationCacheReuse", numCacheReuse / iterationNumRequests)
  context:result("iterationBestFormula", bestFinalState:canonize():print())
  context:result("cacheSize", cacheSize)
  context:result("bestFormulaScore", bestFormulaScore)
  context:result("bestFormula", bestFormulaString)     

  context:leave()
end

--context:result("bestScore", bestScore)
--context:result("bestFormula", bestFinalState:print())
--context:result("bestCanonizedFormula", bestFinalState:canonize():print())