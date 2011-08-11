require 'Statistics'
require 'Sampler'
require 'Vector'
require 'Optimizer'
require 'StoppingCriterion'

objective = |input| input:l1norm()

crit = StoppingCriterion.MaxIterationsWithoutImprovement{}
eda = Optimizer.EstimationOfDistributionAlgorithm{stoppingCriterion = crit, maxIterations=0, numCandidates=100, numBests=10}

local samplers = {}
for i=1,5 do
  samplers[i] = Sampler.Gaussian{}
end
context:call("EDA", eda, objective, Sampler.IndependentVector{samplers = samplers})
