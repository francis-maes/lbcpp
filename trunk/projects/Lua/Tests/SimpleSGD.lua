require 'Vector'
require 'Dictionary'
require 'Parser'
require 'Data'
require 'Statistics'
require 'IterationFunction'
require 'Loss'

--for k,v in ipairs(examples) do 
--  for k2,v2 in ipairs(v) do
--    print (k2,v2)
--  end
--end

--problem = {
--  initialSolution,
--  numSamples,
--  objective,  --index, parameters -> value, gradient
--  scores, --{name, parameter->score} map
--  principalScore -- score name
--}

-- objective: index, parameters -> value, gradient
-- validation: parameters -> value
subspecified function StochasticGradientDescent(problem)
  -- todo: parameter randomizeExamples = {default = true}
  parameter rate = {default = IterationFunction.constant{1}}
  -- todo: StoppingCriterion
  parameter maxIterations = {default = 0, min = 0}
  parameter restoreBestParameters = {default = true}

  local parameters = problem.initialSolution
  assert(parameters)
  local iter = 1
  local epoch = 1

  local validationScore = math.huge
  local bestPrincipalScore = math.huge
  local bestParameters = parameters

  local function iteration()
    local scoreStats = Statistics.mean()
    for i=1,problem.numSamples do
      local index = i -- todo: randomize examples
      
      local score, gradient = problem.objective(index, parameters)
      --print (score,gradient)
      scoreStats:observe(score)
      if gradient then
        parameters:add(gradient, -rate(epoch))
        --print (parameters, parameters:l2norm())
      end
      epoch = epoch + 1
    end

    context:result("iteration", iter)
    context:result("score stats", scoreStats)
    context:result("mean score", scoreStats:getMean())
    context:result("parameters l0norm", parameters:l0norm())
    context:result("parameters l1norm", parameters:l1norm())
    context:result("parameters l2norm", parameters:l2norm())
    context:result("parameters", parameters:clone())
    context:result("rate", rate(epoch))
    context:result("epoch", epoch)

    local principalScore
    for scoreName,scoreFunction in pairs(problem.scores) do
      local score = scoreFunction(parameters)
      context:result(scoreName, score)
      if scoreName == problem.principalScore then
        principalScore = score
      end
    end
    if restoreBestParameters then
      assert(principalScore)
      if principalScore > bestPrincipalScore then
        bestPrincipalScore = principalScore
        bestParameters = parameters:clone()
      end
    end
  end

  while maxIterations == 0 or iter <= maxIterations do
    context:call("Iteration " .. iter,  iteration)
    iter = iter + 1
  end

  if restoreBestParameters then
    parameters = bestParameters
    validationScore = bestValidationScore
  end
  return parameters, validationScore
end

----------------------------------------------

local trainFilename = package.inteluaPath .. "/../../Examples/Data/BinaryClassification/a1a.train"
local testFilename = package.inteluaPath .. "/../../Examples/Data/BinaryClassification/a1a.test"
local labels = Dictionary.new()
local trainExamples = Data.load(Parser.libSVMClassification, 100, trainFilename, labels)
local testExamples = Data.load(Parser.libSVMClassification, 100, testFilename, labels)

print (#trainExamples .. " training examples, " .. #testExamples .. " testing examples " .. #labels .. " labels")

local function decomposableObjectiveFunction(index, parameters)
  local lossFunction = Loss.binary{Loss.hinge}
  local example = trainExamples[index]
  local input = example[1]
  local prediction = parameters:dot(input)
  --print ("prediction", prediction, parameters:l2norm(), input:l2norm())
  local loss = lossFunction(prediction, example[2])
  local dloss = lossFunction[1](prediction, example[2])
  --print (index,prediction, loss, dloss, example[2])
  return loss, input * dloss
end

local function linearBinaryClassifierAccuracy(parameters, examples)
  local stats = Statistics.mean()  
  for i,example in ipairs(examples) do
    local prediction = parameters:dot(example[1]) * (example[2] and 1 or -1)
    stats:observe(prediction > 0 and 1 or 0)
  end
  return stats:getMean()
end

local function trainAccuracyFunction(parameters)
  return linearBinaryClassifierAccuracy(parameters, trainExamples)
end

local function testAccuracyFunction(parameters)
  return linearBinaryClassifierAccuracy(parameters, testExamples)
end


local problem = {
  initialSolution = Vector.newDense(),
  numSamples = #trainExamples,
  objective = decomposableObjectiveFunction,
  scores = { ["Train score"] = trainAccuracyFunction, ["Test score"] = testAccuracyFunction },
  principalScore = "Train score"
}

sgd = StochasticGradientDescent{maxIterations=10}
context:enter("SGD")
solution, score = sgd(problem)
context:result("Solution", solution)
context:result("Score", score)
context:leave()


--[[

subspecified function learnBinaryClassifier(examples)

  parameter rate = {default = IterationFunction.constant{1}}
  parameter normalizeRate = {default = true}
  
  local parameters = Vector.newDense()
  local epoch = 0
  local numExamples = #examples

  local rateNormalizer = 1
  if normalizeRate then
    -- make statistics on features sparsity
    local featureStats = Statistics.meanAndVariance()
    for i=1,numExamples do
      featureStats:observe(examples[i][1]:l0norm())
    end
    context:result("featureStats", featureStats)
    rateNormalizer = 1 / featureStats:getMean()
  end

  local function iteration(parameters, iteration)
    context:result("iteration", iteration)

    local lossStats = Statistics.mean()
    for i=1,#examples do
      local example = examples[i]
      --print (parameters, example[1])
      local prediction = parameters.dot(parameters, example[1])
      local sign = examples[2] == 2 and 1 or -1
      local score = prediction * sign
      --print (score)
      --assert(sign == 1 or sign == -1)
      local hingeLoss = math.max(0, 1 - score)
      lossStats:observe(hingeLoss)
      
      if score < 1 then
        local currentRate = rate(epoch)
        parameters:add(example[1], rateNormalizer * currentRate * sign)
      end
      epoch = epoch + 1
    end
    context:result("loss stats", lossStats)
    context:result("mean hinge loss", lossStats:getMean())
    context:result("parameters l0norm", parameters:l0norm())
    context:result("parameters l1norm", parameters:l1norm())
    context:result("parameters l2norm", parameters:l2norm())
    context:result("parameters", parameters:clone())
    context:result("rate", rate(epoch))
    context:result("epoch", epoch)
  end

  for i = 1,10 do
    context:call("iteration " .. i,  iteration, parameters, i)
  end
end


learner = learnBinaryClassifier{rate = IterationFunction.invLinear{2.0, 1000}}

params = context:call("learn binary classifier", learner, examples)
]]