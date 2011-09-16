require 'Vector'
require 'Dictionary'
require 'Parser'
require 'Data'
require 'Statistics'
require 'IterationFunction'
require 'Loss'
require 'Predictor'
require 'Optimizer'
require 'Evaluator'

--for k,v in ipairs(examples) do 
--  for k2,v2 in ipairs(v) do
--    print (k2,v2)
--  end
--end

local dataset = "/Users/francis/Desktop/datasets/australian_scale.all.0.5"

local trainFilename = dataset .. ".train"
local testFilename = dataset .. ".test"
local labels = Dictionary.new()
local trainExamples = Data.load(Parser.libSVMClassification, 0, trainFilename, labels)
local testExamples = Data.load(Parser.libSVMClassification, 0, testFilename, labels)
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

print (#trainExamples .. " training examples, " .. #testExamples .. " testing examples " .. labels:size() .. " labels")

local classifier = Predictor.LinearBinaryClassifier{}

local function makeObjectiveFunction(updateRule)
  return function (index, parameters, epoch)
    local example = trainExamples[index]
    local x = example[1]
    local supervision = example[2]
    local prediction = parameters:dot(x)
    local sign = supervision == 2 and 1 or -1
    local score = prediction * sign
    local dloss = updateRule(score, epoch, supervision)
    return score > 0 and 1 or 0, x * (sign * dloss)
  end
end


local function evaluateUpdateRuleFold(updateRule, trainExamples, validationExamples)

  local function trainAccuracyFunction(parameters)
   return Evaluator.accuracy(Predictor.LinearBinaryClassifier{theta=parameters}, trainExamples)
  end 

  local function validationAccuracyFunction(parameters)
    return Evaluator.accuracy(Predictor.LinearBinaryClassifier{theta=parameters}, validationExamples)
  end

  local problem = {
    initialSolution = Vector.newDense(),
    numSamples = #trainExamples,
    objective = makeObjectiveFunction(updateRule),
    scores = { ["Train score"] = trainAccuracyFunction, ["Validation score"] = validationAccuracyFunction },
    principalScore = "Validation score"
  }

  sgd = Optimizer.StochasticGradientDescent{maxIterations=2,restoreBestParameters=false}

  solution, score = sgd(problem)
  
  return score
end

local function evaluateUpdateRule(updateRule, folds, trainExamples, testExamples)
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

local function updateRuleObjective(updateRule)
  return evaluateUpdateRule(updateRule, folds, trainExamples, testExamples)
end

--context:call("constant rate 1", evaluateUpdateRule, ConstantRateWithHingeLoss{rate=0.0}, folds, trainExamples, testExamples)
--context:call("constant rate 10", evaluateUpdateRule, ConstantRateWithHingeLoss{rate=1.0}, folds, trainExamples, testExamples)
--context:call("constant rate 0.1", evaluateUpdateRule, ConstantRateWithHingeLoss{rate=-1.0}, folds, trainExamples, testExamples)
--context:call("invlinear", evaluateUpdateRule, InvLinearRateWithHingeLoss{initialValue=0.0, halfPeriod=2}, folds, trainExamples, testExamples)

local function optimizeUpdateRule(numParameters, functor)

  local evaluations = {}

  local objective = function (parameters)
    local cvScore, testScore = updateRuleObjective(functor(parameters))
    table.insert(evaluations, {parameters, cvScore, testScore})
    return - cvScore
  end

  objective = lbcpp.LuaFunction.create(objective, "DenseDoubleVector<EnumValue,Double>", "Double")
  local optimizer = Optimizer.CMAES{numIterations=10}
  local score,solution = optimizer{objective = objective, initialGuess = Vector.newDense(numParameters)}
  print ("score", score, "solution", solution)
  local updateRule = functor(solution)
  local res = context:call("Test", evaluateUpdateRuleFold, updateRule, trainExamples, testExamples)

  local function makeEvalutionsCurve()
    for i,e in ipairs(evaluations) do
      context:enter("Evaluation " .. i)
      context:result("evaluation", i)
      local parameters = e[1]
      for index=1,#parameters do
        context:result("parameter " .. index, parameters[index])
      end
      context:result("cvScore", e[2])
      context:result("testScore", e[3])
      context:leave()
    end
  end
  context:call("Evaluations", makeEvalutionsCurve)

  return res
end


------------------------------

subspecified function ConstantRateWithHingeLoss(score, epoch, supervision)
  parameter rate = {default=0.0}
  local r = math.pow(10, rate)
  return score > 1 and 0 or -r
end

local function constantRateWithHingeLossFunctor(parameters)
  return ConstantRateWithHingeLoss{rate=parameters[1]}
end

subspecified function InvLinearRateWithHingeLoss(score, epoch, supervision)
  parameter initialValue={default=0.0}
  parameter halfPeriod={default=3.0}
  local hp = math.pow(10.0,halfPeriod)
  local r = math.pow(10,initialValue) * hp / (hp + epoch)
  return score > 1 and 0 or -r
end

local function invLinearRateWithHingeLossFunctor(parameters)
  return InvLinearRateWithHingeLoss{initialValue=parameters[1], halfPeriod=parameters[2]}
end

subspecified function Simple0(score, epoch, supervision)
  parameter parameters={}
  local a = parameters[1]
  local b = parameters[2]
  return b / (1 + math.exp(a * score))
end


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

--context:call("optimize constant", optimizeUpdateRule, 1, constantRateWithHingeLossFunctor)
--context:call("optimize invlinear", optimizeUpdateRule, 2, invLinearRateWithHingeLossFunctor)
--context:call("optimize simple0", optimizeUpdateRule, 2, |p| Simple0{parameters=p})
--context:call("optimize simple1", optimizeUpdateRule, 5, |p| Simple1{parameters=p})
context:call("optimize simple2", optimizeUpdateRule, 5, |p| Simple2{parameters=p})

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