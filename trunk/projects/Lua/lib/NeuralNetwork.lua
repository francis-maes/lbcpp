-- Becker Julien, 05/01/2012
-- Neural Network
---- Some developpement of neural network architecture and tests
---- Perceptron learned with gradient descend

require 'Context'
require 'Random'
random = Random.new(2)

-- BEGIN Perceptron
Perceptron = {numIterations = 10, learningRate = 0.1}

function Perceptron:new (o)
  o = o or {}
  setmetatable(o, self)
  self.__index = self
  return o
end

function Perceptron:predict(input)
  return self:dot(input) < 0 and -1 or 1
end

function Perceptron:dot(v)
  local dotProduct = self.bias;
  for i = 1, #self.weights do
    dotProduct = dotProduct + self.weights[i] * v[i]
  end
  return dotProduct
end

function Perceptron:updateWeights(example)
  for i = 1, #self.weights do
    self.weights[i] = self.weights[i] + self.learningRate * example.output * example.input[i]
  end
  self.bias = self.bias + self.learningRate * example.output
end

function Perceptron:learn(examples, testingExamples, evaluator)
  context:enter("Perceptron - Learning on " .. #examples .. " examples and " .. #examples[1].input .. " variables")
  self.bias = 0
  self.weights = {}
  for i = 1, #examples[1].input do self.weights[i] = 0 end

  local bestLoss = math.huge
  local bestIteration = 0
  local bestWeights = {}
  local bestBias = {}
  for i = 1, self.numIterations do
    context:enter("Iteration " .. i)
    local hingeLoss = 0
    for j = 1, #examples do
      local prediction = self:dot(examples[j].input)
      if prediction * examples[j].output <= 1 then
        self:updateWeights(examples[j])
        hingeLoss = hingeLoss + math.abs(prediction - examples[j].output)
      end
    end

    if hingeLoss < bestLoss then
      bestLoss = hingeLoss
      bestIteration = i
      bestWeights = self.weights
      bestBias = self.bias
    end

    context:result("Iteration", i)
    context:result("HingeLoss", hingeLoss)
    if evaluator then
      context:result("Train Evaluation", evaluator(|x| self:predict(x), examples))
      if testingExamples then
        context:result("Test Evaluation", evaluator(|x| self:predict(x), testingExamples))
      end
    end
    print("Weights: ", unpack(self.weights))
    print("Bias: ", self.bias)

    context:leave()

    if self.numIterationsWithoutImprovement and i - bestIteration >= self.numIterationsWithoutImprovement then
      print("No improvement")
      break
    end
  end

  print("Restoring best parameters from iteration " .. bestIteration)
  self.weights = bestWeights
  self.bias = bestBias

  context:leave()
end
-- END Perceptron

-- BEGIN FeedForwardNetwork
FeedForwardNetwork = {}

function FeedForwardNetwork:predict(input)
  return self:activate(input) > 0 and 1 or -1
end

function FeedForwardNetwork:dot(v1, v2)
  local dotProduct = 0;
  for i = 1, #v1 do
    dotProduct = dotProduct + v1[i] * v2[i]
  end
  return dotProduct
end

function FeedForwardNetwork:learn(data, evaluator)
  -- Initialize structure
  -- -- Hidden to Output
  self.weights = {}
  for i = 1, self.numHiddenNeurons do
    self.weights[i] = random:sample()
  end
  -- -- Input to Hidden
  self.hiddenLayer = {}
  for i = 1, self.numHiddenNeurons do
    self.hiddenLayer[i] = {weights = {}, activation = 0}
    for j = 1, #data[1].input do
      self.hiddenLayer[i].weights[j] = random:sample()
    end
  end

  -- Learn
  for i = 1, self.numIterations do
    context:enter("Iteration " .. i)
    local hingeLoss = 0
    for j = 1, #data do
      local prediction = self:activate(data[j].input)
      if (prediction * data[j].output <= 1) then
        self:backPropagate(data[j].input, prediction, data[j].output)
        hingeLoss = hingeLoss + math.abs(prediction - data[j].output)
      end
    end
    context:result("Iteration", i)
    context:result("HingeLoss", hingeLoss)
    if evaluator then
      context:result("Train Evaluation", evaluator(|x| self:predict(x), data))
    end
    context:leave(hingeLoss)
  end
end

function FeedForwardNetwork:activate(input)
  local interInput = {}
  for i = 1, self.numHiddenNeurons do
    local dotProduct = self:dot(input, self.hiddenLayer[i].weights)
    self.hiddenLayer[i].activation = self:sigmoid(dotProduct)
    interInput[i] = self.hiddenLayer[i].activation
  end
  return self:sigmoid(self:dot(interInput, self.weights))
end

function FeedForwardNetwork:backPropagate(input, prediction, supervision)
   -- compute deltas
  local outputDelta = self:derivateSigmoid(prediction) * (supervision - prediction)
  local hiddenDeltas = {}
  for i = 1, self.numHiddenNeurons do
    hiddenDeltas[i] = self:derivateSigmoid(self.hiddenLayer[i].activation) * (outputDelta * self.weights[i])
  end
  -- update weigths
  for i = 1, self.numHiddenNeurons do
    self.weights[i] = self.weights[i] + self.learningRate * outputDelta * self.hiddenLayer[i].activation
  end
  for i = 1, self.numHiddenNeurons do
    for j = 1, #input do
      self.hiddenLayer[i].weights[j] = self.hiddenLayer[i].weights[j] + self.learningRate * hiddenDeltas[i] * input[j]
    end
  end
end

function FeedForwardNetwork:sigmoid(x)
  return math.tanh(x)
end

function FeedForwardNetwork:derivateSigmoid(x)
  return 1 - math.tanh(x) * math.tanh(x)
end
-- END FeedForwardNetwork

-- BEGIN Generation of data
function sampleBool()
  return random:sample() < 0.5
end

function generateData(numData, numVariables, operator)
  local res = {}
  for i = 1, numData
  do
    local data = {}
    for j = 1, numVariables do
      data[j] = sampleBool() and 1 or 0
    end
    res[i] = {input = data, output = operator(data)}
  end
  return res
end

function andOperator(x)
  return (x[1] == 1 and x[2] == 1) and 1 or -1
end

function xorOperator(x)
--  x[#x + 1] = x[1] * x[2]
  return x[1] ~= x[2] and 1 or -1
end
-- END Generation of data

-- BEGIN Evaluator
function accuracy(f, data)
  local sum = 0
  for i = 1, #data do
    sum = sum + (f(data[i].input) == data[i].output and 1 or 0)
  end
  return sum / #data
end
-- END Evaluator

function convertToBinaryClassificationExamples(examples)
  for i = 1, #examples do
    examples[i] = {input = examples[i][1], output = (examples[i][2] == 1 and 1 or -1)}
  end
  return examples
end

-- BEGIN Test Zone
function mainPerceptron()
  print "--- Test ---"

  print "Generating Data"
  local data = generateData(10, 5, xorOperator)
  local testingData = generateData(100, 5, xorOperator)

  local p = Perceptron
  p.numIterations = 100
  context:enter("Learning")
  p:learn(data, accuracy)
  context:leave()

  print("Train Evaluation: ", accuracy(|x| p:predict(x), data))
  print("Test Evaluation: ", accuracy(|x| p:predict(x), testingData))

  print "--- End Test ---"
end

function mainFeedForwardNetwork()
  print "BEGIN FeedForwardNetwork"

  local data = generateData(100, 2, andOperator)

  ffn = FeedForwardNetwork

  context:enter("Learning")
  ffn.numHiddenNeurons = 3
  ffn.numIterations = 100
  ffn.learningRate = 0.1
  ffn:learn(data, accuracy)
  context:leave()

  print("Train Evaluation: ", accuracy(|x| ffn:predict(x), data))

  print "END FeedForwardNetwork"
end

--mainFeedForwardNetwork()
-- END Test Zone

function createIntermediateDatasets(predictor, examples)
  local negativeExamples = {}
  local positiveExamples = {}
  for i = 1, #examples do
    local examplesList = predictor(examples[i].input) == -1 and negativeExamples or positiveExamples
    table.insert(examplesList, examples[i])
  end
  return negativeExamples, positiveExamples
end

RecursivePerceptron = {}

function RecursivePerceptron:predict(input)
  return self:recursivePredict(self.root, input)
end

function RecursivePerceptron:recursivePredict(parent, input)
  local res = parent:predict(input)
  local nextPerceptron = res == -1 and parent.negativePerceptron or parent.positivePerceptron
  return nextPerceptron and self:recursivePredict(nextPerceptron, input) or res
end

function RecursivePerceptron:train(trainingExamples, testingExamples, evaluator)
  context:enter("Training RecursivePerceptron")
  self.root = self:recursiveTrain(trainingExamples, testingExamples, evaluator, self.maxDeep)
  context:leave()
end

function RecursivePerceptron:recursiveTrain(trainingExamples, testingExamples, evaluator, maxDeep)
  print("Num. training examples", #trainingExamples)
  print("Num. testing examples", #testingExamples)

  local parent = Perceptron:new()
  parent.numIterations = 1000
  parent.numIterationsWithoutImprovement = 30
  parent.learningRate = 0.01
  parent:learn(trainingExamples, testingExamples, evaluator)

  print("Global Training", evaluator(|x| parent:predict(x), trainingExamples))
  print("Global Testing", evaluator(|x| parent:predict(x), testingExamples))

  if maxDeep <= 1 then
    return parent
  end

  local negTrainExamples, posTrainExamples = createIntermediateDatasets(|x| parent:predict(x), trainingExamples)
  local negTestExamples, posTestExamples = createIntermediateDatasets(|x| parent:predict(x), testingExamples)

  if #negTrainExamples > 0 and #posTrainExamples > 0 then
    print("Negative Pre-Training", accuracy(|x| parent:predict(x), negTrainExamples))
    print("Negative Pre-Testing", accuracy(|x| parent:predict(x), negTestExamples))

    context:enter("Negative examples")
    parent.negativePerceptron = self:recursiveTrain(negTrainExamples, negTestExamples, evaluator, maxDeep - 1)
    context:leave()

    print("Negative Post-Training", accuracy(|x| parent.negativePerceptron:predict(x), negTrainExamples))
    print("Negative Post-Testing", accuracy(|x| parent.negativePerceptron:predict(x), negTestExamples))

    print("Positive Pre-Training", accuracy(|x| parent:predict(x), posTrainExamples))
    print("Positive Pre-Testing", accuracy(|x| parent:predict(x), posTestExamples))

    context:enter("Positive examples")
    parent.positivePerceptron = self:recursiveTrain(posTrainExamples, posTestExamples, evaluator, maxDeep - 1)
    context:leave()

    print("Positive Post-Training", accuracy(|x| parent.positivePerceptron:predict(x), posTrainExamples))
    print("Positive Post-Testing", accuracy(|x| parent.positivePerceptron:predict(x), posTestExamples))
  end

  return parent
end

require 'Data'
require 'Parser'
require 'Dictionary'

labels = Dictionary.new()
local trainingExamples = convertToBinaryClassificationExamples(Data.load(Parser.libSVMClassification, 0, "/Users/jbecker/Documents/Workspace/LBC++/projects/Examples/Data/BinaryClassification/a1a.train", labels))
local testingExamples = convertToBinaryClassificationExamples(Data.load(Parser.libSVMClassification, 300, "/Users/jbecker/Documents/Workspace/LBC++/projects/Examples/Data/BinaryClassification/a1a.test", labels))

--print("#Training Examples : " .. #trainingExamples)
--print("#Testing Examples : " .. #testingExamples)

local rp = RecursivePerceptron
rp.maxDeep = 5
rp:train(trainingExamples, testingExamples, accuracy)

print("Train Evaluation: ", accuracy(|x| rp:predict(x), trainingExamples))
print("Test Evaluation: ", accuracy(|x| rp:predict(x), testingExamples))