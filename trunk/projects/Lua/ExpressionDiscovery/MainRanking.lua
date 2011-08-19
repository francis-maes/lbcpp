
-- Main experiment script


require '../ExpressionDiscovery/DecisionProblem'
require '../ExpressionDiscovery/ReversePolishNotationProblem'
require '../ExpressionDiscovery/FormulaFeatures'
require 'AST'
require 'Dictionary'
require 'Vector'
require 'Evaluator'
require 'Statistics'
require 'Random'


--context.randomGenerator = Random.new(0)

--[[
--myFormula = AST.binaryOperation('add', AST.binaryOperation('mul', AST.identifier('a'), AST.identifier('b')), AST.identifier('c'))
myFormula = AST.binaryOperation('div',
   AST.binaryOperation('sub', AST.identifier('a'), AST.identifier('b')),
   AST.binaryOperation('mod', AST.identifier('a'), AST.identifier('c')))
--myFormula = AST.binaryOperation('mod', AST.identifier('a'), AST.identifier('b'))
 
print (myFormula:print())
v = formulaAllPathFeaturesToSparseVector(myFormula)
print (v)
print (allPathFeaturesDictionary)
]]


-- Application : symbolic regression

local function cacheObjectiveFunction(objective, cache)
  return function (candidate, candidateDescription)
    local score = cache[candidateDescription]
    if score == nil then
      score = objective(candidate)
     -- print ("Evaluate " .. candidateDescription .. " ==> " .. score)
      cache[candidateDescription] = score
    end
    return score
  end
end

local function makeSymbolicRegressionObjective()
  local dataset = {}
  for i = 1,100 do
    local example = {
      Stochastic.standardGaussian(), 
      Stochastic.standardGaussian(), 
      Stochastic.standardGaussian(), 
      Stochastic.standardGaussian()
    }
    --table.insert(example, example[1]) -- a 
    --table.insert(example, example[1] * example[2] + example[3])  -- a b * c +
    table.insert(example, (example[1] + 5) * example[2] + example[3] - 10) -- a * b + b * 5 + c - 10
    table.insert(dataset, example)
  end
  return |f| Evaluator.meanSquaredError(f, dataset)
end


problemCache = {}
problem = DecisionProblem.ReversePolishNotation{
  constants = {1,2,5,10},
  objective = cacheObjectiveFunction(makeSymbolicRegressionObjective(), problemCache),
  maxSize = 11
}

policy = DecisionProblem.randomPolicy

-- Generate Datasets

function sampleFormula(problem, policy)
  assert(problem)
  local x = problem.x0
  while not problem.isFinal(x) do
    local u = policy(problem, x)
    x = problem.f(x, u)
  end
  return x
end

--[[
function sampleDataset(problem, policy, size)
  local population = {}
  for i=1,size do
    local formula = sampleFormula(problem, policy)
    local expr = buildExpression(problem.__parameters.variables, formula)
    local score = problem.__parameters.objective(expr, formula:print())
    population[formula:print()] = {formula, score}
  end
  local res = {}
  for description,example in pairs(population) do
    table.insert(res, example)
  end
  return res
end

dataset = sampleDataset(problem, policy, 1000)

trainingDataset = {}--sampleDataset(problem, policy, 100)
testingDataset = {}--sampleDataset(problem, policy, 100)
for i,example in ipairs(dataset) do
  table.insert(i % 2 == 0 and trainingDataset or testingDataset, example)
end

table.sort(trainingDataset, |a,b| a[2] < b[2])
table.sort(testingDataset, |a,b| a[2] < b[2])
print (#trainingDataset, "Training examples", #testingDataset, "Testing examples")
]]

-- Train ranking machine

function trainRankingMachine(trainingDataset)
  local theta = Vector.newDense()
  local splitPoint = math.ceil(#trainingDataset / 4)
  print ("Split Point: ", splitPoint)
  for i=1,100 do
    context:enter("Iteration " .. i)
    local accuracyStats = Statistics.mean()
    for j=1,100 do
      local positiveIndex = Stochastic.uniformInteger(1,splitPoint)
      local negativeIndex = Stochastic.uniformInteger(positiveIndex+1,#trainingDataset)
      local positiveExample = trainingDataset[positiveIndex]
      local negativeExample = trainingDataset[negativeIndex]
      local positiveFeatures = formulaAllPathFeaturesToSparseVector(positiveExample[1])
      local negativeFeatures = formulaAllPathFeaturesToSparseVector(negativeExample[1])
      local positiveScore = theta:dot(positiveFeatures)
      local negativeScore = theta:dot(negativeFeatures)
      accuracyStats:observe(positiveScore > negativeScore and 1 or 0)
      if positiveScore - negativeScore < 1 then
        theta:add(positiveFeatures)
        theta:add(negativeFeatures, -1)
      end
    end
    context:result("iteration", i)
    context:result("accuracy", accuracyStats:getMean())
    context:result("theta l2norm", theta:l2norm())
    context:result("theta l0norm", theta:l0norm())
    context:leave(accuracyStats:getMean())
  end
  return theta
end

local function testRankingMachine(theta, dataset)
  for i,example in ipairs(dataset) do
    local features = formulaAllPathFeaturesToSparseVector(example[1])
    local score = theta:dot(features)
    context:enter("Example " .. i)
    context:result("example", i)
    context:result("formula", example[1]:print())
    context:result("score", score)
    context:result("objective", math.exp(-example[2]))
    context:leave()
  end
end

local function displayWeightVector(theta, dictionary)
  local entries = {}
  local n = #theta
  for i=1,n do
    table.insert(entries, {dictionary:get(i), theta[i]})
  end
  table.sort(entries, |a,b| a[2] > b[2])
  for i,e in ipairs(entries) do
    print (e[1],e[2])
  end
end

local function rankingDrivenPolicy(theta, temperature)

  local function rankingScore(formula, addRootNode)
    return theta:dot(formulaAllPathFeaturesToSparseVector(formula, addRootNode))
  end

  local function stateScore(x)
    if type(x) == "table" then
      local res = 0
      for i,formula in ipairs(x) do
        res = res + rankingScore(formula, false)
      end
      return res
    else
      return rankingScore(x, true)
    end
  end

  return function (problem, x)
    local U = problem.U(x)
    local probs = {}
    local Z = 0
    local currentScore = stateScore(x)
    print ("State: " .. problem.stateToString(x) .. " score = " .. currentScore)
    for i,u in ipairs(U) do
      local nextScore = stateScore(problem.f(x, u))
      local prob = math.exp((nextScore - currentScore) * temperature)
      table.insert(probs, prob)
      Z = Z + prob
    end
    --print (table.concat(probs, ","))    
    local dbg = ""
    for i,u in ipairs(U) do
      dbg = dbg .. " " .. problem.actionToString(u) .. " [" .. (probs[i] / Z) .. "]"
    end
    print (dbg)

    local r = Stochastic.standardUniform() * Z
    for i,u in ipairs(U) do
      local prob = probs[i]
      if r < prob then
        --print ("==> action", i, problem.actionToString(u))
        return u
      else
        r = r - prob
      end
    end
    assert(false)
  end
end

local function formulaRankingEDA(problem, populationSize)

  local policy = DecisionProblem.randomPolicy
  local theta = Vector.newDense()
  local population = {}

  local function addFormulaToPopulation(formula)
    local expr = buildExpression(problem.__parameters.variables, formula)
    local score = problem.__parameters.objective(expr, formula:print())
    population[formula:print()] = {formula, score}
  end

  local function makeInitialPopulation()
    local function exhaustiveSearch(x, depth)
      if problem.isFinal(x) then
        addFormulaToPopulation(x)
      elseif depth > 0 then
        local U = problem.U(x)
        for i,u in ipairs(U) do
          exhaustiveSearch(problem.f(x, u), depth - 1)
        end
      end
    end
    exhaustiveSearch(problem.x0, 4)    
  end

  local function sampleNewCandidates()
    local surrogateScoreStats = Statistics.meanVarianceAndBounds()
    for i=1,populationSize do
      local formula = sampleFormula(problem, policy)
      addFormulaToPopulation(formula)
      local rankingScore = theta:dot(formulaAllPathFeaturesToSparseVector(formula))
      surrogateScoreStats:observe(rankingScore)
    end
    return surrogateScoreStats
  end

  local function sortAndKeepBestCandidates()
    local sortedCandidates = {}
    for name,example in pairs(population) do
      table.insert(sortedCandidates, example)
    end
    table.sort(sortedCandidates, |a,b| a[2] < b[2])
    for i=#sortedCandidates,populationSize+1,-1 do
      table.remove(sortedCandidates, i)
    end
    population = {}
    for i,example in ipairs(sortedCandidates) do
      population[example[1]:print()] = example
    end
    return sortedCandidates
  end

  local function makeScoreStatistics(sortedCandidates)
    local scoreStats = Statistics.meanVarianceAndBounds()
    local numInfinite = 0
    for i,candidate in ipairs(sortedCandidates) do
      local score = candidate[2]
      if score < math.huge then
        scoreStats:observe(score) -- remove -math.huge scores that make statistics unreadable
      else
        numInfinite = numInfinite + 1
      end
    end
    context:result("Num sorted candidates", #sortedCandidates)
    context:result("Best score", scoreStats:getMinimum())
    context:result("Mean score", scoreStats:getMean())
    context:result("Worst score", scoreStats:getMaximum())
    context:result("Num infinit scores", numInfinite)
    context:result("Score stats", scoreStats)
  end

  local function iteration (iter)
    context:result("iteration", iter)

    if iter == 1 then
      makeInitialPopulation()
    else
      local scoreStats = context:call("Sample new candidates", sampleNewCandidates) -- grows the current population
      context:result("Mean sampled score", scoreStats:getMean())
    end

    local sortedCandidates = context:call("Sort candidates", sortAndKeepBestCandidates) -- sort the current population and restrict its size to "populationSize"
    makeScoreStatistics(sortedCandidates)
    for best=1,10 do
      if best <= #sortedCandidates then
        context:result("Best formula " .. best, sortedCandidates[best][1]:print() .. ' (' .. sortedCandidates[best][2] .. ')')
      end
    end

    theta = context:call("Train ranking machine", trainRankingMachine, sortedCandidates)
    context:call("Test ranking machine", testRankingMachine, theta, sortedCandidates)

    context:call("Weights", displayWeightVector, theta, allPathFeaturesDictionary)

    policy = rankingDrivenPolicy(theta, 0.1)

    policy(problem, {})
    policy(problem, {AST.identifier('c')})
    policy(problem, {AST.identifier('c'), AST.identifier('a')})
    policy(problem, {AST.identifier('c'), AST.identifier('a'), AST.identifier('b')})
    policy(problem, {AST.identifier('a'), AST.identifier('b')})
    policy(problem, {AST.binaryOperation("mul", AST.identifier('a'), AST.identifier('b'))})

    return sortedCandidates[1][2] -- best score
  end
  
  for iter=1,10 do
    context:call("Iteration " .. iter, iteration, iter)
  end
end

context:call("Formula Ranking EDA", formulaRankingEDA, problem, 500)

--context:call("Training Ranking machine", trainRankingMachine, trainingDataset)
--context:call("Training data", testRankingMachine, theta, trainingDataset)
--context:call("Testing data", testRankingMachine, theta, testingDataset)