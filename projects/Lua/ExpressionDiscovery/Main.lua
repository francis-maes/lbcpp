
-- Main experiment script

require '../ExpressionDiscovery/ReversePolishNotationProblem'
require '../ExpressionDiscovery/MCTS'
require '../ExpressionDiscovery/NestedMonteCarlo'
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


-- Test1 : symbolic regression

local function makeSymbolicRegressionObjective()
  local dataset = {}
  for i = 1,100 do
    local x = {
      Stochastic.standardGaussian(), 
      Stochastic.standardGaussian(), 
      Stochastic.standardGaussian(), 
      Stochastic.standardGaussian()
    }
    local y = x[1] * x[2] + x[3]  -- a b * c +
    table.insert(dataset, {x,y})
  end
  return |f| Evaluator.meanSquaredError(f, dataset)
end


symbolicRegressionProblem = DecisionProblem.ReversePolishNotation{
  constants = {1,2,5,10},
  objective = decorateObjectiveFunction(makeSymbolicRegressionObjective()),
  maxSize = 10
}

-- Test2 : bandits formula

local function banditsFormulaObjective(f)

  local policy = DiscreteBandit.indexBasedPolicy{indexFunction = f}.__get
  local bandits = {Sampler.Bernoulli{p=0.9}, Sampler.Bernoulli{p=0.6}}
  return DiscreteBandit.estimatePolicyRegret(policy, bandits, 100, 100)  -- num estimations and time horizon
  
end

banditsProblem = DecisionProblem.ReversePolishNotation{
  variables = {"rk", "sk", "tk", "t"},
  constants = {1,2,5,10},
  objective = decorateObjectiveFunction(banditsFormulaObjective),
  maxSize = 10
}

-- Main


local problem = symbolicRegressionProblem

local mcts = DecisionProblem.SinglePlayerMCTS{numEpisodes=100000, indexFunction=DiscreteBandit.ucb1C{2}, verbose=false}
local nestedMC1 = DecisionProblem.NestedMonteCarlo{level=1}
local nestedMC2 = DecisionProblem.NestedMonteCarlo{level=2}
local nestedMC3 = DecisionProblem.NestedMonteCarlo{level=3}

context:call("MCTS", testSearchAlgorithm, problem, mcts)
context:call("NestedMC1", testSearchAlgorithm, problem, nestedMC1)
context:call("NestedMC2", testSearchAlgorithm, problem, nestedMC2)
context:call("NestedMC3", testSearchAlgorithm, problem, nestedMC3)
