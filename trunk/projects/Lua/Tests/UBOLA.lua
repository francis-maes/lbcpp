-- Upper Bound Open Loop Action Search -- 

require 'AST'
require 'Stochastic'
require 'Evaluator'
require 'DiscreteBandit'
require 'Sampler'

-- Transforms a formula represented through its abstract syntax tree
--  into an executable function
local function buildExpression(variables, expression)
  local code = "return function ("
  for j,variable in ipairs(variables) do
    code = code .. variable
    if j < #variables then
      code = code .. ","
    end
  end
  code = code .. ") return " .. expression:print() .. " end"
  --print (code)
  chunk,err = loadstring(code)
  if not chunk then
    error(err)
  end
  return chunk()
end

local function expressionSize(expression)
  local res = 1
  local n = expression:getNumSubNodes()
  for i=1,n do
    res = res + expressionSize(expression:getSubNode(i))
  end
  return res
end

local function stackOrExpressionSize(x)
  if type(x) == "table" then
    local res = 0
    for i,expression in ipairs(x) do
      res = res + expressionSize(expression)
    end
    return res
  else
    return expressionSize(x)     
  end
end

-- Reverse polish notation Sequential Decision Process

local RPNDecisionProblem = subspecified {

  parameter variables = {default={"a", "b", "c", "d"}},
  parameter constants = {default={1}},
  parameter unaryOperations = {default={"unm"}},
  parameter binaryOperations = {default={"add", "sub", "mul", "div"}},
  parameter objective = {},
  parameter maxSize = {default = 0, min = 0},

  x0 = {},
  discount = 1,

  isFinal = function (stack)
    return type(stack) ~= "table"
  end,

  -- Transition Function
  f = function (stack, ast)

    if ast.className == "lua::Return" then
      assert (#stack > 0)
      return stack[#stack]
    end

    local n = ast:getNumSubNodes()
    local m = #stack -- stack size
    assert(n <= m)
    local res = {}

    -- copy m-n first elements (pop n elements)
    for i=1,m-n do  
      res[i] = stack[i] 
    end

    -- fill operands using the popped elements
    if n > 0 then
      ast = ast:clone()
      for i=1,n do
        ast:setSubNode(i, stack[m-n+i])
      end
    end
   
    -- insert ast
    table.insert(res, ast)
    
    -- return new stack
    return res

  end,

  -- Available actions function
  U = function (stack)
    if type(stack) ~= "table" then
      return nil -- this is a final state
    end
    local ms = maxSize -- FIXME: there is a bug in the lua here !!! 
    if ms > 0 and stackOrExpressionSize(stack) >= ms then
      return {AST.returnStatement()} -- maxSize is reached, must finish now
    end

    local res = {}
    local m = #stack
    if m >= 2 then
      for i,v in ipairs(binaryOperations) do
        table.insert(res, AST.binaryOperation(v))
      end
    end
    if m >= 1 then
      for i,v in ipairs(unaryOperations) do
        table.insert(res, AST.unaryOperation(v))
      end
      table.insert(res, AST.returnStatement())
    end
    for i,v in ipairs(variables) do
      table.insert(res, AST.identifier(v))
    end
    for i,c in ipairs(constants) do
      table.insert(res, AST.literalNumber(c))
    end
    return res
  end,

  -- Reward function
  g = function (stack, ast)
    if ast.className == "lua::Return" then
      assert (#stack > 0)
      return math.exp(-objective(buildExpression(variables, stack[#stack])))
    else
      return 0
    end
  end,

  stateToString = function (x)
    if type(x) == "table" then
      local res = "{"
      for i,f in ipairs(x) do
        res = res .. f:print()
        if i < #x then
          res = res .. ", "
        end
      end
      return res .. "}"
    else
      return tostring(x)
    end
  end,

  actionToString = function (u)
    if u.className == "lua::UnaryOperation" then
      local strings = {"not", "len", "unm"}
      return strings[u.op + 1]
    elseif u.className == "lua::BinaryOperation" then
      local strings = {"add", "sub", "mul", "div",
                       "mod", "pow", "concat", "eq",
                       "lt", "le", "and", "or"}
      return strings[u.op + 1]
    else
      return u:print()
    end
  end
}


-- General sequential decision procedures

local function randomEpisode(problem, x)
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

local function nestedMonteCarlo(problem, x, level)

  local function actionSequenceToString(actionSequence)
    local res = ""
    for i,u in ipairs(actionSequence) do
      res = res .. problem.actionToString(u) .. " "
    end
    return res
  end

  local function argmax(episodeFunction, x)
    local bestScore = -math.huge
    local bestActionSequence
    local bestFinalState

    local U = problem.U(x)
    assert(U ~= nil)
    for i,u in ipairs(U) do
      local finalState, actionSequence, score = episodeFunction(problem.f(x,u))
      if score > bestScore then
        bestFinalState = finalState
        bestActionSequence = actionSequence
        table.insert(bestActionSequence, 1, u)
        bestScore = score
      end
    end
    --print ("argmax", problem.stateToString(x), actionSequenceToString(bestActionSequence))
    return bestFinalState, bestActionSequence, bestScore
  end

  local bestScore = -math.huge
  local bestActionSequence
  local bestFinalState
  local previousActions = {}
 
  while not problem.isFinal(x) do
    local finalState, actionSequence, score
    if level == 1 then
      finalState, actionSequence, score = argmax(|x| randomEpisode(problem, x), x)
    else
      finalState, actionSequence, score = argmax(|x| nestedMonteCarlo(problem, x, level-1), x)
    end
    --print (actionSequenceToString(actionSequence), finalState:print(), score)

    if score > bestScore then
      bestFinalState = finalState
      bestScore = score
      bestActionSequence = {}
      for i,u in ipairs(previousActions) do table.insert(bestActionSequence, u) end
      for i,u in ipairs(actionSequence) do table.insert(bestActionSequence, u) end
    end

    if not bestActionSequence then
      return bestFinalState, bestActionSequence, bestScore
    end

    local u = bestActionSequence[#previousActions + 1]
    --print ("Selected action: " .. problem.actionToString(u))
    x = problem.f(x,u)
    table.insert(previousActions, u)
  end
  return bestFinalState, bestActionSequence, bestScore
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


symbolicRegressionProblem = RPNDecisionProblem{
  constants = {1,2,5,10},
  objective = makeSymbolicRegressionObjective,
  maxSize = 10
}

-- Test2 : bandits formula

local function banditsFormulaObjective(f)

  local policy = DiscreteBandit.indexBasedPolicy{indexFunction = f}.__get
  local bandits = {Sampler.Bernoulli{p=0.9}, Sampler.Bernoulli{p=0.6}}
  return DiscreteBandit.estimatePolicyRegret(policy, bandits, 100, 100)  -- num estimations and time horizon
  
end

banditsProblem = RPNDecisionProblem{
  variables = {"rk", "sk", "tk", "t"},
  constants = {1,2,5,10},
  objective = banditsFormulaObjective,
  maxSize = 10
}

-- Main

local problem = banditsProblem

print ("coucou")
local bestScore = math.huge
for i=1,10 do
  local bestFormula, bestActionSequence, bestReturn = nestedMonteCarlo(problem, problem.x0, 2)
  print (bestFormula:print(), bestReturn)
end




-------------------------

subspecified function UbolaSearch(x0, f)

  parameter numEpisodes = {default = 10000}
  parameter numEpisodesPerIteration = {default = 100}

  local episodeNumber = 1

  local function episode()

    

  end

  local numIterations = math.ceil(numEpisodes / numEpisodesPerIteration)
  for iter=1,numIterations do
    context:enter("Episodes " .. episodeNumber .. " -- " .. (episodeNumber + numEpisodesPerIteration - 1))
    for e=1,numEpisodesPerIteration do
      episode()
      episodeNumber = episodeNumber + 1
    end
    context:leave()
  end
end

local x0 = {}
local f = nil

local ubola = UbolaSearch{}
--ubola(x0, f)