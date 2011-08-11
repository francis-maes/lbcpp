require 'Statistics'
require 'Sampler'
require 'Vector'
require 'Optimizer'


objective = |input| input:l1norm()

eda = Optimizer.EstimationOfDistributionAlgorithm{numIterations=10, numCandidates=100, numBests=10}

local samplers = {}
for i=1,100 do
  samplers[i] = Sampler.Gaussian{}
end
context:call("EDA", eda, objective, Sampler.IndependentVector{samplers = samplers})

