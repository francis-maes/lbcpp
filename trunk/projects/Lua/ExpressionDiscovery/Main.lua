
-- Main experiment script

require '../ExpressionDiscovery/ReversePolishNotationProblem'
require '../ExpressionDiscovery/MCTS'
require '../ExpressionDiscovery/NestedMonteCarlo'
require '../ExpressionDiscovery/UBOLA'
require 'DiscreteBandit'
require 'Evaluator'
require 'Sampler'


-- decorate objective function 

local numberOfObjectiveEvaluations = 0
local bestSolutionScore = math.huge 
local bestSolutionFound
local bestSolutionNumberOfEvaluations

local function decorateObjectiveFunction(objective)
  
  return function (candidate, candidateDescription)
    numberOfObjectiveEvaluations = numberOfObjectiveEvaluations + 1
    local score = objective(candidate)
    if score < bestSolutionScore then
      bestSolutionScore = score
      bestSolutionFound = candidateDescription
      bestSolutionNumberOfEvaluations = numberOfObjectiveEvaluations
    end
    return score
  end

end

local function actionSequenceToString(problem, actionSequence)
  if actionSequence then
    local res = ""
    for i,u in ipairs(actionSequence) do
      res = res .. problem.actionToString(u) .. " "
    end
    return res
  else
    return "<empty sequence>"
  end
end


local function testSearchAlgorithm(problem, algorithm)

  local function run()

    numberOfObjectiveEvaluations = 0
    bestSolutionScore = math.huge 
    bestSolutionFound = nil
    bestSolutionNumberOfEvaluations = nil
  
    local bestFormula, bestActionSequence, bestReturn = algorithm(problem)
  
    context:result("bestFormula", bestFormula:print())
    context:result("bestActionSequence", actionSequenceToString(problem, bestActionSequence))
    context:result("bestReturn", bestReturn)
    context:result("numEvaluations", numberOfObjectiveEvaluations)
    context:result("bestScore", bestSolutionScore)
    context:result("bestFormulaCheck", bestSolutionFound)
    context:result("bestSolutionNumEvaluation", bestSolutionNumberOfEvaluations)
  
    context:information("Performed " .. numberOfObjectiveEvaluations .. " evaluations")
    context:information("Found \"" .. bestSolutionFound .. "\" after " ..
                bestSolutionNumberOfEvaluations .. " evaluations (score = " ..
                bestSolutionScore .. ")")
  end

  for i=1,5 do
    context:call("Run " .. i, run)
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
    table.insert(dataset, example)
  end
  return |f| Evaluator.meanSquaredError(f, dataset)
end


symbolicRegressionProblem = DecisionProblem.ReversePolishNotation{
  constants = {1,2,5,10},
  objective = decorateObjectiveFunction(makeSymbolicRegressionObjective()),
  maxSize = 5
}

-- Test2 : bandits formula

local function banditsFormulaObjective(f)

  local policy = DiscreteBandit.indexBasedPolicy{indexFunction = f}.__get
  local bandits = {Sampler.Bernoulli{p=0.9}, Sampler.Bernoulli{p=0.6}}
  return DiscreteBandit.estimatePolicyRegret(policy, bandits, 10, 100)  -- num estimations and time horizon
  
end

banditsProblem = DecisionProblem.ReversePolishNotation{
  variables = {"rk", "sk", "tk", "t"},
  constants = {1,2,5,10},
  objective = decorateObjectiveFunction(banditsFormulaObjective),
  maxSize = 10
}

-- Main

local problem = symbolicRegressionProblem

--for maxSize=1,3 do 
--  problem.__parameters.maxSize = maxSize

--  local function exhaustiveSearch(x)
--    if problem.isFinal(x) then
--      print (problem.stateToString(x))
--    else
--      local U = problem.U(x)
--      for i,u in ipairs(U) do
--        exhaustiveSearch(problem.f(x, u))
--      end
--    end
--  end
--  context:call("Exhaustive search " .. maxSize, exhaustiveSearch, problem, problem.x0)
--end


local mctsUCB = DecisionProblem.SinglePlayerMCTS{numEpisodes=50000, indexFunction=DiscreteBandit.ucb1C{2}, verbose=false}
local mctsUCB2 = DecisionProblem.SinglePlayerMCTS{numEpisodes=50000, indexFunction=DiscreteBandit.ucb1C{0.4}, verbose=false}
local mctsUCBBernoulli = DecisionProblem.SinglePlayerMCTS{numEpisodes=50000, indexFunction=DiscreteBandit.ucb1Bernoulli, verbose=false}

local nestedMC1 = DecisionProblem.NestedMonteCarlo{level=1}
local nestedMC2 = DecisionProblem.NestedMonteCarlo{level=2}
local nestedMC3 = DecisionProblem.NestedMonteCarlo{level=3}

local ubola = DecisionProblem.Ubola{numEpisodes = 10000, verbose = true}

context:call("Ubola", testSearchAlgorithm, problem, ubola)

--context:call("MCTS-UCB 1", testSearchAlgorithm, problem, mctsUCB)
--context:call("MCTS-UCB 2", testSearchAlgorithm, problem, mctsUCB2)
--context:call("MCTS-UCB Bernoulli", testSearchAlgorithm, problem, mctsUCBBernoulli)
--context:call("NestedMC1", testSearchAlgorithm, problem, nestedMC1)
--context:call("NestedMC2", testSearchAlgorithm, problem, nestedMC2)
--context:call("NestedMC3", testSearchAlgorithm, problem, nestedMC3)