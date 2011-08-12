-- Francis Maes, 12/08/2011
-- Test Conditional Gaussian

require '../ExpressionDiscovery/DecisionProblem'
require 'Predictor'
require 'Stochastic'
require 'Sampler'
require 'Statistics'

examples = {}

context:enter("test dataset")
for i=1,100 do
  local x = Stochastic.standardGaussian()
  local y = Sampler.Gaussian{mu = 1 + x * 2, sigma = math.exp(x)}()
  context:enter("point " .. i)
    context:result("x", x)
    context:result("y", y)
  context:leave()
  
  table.insert(examples, {x,y})
end
context:leave()

local predictor = Predictor.ConditionalGaussian{}
context:enter("learning")
for i=1,100 do
  context:enter("Iteration " .. i)
  context:result("iteration", i)
  local stats = Statistics:mean()
  for j=1,#examples do
    local example = examples[Stochastic.uniformInteger(1,#examples)]
    local features = Vector.newDense(2)
    features[1] = 1
    features[2] = example[1]
    local loss, dlossdmu, dlossdsigma = predictor.lossAndGradient(features, example[2])
    print (loss, dlossdmu, dlossdsigma)
    predictor.__parameters.thetaMu:add(dlossdmu, -0.001)
    predictor.__parameters.thetaSigma:add(dlossdsigma, -0.001)
    stats:observe(loss)
  end
  print(stats)
  context:result("mean loss", stats:getMean())
  context:result("thetaMu", predictor.__parameters.thetaMu:clone())
  context:result("thetaSigma", predictor.__parameters.thetaSigma:clone())
  context:leave(stats:getMean())
end
context:leave()

context:enter("test predictor")
for i=1,1000 do
  local x = Stochastic.standardGaussian()
  local features = Vector.newDense(2)
  features[1] = 1
  features[2] = x

  local mu,sigma = predictor(features)
  local y = Sampler.Gaussian{mu=mu,sigma=sigma}()
  context:enter("point " .. i)
    context:result("x", x)
    context:result("y", y)
  context:leave()
end
context:leave()
