-- Optimize Formula Constants

require 'AST'
require 'DiscreteBandit'
require 'Stochastic'
require 'Sampler'
require 'Random'
require 'Vector'
require 'Optimizer'
require 'Language.LuaChunk'
require '../ExpressionDiscovery/ReversePolishNotationProblem'

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

banditObjective = makeBanditObjective(2, 10, 10, 1, 100)

local function getNumConstants(ast)
  local res = 0
  local function f(ast)
    if ast.className == "lua::Index" then
      if ast:getSubNode(1):print() == "math" then
        return
      end
      local index = tonumber(ast:getSubNode(2):print())
      if index > res then res = index end
    else
      for i=1,ast:getNumSubNodes() do f(ast:getSubNode(i)) end
    end
  end
  f(ast)
  return res
end

function evaluateBanditFormulaStructure(str)
  local ast = AST.parseExpressionFromString(str)
  local f = buildExpression(ast, {"rk", "sk", "tk", "t"}, true)
  local numConstants = getNumConstants(ast)
  local objective = lbcpp.LuaFunction.create(|csts| banditObjective(|rk,sk,tk,t| f(csts, rk, sk, tk, t) ),
                "DenseDoubleVector<EnumValue,Double>", "Double")
  if numConstants == 0 then
    return banditObjective(|rk,sk,tk,t| f(nil, rk, sk, tk, t))
  else
    local optimizer = Optimizer.CMAES{numIterations=10}
    local score,solution = optimizer{objective = objective, initialGuess = Vector.newDense(numConstants)}
    return score
  end
end
