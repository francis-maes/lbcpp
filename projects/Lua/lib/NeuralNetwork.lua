-- Becker Julien, 05/01/2012
-- Neural Network
---- Some developpement of neural network architecture and tests
---- Perceptron learned with gradient descend

require 'Context'

Perceptron = {numIterations = 10, learningRate = 0.1}

function Perceptron.predict(self, input)
  if self:dot(input) < 0 then
    return -1;
  end
  return 1;
end

function Perceptron.dot(self, input)
  dotProduct = self.bias;
  for i = 1, #self.weights do
    dotProduct = dotProduct + self.weights[i] * input[i]
  end
  return dotProduct
end

function Perceptron.updateWeights(self, data)
  for i = 1, #self.weights do
    self.weights[i] = self.weights[i] + self.learningRate * data.output * data.input[i]
  end
  self.bias = self.bias + self.learningRate * data.output
end

function Perceptron.learn(self, data)
  self.bias = 0
  self.weights = {}
  for i = 1, #data[1].input do self.weights[i] = 0 end

  for i = 1, self.numIterations do
    context:enter("Iteration " .. i)
    hingeLoss = 0
    for j = 1, #data do
      prediction = self:dot(data[j].input)
      if prediction * data[j].output <= 1 then
        self:updateWeights(data[j])
        hingeLoss = hingeLoss + math.abs(prediction - data[j].output)
      end
    end
    context:result("Iteration", i)
    context:result("HingeLoss", hingeLoss)
    context:leave()
  end
end

-- BEGIN Generation of data
require 'Random'
randomGenerator = Random.new(2)
function sampleBool()
  return randomGenerator:sample() < 0.5
end

function generateData(numData, numVariables, operator)
  res = {}
  for i = 1, numData
  do
    data = {}
    for j = 1, numVariables
    do
      data[j] = 0
      if sampleBool() then
        data[j] = 1
      end
    end
    res[i] = {input = data, output = operator(data)}
  end
  return res
end

function andOperator(x)
  if x[1] == 1 and x[2] == 1 then
    return 1
  end
  return -1
end
-- END Generation of data

-- BEGIN Test Zone
print "--- Test ---"

print "Generating Data"
data = generateData(10, 2, andOperator)

p = Perceptron
p.numIterations = 100
context:enter("Learning")
p:learn(data)
context:leave()
print("Prediction: ", p:predict({1, 0}))


print "--- End Test ---"
-- END Test Zone