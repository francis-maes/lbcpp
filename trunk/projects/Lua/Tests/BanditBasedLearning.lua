require 'Vector'
require 'Dictionary'
require 'Parser'
require 'Data'
require 'Statistics'
require 'IterationFunction'
require 'Predictor'
require 'Evaluator'
require 'Optimizer'

-- Load Dataset

local directory = "C:\\Projets\\lbcpp\\workspace\\ClassificationTests\\datasets"
--local directory = "/Users/francis/Desktop/datasets"
local dataset = "large/news20.binary.0.5"
--local dataset = "australian_scale.all.0.5"

local useStochasticGD = true
local numGDIterations = 25


dataset = directory .. "/" .. dataset
local trainFilename = dataset .. ".train"
local testFilename = dataset .. ".test"
local labels = Dictionary.new()
local trainExamples = Data.load(Parser.libSVMClassification, 0, trainFilename, labels)
local testExamples = Data.load(Parser.libSVMClassification, 0, testFilename, labels)

print (#trainExamples .. " training examples, " .. #testExamples .. " testing examples " .. labels:size() .. " labels")

-- Make cross validation folds
local function makeFolds(examples, numFolds)
  local res = {}

  local numSamples = #examples
  local order = Stochastic.uniformOrder(numSamples)
  local meanFoldSize = numSamples / numFolds;
  local scores = Statistics.mean()
  for i=1,numFolds do
    local foldBegin = 1 + math.floor((i - 1) * meanFoldSize)
    local foldEnd = 1 + math.floor(i * meanFoldSize)
    local trainExamples = {}
    local testExamples = {}
    for index=1,numSamples do
      local tbl = (index >= foldBegin and index < foldEnd) and testExamples or trainExamples
      table.insert(tbl, examples[index])
    end
    print ("Fold ", i, "num training:", #trainExamples, "num testing", #testExamples)
    table.insert(res, {trainExamples, testExamples})
  end
  return res
end
folds = makeFolds(trainExamples, 5)


local function makeSGDObjectiveFunction(updateRule)
  return function (index, parameters, epoch)
    local example = trainExamples[index]
    local x = example[1]
    local supervision = example[2]
    local prediction = parameters:dot(x)
    local sign = supervision == 2 and 1 or -1
    local score = prediction * sign
    local dloss = updateRule:lossGradient(score, epoch, supervision) * updateRule:learningRate(epoch)
    return score > 0 and 0 or 1, x * (sign * dloss)
  end
end

-- Batch learning procedure
local function evaluateUpdateRuleFold(updateRule, trainExamples, validationExamples, returnScoreMean)

  local function trainAccuracyFunction(parameters)
   return Evaluator.accuracy(Predictor.LinearBinaryClassifier{theta=parameters}, trainExamples)
  end 

  local function validationAccuracyFunction(parameters)
    return Evaluator.accuracy(Predictor.LinearBinaryClassifier{theta=parameters}, validationExamples)
  end


  if useStochasticGD then
    -- Stochastic Gradient Descent
    local problem = {
      initialSolution = Vector.newDense(),
      numSamples = #trainExamples,
      objective = makeSGDObjectiveFunction(updateRule),
      scores = { ["Train score"] = trainAccuracyFunction, ["Validation score"] = validationAccuracyFunction },
      principalScore = "Validation score"
    }

    sgd = Optimizer.StochasticGradientDescent{maxIterations=numGDIterations,restoreBestParameters=false}

    solution, score = sgd(problem)
    return score
  else
    -- Batch Gradient Descent

    local numIterations=numGDIterations
    local parameters = Vector.newDense()
    local gradients = {}
    local epoch = 1
  
    local function doIteration(iteration)
      context:result("iteration", iteration)
  
      local gradient = Vector.newDense()
      for i,example in ipairs(trainExamples) do
        local x = example[1]
        local supervision = example[2]
        local prediction = parameters:dot(x)
        local sign = supervision == 2 and 1 or -1
        local score = prediction * sign
        local dloss = updateRule:lossGradient(score, supervision) * updateRule:learningRate(epoch)
        gradient:add(x, sign * dloss)

        epoch = epoch + 1 
        if (epoch % 100) == 0 then
          table.insert(gradients, 1, gradient)
          updateRule:update(parameters, gradients)
        end
      end
          
      
      --local l2norm = parameters:l2norm()
      --if l2norm > 1 then
      --  parameters:mul(1 / l2norm)
      --end
      local validationScore = validationAccuracyFunction(parameters)
      context:result("validation score", validationScore)
      context:result("parameters l2norm", parameters:l2norm())
      context:result("gradient l2norm", gradient:l2norm())
      context:result("gradient", gradient)
      context:result("parameters", parameters:clone())
      return validationScore
    end
  
    local score = 0.0
    local lastScore
    for iteration=1,numIterations do
      lastScore = context:call("Iteration " .. iteration, doIteration, iteration)
      score = score + lastScore
    end    
    return returnScoreMean and score / numIterations or lastScore
  end
end


--
-- Cross Validation
--
local function crossValidateUpdateRule(updateRule)
  local numFolds = #folds

  -- evaluate for each fold
  local scores = Statistics.mean()
  for i=1,numFolds do
    local trainExamples = folds[i][1]
    local testExamples = folds[i][2]
    local score = context:call("Fold " .. i .. " / " .. numFolds, evaluateUpdateRuleFold, updateRule, trainExamples, testExamples)
    scores:observe(score)
  end
  local cvScore = scores:getMean()
  local testScore = context:call("Test", evaluateUpdateRuleFold, updateRule, trainExamples, testExamples)
  context:result("cvScore", cvScore)
  context:result("testScore", testScore)
  print ("CVScore: " .. cvScore .. ", Test Score: " .. testScore)
  return cvScore, testScore
end

--
-- Bandit optimization
--
local function evaluateUpdateRuleOnRandomSplit(updateRule, trainExamples)
  -- train and test on a random split
  local train = {}
  local validation = {}
  for i,e in ipairs(trainExamples) do
    local tbl = Stochastic.bernoulli(0.1) == 1 and validation or train
    table.insert(tbl, e)
  end
  print ("Truc: " .. #train .. " train examples " .. #validation .. " validation examples")
  return evaluateUpdateRuleFold(updateRule, train, validation, true)
end

local function optimizeUpdateRule(numParameters, functor)

  local evaluations = {}

  local banditObjective = function (parameters)
    local score = evaluateUpdateRuleOnRandomSplit(functor(parameters), trainExamples)
    table.insert(evaluations, {parameters, score})
    return score
  end

  local optimisticObjective = function (parameters)
    local score = evaluateUpdateRuleFold(functor(parameters), trainExamples, testExamples, false)
    table.insert(evaluations, {parameters, score})
    return -score  -- for minimization problems
  end

  local objective = optimisticObjective

  objective = lbcpp.LuaFunction.create(objective, "DenseDoubleVector<EnumValue,Double>", "Double")
  local optimizer = Optimizer.CMAES{numIterations=50}
  --local optimizer = Optimizer.HOO{numIterations=numParameters * 100, nu=1.0, rho=0.5, maxDepth=math.max(5,numParameters * 2)}
  local score,solution = optimizer{objective = objective, initialGuess = Vector.newDense(numParameters)}
  print ("score", score, "solution", solution)
  local updateRule = functor(solution)
  local res = context:call("Test", evaluateUpdateRuleFold, updateRule, trainExamples, testExamples, false)

  local function makeEvalutionsCurve()
    for i,e in ipairs(evaluations) do
      context:enter("Evaluation " .. i)
      context:result("evaluation", i)
      local parameters = e[1]
      for index=1,#parameters do
        context:result("parameter " .. index, parameters[index])
      end
      context:result("score", e[2])
      context:leave()
    end
  end
  context:call("Evaluations", makeEvalutionsCurve)

  return res
end


local function randomOptimizeUpdateRule(numParameters, functor) 
  local objective = function (parameters)
    local score = evaluateUpdateRuleFold(functor(parameters), trainExamples, testExamples, false)
    return score
  end

  local bestParameters
  local bestScore = -math.huge

  for i=1,numParameters*100 do
    context:enter("Iteration " .. i)
    context:result("sample", i)
    local parameters = Vector.newDense(numParameters)
    for j=1,numParameters do
      local param = Stochastic.standardUniform()
      context:result("param " .. j, param)
      parameters[j] = param
    end
    
    score = context:call("Evaluate " .. tostring(parameters), objective, parameters)
    context:result("testScore", score)
    
    if score > bestScore then
      bestScore = score
      bestParameters = parameters
    end
    context:leave(bestScore)
  end
  return bestScore, bestParameters
end


------------------------------

ConstantRateWithHingeLoss = subspecified {
  parameter rate = {default = 1},
  lossGradient = function (self, score, supervision)
    return score > 1 and 0 or -1
  end,
  learningRate = function (self, epoch)
    return rate;
  end,
  update = function (self, parameters, gradients)
    parameters:add(gradients[1], -rate)
    --parameters:mul(0.9)
  end
}

local function constantRateWithHingeLossFunctor(parameters)
  return ConstantRateWithHingeLoss{rate=math.pow(10, -5 + 10 * parameters[1])}.__get
end

InvLinearRateWithHingeLoss = subspecified {
  parameter rate = {default=1},
  parameter halfPeriod = {default=1},
  lossGradient = function (self, score, supervision)
    return score > 1 and 0 or -1
  end,
  learningRate = function (self, epoch)
    return rate * halfPeriod / (halfPeriod + epoch)
  end,
  update = function (self, parameters, gradients)
    local iteration = #gradients - 1
    local r =  self:learningRate(#gradients)
    parameters:add(gradients[1], -r)
    --parameters:mul(0.9)
  end
}

local function invLinearRateWithHingeLossFunctor(parameters)
  return InvLinearRateWithHingeLoss{rate=math.pow(10, -5 + 10 * parameters[1]), halfPeriod=math.pow(10, 4 * parameters[2])}
end


RegularizedInvLinearRateWithHingeLoss = subspecified {
  parameter p = {},
  lossGradient = function (self, score, supervision)
    return score > 1 and 0 or -1
  end,
  learningRate = function (self, epoch)
    local rate = math.pow(10, -5 + 10 * p[1])
    local halfPeriod = math.pow(10, 5 * p[2])
    return rate * halfPeriod / (halfPeriod + epoch)
  end,
  update = function (self, parameters, gradients)
    local iteration = #gradients - 1
    parameters:add(gradients[1], -self:learningRate(#gradients))
    parameters:mul(1 - p[3] / 2)
  end
}

Simple0 = subspecified {
  parameter p = {},
  lossGradient = function (self, score, supervision)
    local g = supervision == 2 and p[1] or p[2]
    return score > p[3] and 0 or -g
  end,
  learningRate = function (self, epoch)
    return 1
  end,
  update = function (self, parameters, gradients)
    local d = math.max(#parameters, #gradients[1])
    if #parameters < d then parameters:resize(d) end
    for i=1,d do
      local g = gradients[1][i]
      local prevG = #gradients >= 2 and gradients[2][i] or 0.0
      local param = i < #parameters and parameters[i] or 0.0

      param = param * (2*p[4]-1) + g * (2*p[5]-1) + --prevG * (2*p[6]-1) +
              param * g * (2*p[7]-1) -- + param * prevG * (2*p[8]-1) + g * prevG * (2*p[9]-1)

      if math.abs(param) < p[10] then
        param = 0
      end
      if param > 1 then
        param = 1
      elseif param < -1 then
        param =-1
      end
      parameters[i] = param
    end
  end
}

Simple1 = subspecified {
  parameter p = {},
  lossGradient = function (self, score, supervision)
    local g = math.pow(10, supervision == 2 and (2*p[1]-1) or (2*p[2]-1))
    if score < 2*p[3]-1 then
      return g * (2*p[4]-1)
    elseif score < p[5] then
      return g * (2*p[6]-1)
    else
      return g * (2*p[7]-1)
    end
  end,
  learningRate = function (self, epoch)
    local halfPeriod = math.pow(10, 5 * p[8])
    return halfPeriod / (halfPeriod + epoch)
  end
}

--[[
subspecified function Simple1(score, epoch, supervision)
  parameter parameters={}
  local threshold = parameters[1]
  local a = parameters[2]
  local b = parameters[3]
  local c = parameters[4]
  local d = parameters[5]
  local ds = 1 / (1 + math.exp(score - threshold))
  if ds > 0.5 then
    return a * ds + b
  else
    return c * ds + d
  end
end

subspecified function Simple2(score, epoch, supervision)
  parameter parameters={}
  local normalized = 1 / (1 + math.exp(-score))
  --print (score, "=>", math.floor(normalized * 10), "=>", parameters[1 + math.floor(normalized * 10)])
  return parameters[1 + math.floor(normalized * 5)]
end
]]


----------------------
--[[
context:enter("supercurve")

for p=0,1,0.01 do
  context:enter("P=" .. p)
  context:result("p", p)
  crossValidateUpdateRule(constantRateWithHingeLossFunctor({p}))
  context:leave()
end
context:leave()
]]

-------------

context:call("Test Constant(1)", evaluateUpdateRuleFold, ConstantRateWithHingeLoss{rate=1.0}.__get, trainExamples, testExamples, false)
context:call("test simple1", randomOptimizeUpdateRule, 8, |p| Simple1{p=p})
context:call("test constant", randomOptimizeUpdateRule, 1, constantRateWithHingeLossFunctor)
context:call("test invlinear", randomOptimizeUpdateRule, 2, invLinearRateWithHingeLossFunctor)


--[[
context:call("optimize simple1", optimizeUpdateRule, 8, |p| Simple1{p=p})
context:call("optimize constant", optimizeUpdateRule, 1, constantRateWithHingeLossFunctor)
context:call("optimize invlinear", optimizeUpdateRule, 2, invLinearRateWithHingeLossFunctor)

context:call("test invlinear", randomOptimizeUpdateRule, 2, invLinearRateWithHingeLossFunctor)

context:call("test invlinearreg", randomOptimizeUpdateRule, 3, |p| RegularizedInvLinearRateWithHingeLoss{p=p})
context:call("optimize invlinearreg", optimizeUpdateRule, 3, |p| RegularizedInvLinearRateWithHingeLoss{p=p})
]]

--[[
context:call("test simple1", randomOptimizeUpdateRule, 4, |p| Simple1{p=p})
context:call("test simple0", randomOptimizeUpdateRule, 10, |p| Simple0{p=p})
]]


--[[
context:call("optimize simple0", optimizeUpdateRule, 10, |p| Simple0{p=p})

context:call("optimize simple1", optimizeUpdateRule, 5, |p| Simple1{parameters=p})
context:call("optimize simple2", optimizeUpdateRule, 6, |p| Simple2{parameters=p})
]]

----------------

local function makeCurves(numParameters, functor)


  local objective = |parameters| 1.0 - updateRuleObjective(functor(parameters))

  local parameters = Vector.newDense(numParameters)

  for k=-5,5,0.1 do
    context:enter("k=" .. k)
    context:result("k", k)
    parameters[1] = k
    context:result("y", objective(parameters))
    context:leave()
  end
end

--context:call("optimize invlinear", makeCurves, 11, |p| Simple2{parameters=p})