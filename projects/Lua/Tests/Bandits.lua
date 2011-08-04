require 'Statistics'
require 'IterationFunction'
require 'DiscreteBandit'
require 'Random'
require 'Stochastic'

------------- Sampler ---------------

Sampler = {}

--stochastic subspecified function Sampler.bernoulli()
--  parameter p = {default = 0.5, min = 0, max = 1, sampler = Stochastic.standardUniform}
--  return Stochastic.bernoulli(p)
--end


-- FIXME: Problem: we cannot directly declare a subspecified table, because there would be no place
--  do put the parameter statements so that they can be used in different fields of the table
-- apriori, we need to extend the grammar of tables to allow parameter statements inside ....
Sampler.bernoulli = subspecified function ()
  parameter p = {default = 0.5, min = 0, max = 1, sampler = Stochastic.standardUniform}

  return setmetatable({
    sample = function () return Stochastic.bernoulli:sample(p) end,
    expectation = function () return p end,
  }, Stochastic.MT)
end

Sampler.truncatedGaussian = subspecified function ()
  parameter mean = {default = 0.5}
  parameter stddev = {default = 0.5}

  return setmetatable({
    sample = function ()
      local res = 0
      repeat
        res = Stochastic.standardGaussian:sample() * stddev + mean
      until res >= 0 and res <= 1
      return res
    end,
    expectation = function () return mean end,
  }, Stochastic.MT)
end

--subspecified function Sampler.bernoulli()
--  parameter p = {default = 0.5, min = 0, max = 1, sampler = Stochastic.standardUniform}
--  return Stochastic.bernoulli:sample(p)
--end  

---


-------------- main -----------------

context.randomGenerator = Random.new(1664)

numProblems = 10000
numEstimations = 1
numTimeSteps = 1000

-- create training problems
trainingProblems = {}
for i = 1,numProblems do
  local p1 = Stochastic.standardUniform()
  local p2 = Stochastic.standardUniform()
  table.insert(trainingProblems, {Sampler.bernoulli{p = p1}(), Sampler.bernoulli{p = p2}()})
end

gaussianTestProblems = {}
for i = 1,numProblems do
  local mean1 = Stochastic.standardUniform()
  local stddev1 = Stochastic.standardUniform()
  local mean2 = Stochastic.standardUniform()
  local stddev2 = Stochastic.standardUniform()
  table.insert(gaussianTestProblems, {
    Sampler.truncatedGaussian{mean = mean1, stddev = stddev1}(),
    Sampler.truncatedGaussian{mean = mean2, stddev = stddev2}()
  })
end

-- create policies
local policies = {}
local function addPolicy(name, policy)
  table.insert(policies, {name, policy})
end


addPolicy("ucb1Tuned", DiscreteBandit.indexBasedPolicy{indexFunction = DiscreteBandit.ucb1Tuned}.__get)
addPolicy("KL-ucb c=0", DiscreteBandit.indexBasedPolicy{indexFunction = DiscreteBandit.klUcb{c=0}}.__get)
addPolicy("KL-ucb c=3", DiscreteBandit.indexBasedPolicy{indexFunction = DiscreteBandit.klUcb{c=3}}.__get)

--[[

addPolicy("ucb1", DiscreteBandit.indexBasedPolicy{indexFunction = DiscreteBandit.ucb1}.__get)
addPolicy("ucb1Tuned", DiscreteBandit.indexBasedPolicy{indexFunction = DiscreteBandit.ucb1Tuned}.__get)
addPolicy("ucb1Normal", DiscreteBandit.indexBasedPolicy{indexFunction = DiscreteBandit.ucb1Normal}.__get)
addPolicy("ucb1V", DiscreteBandit.indexBasedPolicy{indexFunction = DiscreteBandit.ucbV{c = 1, zeta = 1}}.__get)
addPolicy("e-greedy", DiscreteBandit.epsilonGreedy{c = 1, d = 1}.__get)

addPolicy("ucb1_10", DiscreteBandit.indexBasedPolicy{indexFunction = DiscreteBandit.ucb1C{C=0.170}}.__get)
addPolicy("ucb1_100", DiscreteBandit.indexBasedPolicy{indexFunction = DiscreteBandit.ucb1C{C=0.173}}.__get)
addPolicy("ucb1_1000", DiscreteBandit.indexBasedPolicy{indexFunction = DiscreteBandit.ucb1C{C=0.187}}.__get)

addPolicy("ucbV_10", DiscreteBandit.indexBasedPolicy{indexFunction = DiscreteBandit.ucbV{c=1.542, zeta=0.0631}}.__get)
addPolicy("ucbV_100", DiscreteBandit.indexBasedPolicy{indexFunction = DiscreteBandit.ucbV{c=1.681, zeta=0.0347}}.__get)
addPolicy("ucbV_1000", DiscreteBandit.indexBasedPolicy{indexFunction = DiscreteBandit.ucbV{c=1.304, zeta=0.0852}}.__get)

addPolicy("e-greedy_10", DiscreteBandit.epsilonGreedy{c=0.0499, d=1.505}.__get)
addPolicy("e-greedy_100", DiscreteBandit.epsilonGreedy{c=1.096, d=1.349}.__get)
addPolicy("e-greedy_1000", DiscreteBandit.epsilonGreedy{c=0.845, d=0.738}.__get)
]]

-- evaluate policies
function evaluatePolicy(description, policy)
  --context:call(description .. " train", DiscreteBandit.estimatePolicyRegretOnProblems, policy, trainingProblems, numEstimations, numTimeSteps)
  --context:call(description + " b-test", DiscreteBandit.estimatePolicyRegretOnProblems, policy, problems, numEstimations, numTimeSteps)
  context:call(description .. " g-test", DiscreteBandit.estimatePolicyRegretOnProblems, policy, gaussianTestProblems, numEstimations, numTimeSteps)
end

for i,t in ipairs(policies) do
  evaluatePolicy(t[1], t[2], trainingProblems)
end