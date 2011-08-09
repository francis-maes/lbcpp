require 'Statistics'
require 'IterationFunction'
require 'DiscreteBandit'
require 'Random'
require 'Stochastic'
require 'Sampler'

context.randomGenerator = Random.new(1664)

numProblems = 10
numEstimations = 10
numTimeSteps = 10

-- create training problems
trainingProblems = {}
for i = 1,numProblems do
  local p1 = 0.9 --Stochastic.standardUniform()
  local p2 = 0.8 --Stochastic.standardUniform()
  table.insert(trainingProblems, {Sampler.Bernoulli{p = p1}, Sampler.Bernoulli{p = p2}})
end

gaussianTestProblems = {}
for i = 1,numProblems do
  local mean1 = Stochastic.standardUniform()
  local stddev1 = Stochastic.standardUniform()
  local mean2 = Stochastic.standardUniform()
  local stddev2 = Stochastic.standardUniform()
  table.insert(gaussianTestProblems, {
    Sampler.TruncatedGaussian{mu = mean1, sigma = stddev1},
    Sampler.TruncatedGaussian{mu = mean2, sigma = stddev2}
  })
end

-- create policies
local policies = {}
local function addPolicy(name, policy)
  table.insert(policies, {name, policy})
end


addPolicy("ucb1", DiscreteBandit.indexBasedPolicy{indexFunction = DiscreteBandit.ucb1}.__get)
addPolicy("ucb1Tuned", DiscreteBandit.indexBasedPolicy{indexFunction = DiscreteBandit.ucb1Tuned}.__get)
addPolicy("ucb1Normal", DiscreteBandit.indexBasedPolicy{indexFunction = DiscreteBandit.ucb1Normal}.__get)
addPolicy("ucb1V", DiscreteBandit.indexBasedPolicy{indexFunction = DiscreteBandit.ucbV{c = 1, zeta = 1}}.__get)
addPolicy("e-greedy", DiscreteBandit.epsilonGreedy{c = 1, d = 1}.__get)

addPolicy("KL-ucb c=0", DiscreteBandit.indexBasedPolicy{indexFunction = DiscreteBandit.klUcb{c=0}}.__get)
addPolicy("KL-ucb c=3", DiscreteBandit.indexBasedPolicy{indexFunction = DiscreteBandit.klUcb{c=3}}.__get)

addPolicy("ucb1_10", DiscreteBandit.indexBasedPolicy{indexFunction = DiscreteBandit.ucb1C{C=0.170}}.__get)
addPolicy("ucb1_100", DiscreteBandit.indexBasedPolicy{indexFunction = DiscreteBandit.ucb1C{C=0.173}}.__get)
addPolicy("ucb1_1000", DiscreteBandit.indexBasedPolicy{indexFunction = DiscreteBandit.ucb1C{C=0.187}}.__get)

addPolicy("ucbV_10", DiscreteBandit.indexBasedPolicy{indexFunction = DiscreteBandit.ucbV{c=1.542, zeta=0.0631}}.__get)
addPolicy("ucbV_100", DiscreteBandit.indexBasedPolicy{indexFunction = DiscreteBandit.ucbV{c=1.681, zeta=0.0347}}.__get)
addPolicy("ucbV_1000", DiscreteBandit.indexBasedPolicy{indexFunction = DiscreteBandit.ucbV{c=1.304, zeta=0.0852}}.__get)

addPolicy("e-greedy_10", DiscreteBandit.epsilonGreedy{c=0.0499, d=1.505}.__get)
addPolicy("e-greedy_100", DiscreteBandit.epsilonGreedy{c=1.096, d=1.349}.__get)
addPolicy("e-greedy_1000", DiscreteBandit.epsilonGreedy{c=0.845, d=0.738}.__get)

-- evaluate policies
function evaluatePolicy(description, policy)
  --context:call(description .. " train", DiscreteBandit.estimatePolicyRegretOnProblems, policy, trainingProblems, numEstimations, numTimeSteps)
  --context:call(description + " b-test", DiscreteBandit.estimatePolicyRegretOnProblems, policy, problems, numEstimations, numTimeSteps)
  context:call(description .. " g-test", DiscreteBandit.estimatePolicyRegretOnProblems, policy, gaussianTestProblems, numEstimations, numTimeSteps)
end

for i,t in ipairs(policies) do
  evaluatePolicy(t[1], t[2], trainingProblems)
end