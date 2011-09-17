
require 'Optimizer'
require 'Stochastic'
require 'Random'

context.randomGenerator = Random.new()


local histogram = {}
for i=1,101 do
  table.insert(histogram, 0)
end

function testFunction(x)
  local index = 1 + math.floor(x[1] * 100)
  histogram[index] = histogram[index] + 1

  local p = (math.sin(13 * x[1]) * math.sin(27 * x[2]) + 1) / 2
  return Stochastic.bernoulli(p)
end

local objective = lbcpp.LuaFunction.create(testFunction, "DenseDoubleVector<EnumValue,Double>", "Double")
local optimizer = Optimizer.HOO{numIterations=10000,nu=1,rho=0.5}
local score,solution = optimizer{objective = objective, initialGuess=Vector.newDense(2)}
print ("score", score, "solution", solution)

context:enter("histogram")
for i=1,#histogram do
  context:enter("interval " .. i)
  context:result("interval", i)
  context:result("count", histogram[i])
  context:leave()
end
context:leave()