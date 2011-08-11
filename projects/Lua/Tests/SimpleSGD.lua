require 'Vector'
require 'Dictionary'
require 'Parser'
require 'Data'
require 'Statistics'
require 'IterationFunction'
require 'Loss'
require 'Predictor'
require 'Optimizer'

--for k,v in ipairs(examples) do 
--  for k2,v2 in ipairs(v) do
--    print (k2,v2)
--  end
--end


local trainFilename = package.inteluaPath .. "/../../Examples/Data/BinaryClassification/a1a.train"
local testFilename = package.inteluaPath .. "/../../Examples/Data/BinaryClassification/a1a.test"
local labels = Dictionary.new()
local trainExamples = Data.load(Parser.libSVMClassification, 100, trainFilename, labels)
local testExamples = Data.load(Parser.libSVMClassification, 100, testFilename, labels)

print (#trainExamples .. " training examples, " .. #testExamples .. " testing examples " .. #labels .. " labels")

local classifier = Predictor.LinearBinaryClassifier{}

local function decomposableObjectiveFunction(index, parameters)
  local example = trainExamples[index]
  return Predictor.LinearBinaryClassifier{theta=parameters}.lossAndGradient(example[1], example[2])
end

local function trainAccuracyFunction(parameters)
  return Predictor.accuracy(Predictor.LinearBinaryClassifier{theta=parameters}, trainExamples)
end

local function testAccuracyFunction(parameters)
  return Predictor.accuracy(Predictor.LinearBinaryClassifier{theta=parameters}, testExamples)
end


local problem = {
  initialSolution = Vector.newDense(),
  numSamples = #trainExamples,
  objective = decomposableObjectiveFunction,
  scores = { ["Train score"] = trainAccuracyFunction, ["Test score"] = testAccuracyFunction },
  principalScore = "Train score"
}

sgd = Optimizer.StochasticGradientDescent{maxIterations=10}
context:enter("SGD")
solution, score = sgd(problem)
context:result("Solution", solution)
context:result("Score", score)
context:leave()
