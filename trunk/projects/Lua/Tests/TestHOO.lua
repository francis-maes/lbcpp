
require 'Optimizer'
require 'Stochastic'
require 'Random'
require 'Statistics'

context.randomGenerator = Random.new()

local dimension = 5


local histogram = {}
for i=1,101 do
  table.insert(histogram, 0)
end

local xopt = Vector.newDense(dimension)
for i=1,dimension do
  xopt[i] = 0.2
end


function testFunction(x)
--  local index = 1 + math.floor(x[1] * 100)
--  histogram[index] = histogram[index] + 1
--  local p = (math.sin(13 * x[1]) * math.sin(27 * x[1]) + 1) / 2

  local d = x - xopt
  local e = d:l2norm()
  return math.max(0.0, 1 - e)
end

local function makeBernoulliFunction(fun)
  local sampler = |x| Stochastic.bernoulli(fun(x))
  return lbcpp.LuaWrapperFunction.create(sampler, "DenseDoubleVector<PositiveIntegerEnumeration,Double>", "Double")

end

local objectiveExpectation = testFunction
local objective = makeBernoulliFunction(objectiveExpectation)

for maxDepth=1,100 do
  local C = 0.5
  local horizon = 2048 -- math.pow(2, hpow)
  context:enter("maxdepth = " .. maxDepth)
  context:result("maxDepth", maxDepth)
  --context:result("log2(horizon)", hpow)

  local regretStats = Statistics.mean()
  for i=1,10 do

    local optimizer = Optimizer.HOO{numIterations=horizon,nu=1,rho=0.5,maxDepth=maxDepth, C=C, playCenteredArms=false}
    local score,solution = optimizer{objective = objective, initialGuess=Vector.newDense(dimension)}
    print ("score", score, "solution", solution)
    if solution then
      local reward = objectiveExpectation(solution)
      print ("regret: ", 1.0 - reward)
      regretStats:observe(1.0 - reward)
    end
  end

  context:result("regret", regretStats:getMean())
  context:leave()
end



--[[
context:enter("histogram")
for i=1,#histogram do
  context:enter("interval " .. i)
  context:result("interval", i)
  context:result("count", histogram[i])
  context:leave()
end
context:leave()
]]--