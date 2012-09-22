
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
require 'Sampler'

--context.randomGenerator = Random.new(0)

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
    table.insert(example, example[1] * example[2] + example[3])  -- a b * c +
    --table.insert(example, (example[1] + 5) * example[2] + example[3] - 10) -- a * b + b * 5 + c - 10
    table.insert(dataset, example)
  end
  return |f| Evaluator.meanSquaredError(f, dataset)
end


problemCache = {}
problem = DecisionProblem.ReversePolishNotation{
  constants = {1,2,5,10},
  objective = cacheObjectiveFunction(makeSymbolicRegressionObjective(), problemCache),
  maxSize = 10
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

-- Train ranking machine

function trainRankingMachine(trainingDataset)
  local theta = Vector.newDense()
  for i=1,100 do
    context:enter("Iteration " .. i)
    local accuracyStats = Statistics.mean()
    for j=1,100 do
      local positiveIndex = math.min(#trainingDataset - 1, Sampler.Geometric{p=0.2}())
      local negativeIndex = Stochastic.uniformInteger(positiveIndex+1,#trainingDataset)
      local positiveExample = trainingDataset[positiveIndex]
      local negativeExample = trainingDataset[negativeIndex]
      local positiveTrajectory = formulaToTrajectory(positiveExample[1])
      local negativeTrajectory = formulaToTrajectory(negativeExample[1])
      local minLength = math.min(#positiveTrajectory / 2, #negativeTrajectory / 2)
      for k=1,minLength do

        local positiveFeatures = formulaActionFeaturesToSparseVector(positiveTrajectory[k*2 - 1], positiveTrajectory[k*2])
        local negativeFeatures = formulaActionFeaturesToSparseVector(negativeTrajectory[k*2 - 1], negativeTrajectory[k*2])
        local positiveScore = theta:dot(positiveFeatures)
        local negativeScore = theta:dot(negativeFeatures)
        accuracyStats:observe(positiveScore > negativeScore and 1 or 0)
        if positiveScore - negativeScore < 1 then
          theta:add(positiveFeatures)
          theta:add(negativeFeatures, -1)
        end
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
    local features = formulaActionFeaturesToSparseVector({example[1]}, AST.returnStatement())
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

local function rankingDrivenPolicy(theta, temperature, population)

  return function (problem, x, verbose)
    local U = problem.U(x)
    local probs = {}
    local Z = 0
    for i,u in ipairs(U) do
      local prob
      if u.className == "lua::Return" and population[x[#x]:print] ~= nil then
        prob = 0
      else
        local Q = theta:dot(formulaActionFeaturesToSparseVector(x,u))
        prob = math.exp(Q * temperature)
      end
      table.insert(probs, prob)
      Z = Z + prob
    end

    if Z == 0 then -- happens if the only remaining action is Return
      return DecisionProblem.randomPolicy(problem, x)
    end

    if verbose then
      local dbg = ""
      local function floatToPercentString(flt)
        return tostring(math.floor(flt * 1000) / 10) .. "%"
      end
      for i,u in ipairs(U) do
        dbg = dbg .. " " .. problem.actionToString(u) .. " [" .. floatToPercentString(probs[i] / Z) .. "]"
      end
      print (dbg)
    end

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
    --local dbg = "Simplify " .. formula:print() .. " ==> "
    formula = simplifyAndMakeFormulaUnique(formula)
    --print(dbg .. formula:print())
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
      print ("==>" .. formula:print())
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
    for best=1,#sortedCandidates/4 do
      context:result("Best formula " .. best, sortedCandidates[best][1]:print() .. ' (' .. sortedCandidates[best][2] .. ')')
    end

    theta = context:call("Train ranking machine", trainRankingMachine, sortedCandidates)
    context:call("Test ranking machine", testRankingMachine, theta, sortedCandidates)

    context:call("Ranking weights", displayWeightVector, theta, formulaActionFeaturesDictionary)

    policy = rankingDrivenPolicy(theta, 0.5, population)

    policy(problem, {}, true)
    policy(problem, {AST.identifier('a')}, true)
    policy(problem, {AST.identifier('b')}, true)
    policy(problem, {AST.identifier('c')}, true)
    policy(problem, {AST.binaryOperation('mul', AST.identifier('a'), AST.identifier('b'))}, true)
    policy(problem, {AST.binaryOperation('mul', AST.identifier('a'), AST.identifier('b')), AST.identifier('c')}, true)

    return sortedCandidates[1][2] -- best score
  end
  
  for iter=1,10 do
    context:call("Iteration " .. iter, iteration, iter)
  end
end

context:call("Formula Ranking EDA", formulaRankingEDA, problem, 100)

--context:call("Training Ranking machine", trainRankingMachine, trainingDataset)
--context:call("Training data", testRankingMachine, theta, trainingDataset)
--context:call("Testing data", testRankingMachine, theta, testingDataset)