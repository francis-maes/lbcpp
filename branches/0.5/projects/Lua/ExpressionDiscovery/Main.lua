
-- Main experiment script

require '../ExpressionDiscovery/SearchAlgorithm'
require '../ExpressionDiscovery/ReversePolishNotationProblem'
require '../ExpressionDiscovery/MCTS'
require '../ExpressionDiscovery/NestedMonteCarlo'
require '../ExpressionDiscovery/UBOLA'
require '../ExpressionDiscovery/FormulaFeatures'

require 'DiscreteBandit'
require 'Evaluator'
require 'Sampler'
require 'Random'

--context.randomGenerator = Random.new(0)

-- decorate objective function
local trainingDataset = {}
local testingDataset = {}
local allExamples = {}

local numEvaluations = 0
local function decorateObjectiveFunction(objective)
  return function (candidate, candidateDescription, candidateAST)
    local score = allExamples[candidateDescription]
    if score == nil then
      score = objective(candidate)
      print ("Evaluate " .. candidateDescription .. " ==> " .. score)
      local dataset = numEvaluations % 2 == 0 and trainingDataset or testingDataset
      numEvaluations = numEvaluations + 1
      table.insert(dataset, {formulaFeatures(candidateAST), score})
      allExamples[candidateDescription] = score
    end
    return score
  end
end


-- Test1 : symbolic regression

local function makeSymbolicRegressionObjective()
  local dataset = {}
  for i = 1,100 do
    local example = {
      Stochastic.standardGaussian(), 
      Stochastic.standardGaussian(), 
      Stochastic.standardGaussian(), 
      Stochastic.standardGaussian()
    }
    table.insert(example, example[1] * example[2] + example[3])  -- a b * c +
    --table.insert(example, (example[1] + 5) * example[2] + example[3] - 10) -- a * b + b * 5 + c - 10
    table.insert(dataset, example)
  end
  return |f| Evaluator.meanSquaredError(f, dataset)
end


symbolicRegressionProblem = DecisionProblem.ReversePolishNotation{
  constants = {1,2,5,10},
  objective = decorateObjectiveFunction(makeSymbolicRegressionObjective()),
  maxSize = 5
}.__get

-- Test2 : bandits formula

local function banditsFormulaObjective(f)

  local policy = DiscreteBandit.indexBasedPolicy{indexFunction = f}.__get
  local bandits = {Sampler.Bernoulli{p=0.9}, Sampler.Bernoulli{p=0.6}}
  return DiscreteBandit.estimatePolicyRegret(policy, bandits, 10, 100)  -- num estimations and time horizon
  
end

banditsProblem = DecisionProblem.ReversePolishNotation{
  variables = {"rk", "sk", "tk", "t"},
  constants = {1,2,5,10},
  objective = banditsFormulaObjective,
  maxSize = 5
}.__get

-- Main

local problem = symbolicRegressionProblem

--[[
-- Make Dataset

local algo = DecisionProblem.SinglePlayerMCTS{
  indexFunction=DiscreteBandit.ucb1C{2}, verbose=false,
  partialExpand = true, fullPathExpand = false, useMaxReward = true}.__get
context:call("mcts", runSearchAlgorithm, problem, algo, 100, 100)

print ("datasets size", #trainingDataset, #testingDataset)
print ("num features", #formulaFeatureDictionary.content)

-- Train predictor

local predictor = Predictor.ConditionalGaussian{thetaMu = Vector.newDense(), thetaSigma = Vector.newDense()}
local alpha = 0.001
local function trainPredictor()
  local function iteration(iter)
    context:result("iteration", iter)
    local lossStats = Statistics.mean()
    local muStats = Statistics.mean()
    local sigmaStats = Statistics.mean()
    for i,example in ipairs(trainingDataset) do
      local features = example[1]
      local score = math.exp(-example[2])
      local mu, sigma = predictor.predict(features)
      muStats:observe(mu)
      sigmaStats:observe(sigma)
      local loss, dlossdmu, dlossdsigma = predictor.lossAndGradient(features, score)
      lossStats:observe(loss)
    --print ("loss: ", loss, dlossdmu:l2norm(), dlossdsigma:l2norm())
      predictor.__parameters.thetaMu:add(dlossdmu, -alpha)
      predictor.__parameters.thetaSigma:add(dlossdsigma, -alpha)
    end
    
    context:result("mean loss", lossStats:getMean())
    context:result("mean mu", muStats:getMean())
    context:result("mean sigma", sigmaStats:getMean())
    context:result("theta mu norm", predictor.__parameters.thetaMu:l2norm())
    context:result("theta sigma norm", predictor.__parameters.thetaSigma:l2norm())

    lossStats = Statistics.mean()
    for i,example in ipairs(testingDataset) do
      local features = example[1]
      local score = math.exp(-example[2])
      lossStats:observe(predictor.loss(features, score))
    end
    context:result("testing mean loss", lossStats:getMean())
    return lossStats:getMean()
  end

  local res
  for iter=1,100 do
    res = context:call("Iteration " .. iter, iteration, iter)    
  end
  context:result("thetaMu", predictor.__parameters.thetaMu)
  context:result("thetaSigma", predictor.__parameters.thetaSigma)
  return res
end

context:call("Training", trainPredictor)

--]]

--[[
for maxSize=1,3 do 
  problem.__parameters.maxSize = maxSize

  local function exhaustiveSearch(x)
    if problem.isFinal(x) then
      print (problem.stateToString(x))
    else
      local U = problem.U(x)
      for i,u in ipairs(U) do
        exhaustiveSearch(problem.f(x, u))
      end
    end
  end
  context:call("Exhaustive search " .. maxSize, exhaustiveSearch, problem, problem.x0)
end
--]]

local algo = DecisionProblem.SinglePlayerMCTS{
  indexFunction=DiscreteBandit.ucb1C{2}, verbose=false,
  partialExpand = false, fullPathExpand = false, useMaxReward = true}.__get

algo = DecisionProblem.Ubola{
  C = 1, alpha = 0.0001, verbose = false
}.__get

local actionFeaturesDictionary = Dictionary.new()
function problem:actionFeatures(x,u)

  local res = Vector.newSparse()
  local function setFeature(name, value)
    res[actionFeaturesDictionary:add(name)] = value or 1.0
  end

  local function makeTerminalFeature(ast)
    local cn = ast.className
    if cn == "lua::LiteralNumber" then
      return "number=" .. ast.value
    elseif cn == "lua::Identifier" then
      return "identifier=" .. ast.identifier
    elseif cn == "lua::UnaryOperation" then
      local ops = {"not", "len", "unm"}
      return ops[ast.op + 1]
    elseif cn == "lua::BinaryOperation" then
      local ops = {"add", "sub", "mul", "div", 
                   "mod", "pow", "concat", "eq",
                   "lt", "le", "and", "or"}
      return ops[ast.op + 1]
    elseif cn == "lua::Return" then
      return "return"
    else
      assert(false)
    end
  end

  local numExpressions = #x
  local size = stackOrExpressionSize(x)
  local prefix = "S" .. size .. " N" .. numExpressions .. " U" .. makeTerminalFeature(u)
  setFeature(prefix)
  for i,expr in ipairs(x) do
    local feature = prefix .. " " .. (#x - i + 1) .. makeTerminalFeature(expr)
    setFeature(feature)
  end
  setFeature("unit")
  return res
end

--context:call("random",  runSearchAlgorithm, symbolicRegressionProblem, randomSearchAlgorithm, 100, 100)
--context:call("nested1SearchAlgorithm", runSearchAlgorithm, symbolicRegressionProblem, nested1SearchAlgorithm, 100, 100)
context:call("mcts", runSearchAlgorithm, problem, algo, 100, 100)


--[[
local mctsUCB = DecisionProblem.SinglePlayerMCTS{numEpisodes=10000, indexFunction=DiscreteBandit.ucb1C{2}, verbose=false}
local mctsUCB2 = DecisionProblem.SinglePlayerMCTS{numEpisodes=10000, indexFunction=DiscreteBandit.ucb1C{0.4}, verbose=false}
local mctsUCBBernoulli = DecisionProblem.SinglePlayerMCTS{numEpisodes=10000, indexFunction=DiscreteBandit.ucb1Bernoulli, verbose=false}


context:call("MCTS-UCB 1", testSearchAlgorithm, problem, mctsUCB2)
context:call("MCTS-UCB 1", testSearchAlgorithm, problem, mctsUCB2)


local nestedMC1 = DecisionProblem.NestedMonteCarlo{level=1}
local nestedMC2 = DecisionProblem.NestedMonteCarlo{level=2}
local nestedMC3 = DecisionProblem.NestedMonteCarlo{level=3}

for i,C in ipairs({0, 1, 2, 3, 4, 5, 10}) do
  for j,alpha in ipairs({0.0001, 0.001, 0.01, 0.1}) do
    local ubola = DecisionProblem.Ubola{C=C, alpha=alpha, numEpisodes = 10000, numEpisodesPerIteration=10, verbose = false}
    context:call("Ubola C=" .. C .. " alpha = " .. alpha, testSearchAlgorithm, problem, ubola)
  end
end

context:call("MCTS-UCB 2", testSearchAlgorithm, problem, mctsUCB2)
context:call("MCTS-UCB Bernoulli", testSearchAlgorithm, problem, mctsUCBBernoulli)
context:call("NestedMC1", testSearchAlgorithm, problem, nestedMC1)
context:call("NestedMC2", testSearchAlgorithm, problem, nestedMC2)
context:call("NestedMC3", testSearchAlgorithm, problem, nestedMC3)

]]