require 'Vector'
require 'Dictionary'
require 'Parser'
require 'Data'
require 'Statistics'
require 'IterationFunction'


local function hingeLoss(derivable x)
  return math.max(1 - x, 0.0)
end

local function binaryClassificationLoss(discriminantLoss)
  return function (derivable x, supervision)
    local sign = supervision and 1 or -1
    return discriminantLoss(x * sign)
  end
end

--subspecified function linearBinaryClassifier(features, supervision)
--  parameter theta = {default = Vector.newDense()}
--  parameter loss = {default = binaryClassificationLoss(hingeLoss)}

--  local score = theta:dot(features)
--  if supervision ~= nil then
--    context:goal(loss(score, supervision), loss.dx(score, supervision))
--  end
--  return score > 0
--end

subspecified function linearBinaryClassifier(features)
  parameter theta = {default = Vector.newDense()}
  return theta:dot(features) > 0
end

function linearBinaryClassifierLoss(derivable theta, example)
  local loss = binaryClassificationLoss(hingeLoss)
  return loss(theta:dot(example[1]) > 0, example[2])
end

optimizer = Optimizer.sgd{}
bestTheta = optimizer:optimize(linearBinaryClassifierLoss, examples)
....



local labels = Dictionary.new()
local path = "C:/Projets/lbcpp/projects/Examples/Data/BinaryClassification"

local function loadData(filename, maxCount)
  return Data.load(Parser.libSVMClassification, maxCount, path .. "/" .. filename, labels)
end

trainExamples = loadData("a1a.train", 100)
testExamples = loadData("a1a.test", 100)

print (#trainExamples .. " training examples, " .. #testExamples .. " testing examples, " .. #labels .. " labels")

local myClassifier = linearBinaryClassifier{}
print (myClassifier)

print (myClassifier.parameters.theta)

local features = trainExamples[1][1]
context:result("features", features)
myClassifier(features)
myClassifier(features, true)


Optimizer.sgd()


-----------------

subspecified function learnBinaryClassifier(examples)

  parameter rate = {default = IterationFunction.constant(1)}
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
      local loss = hingeLoss(score)
      lossStats:observe(loss)
      local dloss = hingeLoss.dx(score)
      local currentRate = rate(epoch)
      parameters:add(example[1], - dloss * rateNormalizer * currentRate * sign)
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

-----------